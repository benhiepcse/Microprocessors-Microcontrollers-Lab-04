/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler.h"
#include "tasks.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEMO_MODE_A 1
#define DEMO_MODE_B 2
#define DEMO_MODE_C 3
#define DEMO_MODE_5_LED 4

#define DEMO_MODE DEMO_MODE_5_LED
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if DEMO_MODE == DEMO_MODE_A

static void DemoA_LogTick(void)
{
    Tasks_LogTimestamp("A");
}

#elif DEMO_MODE == DEMO_MODE_B

static void DemoB_Log300(void)
{
    Tasks_LogTimestamp("B300");
}

static void DemoB_Log500(void)
{
    Tasks_LogTimestamp("B500");
}

#elif DEMO_MODE == DEMO_MODE_C

static void DemoC_Log1000(void)
{
    Tasks_LogTimestamp("C1");
}

static void DemoC_Log2000(void)
{
    Tasks_LogTimestamp("C2");
}

static void DemoC_Log3000(void)
{
    Tasks_LogTimestamp("C3");
}

#elif DEMO_MODE == DEMO_MODE_5_LED

static void Led_Yellow(void)
{
    HAL_GPIO_TogglePin(YELLOW_GPIO_Port, YELLOW_Pin);
    Tasks_LogTimestamp("T2");
}

static void Led_Pink(void)
{
    HAL_GPIO_TogglePin(PINK_GPIO_Port, PINK_Pin);
    Tasks_LogTimestamp("T3");
}

static void Led_Green(void)
{
    HAL_GPIO_TogglePin(GREEN_GPIO_Port, GREEN_Pin);
    Tasks_LogTimestamp("T4");
}

static void Led_Blue(void)
{
    HAL_GPIO_TogglePin(BLUE_GPIO_Port, BLUE_Pin);
    Tasks_LogTimestamp("T5");
}

#else
#error "Unsupported DEMO_MODE"
#endif
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    SCH_Init();

  #if DEMO_MODE == DEMO_MODE_A

    if (SCH_Add_Task(DemoA_LogTick, 1U, 1U) == NO_TASK_ID)
    {
        Error_Handler();
    }

  #elif DEMO_MODE == DEMO_MODE_B

    if (SCH_Add_Task(DemoB_Log300, MS_TO_TICK(300U),
                     MS_TO_TICK(300U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(DemoB_Log500, MS_TO_TICK(500U),
                     MS_TO_TICK(500U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

  #elif DEMO_MODE == DEMO_MODE_C

    if (SCH_Add_Task(DemoC_Log1000, 100U, 0U) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(DemoC_Log2000, 200U, 0U) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(DemoC_Log3000, 300U, 0U) == NO_TASK_ID)
    {
        Error_Handler();
    }

  #elif DEMO_MODE == DEMO_MODE_5_LED

    if (SCH_Add_Task(Tasks_RedLed, MS_TO_TICK(500U),
                     MS_TO_TICK(500U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(Led_Yellow, MS_TO_TICK(1000U),
                     MS_TO_TICK(1000U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(Led_Pink, MS_TO_TICK(1500U),
                     MS_TO_TICK(1500U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(Led_Green, MS_TO_TICK(2000U),
                     MS_TO_TICK(2000U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

    if (SCH_Add_Task(Led_Blue, MS_TO_TICK(2500U),
                     MS_TO_TICK(2500U)) == NO_TASK_ID)
    {
        Error_Handler();
    }

  #endif

    __HAL_TIM_SET_COUNTER(&htim2, 0U);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);

    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      SCH_Dispatch_Tasks();
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

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        SCH_Update();
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
