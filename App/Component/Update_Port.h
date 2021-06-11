/**
 *  @file Update_Port.h
 *
 *  @date 2021-02-26
 *
 *  @author aron566
 *
 *  @brief 更新操作接口
 *  
 *  @version V1.0
 */
#ifndef UPDATE_PORT_H
#define UPDATE_PORT_H
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

/*更新接口初始化*/
void Update_Port_Init(void);
/*写入新固件信息*/
bool Update_Port_Write_New_Frimware_Info(const char *Frimware_Name, uint32_t Size, const char *Partition_Name);
/*获取固件名*/
const char *Update_Port_Get_Frimware_Name(void);
/*获取固件大小*/
uint32_t Update_Port_Get_Frimware_Size(void);
/*写入下载成功标识*/
void Update_Port_Write_Download_Complete_Flag(void);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
