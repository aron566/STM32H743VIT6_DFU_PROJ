/**
 *  @file fal_flash_stm32h7_port.c
 *
 *  @date 2021-01-31
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief FLASH FAL操作接口 User main memory
 *
 *  @details BANK1 RANGE:0x0800 0000- 0x080F FFFF TOTAL_SIZE:128KB*8
 *           BANK2 RANGE:0x0810 0000- 0x081F FFFF TOTAL_SIZE:128KB*8
 *           write granularities : 8-256bit
 *			 STM32H743VIT6	
 *  @version V1.0
 */
#include <fal.h>
#include <string.h>
#include <stm32h7xx.h>

/* base address of the flash sectors */
#define BANK_1_ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base address of Sector 0, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_1      ((uint32_t)0x08020000) /* Base address of Sector 1, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_2      ((uint32_t)0x08040000) /* Base address of Sector 2, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_3      ((uint32_t)0x08060000) /* Base address of Sector 3, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_4      ((uint32_t)0x08080000) /* Base address of Sector 4, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_5      ((uint32_t)0x080A0000) /* Base address of Sector 5, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_6      ((uint32_t)0x080C0000) /* Base address of Sector 6, 128 K bytes*/
#define BANK_1_ADDR_FLASH_SECTOR_7      ((uint32_t)0x080E0000) /* Base address of Sector 7, 128 K bytes*/

#define BANK_2_ADDR_FLASH_SECTOR_0      ((uint32_t)0x08100000) /* Base address of Sector 0, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_1      ((uint32_t)0x08120000) /* Base address of Sector 1, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_2      ((uint32_t)0x08140000) /* Base address of Sector 2, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_3      ((uint32_t)0x08160000) /* Base address of Sector 3, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_4      ((uint32_t)0x08180000) /* Base address of Sector 4, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_5      ((uint32_t)0x081A0000) /* Base address of Sector 5, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_6      ((uint32_t)0x081C0000) /* Base address of Sector 6, 128 K bytes*/
#define BANK_2_ADDR_FLASH_SECTOR_7      ((uint32_t)0x081E0000) /* Base address of Sector 7, 128 K bytes*/

#ifndef FLASH_PAGE_SIZE
  #define FLASH_PAGE_SIZE FLASH_SECTOR_SIZE
#endif

#define FLASH_PAGE_0_ADDR               BANK_1_ADDR_FLASH_SECTOR_0	/* 物理flash原始起始页地址 */
#define FLASH_START_PAGE_ADDR	          BANK_1_ADDR_FLASH_SECTOR_1	/* 分区表起始地址 */
#define CURRENT_PART_START_PAGE	        ((FLASH_START_PAGE_ADDR-FLASH_PAGE_0_ADDR)/FLASH_PAGE_SIZE)	/* 当前分区实际物理起始页 */
#define FLASH_PAGE_NUM_MAX		          FLASH_SECTOR_TOTAL*2
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
		/*搜索完成标记*/
		uint8_t start_page_flag = 0,end_page_flag = 0;
		/*计算起始地址所在页*/
		if(Startaddress >= stm32h7_onchip_flash.addr)
		{
				for(uint32_t i = 0; i < FLASH_PAGE_NUM_MAX; i++)
				{
						/*计算首地址所在页*/
						if(start_page_flag == 0)
						{
							if((i*FLASH_PAGE_SIZE+FLASH_PAGE_0_ADDR+FLASH_PAGE_SIZE-1) >= Startaddress)
							{
								log_i("first erase page:%d",i);
								Startpage = i;
								start_page_flag = 1;
							}
						}
						/*计算尾地址所在页*/
						if(end_page_flag == 0)
						{
							if((i*FLASH_PAGE_SIZE+FLASH_PAGE_0_ADDR+FLASH_PAGE_SIZE-1) >= (Startaddress+size-1))
							{
								log_i("end erase page:%d",i);
								Endpage = i;
								end_page_flag = 1;
								break;
							}
						}
				}
		}
		else
		{
				/*地址非法*/
				*get_pages = 0;
				return Startpage;
		}
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
static uint32_t stm32_get_sector_size(uint32_t pages) 
{
	return pages*FLASH_PAGE_SIZE;	//!< 128K一页			
}

/**
 * Get the bankx
 *
 * @param addr
 *
 * @return bank num
 */
static uint8_t stm32_get_addr_bank(uint32_t addr)
{
  if(addr >= FLASH_PAGE_0_ADDR && addr < BANK_2_ADDR_FLASH_SECTOR_0)
  {
    return FLASH_BANK_1;
  }
  else if(addr >= BANK_2_ADDR_FLASH_SECTOR_0)
  {
    return FLASH_BANK_2;
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
    uint32_t addr = stm32h7_onchip_flash.addr + offset;
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size, bool block_mode)
{
    uint32_t addr = stm32h7_onchip_flash.addr + offset;
    uint8_t write_buf[FLASH_NB_32BITWORD_IN_FLASHWORD*4] = {0};
    size_t wr_size = 0;
    HAL_FLASH_Unlock();
    size_t write_cnt_max = (((size/32)*32) < size)?(size/32)+1:(size/32);
    for(size_t i = 0; i < write_cnt_max; i++)
    {
      /*检查已写入长度*/
      wr_size = (((i+1)*32) > size)?(size-(i*32)):32;
      memmove(write_buf, buf+(i*32), wr_size);
      if(block_mode == true)
      {
        if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr+(i*32), (uint32_t)write_buf))
        {
          return -1;
        }
      }
      else
      {
        if(HAL_OK != HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_FLASHWORD, addr+(i*32), (uint32_t)write_buf))
        {
          return -1;
        }
      }
      memset(write_buf, 0xFF, FLASH_NB_32BITWORD_IN_FLASHWORD*sizeof(uint32_t));
    }
    HAL_FLASH_Lock();
    return size;
}

static int erase(long offset, size_t size, bool block_mode)
{
    HAL_StatusTypeDef flash_status;
    size_t erased_size = 0;
    
    uint32_t addr = stm32h7_onchip_flash.addr + offset;
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t erasepages = 0;
    uint32_t error;
    /* start erase */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
    /* it will stop when erased size is greater than setting size */
		
    erase_config.Sector = stm32_get_sector(addr ,size ,&erasepages);

    /*擦除页地址，单BANK操作*/
    // erase_config.Sector = erase_config.Sector>=8?erase_config.Sector-8:erase_config.Sector;
    // /*所属BANK*/
    // erase_config.Banks = stm32_get_addr_bank(addr);
    // /*按页擦除*/
    // erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
    // /*擦除页数量*/
    // erase_config.NbSectors = erasepages;
    // /*擦除使用电压范围*/
    // erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    // log_i("erase page:0x%02X,PAGES:%d,BANK:%u", erase_config.Sector, erasepages, erase_config.Banks);
    // if(block_mode == true)
    // {
    //   flash_status = HAL_FLASHEx_Erase(&erase_config, &error);
    // }
    //else
    // {
    //   flash_status = HAL_FLASHEx_Erase_IT(&erase_config);
    // }
    // if (flash_status != HAL_OK)
    // {
    //   log_i("erase error");
    //   return -1;
    // }

    /*循环擦除页地址，多BANK操作*/
    for(uint32_t sec_index = 0; sec_index < erasepages; sec_index++)
    {
      erase_config.Sector = erase_config.Sector>=8?erase_config.Sector-8:erase_config.Sector;
      
      /*所属BANK*/
      erase_config.Banks = stm32_get_addr_bank(addr+(sec_index*FLASH_PAGE_SIZE));
      /*按页擦除*/
      erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
      /*擦除页数量*/
      erase_config.NbSectors = 1;
      /*擦除使用电压范围*/
      erase_config.VoltageRange = FLASH_VOLTAGE_RANGE_3;
      
      log_i("erase page:0x%02X,PAGES:%d,BANK:%u", erase_config.Sector, 1, erase_config.Banks);
      
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
      erase_config.Sector++;
    }

    erased_size += stm32_get_sector_size(erasepages);
    log_i("erase size:%d",erased_size);
		
    HAL_FLASH_Lock();

    return size;
}

const struct fal_flash_dev stm32h7_onchip_flash =
{
    .name       = "stm32_onchip",
    .addr       = FLASH_PAGE_0_ADDR,
    .len        = FLASH_PAGE_SIZE*FLASH_PAGE_NUM_MAX,
    .blk_size   = FLASH_PAGE_SIZE,
    .ops        = {init, read, write, erase},
    .write_gran = 32*FLASH_NB_32BITWORD_IN_FLASHWORD
};
