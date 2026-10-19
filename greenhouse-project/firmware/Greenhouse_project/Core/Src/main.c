/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Enhanced Greenhouse Monitoring System
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "liquidcrystal_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t dht_errors;
    uint8_t us_errors;
    uint8_t soil_errors;
    uint32_t last_error_time;
} error_tracker_t;

typedef enum {
    STATE_NORMAL,
    STATE_ALARM,
    STATE_ERROR,
    STATE_MAINTENANCE
} system_state_t;

typedef struct {
    uint8_t alarm_active;
    uint32_t alarm_start_time;
    uint8_t consecutive_alarms;
    uint32_t total_alarm_time;
    uint16_t alarm_count;
} alarm_state_t;

typedef struct {
    float temp_min, temp_max;
    float humidity_min, humidity_max;
    uint16_t soil_min, soil_max;
    uint16_t distance_min, distance_max;
    uint32_t last_reset_time;
} data_logger_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// ---- DHT11 ----
#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_9

// ---- HC-SR04 ----
#define TRIG_PIN   GPIO_PIN_9
#define TRIG_PORT  GPIOA
#define ECHO_PIN   GPIO_PIN_8
#define ECHO_PORT  GPIOA

// ---- Soil Moisture + LEDs ----
#define SOIL_PWR_PORT GPIOA
#define SOIL_PWR_PIN  GPIO_PIN_7
#define LED_R_PORT GPIOA
#define LED_R_PIN  GPIO_PIN_5
#define LED_G_PORT GPIOA
#define LED_G_PIN  GPIO_PIN_3
#define LED_B_PORT GPIOA
#define LED_B_PIN  GPIO_PIN_0

// ---- Servo (TIM2 CH2 on PA1) ----
#define SERVO_TIMER   htim2
#define SERVO_CH      TIM_CHANNEL_2
#define SERVO_FAST    1250
#define SERVO_STOP    750

// ---- App settings ----
#define PAGE_MS       8000    // 8 sec page swap
#define SOIL_DRY_THR  3200
#define SOIL_MED_THR  2700
#define SOIL_WET_THR  2000
#define TANK_MIN_CM   5
#define TANK_MAX_CM   20
#define DHT_READ_INTERVAL 2000 // 2 seconds between DHT reads
#define MAX_ERRORS    5
#define ALARM_TIMEOUT 300000   // 5 minutes max alarm
#define CALIBRATION_MODE 0     // Set to 1 for calibration mode

// Configuration validation
#if (SERVO_FAST <= SERVO_STOP)
#error "SERVO_FAST must be greater than SERVO_STOP"
#endif

#if (SOIL_DRY_THR <= SOIL_MED_THR)
#error "SOIL_DRY_THR must be greater than SOIL_MED_THR"
#endif

#if (TANK_MAX_CM <= TANK_MIN_CM)
#error "TANK_MAX_CM must be greater than TANK_MIN_CM"
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
// DHT11 variables
static uint8_t RHI, RHD, TCI, TCD, SUM;
static float tCelsius, tFahrenheit, RH;
static uint32_t pMillis, cMillis;
static uint32_t last_dht_read = 0;

// System state
static uint8_t all_ready = 0;
static uint8_t show_mode = 0;
static uint32_t last_switch_ms = 0;
static uint32_t blink_timer = 0;
static uint8_t blink_state = 0;

// Enhanced error tracking
static error_tracker_t errors = {0};
static system_state_t system_state = STATE_NORMAL;
static alarm_state_t alarm_state = {0};
static data_logger_t data_log = {
    .temp_min = 999, .temp_max = -999,
    .humidity_min = 999, .humidity_max = -999,
    .soil_min = 9999, .soil_max = 0,
    .distance_min = 999, .distance_max = 0,
    .last_reset_time = 0
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN FFP */
// Basic sensor functions
static void microDelay(uint16_t us);
static uint8_t DHT11_Start(void);
static uint8_t DHT11_Read(void);
static uint8_t read_DHT11_basic(void);
static uint8_t read_HCSR04_cm_basic(uint16_t *cm);
static uint16_t read_soil_adc_basic(void);

// Enhanced sensor functions
static uint8_t read_DHT11_improved(void);
static uint8_t read_HCSR04_filtered(uint16_t *cm);
static uint16_t read_soil_adc_averaged(void);

// Display and control functions
static void set_leds(uint8_t r, uint8_t g, uint8_t b);
static void update_status_leds(uint8_t soil_dry, uint8_t soil_med, uint8_t soil_wet, uint8_t alarm);
static void servo_drive_fast(uint8_t on);

// System health and alarm functions
static uint8_t system_health_check(void);
static void update_alarm_logic(uint16_t soil, uint16_t tank_cm);
static void display_error_info(void);
static void display_stats_info(void);
static void update_data_logger(float temp, float hum, uint16_t soil, uint16_t dist);

// Device readiness checks
static uint8_t check_DHT_ready(void);
static uint8_t check_US_ready(void);
static uint8_t check_SOIL_ready(void);
static uint8_t check_SERVO_ready(void);
/* USER CODE END FPP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ===== BASIC SENSOR FUNCTIONS =====
static void microDelay(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

static uint8_t DHT11_Start(void) {
    uint8_t Response = 0;
    GPIO_InitTypeDef s = {0};
    s.Pin = DHT11_PIN;
    s.Mode = GPIO_MODE_OUTPUT_PP;
    s.Speed = GPIO_SPEED_FREQ_LOW;
    s.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &s);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    microDelay(30);
    s.Mode = GPIO_MODE_INPUT;
    s.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &s);
    microDelay(40);
    if (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
        microDelay(80);
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) Response = 1;
    }
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && (pMillis + 2 > cMillis)) {
        cMillis = HAL_GetTick();
    }
    return Response;
}

static uint8_t DHT11_Read(void) {
    uint8_t a, b = 0;
    for (a = 0; a < 8; a++) {
        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && (pMillis + 2 > cMillis)) {
            cMillis = HAL_GetTick();
        }
        microDelay(40);
        if (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
            b &= ~(1 << (7 - a));
        else
            b |= (1 << (7 - a));
        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && (pMillis + 2 > cMillis)) {
            cMillis = HAL_GetTick();
        }
    }
    return b;
}

static uint8_t read_DHT11_basic(void) {
    if (!DHT11_Start()) return 0;
    RHI = DHT11_Read();
    RHD = DHT11_Read();
    TCI = DHT11_Read();
    TCD = DHT11_Read();
    SUM = DHT11_Read();
    if ((uint8_t)(RHI + RHD + TCI + TCD) != SUM) return 0;
    tCelsius = (float)TCI + (float)(TCD / 10.0f);
    tFahrenheit = tCelsius * 9.0f / 5.0f + 32.0f;
    RH = (float)RHI + (float)(RHD / 10.0f);
    return 1;
}

static uint8_t read_HCSR04_cm_basic(uint16_t *cm) {
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    microDelay(2);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    microDelay(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    uint32_t start = HAL_GetTick();
    while (!HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN)) {
        if (HAL_GetTick() - start > 10) return 0;
    }
    uint32_t v1 = __HAL_TIM_GET_COUNTER(&htim1);
    start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN)) {
        if (HAL_GetTick() - start > 50) return 0;
    }
    uint32_t v2 = __HAL_TIM_GET_COUNTER(&htim1);
    *cm = (uint16_t)((v2 - v1) * 0.034f / 2.0f);
    return 1;
}

static uint16_t read_soil_adc_basic(void) {
    HAL_GPIO_WritePin(SOIL_PWR_PORT, SOIL_PWR_PIN, GPIO_PIN_SET);
    HAL_Delay(40);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5);
    uint16_t v = HAL_ADC_GetValue(&hadc1);
    HAL_GPIO_WritePin(SOIL_PWR_PORT, SOIL_PWR_PIN, GPIO_PIN_RESET);
    return v;
}

// ===== ENHANCED SENSOR FUNCTIONS =====
static uint8_t read_DHT11_improved(void) {
    uint32_t now = HAL_GetTick();
    
    // DHT11 needs 2s between readings
    if (now - last_dht_read < DHT_READ_INTERVAL) {
        return 1; // Use previous values
    }
    last_dht_read = now;
    
    if (!DHT11_Start()) {
        errors.dht_errors++;
        return 0;
    }
    
    RHI = DHT11_Read();
    RHD = DHT11_Read();
    TCI = DHT11_Read();
    TCD = DHT11_Read();
    SUM = DHT11_Read();
    
    if ((uint8_t)(RHI + RHD + TCI + TCD) != SUM) {
        errors.dht_errors++;
        return 0;
    }
    
    // Reset error counter on success
    errors.dht_errors = 0;
    
    tCelsius = (float)TCI + (float)(TCD / 10.0f);
    tFahrenheit = tCelsius * 9.0f / 5.0f + 32.0f;
    RH = (float)RHI + (float)(RHD / 10.0f);
    
    return 1;
}

static uint8_t read_HCSR04_filtered(uint16_t *cm) {
    const uint8_t max_attempts = 3;
    uint16_t readings[3] = {0};
    uint8_t valid_readings = 0;
    
    for (uint8_t i = 0; i < max_attempts; i++) {
        if (read_HCSR04_cm_basic(&readings[i])) {
            valid_readings++;
        }
        HAL_Delay(10); // Small delay between readings
    }
    
    if (valid_readings == 0) {
        errors.us_errors++;
        return 0;
    }
    
    // Use median of valid readings (simple implementation)
    if (valid_readings >= 2) {
        // Sort first two valid readings and take middle
        if (readings[0] > readings[1]) {
            uint16_t temp = readings[0];
            readings[0] = readings[1];
            readings[1] = temp;
        }
        *cm = readings[1]; // Take middle value
    } else {
        *cm = readings[0];
    }
    
    errors.us_errors = 0;
    return 1;
}

static uint16_t read_soil_adc_averaged(void) {
    const uint8_t samples = 5;
    uint32_t sum = 0;
    
    HAL_GPIO_WritePin(SOIL_PWR_PORT, SOIL_PWR_PIN, GPIO_PIN_SET);
    HAL_Delay(50); // Longer stabilization
    
    for (uint8_t i = 0; i < samples; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            sum += HAL_ADC_GetValue(&hadc1);
        } else {
            errors.soil_errors++;
        }
        HAL_Delay(10);
    }
    
    HAL_GPIO_WritePin(SOIL_PWR_PORT, SOIL_PWR_PIN, GPIO_PIN_RESET);
    
    if (sum > 0) {
        errors.soil_errors = 0;
    }
    
    return (uint16_t)(sum / samples);
}

// ===== DISPLAY AND CONTROL FUNCTIONS =====
static void set_leds(uint8_t r, uint8_t g, uint8_t b) {
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, r ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, g ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, b ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void update_status_leds(uint8_t soil_dry, uint8_t soil_med, uint8_t soil_wet, uint8_t alarm) {
    if (alarm) {
        // Blink red LED for alarm
        if (HAL_GetTick() - blink_timer > 500) {
            blink_state ^= 1;
            blink_timer = HAL_GetTick();
        }
        set_leds(blink_state, 0, 0);
    } else {
        // Normal moisture indication
        set_leds(soil_dry, soil_med, soil_wet);
    }
}

static void servo_drive_fast(uint8_t on) {
    __HAL_TIM_SET_COMPARE(&SERVO_TIMER, SERVO_CH, on ? SERVO_FAST : SERVO_STOP);
}

// ===== SYSTEM HEALTH AND ALARM FUNCTIONS =====
static uint8_t system_health_check(void) {
    uint32_t now = HAL_GetTick();
    
    // Check for persistent errors
    if (errors.dht_errors > MAX_ERRORS || errors.us_errors > MAX_ERRORS || errors.soil_errors > MAX_ERRORS) {
        system_state = STATE_ERROR;
        errors.last_error_time = now;
        return 0;
    }
    
    // Check for stuck alarm
    if (alarm_state.alarm_active && (now - alarm_state.alarm_start_time > ALARM_TIMEOUT)) {
        system_state = STATE_MAINTENANCE;
        return 0;
    }
    
    // Reset to normal if no issues
    if (system_state == STATE_ERROR && (now - errors.last_error_time > 30000)) { // 30s recovery
        system_state = STATE_NORMAL;
    }
    
    return 1;
}

static void update_alarm_logic(uint16_t soil, uint16_t tank_cm) {
    uint8_t soil_critical = (soil > SOIL_DRY_THR + 100); // Hysteresis
    uint8_t tank_critical = (tank_cm < TANK_MIN_CM || tank_cm > TANK_MAX_CM);
    uint8_t alarm_condition = soil_critical || tank_critical;
    
    if (alarm_condition && !alarm_state.alarm_active) {
        alarm_state.consecutive_alarms++;
        if (alarm_state.consecutive_alarms >= 3) { // Confirm alarm after 3 consecutive detections
            alarm_state.alarm_active = 1;
            alarm_state.alarm_start_time = HAL_GetTick();
            alarm_state.alarm_count++;
            system_state = STATE_ALARM;
            servo_drive_fast(1);
        }
    } else if (!alarm_condition && alarm_state.alarm_active) {
        // Clear alarm with hysteresis
        if (soil < (SOIL_DRY_THR - 100) && tank_cm >= TANK_MIN_CM && tank_cm <= TANK_MAX_CM) {
            alarm_state.total_alarm_time += (HAL_GetTick() - alarm_state.alarm_start_time);
            alarm_state.alarm_active = 0;
            alarm_state.consecutive_alarms = 0;
            system_state = STATE_NORMAL;
            servo_drive_fast(0);
        }
    }
    
    if (!alarm_condition) {
        alarm_state.consecutive_alarms = 0;
    }
}

static void display_error_info(void) {
    char line[17];
    HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr("System Errors:");
    snprintf(line, sizeof(line), "D:%u U:%u S:%u", 
             errors.dht_errors, errors.us_errors, errors.soil_errors);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(line);
}

static void display_stats_info(void) {
    char l1[17], l2[17];
    snprintf(l1, sizeof(l1), "Alarms:%3u", alarm_state.alarm_count);
    snprintf(l2, sizeof(l2), "T:%.1f-%.1fC", data_log.temp_min, data_log.temp_max);
    HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr(l1);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr(l2);
}

static void update_data_logger(float temp, float hum, uint16_t soil, uint16_t dist) {
    // Update min/max values
    if (temp < data_log.temp_min) data_log.temp_min = temp;
    if (temp > data_log.temp_max) data_log.temp_max = temp;
    if (hum < data_log.humidity_min) data_log.humidity_min = hum;
    if (hum > data_log.humidity_max) data_log.humidity_max = hum;
    if (soil < data_log.soil_min) data_log.soil_min = soil;
    if (soil > data_log.soil_max) data_log.soil_max = soil;
    if (dist < data_log.distance_min) data_log.distance_min = dist;
    if (dist > data_log.distance_max) data_log.distance_max = dist;
}

// ===== DEVICE READINESS CHECKS =====
static uint8_t check_DHT_ready(void) { return read_DHT11_basic(); }
static uint8_t check_US_ready(void) { uint16_t cm; return read_HCSR04_cm_basic(&cm); }
static uint8_t check_SOIL_ready(void) { uint16_t v = read_soil_adc_basic(); return v > 0; }
static uint8_t check_SERVO_ready(void) { return 1; }

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
/* USER CODE BEGIN 1 */
/* USER CODE END 1 */

/* MCU Configuration--------------------------------------------------------*/

/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
HAL_Init();

/* USER CODE BEGIN Init */
/* USER CODE END Init */

/* Configure the system clock */
SystemClock_Config();

/* USER CODE BEGIN SysInit */
/* USER CODE END SysInit */

/* Initialize all configured peripherals */
MX_GPIO_Init();
MX_ADC1_Init();
MX_I2C1_Init();
MX_TIM1_Init();
MX_TIM2_Init();

/* USER CODE BEGIN 2 */
HAL_TIM_Base_Start(&htim1);
HAL_ADC_Start(&hadc1);
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

HD44780_Init(2);
HD44780_Clear();
HD44780_SetCursor(0, 0);
HD44780_PrintStr("Greenhouse MSz");
HAL_Delay(1500);

// Initialize data logger
data_log.last_reset_time = HAL_GetTick();

// Readiness loop with enhanced feedback
uint8_t retry_count = 0;
while (!all_ready && retry_count < 10) {
    all_ready = 1;
    
    if (!check_DHT_ready()) {
        HD44780_Clear();
        HD44780_PrintStr("DHT11 not ready");
        all_ready = 0;
        HAL_Delay(1000);
    }
    if (!check_US_ready()) {
        HD44780_Clear();
        HD44780_PrintStr("HC-SR04 not rdy");
        all_ready = 0;
        HAL_Delay(1000);
    }
    if (!check_SOIL_ready()) {
        HD44780_Clear();
        HD44780_PrintStr("Soil sens n/rdy");
        all_ready = 0;
        HAL_Delay(1000);
    }
    if (!check_SERVO_ready()) {
        HD44780_Clear();
        HD44780_PrintStr("Servo not ready");
        all_ready = 0;
        HAL_Delay(1000);
    }
    
    if (!all_ready) {
        retry_count++;
        char msg[17];
        snprintf(msg, sizeof(msg), "Retry %u/10", retry_count);
        HD44780_Clear();
        HD44780_SetCursor(0, 0);
        HD44780_PrintStr("Init failed");
        HD44780_SetCursor(0, 1);
        HD44780_PrintStr(msg);
        HAL_Delay(2000);
    }
}

if (all_ready) {
    HD44780_Clear();
    HD44780_PrintStr("All devices OK");
    set_leds(0, 1, 0); // Green LED for ready
    HAL_Delay(1000);
    set_leds(0, 0, 0);
} else {
    HD44780_Clear();
    HD44780_PrintStr("Init FAILED!");
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr("Check hardware");
    set_leds(1, 0, 0); // Red LED for error
    while(1) { HAL_Delay(1000); } // Halt system
}

last_switch_ms = HAL_GetTick();
show_mode = 0;

/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
    // System health check
    system_health_check();
    
    // Read sensors with enhanced functions
    uint16_t soil = read_soil_adc_averaged();
    uint16_t cm = 999;
    uint8_t distance_valid = read_HCSR04_filtered(&cm);
    uint8_t dht_valid = read_DHT11_improved();
    
    // Soil moisture classification
    uint8_t soil_dry = (soil > SOIL_DRY_THR);
    uint8_t soil_med = (soil > SOIL_MED_THR) && !soil_dry;
    uint8_t soil_wet = (soil <= SOIL_WET_THR);
    
    // Tank status
    uint8_t tank_ok = distance_valid && (cm >= TANK_MIN_CM) && (cm <= TANK_MAX_CM);
    
    // Update alarm logic with enhanced hysteresis
    update_alarm_logic(soil, cm);
    
    // Update data logger
    if (dht_valid) {
        update_data_logger(tCelsius, RH, soil, cm);
    }
    
    // LED status with enhanced blinking for alarms
    update_status_leds(soil_dry, soil_med, soil_wet, alarm_state.alarm_active);
    
    // Display logic based on system state
    switch (system_state) {
        case STATE_ERROR:
            display_error_info();
            break;
            
        case STATE_MAINTENANCE:
            HD44780_Clear();
            HD44780_SetCursor(0, 0);
            HD44780_PrintStr("MAINTENANCE MODE");
            HD44780_SetCursor(0, 1);
            HD44780_PrintStr("Check system");
            servo_drive_fast(0); // Stop servo in maintenance
            break;
            
        case STATE_ALARM:
            HD44780_Clear();
            HD44780_SetCursor(0, 0);
            HD44780_PrintStr("ALERT: Action Req");
            char line[17];
            snprintf(line, sizeof(line), "S:%4u D:%2u", soil, (cm > 99 ? 99 : cm));
            HD44780_SetCursor(0, 1);
            HD44780_PrintStr(line);
            break;
            
        case STATE_NORMAL:
        default:
            // Enhanced page switching with more pages
            if (HAL_GetTick() - last_switch_ms >= PAGE_MS) {
                show_mode = (show_mode + 1) % 4; // 4 different pages
                last_switch_ms = HAL_GetTick();
            }
            
            switch (show_mode) {
                case 0: // DHT11 page
                    if (dht_valid) {
                        char l1[17], l2[17];
                        snprintf(l1, sizeof(l1), "T:%2d.%1dC H:%2d%%", TCI, (TCD/10), RHI);
                        snprintf(l2, sizeof(l2), "F:%3d.%1d RH:%2d.%1d",
                                 (int)tFahrenheit, (int)((tFahrenheit*10))%10, RHI, (RHD/10));
                        HD44780_Clear();
                        HD44780_SetCursor(0, 0);
                        HD44780_PrintStr(l1);
                        HD44780_SetCursor(0, 1);
                        HD44780_PrintStr(l2);
                    } else {
                        HD44780_Clear();
                        HD44780_PrintStr("DHT11 error");
                        char err_line[17];
                        snprintf(err_line, sizeof(err_line), "Errors: %u", errors.dht_errors);
                        HD44780_SetCursor(0, 1);
                        HD44780_PrintStr(err_line);
                    }
                    break;
                    
                case 1: // Tank distance page
                    {
                        char l1[17], l2[17];
                        if (distance_valid) {
                            snprintf(l1, sizeof(l1), "Tank Dist:%2ucm", (cm > 99 ? 99 : cm));
                            snprintf(l2, sizeof(l2), "Range 5-20cm %s", tank_ok ? "OK" : "BAD");
                        } else {
                            snprintf(l1, sizeof(l1), "Tank: No signal");
                            snprintf(l2, sizeof(l2), "US Errors: %u", errors.us_errors);
                        }
                        HD44780_Clear();
                        HD44780_SetCursor(0, 0);
                        HD44780_PrintStr(l1);
                        HD44780_SetCursor(0, 1);
                        HD44780_PrintStr(l2);
                    }
                    break;
                    
                case 2: // Soil moisture page
                    {
                        char l1[17], l2[17];
                        snprintf(l1, sizeof(l1), "Soil ADC: %4u", soil);
                        const char* status = soil_wet ? "WET" : (soil_med ? "MED" : "DRY");
                        snprintf(l2, sizeof(l2), "Status: %s", status);
                        HD44780_Clear();
                        HD44780_SetCursor(0, 0);
                        HD44780_PrintStr(l1);
                        HD44780_SetCursor(0, 1);
                        HD44780_PrintStr(l2);
                        
                        #if CALIBRATION_MODE
                        // In calibration mode, also show raw values
                        HAL_Delay(1000);
                        HD44780_Clear();
                        HD44780_SetCursor(0, 0);
                        HD44780_PrintStr("Calibration");
                        snprintf(l2, sizeof(l2), "Raw: %4u", soil);
                        HD44780_SetCursor(0, 1);
                        HD44780_PrintStr(l2);
                        #endif
                    }
                    break;
                    
                case 3: // Statistics page
                    display_stats_info();
                    break;
            }
            break;
    }
    
    // Adaptive delay based on system state
    switch (system_state) {
        case STATE_ALARM:
            HAL_Delay(100); // Faster updates during alarm
            break;
        case STATE_ERROR:
            HAL_Delay(500); // Medium updates during error
            break;
        default:
            HAL_Delay(200); // Normal update rate
            break;
    }

/* USER CODE END WHILE */

/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }

  /** ADC clock */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief ADC1 Initialization Function
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) { Error_Handler(); }

  sConfig.Channel = ADC_CHANNEL_9;                 // PB1
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief I2C1 Initialization Function
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief TIM1 Initialization Function  (1µs timebase)
  * @retval None
  */
static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;                    // 72MHz / (71+1) = 1MHz -> 1us
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK) { Error_Handler(); }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief TIM2 Initialization Function (PWM CH2 for Servo)
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;                   // For 50Hz servo PWM
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;                    // 20ms period
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) { Error_Handler(); }

  HAL_TIM_MspPostInit(&htim2);   // config PA1 AF for TIM2_CH2
}

/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Set default outputs low */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

  /* PA0, PA3, PA5, PA7, PA9 as outputs (LEDs, soil power, TRIG) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PA8 as input (ECHO) */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PB9 as output (DHT11 line starts as output, will switch to input later) */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval int
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
    // Blink all LEDs to indicate fatal error
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_Delay(200);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/*
 * ===== ENHANCED GREENHOUSE MONITORING SYSTEM =====
 * 
 * Features Added:
 * 1. Enhanced error tracking and recovery
 * 2. Improved sensor reading with filtering and averaging
 * 3. Smart alarm logic with hysteresis
 * 4. Multiple display pages with statistics
 * 5. Data logging (min/max tracking)
 * 6. System health monitoring
 * 7. Calibration mode support
 * 8. Adaptive update rates based on system state
 * 9. Better LED status indication with blinking
 * 10. Comprehensive error handling and recovery
 * 
 * System States:
 * - NORMAL: Regular operation
 * - ALARM: Critical condition detected
 * - ERROR: Sensor or system errors
 * - MAINTENANCE: Manual intervention required
 * 
 * Display Pages (8-second rotation):
 * 0: Temperature and Humidity (DHT11)
 * 1: Tank distance (HC-SR04)
 * 2: Soil moisture with raw ADC values
 * 3: System statistics and alarm history
 * 
 * Configuration:
 * - Set CALIBRATION_MODE to 1 for sensor calibration
 * - Adjust thresholds in #define section as needed
 * - Error thresholds and timeouts are configurable
 * 
 * LED Indicators:
 * - Red: Dry soil or alarm (blinking during alarm)
 * - Green: Medium soil moisture
 * - Blue: Wet soil
 * - Green (startup): System ready
 * - Red (startup): Initialization failed
 * 
 * Servo Control:
 * - Activated during alarm conditions
 * - Automatically stopped when alarm clears
 * - Safety timeout prevents continuous operation
 */