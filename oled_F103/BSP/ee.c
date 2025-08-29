
/************************************************************************************************************
**************    Include Headers
************************************************************************************************************/

#include "ee.h"
#include <string.h>

EE_HandleTypeDef eeHandle;

/**
  * @brief Initializes the EEPROM emulation module.
  * @note This function initializes the EEPROM emulation module to enable read and write operations.
  * @param StoragePointer: Pointer to the start address of the EEPROM emulation area.
  * @param Size: Size of the EEPROM emulation area in bytes.
  * @return Boolean value indicating the success of the initialization:
  *       - true: Initialization successful.
  *       - false: Initialization failed.
  */
bool  EE_Init(EE_HandleTypeDef *pHandle, void *StoragePointer, uint32_t Size, uint32_t Offset)
{
  bool answer = false;
  do
  {
    if (!pHandle || !StoragePointer){
      break;
    }

    if ((Offset + Size) > USER_EE_SIZE){
      break;
    }

    pHandle->Size = Size;
    pHandle->Offset = Offset;
    pHandle->DataPointer = (uint8_t*)StoragePointer;
    answer = true;

  } while (0);

  return answer;
}

/***********************************************************************************************************/

/**
  * @brief Retrieves the capacity of the EEPROM emulation area.
  * @note This function returns the total capacity of the EEPROM emulation area in bytes.
  * @return Capacity of the EEPROM emulation area in bytes.
  */
uint32_t EE_Capacity(void)
{
  return USER_EE_SIZE;
}

/***********************************************************************************************************/

/**
  * @brief Formats the EEPROM emulation area.
  * @note This function formats the EEPROM emulation area,
  * optionally erasing its content.
  * @param EraseBuffer Indicates whether to erase the content of the EEPROM emulation area:
  *    - true: Erase the content of the EEPROM emulation area(In RAM).
  *     - false: Do not erase the content (only format Flash).
  * @return bool Boolean value indicating the success of the operation:
  *     - true: Formatting successful.
  *     - false: Formatting failed.
  */
bool EE_Format(void)
{
  bool answer = false;
  uint32_t error;
  FLASH_EraseInitTypeDef flashErase;
  do
  {
    HAL_FLASH_Unlock();
#ifdef HAL_ICACHE_MODULE_ENABLED
    HAL_ICACHE_Disable();
#endif
#if EE_ERASE == EE_ERASE_PAGE_ADDRESS
    flashErase.TypeErase = FLASH_TYPEERASE_PAGES;
    flashErase.PageAddress = EE_ADDRESS;
    flashErase.NbPages = 1;
#elif EE_ERASE == EE_ERASE_PAGE_NUMBER
    flashErase.TypeErase = FLASH_TYPEERASE_PAGES;
    flashErase.Page = EE_PAGE_SECTOR;
    flashErase.NbPages = 1;
#else
    flashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
    flashErase.Sector = EE_PAGE_SECTOR;
    flashErase.NbSectors = 1;
#endif
#ifdef EE_BANK_SELECT
    flashErase.Banks = EE_BANK_SELECT;
#endif
#ifdef FLASH_VOLTAGE_RANGE_3
    flashErase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
#endif
    if (HAL_FLASHEx_Erase(&flashErase, &error) != HAL_OK)
    {
      break;
    }
    if (error != 0xFFFFFFFF)
    {
      break;
    }
    answer = true;

  } while (0);

  HAL_FLASH_Lock();
#ifdef HAL_ICACHE_MODULE_ENABLED
  HAL_ICACHE_Enable();
#endif
  return answer;
}

/***********************************************************************************************************/

/**
  * @brief Reads data from the EEPROM emulation area.
  * @note This function reads data from the EEPROM emulation area
  *  and loads it into the specified storage pointer.
  */
void EE_Read(EE_HandleTypeDef *pHandle)
{
  if (!pHandle || !pHandle->DataPointer) return;

  uint8_t *data = pHandle->DataPointer;
  uint32_t flash_address = EE_ADDRESS + pHandle->Offset;

  for (uint32_t i = 0; i < pHandle->Size; i++)
  {
    *data = (*(__IO uint8_t*) (flash_address + i));
    data++;
  }
}

/***********************************************************************************************************/

/**
  * @brief Writes data to the EEPROM emulation area.
  * @note This function writes data to the EEPROM emulation area.
  *       Optionally, the area can be formatted first before writing.
  * @param FormatFirst: Indicates whether to format the EEPROM emulation area before writing:
  *       - true: Format the Flash area before writing.
  *       - false: Do not format the Flash area before writing.
  * @retval true if the write operation is successful, false otherwise.
  */
bool EE_Write(EE_HandleTypeDef *pHandle)
{
  if (!pHandle || !pHandle->DataPointer) return false;
  
  bool answer = true;
  uint8_t *data = pHandle->DataPointer;
  uint32_t flash_address = EE_ADDRESS + pHandle->Offset;

  do
  {
    HAL_FLASH_Unlock();
#ifdef HAL_ICACHE_MODULE_ENABLED
    HAL_ICACHE_Disable();
#endif

#if (defined FLASH_TYPEPROGRAM_HALFWORD)
    for (uint32_t i = 0; i < pHandle->Size ; i += 2)
    {
      uint64_t halfWord; 
      memcpy((uint8_t*)&halfWord, data, 2);
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash_address + i, halfWord) != HAL_OK)
      {
        answer = false;
        break;
      }
      data += 2;
    }
#elif (defined FLASH_TYPEPROGRAM_DOUBLEWORD)
    for (uint32_t i = 0; i < pHandle->Size; i += 8)
    {
      uint64_t doubleWord;
      memcpy((uint8_t*)&doubleWord, data, 8);
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flash_address + i, doubleWord) != HAL_OK)
      {
        answer = false;
        break;
      }
      data += 8;
    }
#elif (defined FLASH_TYPEPROGRAM_QUADWORD)
    for (uint32_t i = 0; i < pHandle->Size; i += 16)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, flash_address + i, (uint32_t)data) != HAL_OK)
      {
        answer = false;
        break;
      }
      data += 16;
    }
#elif (defined FLASH_TYPEPROGRAM_FLASHWORD)
    for (uint32_t i = 0; i < pHandle->Size; i += 32)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_address + i, (uint32_t)data) != HAL_OK)
      {
        answer = false;
        break;
      }
      data += 32;
    }
#endif

  } while (0);

  HAL_FLASH_Lock();
#ifdef HAL_ICACHE_MODULE_ENABLED
  HAL_ICACHE_Enable();
#endif
  return answer;
}

/***********************************************************************************************************/
