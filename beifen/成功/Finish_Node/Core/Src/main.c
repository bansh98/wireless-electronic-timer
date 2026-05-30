/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Finish Node - 激光浮空输入版)
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        overflow_cnt++;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    // *** 改用通道2（PA1），因为PA0可能坏了 ***
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 && race_state == 1)
    {
        // A. 锁存当前计数值
        finish_cnt = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        
        // B. 停止定时器
        HAL_TIM_Base_Stop_IT(&htim2);
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
        
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
  
  // 1. 初始化 OLED (使用你提供的4参数版本)
  OLED_Init(); 
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Finish Node", 16);
  OLED_ShowString(0, 2, (uint8_t*)"Initializing", 16);
  
  // 2. 初始化 NRF
  nrf24l01p_rx_init(40, _250kbps); 
  
  HAL_Delay(500);
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Wait for GO...", 16);
  
  // *** 改用PA1（因为PA0可能坏了）***
  // *** 强制重新配置PA1为GPIO输入+上拉（与测试代码一致）***
  GPIO_InitTypeDef GPIO_InitStruct_PA1 = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_PA1.Pin = GPIO_PIN_1; // *** 改用PA1 ***
  GPIO_InitStruct_PA1.Mode = GPIO_MODE_INPUT; 
  GPIO_InitStruct_PA1.Pull = GPIO_PULLUP; // 上拉，与测试代码一致
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_PA1);

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
          // *** 改用PA1，在等待时显示PA1状态，方便调试 ***
          static uint32_t last_check = 0;
          if (HAL_GetTick() - last_check > 300) 
          {
              last_check = HAL_GetTick();
              GPIO_PinState pa1_test = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1); // *** 改用PA1 ***
              sprintf(disp_buff, "PA1=%s", pa1_test == GPIO_PIN_SET ? "HI" : "LO");
              OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16);
          }
          
          // 检测无线信号
          if(nrf24l01p_rx_receive(RxData) == 1) 
          {
              // *** 校验GO信号，防止杂波误触发 ***
              if (RxData[0] == 'G' && RxData[1] == 'O') 
              {
                  race_state = 1; // 开始计时
                  
                  // 1. 重置定时器
                  overflow_cnt = 0;
                  __HAL_TIM_SET_COUNTER(&htim2, 0); 
                  
                  // 2. 清除之前的捕获标志位（防止启动时立即触发）
                  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC2); // *** 改用通道2 ***
                  
                  // 3. 开启定时器中断（触发极性已在tim.c中配置为FALLING）
                  HAL_TIM_Base_Start_IT(&htim2);
                  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // *** 改用通道2 ***
                  
                  // 4. 屏幕提示
                  OLED_Clear();
                  OLED_ShowString(0, 0, (uint8_t*)"RUNNING!!!", 16);
              }
          }
      }
      
      // --- 状态1: 正在计时 ---
      else if (race_state == 1) 
      {
          // *** 改用PA1，实时显示PA1状态，帮助调试 ***
          // *** 根据测试：有激光=HIGH, 遮挡=LOW ***
          static uint32_t last_display = 0;
          if (HAL_GetTick() - last_display > 150) 
          {
              last_display = HAL_GetTick();
              
              GPIO_PinState pa1_level = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1); // *** 改用PA1 ***
              
              sprintf(disp_buff, "PA1=%s OV=%d", 
                      pa1_level == GPIO_PIN_SET ? "HI" : "LO", 
                      overflow_cnt);
              OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16);
          }
      }
      
      // --- 状态2: 比赛结束 ---
      else if (race_state == 2) 
      {
          OLED_ShowString(0, 0, (uint8_t*)"FINISH!      ", 16);
          
          sprintf(disp_buff, "Time: %.3fs", final_time);
          OLED_ShowString(0, 2, (uint8_t*)disp_buff, 16); 
          
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

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}