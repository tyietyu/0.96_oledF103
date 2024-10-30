/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "mpu6050.h"
#include "WS2812B.h"
#include "esp8266.h"
#include "IAP.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint32_t BootStaFlag;  
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
char oled_buff[20] = {0};
uint8_t Rxbuff = 0;

extern uint8_t index_send_msg;
extern uint16_t index_led;
uint8_t led_status = 0;
uint8_t led_vol = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
MPU6050_t MPU6050;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void OLED_demo(void);
void ESP8266_demo(void);
void espTopic_pub_sub_demo(void);
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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
    OLED_Init();
    ESP8266_init();
    ESP8266_demo();
    time2_start();
    // while (MPU6050_Init(&hi2c2) == 1) {};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        espTopic_pub_sub_demo();
        HAL_Delay(100);
        //    RGB_Show();
        //    MPU6050_Read_All(&hi2c2, &MPU6050);
        //    sprintf(oled_buff, "%.2f", MPU6050.Temperature);
        //    OLED_Printf(1, 1, OLED_8X16, oled_buff);
        //    sprintf(&oled_buff[10], "%.2f", MPU6050.Ax);
        //    OLED_Printf(1, 20, OLED_8X16, &oled_buff[10]);
        //    OLED_Update();
        //    HAL_Delay(100);
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

/* USER CODE BEGIN 4 */
void ESP8266_demo(void)
{
    printf("1.RESET ESP8266\r\n");
    while (ESP8266_sw_reset() != 0)
    {
        HAL_Delay(5000);
    }

    printf("2.SETTING STATION MODE\r\n");
    ESP8266_set_mode(1);
    HAL_Delay(1000);

    printf("3.CLOSE ESP8266 ECHO\r\n");
    ESP8266_ate_config(0);
    HAL_Delay(1000);

    printf("4.NO AUTO CONNECT WIFI\r\n");
    while (ESP8266_send_at_cmd((uint8_t *)"AT+CWAUTOCONN=0\r\n", strlen("AT+CWAUTOCONN=0\r\n"), "OK") != 0)
    {
        HAL_Delay(1000);
    }

    printf("5.CONNECT WIFI NETWORK\r\n");
    while (ESP8266_join_wifi() != 0)
    {
        HAL_Delay(8000);
    }

    printf("6.MQTT USER CONFIG\r\n");
    while (ESP8266_config_mqtt() != 0)
    {
        HAL_Delay(8000);
    }

    printf("7.Connect AliyunCloude\r\n");
    while (ESP8266_connect_Aliyun() != 0)
    {
        HAL_Delay(8000);
    }

    printf("8.CONNECT MQTT BROKER\r\n");
    while (ESP8266_connect_tcp_server() != 0)
    {
        HAL_Delay(8000);
    }
    printf(" 9. ESP8266 INIT OK!!!\r\n");
}

void espTopic_pub_sub_demo(void)
{
    led_status=0;
    led_vol=5;
		while(esp8266_send_msg()!=0)
		{
			HAL_Delay(5000);
		}
		HAL_Delay(1000);
		while(ESP8266_Sub_Topic_Aliyun()!=0)
		{
			HAL_Delay(5000);
		}
		while(esp8266_receive_msg()!=0)
		{
			HAL_Delay(5000);
		}
}

void OLED_demo(void)
{
    /*在(16, 0)位置显示字符串"Hello World!"，字体大小为8*16点阵*/
    OLED_ShowString(0, 0, "blog.zeruns.tech", OLED_8X16);

    /*在(0, 18)位置显示字符'A'，字体大小为6*8点阵*/
    OLED_ShowChar(0, 18, 'A', OLED_6X8);

    /*在(16, 18)位置显示字符串"Hello World!"，字体大小为6*8点阵*/
    OLED_ShowString(16, 18, "Hello World!", OLED_6X8);

    /*在(0, 28)位置显示数字12345，长度为5，字体大小为6*8点阵*/
    OLED_ShowNum(0, 28, 12345, 5, OLED_6X8);

    /*在(40, 28)位置显示有符号数字-66，长度为2，字体大小为6*8点阵*/
    OLED_ShowSignedNum(40, 28, -66, 2, OLED_6X8);

    /*在(70, 28)位置显示十六进制数字0xA5A5，长度为4，字体大小为6*8点阵*/
    OLED_ShowHexNum(70, 28, 0xA5A5, 4, OLED_6X8);

    /*在(0, 38)位置显示二进制数字0xA5，长度为8，字体大小为6*8点阵*/
    OLED_ShowBinNum(0, 38, 0xA5, 8, OLED_6X8);

    /*在(60, 38)位置显示浮点数字123.45，整数部分长度为3，小数部分长度为2，字体大小为6*8点阵*/
    OLED_ShowFloatNum(60, 38, 123.45, 3, 2, OLED_6X8);

    /*在(0, 48)位置显示汉字串"你好，世界。"，字体大小为固定的16*16点阵*/
    OLED_ShowChinese(0, 48, "你好，世界。");

    /*在(96, 48)位置显示图像，宽16像素，高16像素，图像数据为Diode数组*/
    OLED_ShowImage(96, 48, 16, 16, Diode);

    /*在(96, 18)位置打印格式化字符串，字体大小为6*8点阵，格式化字符串为"[%02d]"*/
    OLED_Printf(96, 18, OLED_6X8, "[%02d]", 6);

    /*调用OLED_Update函数，将OLED显存数组的内容更新到OLED硬件进行显示*/
    OLED_Update();

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // LED电平翻转
    /*延时3000ms，观察现象*/
    HAL_Delay(3000);

    /*清空OLED显存数组*/
    OLED_Clear();

    /*在(5, 8)位置画点*/
    OLED_DrawPoint(5, 8);

    /*获取(5, 8)位置的点*/
    if (OLED_GetPoint(5, 8))
    {
        /*如果指定点点亮，则在(10, 4)位置显示字符串"YES"，字体大小为6*8点阵*/
        OLED_ShowString(10, 4, "YES", OLED_6X8);
    }
    else
    {
        /*如果指定点未点亮，则在(10, 4)位置显示字符串"NO "，字体大小为6*8点阵*/
        OLED_ShowString(10, 4, "NO ", OLED_6X8);
    }

    /*在(40, 0)和(127, 15)位置之间画直线*/
    OLED_DrawLine(40, 0, 127, 15);

    /*在(40, 15)和(127, 0)位置之间画直线*/
    OLED_DrawLine(40, 15, 127, 0);

    /*在(0, 20)位置画矩形，宽12像素，高15像素，未填充*/
    OLED_DrawRectangle(0, 20, 12, 15, OLED_UNFILLED);

    /*在(0, 40)位置画矩形，宽12像素，高15像素，填充*/
    OLED_DrawRectangle(0, 40, 12, 15, OLED_FILLED);

    /*在(20, 20)、(40, 25)和(30, 35)位置之间画三角形，未填充*/
    OLED_DrawTriangle(20, 20, 40, 25, 30, 35, OLED_UNFILLED);

    /*在(20, 40)、(40, 45)和(30, 55)位置之间画三角形，填充*/
    OLED_DrawTriangle(20, 40, 40, 45, 30, 55, OLED_FILLED);

    /*在(55, 27)位置画圆，半径8像素，未填充*/
    OLED_DrawCircle(55, 27, 8, OLED_UNFILLED);

    /*在(55, 47)位置画圆，半径8像素，填充*/
    OLED_DrawCircle(55, 47, 8, OLED_FILLED);

    /*在(82, 27)位置画椭圆，横向半轴12像素，纵向半轴8像素，未填充*/
    OLED_DrawEllipse(82, 27, 12, 8, OLED_UNFILLED);
    // https://blog.zeruns.tech
    /*在(82, 47)位置画椭圆，横向半轴12像素，纵向半轴8像素，填充*/
    OLED_DrawEllipse(82, 47, 12, 8, OLED_FILLED);

    /*在(110, 18)位置画圆弧，半径15像素，起始角度25度，终止角度125度，未填充*/
    OLED_DrawArc(110, 18, 15, 25, 125, OLED_UNFILLED);

    /*在(110, 38)位置画圆弧，半径15像素，起始角度25度，终止角度125度，填充*/
    OLED_DrawArc(110, 38, 15, 25, 125, OLED_FILLED);

    /*调用OLED_Update函数，将OLED显存数组的内容更新到OLED硬件进行显示*/
    OLED_Update();

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // LED电平翻转

    /*延时1500ms，观察现象*/
    HAL_Delay(1500);
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
