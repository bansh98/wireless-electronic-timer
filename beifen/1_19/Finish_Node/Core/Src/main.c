/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Finish Node - 简化版)
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
// --- 全局变量定义 ---
uint8_t RxData[32];         // 无线接收缓冲区
uint8_t race_state = 0;     // 状态机: 0=等待发令, 1=正在计时, 2=比赛结束
uint32_t finish_cnt = 0;    // 撞线瞬间的定时器数值
uint16_t overflow_cnt = 0;  // 定时器溢出次数
float final_time = 0.0f;    // 最终时间
char disp_buff[20];         // 显示缓存字符串
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 1. 定时器溢出中断 (每 65.536ms 触发一次)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        overflow_cnt++;
    }
}

// 2. 输入捕获中断 (激光被遮挡触发)
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    // 只有在"正在计时"状态下，且是通道1触发
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 && race_state == 1)
    {
        // A. 锁存当前计数值
        finish_cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        
        // B. 停止定时器
        HAL_TIM_Base_Stop_IT(&htim2);
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
        
        // C. 计算时间 (单位: 秒)
        uint64_t total_us = ((uint64_t)overflow_cnt * 65536) + finish_cnt;
        final_time = (float)total_us / 1000000.0f; 
        
        // D. 切换状态
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
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  
  /* USER CODE BEGIN 2 */
  
  // 1. 初始化 OLED
  OLED_Init(); 
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Finish Node", 16);
  OLED_ShowString(0, 2, (uint8_t*)"Initializing", 16);
  
  // 2. 初始化 NRF
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
      
      // --- 状态0: 等待发令 ---
      if (race_state == 0) 
      {
          // 检测无线信号
          if(nrf24l01p_rx_receive(RxData) == 1) 
          {
              race_state = 1; // 开始计时
              
              // 重置定时器
              overflow_cnt = 0;
              __HAL_TIM_SET_COUNTER(&htim2, 0); 
              
              // 开启定时器中断
              HAL_TIM_Base_Start_IT(&htim2);
              HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
              
              // 屏幕提示
              OLED_Clear();
              OLED_ShowString(0, 0, (uint8_t*)"RUNNING!!!", 16);
          }
      }
      
      // --- 状态1: 正在计时 ---
      else if (race_state == 1) 
      {
          // *** 调试: 实时显示PA0电平 ***
          static uint32_t last_display = 0;
          if (HAL_GetTick() - last_display > 100) // 每100ms更新一次
          {
              last_display = HAL_GetTick();
              
              // 读取PA0的电平状态
              GPIO_PinState pa0_raw = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
              
              sprintf(disp_buff, "PA0=%s", pa0_raw == GPIO_PIN_SET ? "HIGH" : "LOW");
              OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16);
              
              sprintf(disp_buff, "CNT=%d", (int)__HAL_TIM_GET_COUNTER(&htim2));
              OLED_ShowString(0, 4, (uint8_t*)disp_buff, 16);
          }
      }
      
      // --- 状态2: 比赛结束 ---
      else if (race_state == 2) 
      {
          OLED_ShowString(0, 0, (uint8_t*)"FINISH!     ", 16);
          
          // 格式化时间字符串
          sprintf(disp_buff, "Time: %.3fs", final_time);
          
          // 显示时间
          OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16); 
          
          // 延时防止刷新太快
          HAL_Delay(200); 
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
