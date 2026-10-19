/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "liquidcrystal_i2c.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* none */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* --- Pin map (matches this file's init) --- */
#define I2C_LCD_ADDR   (0x27)     // set inside your LCD driver if needed

#define TRIG_GPIO      GPIOB
#define TRIG_PIN       GPIO_PIN_0
#define ECHO_GPIO      GPIOB
#define ECHO_PIN       GPIO_PIN_1  // MUST be level-shifted to 3.3V

#define DHT_GPIO       GPIOB
#define DHT_PIN        GPIO_PIN_10 // 4.7k pull-up to 3.3V

#define SOIL_ADC_CH    ADC_CHANNEL_0 // PA0
#define SOIL_PWR_GPIO  GPIOB
#define SOIL_PWR_PIN   GPIO_PIN_12   // optional probe power GPIO

#define LED_G_GPIO     GPIOA
#define LED_G_PIN      GPIO_PIN_10   // Green
#define LED_Y_GPIO     GPIOA
#define LED_Y_PIN      GPIO_PIN_11   // Yellow
#define LED_R_GPIO     GPIOA
#define LED_R_PIN      GPIO_PIN_12   // Red

#define SERVO_TIM      TIM1
#define SERVO_CH       TIM_CHANNEL_1 // PA8

/* --- Behavior & thresholds --- */
#define PAGE_SWAP_MS       10000U
#define LOOP_DELAY_MS       200U

/* Soil decision: raw ADC <= THR => Wet (YL-69 gives lower ADC when wet) */
static uint16_t SOIL_ADC_THR = 2500;

/* Tank % rule: if <=25% start servo until 100% */
#define TANK_MIN_PERCENT    25.0f
#define TANK_FULL_PERCENT   99.0f

/* Servo pulses (us) - tune for your horn */
#define SERVO_PULSE_CLOSED  1000
#define SERVO_PULSE_OPEN    2000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* none */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
/* none */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
/* DWT timing */
static void     DWT_Delay_Init(void);
static uint32_t micros(void);
static void     delay_us(uint32_t us);

/* Sensors */
static float    HCSR04_ReadCM(void);
static bool     DHT11_Read(float *tC, float *rh);
static uint16_t Soil_ADC_SampleRaw(void);

/* Mapping & UI */
typedef enum { TANK_OK=0, TANK_WARN=1, TANK_CRIT_BLINK=2 } tank_band_t;
typedef struct { float pct; tank_band_t band; } tank_info_t;
static tank_info_t Tank_FromDistance(float d_cm);
static void     LEDs_Update(tank_band_t tb, bool soilDry, uint32_t ms);

/* Servo */
static void     Servo_WriteUS(uint16_t us);

/* LCD pages */
static void     LCD_PageIntro1(void);
static void     LCD_PageIntro2(void);
static void     LCD_Page3(float tC, float rh, float tankPct);
static void     LCD_Page4(bool soilDry, float tankPct);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ---- DWT microsecond timing ---- */
static void DWT_Delay_Init(void)
{
  /* Enable TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* Reset the cycle counter */
  DWT->CYCCNT = 0;
  /* Enable the cycle counter */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t micros(void)
{
  /* SystemCoreClock is typically 72 MHz => ticks per us = 72 */
  return (uint32_t)(DWT->CYCCNT / (SystemCoreClock / 1000000U));
}

static void delay_us(uint32_t us)
{
  uint32_t t0 = micros();
  while ((micros() - t0) < us) {}
}

/* ---- Ultrasonic: returns cm, or negative on error ---- */
static float HCSR04_ReadCM(void)
{
  /* Ensure TRIG low for at least 2 us */
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);
  delay_us(2);

  /* 10 us TRIG pulse */
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);

  /* Wait for ECHO to go high (timeout ~30 ms) */
  uint32_t to = micros();
  while (HAL_GPIO_ReadPin(ECHO_GPIO, ECHO_PIN) == GPIO_PIN_RESET) {
    if ((micros() - to) > 30000U) return -1.0f; // no echo start
  }

  /* Measure high time */
  uint32_t start = micros();
  while (HAL_GPIO_ReadPin(ECHO_GPIO, ECHO_PIN) == GPIO_PIN_SET) {
    if ((micros() - start) > 30000U) return -2.0f; // echo too long/out of range
  }
  uint32_t pulse_us = micros() - start;

  /* HC-SR04: distance (cm) ˜ pulse_us / 58.0 */
  return (float)pulse_us / 58.0f;
}

/* ---- DHT11 read (blocking minimal) ----
   Returns true on success and fills *tC & *rh (integers as floats). */
static bool DHT11_Read(float *tC, float *rh)
{
  uint8_t d[5] = {0};

  /* Start signal: output low 18 ms, then high 20-40 us, switch to input */
  GPIO_InitTypeDef g = {0};
  g.Pin   = DHT_PIN;
  g.Mode  = GPIO_MODE_OUTPUT_PP;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT_GPIO, &g);

  HAL_GPIO_WritePin(DHT_GPIO, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);
  HAL_GPIO_WritePin(DHT_GPIO, DHT_PIN, GPIO_PIN_SET);
  delay_us(30);

  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_GPIO, &g);

  /* Response: ~80us low + ~80us high */
  uint32_t t = micros();
  while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN))         if (micros() - t > 120) return false;
  t = micros(); while (!HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN)) if (micros() - t > 120) return false;
  t = micros(); while ( HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN)) if (micros() - t > 120) return false;

  /* 40 bits */
  for (int i=0;i<40;i++) {
    t = micros();
    while (!HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN)) if (micros() - t > 120) return false; // wait 50us low
    /* measure high width */
    t = micros();
    while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN))  if (micros() - t > 120) break;
    uint32_t width = micros() - t;
    d[i/8] <<= 1;
    if (width > 40) d[i/8] |= 1; // ~26-28us => 0, ~70us => 1
  }

  if ((uint8_t)(d[0]+d[1]+d[2]+d[3]) != d[4]) return false;
  *rh = (float)d[0];
  *tC = (float)d[2];
  return true;
}

/* ---- Soil ADC sampling (avg 8) with optional probe power ---- */
static uint16_t Soil_ADC_SampleRaw(void)
{
  /* power probe only while sampling (optional; tie SOIL_PWR_PIN if used) */
  HAL_GPIO_WritePin(SOIL_PWR_GPIO, SOIL_PWR_PIN, GPIO_PIN_SET);
  HAL_Delay(2);
  uint32_t acc = 0;
  for (int i=0;i<8;i++) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5);
    acc += HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    delay_us(300);
  }
  HAL_GPIO_WritePin(SOIL_PWR_GPIO, SOIL_PWR_PIN, GPIO_PIN_RESET);
  return (uint16_t)(acc/8U);
}

/* ---- Tank band per your spec (distance buckets) ----
   10–20 cm => 100–50% (Green steady)
   5–10  cm => 50–25%  (Red steady)
   1–5   cm => 25–0%   (Red/Green alternate blink)
   >20 cm cap at 50% (Green), <1 cm cap at 0% (blink)
*/
static tank_info_t Tank_FromDistance(float d_cm)
{
  tank_info_t ti = {0.0f, TANK_CRIT_BLINK};

  if (!isfinite(d_cm) || d_cm <= 0.0f) { ti.pct = 0.0f;  ti.band = TANK_CRIT_BLINK; return ti; }

  if (d_cm >= 10.0f && d_cm <= 20.0f) {
    ti.pct  = 100.0f - ((d_cm - 10.0f) * (50.0f / 10.0f)); // 10->100, 20->50
    ti.band = TANK_OK;                                     // Green
  } else if (d_cm >= 5.0f && d_cm < 10.0f) {
    ti.pct  =  50.0f - ((d_cm - 5.0f)  * (25.0f / 5.0f));  // 5->50, 10->25
    ti.band = TANK_WARN;                                   // Red
  } else if (d_cm >= 1.0f && d_cm < 5.0f) {
    ti.pct  =  25.0f - ((d_cm - 1.0f)  * (25.0f / 4.0f));  // 1->25, 5->0
    ti.band = TANK_CRIT_BLINK;                             // Red/Green blink
  } else if (d_cm > 20.0f) {
    ti.pct  = 50.0f; ti.band = TANK_OK;
  } else { // d_cm < 1.0
    ti.pct  = 0.0f; ti.band = TANK_CRIT_BLINK;
  }

  if (ti.pct < 0.0f) ti.pct = 0.0f;
  if (ti.pct > 100.0f) ti.pct = 100.0f;
  return ti;
}

/* ---- LED policy (tank band + soil dry priority) ---- */
static void LEDs_Update(tank_band_t tb, bool soilDry, uint32_t ms)
{
  GPIO_PinState g=GPIO_PIN_RESET, y=GPIO_PIN_RESET, r=GPIO_PIN_RESET;

  /* Highest priority: tank critical => blink G/R alternately every 500 ms */
  if (tb == TANK_CRIT_BLINK) {
    if ((ms/500U)%2U==0U) g = GPIO_PIN_SET; else r = GPIO_PIN_SET;
  } else if (soilDry) {
    y = GPIO_PIN_SET; // soil dry => Yellow steady
  } else {
    if (tb == TANK_OK)   g = GPIO_PIN_SET;
    if (tb == TANK_WARN) r = GPIO_PIN_SET;
  }

  HAL_GPIO_WritePin(LED_G_GPIO, LED_G_PIN, g);
  HAL_GPIO_WritePin(LED_Y_GPIO, LED_Y_PIN, y);
  HAL_GPIO_WritePin(LED_R_GPIO, LED_R_PIN, r);
}

/* ---- Servo helpers ---- */
static void Servo_WriteUS(uint16_t us)
{
  __HAL_TIM_SET_COMPARE(&htim1, SERVO_CH, us);
}

/* ---- LCD helpers ---- */
static void LCD_PageIntro1(void)
{
  HD44780_Clear();
  HD44780_SetCursor(0,0); HD44780_PrintStr("CSE331");
  HD44780_SetCursor(0,1); HD44780_PrintStr("Group: 3");
}
static void LCD_PageIntro2(void)
{
  HD44780_Clear();
  HD44780_SetCursor(0,0); HD44780_PrintStr("Smart Green-");
  HD44780_SetCursor(0,1); HD44780_PrintStr("house Manager");
}
static void LCD_Page3(float tC, float rh, float tankPct)
{
  char line[17];
  HD44780_Clear();

  /* Row 1: Temp/RH */
  HD44780_SetCursor(0,0);
  snprintf(line, sizeof line, "Temp/RH %2.0f/%2.0f", tC, rh);
  line[16] = '\0';   // ensure safe termination
  HD44780_PrintStr(line);

  /* Row 2: Tank % */
  HD44780_SetCursor(0,1);
  snprintf(line, sizeof line, "Tank:%3.0f%%      ", tankPct); // pad with spaces
  line[16] = '\0';
  HD44780_PrintStr(line);
}

static void LCD_Page4(bool soilDry, float tankPct)
{
  char line[17];
  HD44780_Clear();

  /* Row 1: Soil state (exactly 16 chars) */
HD44780_SetCursor(0,0);
snprintf(line, sizeof line, "Soil:%-3s", soilDry ? "Dry" : "Wet");  // e.g., "Soil:Dry"
size_t n = strlen(line);
while (n < 16) line[n++] = ' ';   // pad with spaces
line[16] = '\0';
HD44780_PrintStr(line);

  /* Row 2: Tank % */
  HD44780_SetCursor(0,1);
  snprintf(line, sizeof line, "Tank:%3.0f%%      ", tankPct);
  line[16] = '\0';
  HD44780_PrintStr(line);
}

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
  /* Keep SWD, free JTAG if you want PB3/PB4/PB5 */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
  DWT_Delay_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, SERVO_CH);
  Servo_WriteUS(SERVO_PULSE_CLOSED); // start closed

  /* LCD init (2 rows) */
  HD44780_Init(2);
  HD44780_Backlight();

  LCD_PageIntro1(); HAL_Delay(1000);
  LCD_PageIntro2(); HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  bool     showPage3   = true;
  uint32_t lastSwap    = HAL_GetTick();
  char     line[17];
  float    tC=0, rh=0;

  while (1)
  {
    /* --- Read sensors --- */
    (void)DHT11_Read(&tC, &rh);                 // if it fails, values remain last good
    uint16_t soilRaw = Soil_ADC_SampleRaw();
    bool soilDry = (soilRaw > SOIL_ADC_THR);

    float d_cm = HCSR04_ReadCM();
    tank_info_t tank = Tank_FromDistance(d_cm);

    /* --- Servo logic --- */
    bool tankLow     = (tank.pct <= TANK_MIN_PERCENT);
    bool tankIsFull  = (tank.pct >= TANK_FULL_PERCENT);
    bool needServo   = soilDry || tankLow;
    if (needServo && !tankIsFull) Servo_WriteUS(SERVO_PULSE_OPEN);
    else                          Servo_WriteUS(SERVO_PULSE_CLOSED);

    /* --- LEDs --- */
    LEDs_Update(tank.band, soilDry, HAL_GetTick());

    /* --- LCD pages: 10 s swap --- */
    if (HAL_GetTick() - lastSwap >= PAGE_SWAP_MS) {
      showPage3 = !showPage3;
      lastSwap  = HAL_GetTick();
    }
    if (showPage3)  LCD_Page3(tC, rh, tank.pct);
    else            LCD_Page4(soilDry, tank.pct);

    HAL_Delay(LOOP_DELAY_MS);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration (72 MHz HSE->PLL)
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType           = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                        | RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource        = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider       = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider      = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider      = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief I2C1 Initialization Function (PB6=SCL, PB7=SDA)
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance             = I2C1;
  hi2c1.Init.ClockSpeed      = 100000;
  hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1     = 0;
  hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2     = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) { Error_Handler(); }

  /* GPIO config for I2C (open-drain, pull-up external to 3.3V) */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin   = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief ADC1 Initialization Function (PA0 -> ADC1_IN0)
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  __HAL_RCC_ADC1_CLK_ENABLE();

  ADC_ChannelConfTypeDef sConfig = {0};
  hadc1.Instance                   = ADC1;
  hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) { Error_Handler(); }

  sConfig.Channel      = SOIL_ADC_CH;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief TIM1 Initialization Function (PWM CH1 @ 50 Hz on PA8)
  * @retval None
  */
static void MX_TIM1_Init(void)
{
  __HAL_RCC_TIM1_CLK_ENABLE();

  TIM_OC_InitTypeDef sConfigOC = {0};
  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = (uint16_t)((SystemCoreClock/1000000U)-1U); // 1 MHz
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 20000-1;  // 20,000 us -> 50 Hz
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode       = TIM_OCMODE_PWM1;
  sConfigOC.Pulse        = SERVO_PULSE_CLOSED; // default
  sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, SERVO_CH) != HAL_OK) { Error_Handler(); }

  /* GPIO PA8 AF PushPull */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin   = GPIO_PIN_8;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_TIM_MspPostInit(&htim1);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();

  /* LEDs: PA10/11/12 output push-pull low */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  HAL_GPIO_WritePin(LED_G_GPIO, LED_G_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_Y_GPIO, LED_Y_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_R_GPIO, LED_R_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = LED_G_PIN|LED_Y_PIN|LED_R_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* TRIG PB0 output low */
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = TRIG_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIG_GPIO, &GPIO_InitStruct);

  /* ECHO PB1 input (external divider to 3.3V!) */
  GPIO_InitStruct.Pin   = ECHO_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_GPIO, &GPIO_InitStruct);

  /* Soil probe power PB12 (optional) default LOW */
  HAL_GPIO_WritePin(SOIL_PWR_GPIO, SOIL_PWR_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin   = SOIL_PWR_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SOIL_PWR_GPIO, &GPIO_InitStruct);

  /* DHT11 pin initially input; driver will switch modes */
  GPIO_InitStruct.Pin   = DHT_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_GPIO, &GPIO_InitStruct);

  /* Free JTAG, keep SWD (optional) */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* ADC pin PA0 is analog by default after reset; if needed: */
  GPIO_InitStruct.Pin  = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    /* Blink RED fast to indicate error */
    HAL_GPIO_TogglePin(LED_R_GPIO, LED_R_PIN);
    HAL_Delay(100);
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif /* USE_FULL_ASSERT */
