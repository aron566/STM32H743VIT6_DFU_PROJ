/**
 *  @file Flash_Port.c
 *
 *  @date 2021-01-13
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief FLASH操作接口
 *
 *  @details BANK1 RANGE:0x0800 0000- 0x080F FFFF TOTAL_SIZE:128Kb*8
 *           BANK2 RANGE:0x0810 0000- 0x081F FFFF TOTAL_SIZE:128Kb*8
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "Flash_Port.h"
#include "User_Main.h"
#include "fal.h"
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
  ******************************************************************
  * @brief   写入指定分区数据
  * @param   [in]name 分区名称
  * @param   [in]data 存储区
  * @param   [in]Offset 写入地址相对分区起始地址偏移
  * @param   [in]Size 写入大小
  * @param   [in]block_mode true阻塞模式
  * @return  -1失败，写入长度
  * @author  aron566
  * @version V1.0
  * @date    2021-02-11
  ******************************************************************
  */
int Flash_Port_Write_Partition_Data(const char *Partition_Name, const uint8_t *data, uint32_t Offset, uint32_t Size, bool block_mode)
{
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return -1;
  }
  return fal_partition_write(part, Offset, data, Size, block_mode);
}

/**
  ******************************************************************
  * @brief   读取指定分区数据
  * @param   [in]name 分区名称
  * @param   [in]Dest_Buf 存储区
  * @param   [in]Offset 读取地址偏移
  * @param   [in]Read_Size 读取大小
  * @return  -1失败，读取长度
  * @author  aron566
  * @version V1.0
  * @date    2021-02-11
  ******************************************************************
  */
int Flash_Port_Read_Partition_Data(const char *Partition_Name, uint8_t *Dest_Buf, uint32_t Offset, uint32_t Read_Size)
{
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return -1;
  }
  return fal_partition_read(part, Offset, Dest_Buf, Read_Size);
}

/**
  ******************************************************************
  * @brief   擦除指定分区
  * @param   [in]name 分区名称
  * @param   [in]block_mode true阻塞模式
  * @return  true 操作成功
  * @author  aron566
  * @version V1.0
  * @date    2021-01-31
  ******************************************************************
  */
bool Flash_Port_Erase_Partition(const char *Partition_Name, bool block_mode)
{
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return false;
  }
  if(fal_partition_erase_all(part, block_mode) < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/**
  ******************************************************************
  * @brief   擦除指定分区数据
  * @param   [in]name 分区名称
  * @param   [in]Offset 擦除地址相对分区偏移
  * @param   [in]Size 擦除大小
  * @param   [in]block_mode true阻塞模式
  * @return  true 操作成功
  * @author  aron566
  * @version V1.0
  * @date    2021-04-28
  ******************************************************************
  */
bool Flash_Port_Erase_Partition_Data(const char *Partition_Name, uint32_t Offset, uint32_t Size, bool block_mode)
{
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return false;
  }
  if(fal_partition_erase(part, Offset, Size, block_mode) < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/**
  ******************************************************************
  * @brief   获取指定分区地址
  * @param   [in]name 分区名称
  * @return  成功提供分区地址,否则提供bl地址
  * @author  aron566
  * @version V1.0
  * @date    2021-02-08
  ******************************************************************
  */
uint32_t Flash_Port_Get_Partition_Addr(const char *Partition_Name)
{
  uint32_t addr;
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return 0x08000000;
  }
  addr = 0x08000000+(uint32_t)part->offset;
  return addr;
}

/**
  ******************************************************************
  * @brief   获取指定分区大小
  * @param   [in]name 分区名称
  * @return  分区大小,0xFFFFFFFF未获取到
  * @author  aron566
  * @version V1.0
  * @date    2021-02-08
  ******************************************************************
  */
uint32_t Flash_Port_Get_Partition_Size(const char *Partition_Name)
{
  uint32_t size = 0xFFFFFFFF;
  const struct fal_partition *part = fal_partition_find(Partition_Name);
  if(part == NULL)
  {
    return size;
  }
  size = (uint32_t)part->len;
  return size;
}

/**
  ******************************************************************
  * @brief   Flash操作初始化
  * @param   [in]None
  * @return  None
  * @author  aron566
  * @version V1.0
  * @date    2021-01-31
  ******************************************************************
  */
void Flash_Port_Init(void)
{
  fal_init();
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
