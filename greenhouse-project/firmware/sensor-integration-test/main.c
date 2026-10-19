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
#include "stdio.h"
#include "string.h"
#include "liquidcrystal_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
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
#define LED_B_PIN  GPIO_PIN_0   // PA0 = Blue LED (PA1 reserved for TIM2_CH2)

// ---- Servo (TIM2 CH2 on PA1) ----
#define SERVO_TIMER   htim2
#define SERVO_CH      TIM_CHANNEL_2
#define SERVO_FAST    1250
#define SERVO_STOP    750

// ---- App settings ----
#define PAGE_MS       10000   // 10 sec page swap
#define SOIL_DRY_THR  3200
#define SOIL_MED_THR  2700
#define TANK_MIN_CM   5
#define TANK_MAX_CM   20
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
static uint8_t RHI,RHD,TCI,TCD,SUM;
static float tCelsius,tFahrenheit,RH;
static uint32_t pMillis,cMillis;

static uint8_t all_ready=0;
static uint8_t show_mode=0;
static uint32_t last_switch_ms=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
static void microDelay(uint16_t us);
static uint8_t DHT11_Start(void);
static uint8_t DHT11_Read(void);
static uint8_t read_DHT11(void);
static uint8_t read_HCSR04_cm(uint16_t *cm);
static uint16_t read_soil_adc(void);
static void set_leds(uint8_t r,uint8_t g,uint8_t b);
static void servo_drive_fast(uint8_t on);
static uint8_t check_DHT_ready(void);
static uint8_t check_US_ready(void);
static uint8_t check_SOIL_ready(void);
static uint8_t check_SERVO_ready(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void microDelay(uint16_t us){
  __HAL_TIM_SET_COUNTER(&htim1,0);
  while(__HAL_TIM_GET_COUNTER(&htim1)<us);
}
static uint8_t DHT11_Start(void){
  uint8_t Response=0;
  GPIO_InitTypeDef s={0};
  s.Pin=DHT11_PIN; s.Mode=GPIO_MODE_OUTPUT_PP; s.Speed=GPIO_SPEED_FREQ_LOW; s.Pull=GPIO_NOPULL;
  HAL_GPIO_Init(DHT11_PORT,&s);
  HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_SET);
  microDelay(30);
  s.Mode=GPIO_MODE_INPUT; s.Pull=GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_PORT,&s);
  microDelay(40);
  if(!HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)){ microDelay(80); if(HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)) Response=1; }
  pMillis=HAL_GetTick(); cMillis=HAL_GetTick();
  while(HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)&&(pMillis+2>cMillis)){ cMillis=HAL_GetTick(); }
  return Response;
}
static uint8_t DHT11_Read(void){
  uint8_t a,b=0;
  for(a=0;a<8;a++){
    pMillis=HAL_GetTick(); cMillis=HAL_GetTick();
    while(!HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)&&(pMillis+2>cMillis)){ cMillis=HAL_GetTick();}
    microDelay(40);
    if(!HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)) b&=~(1<<(7-a)); else b|=(1<<(7-a));
    pMillis=HAL_GetTick(); cMillis=HAL_GetTick();
    while(HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)&&(pMillis+2>cMillis)){ cMillis=HAL_GetTick();}
  }
  return b;
}
static uint8_t read_DHT11(void){
  if(!DHT11_Start()) return 0;
  RHI=DHT11_Read(); RHD=DHT11_Read(); TCI=DHT11_Read(); TCD=DHT11_Read(); SUM=DHT11_Read();
  if((uint8_t)(RHI+RHD+TCI+TCD)!=SUM) return 0;
  tCelsius=(float)TCI+(float)(TCD/10.0f);
  tFahrenheit=tCelsius*9.0f/5.0f+32.0f;
  RH=(float)RHI+(float)(RHD/10.0f);
  return 1;
}
static uint8_t read_HCSR04_cm(uint16_t *cm){
  HAL_GPIO_WritePin(TRIG_PORT,TRIG_PIN,GPIO_PIN_RESET);
  microDelay(2);
  HAL_GPIO_WritePin(TRIG_PORT,TRIG_PIN,GPIO_PIN_SET);
  microDelay(10);
  HAL_GPIO_WritePin(TRIG_PORT,TRIG_PIN,GPIO_PIN_RESET);
  uint32_t start=HAL_GetTick();
  while(!HAL_GPIO_ReadPin(ECHO_PORT,ECHO_PIN)){ if(HAL_GetTick()-start>10) return 0;}
  uint32_t v1=__HAL_TIM_GET_COUNTER(&htim1);
  start=HAL_GetTick();
  while(HAL_GPIO_ReadPin(ECHO_PORT,ECHO_PIN)){ if(HAL_GetTick()-start>50) return 0;}
  uint32_t v2=__HAL_TIM_GET_COUNTER(&htim1);
  *cm=(uint16_t)((v2-v1)*0.034f/2.0f);
  return 1;
}
static uint16_t read_soil_adc(void){
  HAL_GPIO_WritePin(SOIL_PWR_PORT,SOIL_PWR_PIN,GPIO_PIN_SET);
  HAL_Delay(40);
  HAL_ADC_PollForConversion(&hadc1,5);
  uint16_t v=HAL_ADC_GetValue(&hadc1);
  HAL_GPIO_WritePin(SOIL_PWR_PORT,SOIL_PWR_PIN,GPIO_PIN_RESET);
  return v;
}
static void set_leds(uint8_t r,uint8_t g,uint8_t b){
  HAL_GPIO_WritePin(LED_R_PORT,LED_R_PIN,r?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_G_PORT,LED_G_PIN,g?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_B_PORT,LED_B_PIN,b?GPIO_PIN_SET:GPIO_PIN_RESET);
}
static void servo_drive_fast(uint8_t on){
  __HAL_TIM_SET_COMPARE(&SERVO_TIMER,SERVO_CH,on?SERVO_FAST:SERVO_STOP);
}
static uint8_t check_DHT_ready(void){return read_DHT11();}
static uint8_t check_US_ready(void){uint16_t cm; return read_HCSR04_cm(&cm);}
static uint8_t check_SOIL_ready(void){uint16_t v=read_soil_adc(); return v>0;}
static uint8_t check_SERVO_ready(void){return 1;}
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
HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);

HD44780_Init(2); HD44780_Clear();
HD44780_SetCursor(0,0); HD44780_PrintStr("Greenhouse v1");
HAL_Delay(1000);

// Readiness loop
while(!all_ready){
  all_ready=1;
  if(!check_DHT_ready()){HD44780_Clear();HD44780_PrintStr("DHT11 not ready");all_ready=0;HAL_Delay(1000);}
  if(!check_US_ready()){HD44780_Clear();HD44780_PrintStr("HC-SR04 not rdy");all_ready=0;HAL_Delay(1000);}
  if(!check_SOIL_ready()){HD44780_Clear();HD44780_PrintStr("Soil sens n/rdy");all_ready=0;HAL_Delay(1000);}
  if(!check_SERVO_ready()){HD44780_Clear();HD44780_PrintStr("Servo not ready");all_ready=0;HAL_Delay(1000);}
}
HD44780_Clear();HD44780_PrintStr("All devices OK");
HAL_Delay(800);

last_switch_ms=HAL_GetTick();
show_mode=0;
/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
  // Soil moisture + LEDs
  uint16_t soil=read_soil_adc();
  uint8_t soil_dry=(soil>SOIL_DRY_THR);
  uint8_t soil_med=(soil>SOIL_MED_THR)&&!soil_dry;
  uint8_t soil_wet=!soil_dry&&!soil_med;
  set_leds(soil_dry,soil_med,soil_wet);

  // Tank distance
  uint16_t cm=999;
  uint8_t tank_ok=read_HCSR04_cm(&cm)&&(cm>=TANK_MIN_CM)&&(cm<=TANK_MAX_CM);
  uint8_t tank_low=!tank_ok;

  // Alarm logic
  uint8_t alarm=soil_dry||tank_low;
  if(alarm){
    servo_drive_fast(1);
    HD44780_Clear();
    HD44780_SetCursor(0,0);HD44780_PrintStr("Alart: PW MStart");
    char line[17]; snprintf(line,sizeof(line),"Soil:%4u D:%2u",soil,(cm>99?99:cm));
    HD44780_SetCursor(0,1);HD44780_PrintStr(line);
  } else {
    servo_drive_fast(0);
    // 10s page switch: DHT <-> Tank
    if(HAL_GetTick()-last_switch_ms>=PAGE_MS){ show_mode^=1; last_switch_ms=HAL_GetTick(); }
    if(show_mode==0){
      if(read_DHT11()){
        char l1[17],l2[17];
        snprintf(l1,sizeof(l1),"T:%2d.%1dC H:%2d%%",TCI,(TCD/10),RHI);
        snprintf(l2,sizeof(l2),"F:%3d.%1d RH:%2d.%1d",
                 (int)tFahrenheit,(int)((tFahrenheit*10))%10,RHI,(RHD/10));
        HD44780_Clear();
        HD44780_SetCursor(0,0);HD44780_PrintStr(l1);
        HD44780_SetCursor(0,1);HD44780_PrintStr(l2);
      } else { HD44780_Clear();HD44780_PrintStr("DHT11 error"); }
    } else {
      char l1[17],l2[17];
      snprintf(l1,sizeof(l1),"Tank Dist:%2ucm",(cm>99?99:cm));
      snprintf(l2,sizeof(l2),"OK 5..20cm %s",tank_ok?"Y":"N");
      HD44780_Clear();
      HD44780_SetCursor(0,0);HD44780_PrintStr(l1);
      HD44780_SetCursor(0,1);HD44780_PrintStr(l2);
    }
  }

  HAL_Delay(200);
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
  htim2.Init.Prescaler = 15;                   // as per Chat 4
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;                    // as per Chat 4
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
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif /* USE_FULL_ASSERT */
