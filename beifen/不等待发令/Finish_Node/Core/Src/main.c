/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Finish Node)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>       
#include "nrf24l01p.h"   
#include "oled.h"        
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

/* USER CODE BEGIN PV */
// --- ?????? ---
uint8_t RxData[32];         // ???????
uint8_t race_state = 0;     // ???: 0=????, 1=????, 2=????
uint32_t finish_cnt = 0;    // ??????????
uint16_t overflow_cnt = 0;  // ???????
float final_time = 0.0f;    // ????
char disp_buff[20];         // ???????
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void); // ??????????????

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 1. ??????? (? 65.536ms ????)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        overflow_cnt++;
    }
}

// 2. ?????? (????? - ?????)
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    // ???“????”???,????1??
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 && race_state == 1)
    {
        // A. ???????
        finish_cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        
        // B. ?????
        HAL_TIM_Base_Stop_IT(&htim2);
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
        
        // C. ???? (??: ?)
        uint64_t total_us = ((uint64_t)overflow_cnt * 65536) + finish_cnt;
        final_time = (float)total_us / 1000000.0f; 
        
        // D. ????
        race_state = 2; 
    }
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  
  // 1. ??? GPIO (??:?????? PB8/PB9 ???)
  MX_GPIO_Init();
  
  // 2. ???????
  MX_I2C1_Init(); // ?????I2C,????????
  MX_SPI1_Init();
  MX_TIM2_Init();
  
  /* USER CODE BEGIN 2 */
  
  // --- ???? ---
  
  // 1. ??? OLED (PB8/PB9)
  OLED_Init(); 
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"System Init...", 16);
  
  // 2. ??? NRF (??40)
  nrf24l01p_rx_init(40, _250kbps); 
  
  HAL_Delay(500);
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Wait for GO...", 16);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      
      // --- ??0: ???? ---
      if (race_state == 0) 
      {
          // ??????
          if(nrf24l01p_rx_receive(RxData) == 1) 
          {
              race_state = 1; // ????
              
              // ?????
              overflow_cnt = 0;
              __HAL_TIM_SET_COUNTER(&htim2, 0); 
              
              // ???????
              HAL_TIM_Base_Start_IT(&htim2);
              HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
              
              // ????
              OLED_Clear();
              OLED_ShowString(0, 0, (uint8_t*)"RUNNING!!!", 16);
          }
      }
      
      // --- ??1: ???? ---
      // (?????,????????)
      
      // --- ??2: ???? ---
      else if (race_state == 2) 
      {
          OLED_ShowString(0, 0, (uint8_t*)"FINISH!     ", 16);
          
          // ????????
          sprintf(disp_buff, "Time: %.3fs", final_time);
          
          // ???? (???? OLED ?????)
          OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16); 
          
          // ????????
          HAL_Delay(200); 
          
          // ??????????,??????????
      }
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
  __HAL_RCC_GPIOB_CLK_ENABLE(); // ?? B ???

  /*Configure GPIO pin Output Level */
  // ???? PB8 ? PB9 (I2C ????)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  
  // ????:???? (Open Drain),????OLED??
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 
  
  GPIO_InitStruct.Pull = GPIO_PULLUP; // ??
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // ??
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
  }
}