/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (STM32F103C8T6 Blue Pill)
  ******************************************************************************
  * Notes:
  *  - 4x4 Keypad: Rows -> PB13, PB14, PB15, PA8 (outputs)
  *                 Cols  -> PA9,  PA10, PA11, PA12 (EXTI)
  *  - LCD: HD44780 over I2C1 via PCF8574 backpack (set correct I2C address in
  *          liquidcrystal_i2c.h, e.g. 0x27 or 0x3F, *left-shifted by 1*).
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
// Debounce interval in ms
#define KEYPAD_DEBOUNCE_MS   10

// Convenience macros for row pins
#define ROW1_PORT GPIOB
#define ROW1_PIN  GPIO_PIN_13
#define ROW2_PORT GPIOB
#define ROW2_PIN  GPIO_PIN_14
#define ROW3_PORT GPIOB
#define ROW3_PIN  GPIO_PIN_15
#define ROW4_PORT GPIOA
#define ROW4_PIN  GPIO_PIN_8

// Column pins
#define COL_PORT  GPIOA
#define COL1_PIN  GPIO_PIN_9
#define COL2_PIN  GPIO_PIN_10
#define COL3_PIN  GPIO_PIN_11
#define COL4_PIN  GPIO_PIN_12
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SET_ROW(r1,r2,r3,r4)                 \
  do {                                       \
    HAL_GPIO_WritePin(ROW1_PORT, ROW1_PIN, (r1)?GPIO_PIN_SET:GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(ROW2_PORT, ROW2_PIN, (r2)?GPIO_PIN_SET:GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(ROW3_PORT, ROW3_PIN, (r3)?GPIO_PIN_SET:GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(ROW4_PORT, ROW4_PIN, (r4)?GPIO_PIN_SET:GPIO_PIN_RESET); \
  } while(0)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
GPIO_InitTypeDef GPIO_InitStructPrivate = {0};

volatile uint32_t previousMillis = 0;
volatile uint32_t currentMillis  = 0;
volatile uint8_t  keyPressed     = 0;   // ASCII of last key, 0 if none
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void LCD_ShowKey(uint8_t ascii);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // Initialize LCD (set correct address in liquidcrystal_i2c.h)
  // Many libs take rows; if your lib expects (cols, rows), change to (16,2).
  HD44780_Init(2);
  HD44780_Clear();
  HD44780_SetCursor(0,0);
  HD44780_PrintStr("Ready for input");
  HD44780_SetCursor(0,1);
  HD44780_PrintStr("Press a key...");

  // Idle state: drive all rows high (no key pressed sensed on pulled-down cols)
  SET_ROW(1,1,1,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Just refresh display with the last pressed key
    LCD_ShowKey(keyPressed);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA9 PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * Keypad scan in EXTI callback
 * Debounce using HAL_GetTick(). Do NOT use HAL_Delay() in interrupts.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  (void)GPIO_Pin; // we will read all columns explicitly

  currentMillis = HAL_GetTick();
  if ((currentMillis - previousMillis) < KEYPAD_DEBOUNCE_MS) {
    return;
  }

  // Make columns plain inputs (keep pulldown so idle stays low)
  GPIO_InitStructPrivate.Pin  = COL1_PIN | COL2_PIN | COL3_PIN | COL4_PIN;
  GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(COL_PORT, &GPIO_InitStructPrivate);

  // Helper to settle signals a bit
  #define SETTLE() do { for (volatile int i=0;i<300;i++) __NOP(); } while (0)

  // Scan Row 1: set R1=1, others=0
  uint8_t found = 0;
  SET_ROW(1,0,0,0); SETTLE();
  if (HAL_GPIO_ReadPin(COL_PORT, COL1_PIN)) { keyPressed = 'D'; found=1; }
  else if (HAL_GPIO_ReadPin(COL_PORT, COL2_PIN)) { keyPressed = 'C'; found=1; }
  else if (HAL_GPIO_ReadPin(COL_PORT, COL3_PIN)) { keyPressed = 'B'; found=1; }
  else if (HAL_GPIO_ReadPin(COL_PORT, COL4_PIN)) { keyPressed = 'A'; found=1; }

  // Row 2
  if (!found) {
    SET_ROW(0,1,0,0); SETTLE();
    if (HAL_GPIO_ReadPin(COL_PORT, COL1_PIN)) { keyPressed = '#'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL2_PIN)) { keyPressed = '9'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL3_PIN)) { keyPressed = '6'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL4_PIN)) { keyPressed = '3'; found=1; }
  }

  // Row 3
  if (!found) {
    SET_ROW(0,0,1,0); SETTLE();
    if (HAL_GPIO_ReadPin(COL_PORT, COL1_PIN)) { keyPressed = '0'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL2_PIN)) { keyPressed = '8'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL3_PIN)) { keyPressed = '5'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL4_PIN)) { keyPressed = '2'; found=1; }
  }

  // Row 4
  if (!found) {
    SET_ROW(0,0,0,1); SETTLE();
    if (HAL_GPIO_ReadPin(COL_PORT, COL1_PIN)) { keyPressed = '*'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL2_PIN)) { keyPressed = '7'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL3_PIN)) { keyPressed = '4'; found=1; }
    else if (HAL_GPIO_ReadPin(COL_PORT, COL4_PIN)) { keyPressed = '1'; found=1; }
  }

  // Restore idle state: all rows high
  SET_ROW(1,1,1,1);

  // Restore EXTI mode with pulldowns
  GPIO_InitStructPrivate.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(COL_PORT, &GPIO_InitStructPrivate);

  previousMillis = currentMillis;
}
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
