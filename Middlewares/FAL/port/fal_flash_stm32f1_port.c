/**
 *  @file fal_flash_stm32f1_port.c
 *
 *  @date 2021-01-01
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief STM32F103ZET6
 *
 *  @details None.
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <fal.h>

#include <stm32f1xx.h>

/* base address of the flash sectors */
#define ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base address of Sector 0, 16 K bytes   */
#define ADDR_FLASH_SECTOR_1      ((uint32_t)0x08004000) /* Base address of Sector 1, 16 K bytes   */
#define ADDR_FLASH_SECTOR_2      ((uint32_t)0x08008000) /* Base address of Sector 2, 16 K bytes   */
#define ADDR_FLASH_SECTOR_3      ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 K bytes   */
#define ADDR_FLASH_SECTOR_4      ((uint32_t)0x08010000) /* Base address of Sector 4, 64 K bytes   */
#define ADDR_FLASH_SECTOR_5      ((uint32_t)0x08020000) /* Base address of Sector 5, 128 K bytes  */
#define ADDR_FLASH_SECTOR_6      ((uint32_t)0x08040000) /* Base address of Sector 6, 128 K bytes  */
#define ADDR_FLASH_SECTOR_7      ((uint32_t)0x08060000) /* Base address of Sector 7, 128 K bytes  */
#define ADDR_FLASH_SECTOR_8      ((uint32_t)0x08080000) /* Base address of Sector 8, 128 K bytes  */
#define ADDR_FLASH_SECTOR_9      ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 K bytes  */
#define ADDR_FLASH_SECTOR_10     ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes */
#define ADDR_FLASH_SECTOR_11     ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes */

#define FLASH_PAGE_0_ADDR        ((uint32_t)0x08000000)	/* 物理flash原始起始页地址 */
#define FLASH_START_PAGE_ADDR	 ((uint32_t)0x08020000)	/* 分区表起始地址 */
#define CURRENT_PART_START_PAGE	 ((FLASH_START_PAGE_ADDR-FLASH_PAGE_0_ADDR)/FLASH_PAGE_SIZE)	/* 当前分区实际物理起始页 */
#define FLASH_PAGE_NUM_MAX		 ((uint32_t)255+1)
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
		if(Startaddress >= stm32f1_onchip_flash.addr)
		{
				for(uint32_t i = 0; i < FLASH_PAGE_NUM_MAX;i++)
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

    return Startpage*FLASH_PAGE_SIZE+FLASH_PAGE_0_ADDR;
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
	return pages*FLASH_PAGE_SIZE;	//!< 2K一页			
}
static int init(void)
{
    /* do nothing now */
		return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = stm32f1_onchip_flash.addr + offset;
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size, bool block_mode)
{
  size_t i;
  uint32_t read_data;
  uint32_t addr = stm32f1_onchip_flash.addr + offset;

  HAL_FLASH_Unlock();
  __HAL_FLASH_GET_FLAG(
          FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR
                  | FLASH_FLAG_OPTVERR);
  uint16_t data = 0;
  for (i = 0; i < size; i++)
  {
      /* write data */
      data = *(buf+i);
      if((i+1) < size)
      {
        i++;
        data |= ((uint16_t)*(buf+i)<<8);
      }
      else
      {
         data &= 0x00FF;
      }
      if(block_mode == true)
      {
        if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD ,addr, data))
        {
          return -1;
        }
      }
      else
      {
        if(HAL_OK != HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_HALFWORD ,addr, data))
        {
          return -1;
        }
      }
      read_data = *(uint16_t *) addr;
      addr += 2;
      
      /* check data */
      if (read_data != data)
      {
        return -1;
      }
  }
  HAL_FLASH_Lock();

  return size;
}

static int erase(long offset, size_t size, bool block_mode)
{
  HAL_StatusTypeDef flash_status;
  size_t erased_size = 0;
  
  uint32_t addr = stm32f1_onchip_flash.addr + offset;
  FLASH_EraseInitTypeDef erase_config = {0};
  uint32_t erasepages = 0;
  uint32_t error;
  /* start erase */
  HAL_FLASH_Unlock();
  __HAL_FLASH_GET_FLAG(
  FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR
                  | FLASH_FLAG_OPTVERR);
  /* it will stop when erased size is greater than setting size */
  
  /*擦除页地址*/
  erase_config.PageAddress = stm32_get_sector(addr ,size ,&erasepages);
  log_i("erase page addr:0x%08X,PAGES:%d",erase_config.PageAddress,erasepages);
  
  /*所属BANK*/
  erase_config.Banks = FLASH_BANK_1;
  /*按页擦除*/
  erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
  /*擦除页数量*/
  erase_config.NbPages = erasepages;
  log_i("erase page number:%d",erasepages);
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
  erased_size += stm32_get_sector_size(erasepages);
  log_i("erase size:%d",erased_size);
  
  HAL_FLASH_Lock();

  return size;
}

const struct fal_flash_dev stm32f1_onchip_flash =
{
  .name       = "stm32_onchip",
  .addr       = FLASH_PAGE_0_ADDR,
  .len        = 512*1024,
  .blk_size   = FLASH_PAGE_SIZE,
  .ops        = {init, read, write, erase},
  .write_gran = 32
};

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
