#include "WS2812B.h"
#include "tim.h"
#include "stdlib.h"

/* Static Colors */
const RGB_Color_TypeDef RED      = {255, 0, 0};
const RGB_Color_TypeDef GREEN    = {0, 255, 0};
const RGB_Color_TypeDef BLUE     = {0, 0, 255};
const RGB_Color_TypeDef SKY      = {0, 255, 255};
const RGB_Color_TypeDef MAGENTA  = {255, 0, 220};
const RGB_Color_TypeDef YELLOW   = {128, 216, 0};
const RGB_Color_TypeDef ORANGE   = {127, 106, 0};
const RGB_Color_TypeDef BLACK    = {0, 0, 0};
const RGB_Color_TypeDef WHITE    = {255, 255, 255};

RGB_Color_TypeDef color_table[16] =
{
    {254, 67, 101}, {76, 0, 10}, {249, 15, 173}, {128, 0, 32},
    {158, 46, 36}, {184, 206, 142}, {227, 23, 13}, {178, 34, 34},
    {255, 99, 71}, {99, 38, 18}, {255, 97, 0}, {21, 161, 201},
    {56, 94, 15}, {50, 205, 50}, {160, 32, 240}, {218, 60, 90}
};

/* Buffer to hold PWM values for each LED, plus one row for reset */
uint32_t Pixel_Buf[Pixel_NUM+1][24];

static void Reset_Load(void)
{
    for (uint8_t i = 0; i < 24; i++)
    {
        Pixel_Buf[Pixel_NUM][i] = 0;
    }
}

static void RGB_SendArray(void)
{
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf, (Pixel_NUM+1)*24);
}

static void RGB_Flush(void)
{
    Reset_Load();
    RGB_SendArray();
}

void RGB_SetOne_Color(uint8_t LedId, RGB_Color_TypeDef Color)
{
    if (LedId >= Pixel_NUM) return; // Prevent overflow

    for (uint8_t i = 0; i < 8; i++)
    {
        Pixel_Buf[LedId][i] = (((Color.G) & (1 << (7 - i))) ? CODE_1 : CODE_0);
    }

    for (uint8_t i = 8; i < 16; i++)
    {
        Pixel_Buf[LedId][i] = (((Color.R) & (1 << (15 - i))) ? CODE_1 : CODE_0);
    }

    for (uint8_t i = 16; i < 24; i++)
    {
        Pixel_Buf[LedId][i] = (((Color.B) & (1 << (23 - i))) ? CODE_1 : CODE_0);
    }
}

void RGB_SetMore_Color(uint8_t head, uint8_t tail, RGB_Color_TypeDef color)
{
    for (uint8_t i = head; i <= tail; i++)
    {
        RGB_SetOne_Color(i, color);
    }
}

void RGB_Show(void)
{
    RGB_SetMore_Color(0, Pixel_NUM-1, BLACK); // Clear all LEDs

    for (uint8_t i = 0; i < 8; i++)
    {
        RGB_SetMore_Color(i*8, i*8 + (rand() % 8), color_table[rand() % 16]);
    }

    RGB_Flush();
}
