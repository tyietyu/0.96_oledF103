#ifndef WS2812B_H_
#define WS2812B_H_

#include "main.h"

//0码和1码的定义，设置的时CCR寄存器的值
//由于使用的思PWM输出模式1，计数值<CCR时，输出有效电平-高电平（CubeMX配置默认有效电平为高电平）

#define CODE_1       (84)       //1码定时器计数次数，控制占空比为84/125 = 66%
#define CODE_0       (42)       //0码定时器计数次数，控制占空比为42/125 = 33%
#define Pixel_NUM 4  //LED数量宏定义

//单个LED的颜色控制结构体
typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB_Color_TypeDef;


static void Reset_Load(void); //该函数用于将数组最后24个数据变为0，代表RESET_code
static void RGB_SendArray(void);
static void RGB_Flush(void);  //刷新RGB显示


void RGB_SetOne_Color(uint8_t LedId, RGB_Color_TypeDef Color); //给一个LED装载24个颜色数据码（0码和1码）
//控制多个LED显示相同的颜色
void RGB_SetMore_Color(uint8_t head, uint8_t heal, RGB_Color_TypeDef color);
void RGB_Show(void); //RGB写入函数

#endif


