/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  * This software is licensed under terms in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "liquidcrystal_i2c.h"   // <-- use your added driver
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum { BAND_LONG = 0, BAND_MID, BAND_SHORT } DistanceBand;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// -------- LED pins --------
#define LED_GREEN_PIN   GPIO_PIN_12
#define LED_YELLOW_PIN  GPIO_PIN_13
#define LED_RED_PIN     GPIO_PIN_14
#define LED_PORT        GPIOB

// -------- Moving average --------
#define SAMPLES 16

// -------- Thresholds (replace with your measured ADC counts) --------
// Example placeholders (12-bit ADC: 0..4095)
static uint16_t ADC_25CM = 900;     // ~25 cm (far)   -> LONG
static uint16_t ADC_10CM = 1800;    // ~10 cm         -> MID
static uint16_t ADC_5CM  = 2600;    // ~5  cm         -> SHORT (and closer)

// Small band hysteresis to reduce bouncing around boundaries
#define HYST 40
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
static inline void set_leds(uint8_t g, uint8_t y, uint8_t r) {
  HAL_GPIO_WritePin(LED_PORT, LED_GREEN_PIN,  g ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_PORT, LED_YELLOW_PIN, y ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_PORT, LED_RED_PIN,    r ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
static DistanceBand prev_band = (DistanceBand)(-1);  // force first update
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static uint16_t ReadADC1_PA0(void);
static uint16_t moving_avg(void);
static DistanceBand decide_band(uint16_t adc, DistanceBand last);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t ReadADC1_PA0(void) {
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
    uint16_t v = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return v;
  }
  HAL_ADC_Stop(&hadc1);
  return 0;
}

static uint16_t moving_avg(void) {
  uint32_t sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += ReadADC1_PA0();
  return (uint16_t)(sum / SAMPLES);
}

// Add light hysteresis so the label/LEDs don’t chatter near thresholds
static DistanceBand decide_band(uint16_t adc, DistanceBand last) {
  // lower ADC ˜ farther
  switch (last) {
    case BAND_LONG:
      if (adc >= (ADC_25CM + HYST)) return BAND_MID;
      return BAND_LONG;
    case BAND_MID:
      if (adc <  (ADC_25CM - HYST)) return BAND_LONG;
      if (adc >= (ADC_10CM + HYST)) return BAND_SHORT;
      return BAND_MID;
    case BAND_SHORT:
    default:
      if (adc < (ADC_10CM - HYST)) return BAND_MID;
      return BAND_SHORT;
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);

  // LCD init: your driver takes rows count
  HD44780_Init(2);
  HD44780_Clear();
  HD44780_SetCursor(0,0); HD44780_PrintStr("IR Distance");
  HD44780_SetCursor(0,1); HD44780_PrintStr("Starting...");
  set_leds(1,1,1); HAL_Delay(400); set_leds(0,0,0);
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    uint16_t adc = moving_avg();
    DistanceBand band = decide_band(adc, prev_band);

    if (band != prev_band) {
      prev_band = band;

      // Update LEDs
      if (band == BAND_LONG)      set_leds(1,0,0);
      else if (band == BAND_MID)  set_leds(0,1,0);
      else                        set_leds(0,0,1);

      // Update LCD (only when band changes to avoid flicker)
      HD44780_Clear();
      HD44780_SetCursor(0,0); HD44780_PrintStr("Distance:");
      HD44780_SetCursor(0,1);
      if (band == BAND_LONG)      HD44780_PrintStr("LONG   (>25cm)");
      else if (band == BAND_MID)  HD44780_PrintStr("MID  (10-25cm)");
      else                        HD44780_PrintStr("SHORT (<=10cm)");
    }

    HAL_Delay(120); // ~8 Hz loop
  }
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // 64 MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; // <= 36 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }

  // ADC clock <= 14 MHz ? APB2/6
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
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) { Error_Handler(); }

  sConfig.Channel = ADC_CHANNEL_0;                   // PA0
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5; // stable for AO
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
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // LED outputs PB12, PB13, PB14
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_PIN|LED_YELLOW_PIN|LED_RED_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED_GREEN_PIN|LED_YELLOW_PIN|LED_RED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Note: PB6/PB7 get configured by HAL_I2C_MspInit (generated by CubeMX) as AF_OD + pull-ups.
}

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif
