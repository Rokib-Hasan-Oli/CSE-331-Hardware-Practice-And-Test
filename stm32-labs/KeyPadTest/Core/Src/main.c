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
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// LED pins
#define LED_GREEN_PIN   GPIO_PIN_0   // PA0
#define LED_YELLOW_PIN  GPIO_PIN_1   // PA1
#define LED_RED_PIN     GPIO_PIN_2   // PA2

// Column pins
#define COL1_PIN  GPIO_PIN_13  // PB13
#define COL2_PIN  GPIO_PIN_14  // PB14
#define COL3_PIN  GPIO_PIN_15  // PB15
#define COL4_PIN  GPIO_PIN_8   // PA8

// Row pins (EXTI)
#define ROW1_PIN  GPIO_PIN_9   // PA9
#define ROW2_PIN  GPIO_PIN_10  // PA10
#define ROW3_PIN  GPIO_PIN_11  // PA11
#define ROW4_PIN  GPIO_PIN_12  // PA12
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile char g_key = 0;
volatile uint32_t g_lastTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
// helpers
static inline void leds_off(void);
static inline void led_green(void);
static inline void led_yellow(void);
static inline void led_red(void);
static void columns_idle_high(void);
static void columns_all_low(void);
static char scan_key_from_row(uint16_t row_pin);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Turn OFF all LEDs
static inline void leds_off(void) {
  HAL_GPIO_WritePin(GPIOA, LED_GREEN_PIN | LED_YELLOW_PIN | LED_RED_PIN, GPIO_PIN_RESET);
}
static inline void led_green(void)  { leds_off(); HAL_GPIO_WritePin(GPIOA, LED_GREEN_PIN,  GPIO_PIN_SET); }
static inline void led_yellow(void) { leds_off(); HAL_GPIO_WritePin(GPIOA, LED_YELLOW_PIN, GPIO_PIN_SET); }
static inline void led_red(void)    { leds_off(); HAL_GPIO_WritePin(GPIOA, LED_RED_PIN,    GPIO_PIN_SET); }

// Set all columns HIGH (idle)
static void columns_idle_high(void) {
  HAL_GPIO_WritePin(GPIOB, COL1_PIN | COL2_PIN | COL3_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, COL4_PIN, GPIO_PIN_SET);
}
// Set all columns LOW (for probing)
static void columns_all_low(void) {
  HAL_GPIO_WritePin(GPIOB, COL1_PIN | COL2_PIN | COL3_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, COL4_PIN, GPIO_PIN_RESET);
}

// Return key character based on which row fired EXTI and which column is active
static char scan_key_from_row(uint16_t row_pin)
{
  // Map (row, col) to characters: standard 4x4 layout
  static const char map[4][4] = {
    { '1','2','3','A' },   // R1
    { '4','5','6','B' },   // R2
    { '7','8','9','C' },   // R3
    { '*','0','#','D' }    // R4
  };

  int row = -1;
  if (row_pin == ROW1_PIN) row = 0;
  else if (row_pin == ROW2_PIN) row = 1;
  else if (row_pin == ROW3_PIN) row = 2;
  else if (row_pin == ROW4_PIN) row = 3;
  else return 0;

  char key = 0;

  // Probe C1
  columns_all_low();
  HAL_GPIO_WritePin(GPIOB, COL1_PIN, GPIO_PIN_SET);
  HAL_Delay(1);
  if (HAL_GPIO_ReadPin(GPIOA, row_pin) == GPIO_PIN_SET) key = map[row][0];

  // Probe C2
  if (!key) {
    columns_all_low();
    HAL_GPIO_WritePin(GPIOB, COL2_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(GPIOA, row_pin) == GPIO_PIN_SET) key = map[row][1];
  }

  // Probe C3
  if (!key) {
    columns_all_low();
    HAL_GPIO_WritePin(GPIOB, COL3_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(GPIOA, row_pin) == GPIO_PIN_SET) key = map[row][2];
  }

  // Probe C4
  if (!key) {
    columns_all_low();
    HAL_GPIO_WritePin(GPIOA, COL4_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(GPIOA, row_pin) == GPIO_PIN_SET) key = map[row][3];
  }

  // Restore idle
  columns_idle_high();
  return key;
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
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  // Columns idle HIGH
  columns_idle_high();

  // LEDs OFF initially
  leds_off();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    HAL_Delay(10); // work happens in EXTI callback
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                             |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure LED pins: PA0, PA1, PA2 as outputs */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure Column pins: PB13, PB14, PB15 as outputs */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Configure Row pins: PA9, PA10, PA11, PA12 as EXTI (RISING) with **PULL-DOWN** */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;            // <-- important
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */
// EXTI callback: detect which key, then drive LEDs category
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t now = HAL_GetTick();
  if (now - g_lastTick < 12) return;   // debounce ~12 ms

  // Only handle our rows
  if (GPIO_Pin!=ROW1_PIN && GPIO_Pin!=ROW2_PIN && GPIO_Pin!=ROW3_PIN && GPIO_Pin!=ROW4_PIN)
    return;

  char key = scan_key_from_row(GPIO_Pin);
  if (key) {
    g_key = key;

    // Numbers -> GREEN, A/B/C/D -> RED, */# -> YELLOW
    if (key >= '0' && key <= '9')        led_green();
    else if (key=='A'||key=='B'||key=='C'||key=='D') led_red();
    else if (key=='*' || key=='#')       led_yellow();
    // else: ignore
  }

  g_lastTick = now;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
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
#endif /* USE_FULL_ASSERT */
