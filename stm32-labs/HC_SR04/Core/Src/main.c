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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Pin map (matches your CubeMX config) */
#define TRIG_GPIO      GPIOB
#define TRIG_PIN       GPIO_PIN_0
#define ECHO_GPIO      GPIOB
#define ECHO_PIN       GPIO_PIN_1

#define LED_G_GPIO     GPIOA
#define LED_G_PIN      GPIO_PIN_1   // Green  > 25 cm
#define LED_Y_GPIO     GPIOA
#define LED_Y_PIN      GPIO_PIN_2   // Yellow 10–25 cm
#define LED_R_GPIO     GPIOA
#define LED_R_PIN      GPIO_PIN_3   // Red    0–10 cm
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
/* none */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void DWT_Delay_Init(void);
static uint32_t micros(void);
static float HCSR04_ReadCM(void);
static void ShowRangeLEDs(float cm);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Enable DWT CYCCNT for microsecond timing */
static void DWT_Delay_Init(void)
{
  /* Enable TRC */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  /* Reset the cycle counter */
  DWT->CYCCNT = 0;
  /* Enable the cycle counter */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Current time in microseconds using DWT->CYCCNT */
static uint32_t micros(void)
{
  /* SystemCoreClock is 72 MHz => ticks per us = 72 */
  return DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

/* Trigger HC-SR04 and measure echo pulse width; returns distance in cm
   Negative values indicate timeout/error. */
static float HCSR04_ReadCM(void)
{
  /* Ensure TRIG low for at least 2 us */
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);
  uint32_t t0 = micros();
  while ((micros() - t0) < 2U) {}

  /* 10 us TRIG pulse */
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_SET);
  t0 = micros();
  while ((micros() - t0) < 10U) {}
  HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);

  /* Wait for ECHO to go high (timeout ~30 ms) */
  uint32_t timeout = micros();
  while (HAL_GPIO_ReadPin(ECHO_GPIO, ECHO_PIN) == GPIO_PIN_RESET) {
    if ((micros() - timeout) > 30000U) return -1.0f; // no echo start
  }

  /* Measure high time */
  uint32_t start = micros();
  while (HAL_GPIO_ReadPin(ECHO_GPIO, ECHO_PIN) == GPIO_PIN_SET) {
    if ((micros() - start) > 30000U) return -2.0f; // echo too long/out of range
  }
  uint32_t pulse_us = micros() - start;

  /* HC-SR04 formula: distance (cm) ˜ pulse_us / 58.0 */
  return (float)pulse_us / 58.0f;
}

/* Drive LEDs according to distance bucket */
static void ShowRangeLEDs(float cm)
{
  GPIO_PinState g = GPIO_PIN_RESET, y = GPIO_PIN_RESET, r = GPIO_PIN_RESET;

  if (cm >= 0.0f && cm <= 400.0f) {
    if (cm > 25.0f)         g = GPIO_PIN_SET;
    else if (cm >= 10.0f)   y = GPIO_PIN_SET;
    else                    r = GPIO_PIN_SET;
  }
  HAL_GPIO_WritePin(LED_G_GPIO, LED_G_PIN, g);
  HAL_GPIO_WritePin(LED_Y_GPIO, LED_Y_PIN, y);
  HAL_GPIO_WritePin(LED_R_GPIO, LED_R_PIN, r);
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
  /* Optional: free JTAG pins, keep SWD */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
  /* Initialize DWT microsecond timer (LCD driver also uses DWT) */
  DWT_Delay_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /* Initialize 16x2 LCD (rows=2). Make sure LCD I2C address in liquidcrystal_i2c.h matches your module. */
  HD44780_Init(2);
  HD44780_Clear();
  HD44780_SetCursor(0,0);
  HD44780_PrintStr("CSE331L HC-SR04");
  HAL_Delay(800);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  char line[17];
  while (1)
  {
    float cm = HCSR04_ReadCM();

    /* First row: distance */
    HD44780_SetCursor(0,0);
    if (cm < 0.0f || cm > 400.0f) {
      HD44780_PrintStr("Dist:   --- cm ");
    } else {
      /* "Dist: xxx.x cm " fits 16 chars */
      snprintf(line, sizeof(line), "Dist:%6.1f cm ", cm);
      HD44780_PrintStr(line);
    }

    /* LEDs by range + second row text */
    ShowRangeLEDs(cm);
    HD44780_SetCursor(0,1);
    if (cm < 0.0f || cm > 400.0f) {
      HD44780_PrintStr("Range: OUT     ");
    } else if (cm > 25.0f) {
      HD44780_PrintStr("LED: GREEN     ");
    } else if (cm >= 10.0f) {
      HD44780_PrintStr("LED: YELLOW    ");
    } else {
      HD44780_PrintStr("LED: RED       ");
    }

    HAL_Delay(150);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 (TRIG) */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 (ECHO) */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    /* Blink RED fast to indicate error (optional) */
    HAL_GPIO_TogglePin(LED_R_GPIO, LED_R_PIN);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
