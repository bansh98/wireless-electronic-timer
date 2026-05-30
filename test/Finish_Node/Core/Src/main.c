#include "main.h"
#include "gpio.h"

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  
  // --- 1. 初始化 PC13 (板载LED) ---
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct_LED = {0};
  GPIO_InitStruct_LED.Pin = GPIO_PIN_13;
  GPIO_InitStruct_LED.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
  GPIO_InitStruct_LED.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_LED);

  // --- 2. 初始化 PA0 (激光传感器接口) ---
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct_Sensor = {0};
  GPIO_InitStruct_Sensor.Pin = GPIO_PIN_0;
  GPIO_InitStruct_Sensor.Mode = GPIO_MODE_INPUT; 
  
  // 【关键设置】这里开启上拉(Pull-Up)
  // 即使你的模块是开漏输出，上拉也能保证它正常工作
  GPIO_InitStruct_Sensor.Pull = GPIO_PULLUP;      
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Sensor);

  while (1)
  {
      // 读取 PA0 的电平
      GPIO_PinState sensorState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
      
      // PC13 是低电平点亮 (Low = ON)
      if (sensorState == GPIO_PIN_SET) 
      {
          // 如果 PA0 是高电平 (3.3V) -> 关灯
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); 
      }
      else 
      {
          // 如果 PA0 是低电平 (0V) -> 开灯 (亮)
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); 
      }
      
      // 稍微延时一点点去抖动
      HAL_Delay(10);
  }
}

// --- 下面是系统配置，保持默认即可 ---
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
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void Error_Handler(void) { __disable_irq(); while (1) {} }