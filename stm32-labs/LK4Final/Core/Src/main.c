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

#include "liquidcrystal_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
  GPIO_InitTypeDef GPIO_InitStructPrivate = {0};

  uint32_t previousMillis = 0;

  uint32_t currentMillis = 0;

  uint8_t keyPressed = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

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
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  char snum[5];
  while (1)
  {
    /* USER CODE END WHILE */
sprintf(snum,"%d",keyPressed);

		HD44780_SetCursor(0,0);

		HD44780_PrintStr("Input ASCII");

		HD44780_SetCursor(10,1);

    HD44780_PrintStr(snum);

		HAL_Delay (3000);

		HD44780_Clear();

		HD44780_SetCursor(0,0);

		HD44780_PrintStr("Taking New Value");

		HAL_Delay (3000);

		HD44780_Clear();
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)

{

  currentMillis = HAL_GetTick();

  if (currentMillis - previousMillis > 10) {

    /*Configure GPIO pins : PB6 PB7 PB8 PB9 to GPIO_INPUT*/

    GPIO_InitStructPrivate.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;

    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;

    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;

    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStructPrivate);


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);

    if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9))

    {

      keyPressed = 68; //ASCII value of D

    }

    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10))

    {

      keyPressed = 67; //ASCII value of C

    }

    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))

    {

      keyPressed = 66; //ASCII value of B

    }

    else if(GPIO_Pin == GPIO_PIN_12 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12))

    {

      keyPressed = 65; //ASCII value of A

    }


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);

    if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9))

    {

      keyPressed = 35; //ASCII value of #

    }

    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10))

    {

      keyPressed = 57; //ASCII value of 9

    }

    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))

    {

      keyPressed = 54; //ASCII value of 6

    }

    else if(GPIO_Pin == GPIO_PIN_12 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12))

    {

      keyPressed = 51; //ASCII value of 3

    }


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);

    if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9))

    {

      keyPressed = 48; //ASCII value of 0

    }

    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10))

    {

      keyPressed = 56; //ASCII value of 8

    }

    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))

    {

      keyPressed = 53; //ASCII value of 5

    }

    else if(GPIO_Pin == GPIO_PIN_12 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12))

    {

      keyPressed = 50; //ASCII value of 2

    }


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);

    if(GPIO_Pin == GPIO_PIN_9 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9))

    {

      keyPressed = 42; //ASCII value of *

    }

    else if(GPIO_Pin == GPIO_PIN_10 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10))

    {

      keyPressed = 55; //ASCII value of 7

    }

    else if(GPIO_Pin == GPIO_PIN_11 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))

    {

      keyPressed = 52; //ASCII value of 4

    }

    else if(GPIO_Pin == GPIO_PIN_12 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12))

    {

      keyPressed = 49; //ASCII value of 1

    }


    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);

    /*Configure GPIO pins : PB6 PB7 PB8 PB9 back to EXTI*/

    GPIO_InitStructPrivate.Mode = GPIO_MODE_IT_RISING;

    GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStructPrivate);

    previousMillis = currentMillis;

  }

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
