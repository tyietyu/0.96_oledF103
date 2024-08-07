#include "flash.h"

void iap_write_flash(uint32_t address, uint8_t *data, uint32_t size)
{
    FLASH_Status flashStatus = FLASH_COMPLETE;
    uint32_t i = 0;

    // 解锁Flash编程操作
    FLASH_Unlock();
    // 开始擦除操作
    flashStatus = FLASH_ErasePage(address);
    if (flashStatus != FLASH_COMPLETE)
    {
        // 处理擦除错误
        FLASH_Lock();
        return;
    }
    // 写入数据
    for (i = 0; i < size; i += 2)
    {
        // 确保写入的数据是半字对齐的
        uint16_t halfWord = (data[i] << 8) | data[i + 1];
        flashStatus = FLASH_ProgramHalfWord(address + i, halfWord);
        if (flashStatus != FLASH_COMPLETE)
        {
            // 处理编程错误
            FLASH_Lock();
            return;
        }
    }

    // 锁定Flash编程操作
    FLASH_Lock();
}

void iap_read_flash(uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t i;
    // 逐字节读取Flash中的数据
    for (i = 0; i < size; i++)
    {
        data[i] = *(__IO uint8_t *)(address + i);
    }
}