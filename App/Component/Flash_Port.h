/**
 *  @file Flash_Port.h
 *
 *  @date 2021-01-13
 *
 *  @author aron566
 *
 *  @brief FLASH操作接口
 *  
 *  @version V1.0
 */
#ifndef FLASH_PORT_H
#define FLASH_PORT_H
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
/** Private includes ---------------------------------------------------------*/
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*Flash操作初始化*/
void Flash_Port_Init(void);
/*Flash擦除指定分区*/
bool Flash_Port_Erase_Partition(const char *Partition_Name, bool block_mode);
/*Flash擦除指定分区数据*/
bool Flash_Port_Erase_Partition_Data(const char *Partition_Name, uint32_t Offset, uint32_t Size, bool block_mode);
/*获取指定分区地址*/
uint32_t Flash_Port_Get_Partition_Addr(const char *Partition_Name);
/*获取指定分区大小*/
uint32_t Flash_Port_Get_Partition_Size(const char *Partition_Name);
/*读取指定分区数据*/
int Flash_Port_Read_Partition_Data(const char *Partition_Name, uint8_t *Dest_Buf, uint32_t Offset, uint32_t Read_Size);
/*写入指定分区数据*/
int Flash_Port_Write_Partition_Data(const char *Partition_Name, const uint8_t *data, uint32_t Offset, uint32_t Size, bool block_mode);
  
#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
