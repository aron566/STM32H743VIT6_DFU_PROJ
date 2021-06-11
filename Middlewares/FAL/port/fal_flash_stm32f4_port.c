/**
 *  @file fal_flash_stm32h7_port.c
 *
 *  @date 2021-02-20
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief FLASH FAL操作接口 User main memory
 *
 *  @details STM32F407VET6:BANK1 RANGE:0x0800 0000- 0x080C FFFF TOTAL_SIZE:128KB*11
 *
 *  @version V1.0
 */
#include <fal.h>
#include <string.h>
#include <stm32F4xx.h>

/* base address of the flash sectors */
#define BANK_1_ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base address of Sector 0, 16 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_1      ((uint32_t)0x08004000) /* Base address of Sector 1, 16 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_2      ((uint32_t)0x08008000) /* Base address of Sector 2, 16 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_3      ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_4      ((uint32_t)0x08010000) /* Base address of Sector 4, 64 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_5      ((uint32_t)0x08020000) /* Base address of Sector 5, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_6      ((uint32_t)0x08040000) /* Base address of Sector 6, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_7      ((uint32_t)0x08060000) /* Base address of Sector 7, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_8      ((uint32_t)0x08080000) /* Base address of Sector 8, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_9      ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_10      ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_11      ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes*/

#define MAX_PAGE_SIZE      ((uint32_t)0x08100000) /* MAX*/

#ifndef FLASH_PAGE_SIZE
  #define FLASH_PAGE_SIZE (128*1024U)
#endif

#define FLASH_PAGE_0_ADDR               BANK_1_ADDR_FLASH_SECTOR_0	/* 物理flash原始起始页地址 */
#define FLASH_PAGE_NUM_MAX		          FLASH_SECTOR_TOTAL


/**
 * Get the sector of page num
 *
 * @param address flash address
 *
 * @return num
 */
static uint32_t stm32_get_sector_num(uint32_t Startaddress)
{
    uint32_t Startpage = 0;
    
	/*计算起始地址所在页*/
    if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_0 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_1)
    {
      Startpage = 0;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_1 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_2)
    {
      Startpage = 1;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_2 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_3)
    {
      Startpage = 2;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_3 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_4)
    {
      Startpage = 3;
    }    
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_4 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_5)
    {
      Startpage = 4;
    }    
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_5 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_6)
    {
      Startpage = 5;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_6 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_7)
    {
      Startpage = 6;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_7 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_8)
    {
      Startpage = 7;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_8 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_9)
    {
      Startpage = 8;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_9 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_10)
    {
      Startpage = 9;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_10 && Startaddress < BANK_1_ADDR_FLASH_SECTOR_11)
    {
      Startpage = 10;
    }
    else if(Startaddress >= BANK_1_ADDR_FLASH_SECTOR_11 && Startaddress < MAX_PAGE_SIZE)
    {
      Startpage = 11;
    }
    return Startpage;
}

/**
 * Get the sector of a given address
 *
 * @param address flash address
 *
 * @return The sector of a given address
 */
static uint32_t stm32_get_sector(uint32_t Startaddress ,size_t size ,uint32_t *get_pages)
{
		/*记录起始页与结束页*/
    uint32_t Startpage = 0,Endpage = 0;
    uint32_t Endaddress = Startaddress + size-1;
		/*计算起始地址所在页*/
    Startpage = stm32_get_sector_num(Startaddress);
    Endpage = stm32_get_sector_num(Endaddress);
		/*计算地址范围内的页数*/
		*get_pages = Endpage-Startpage+1;
    /*get sector num*/
    return Startpage;
    /*get addr*/
//    return Startpage*FLASH_PAGE_SIZE+FLASH_PAGE_0_ADDR;
}

/**
 * Get the sector size
 *
 * @param sector sector
 *
 * @return sector size
 */
//static uint32_t stm32_get_sector_size(uint32_t pages) 
//{
//	return pages*FLASH_PAGE_SIZE;
//}

/**
 * Get the bankx
 *
 * @param addr
 *
 * @return bank num
 */
static uint8_t stm32_get_addr_bank(uint32_t addr)
{
  if(addr >= FLASH_PAGE_0_ADDR && addr < MAX_PAGE_SIZE)
  {
    return FLASH_BANK_1;
  }
  else if(addr >= MAX_PAGE_SIZE)
  {
    return FLASH_BANK_1;
  }
  return FLASH_BANK_1;
}

static int init(void)
{
    /* do nothing now */
		return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = stm32f4_onchip_flash.addr + offset;
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size, bool block_mode)
{
  uint32_t addr = stm32f4_onchip_flash.addr + offset;

  HAL_FLASH_Unlock();
  
  for(int i = 0; i < size; i++)
  {
    /* write data */
    if(block_mode == true)
    {
      if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr+i, buf[i]))
      {
        return -1;
      }
    }
    else
    {
      if(HAL_OK != HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_BYTE, addr+i, buf[i]))
      {
        return -1;
      }
    }
  }

  HAL_FLASH_Lock();
  return size;
}

static int erase(long offset, size_t size, bool block_mode)
{
    HAL_StatusTypeDef flash_status;
    
    uint32_t addr = stm32f4_onchip_flash.addr + offset;
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t erasepages = 0;
    uint32_t error;
    /* start erase */
    HAL_FLASH_Unlock();

    /* it will stop when erased size is greater than setting size */
		
    /*擦除页地址*/
    erase_config.Sector = stm32_get_sector(addr ,size ,&erasepages);
    
    /*所属BANK*/
    erase_config.Banks = stm32_get_addr_bank(addr);
    /*按页擦除*/
    erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
    /*擦除页数量*/
    erase_config.NbSectors = erasepages;
    /*擦除使用电压范围*/
    erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    log_i("erase page:0x%02X,PAGES:%u,BANK:%u", erase_config.Sector, erasepages, erase_config.Banks);
    if(block_mode == true)
    {
      flash_status = HAL_FLASHEx_Erase(&erase_config, &error);
    }
    else
    {
      flash_status = HAL_FLASHEx_Erase_IT(&erase_config);
    }
    if (flash_status != HAL_OK)
    {
      log_i("erase error");
      return -1;
    }
		
    HAL_FLASH_Lock();

    return size;
}

const struct fal_flash_dev stm32f4_onchip_flash =
{
    .name       = "stm32_onchip",
    .addr       = FLASH_PAGE_0_ADDR,
    .len        = FLASH_PAGE_SIZE*8,
    .blk_size   = FLASH_PAGE_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 8
};
