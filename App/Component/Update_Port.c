/**
 *  @file Update_Port.c
 *
 *  @date 2021-01-25
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 
 *
 *  @details 1、
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "Update_Port.h"
#include "Flash_Port.h"
#include "utilities.h"
/** Private macros -----------------------------------------------------------*/
#define FRIMWARE_NAME_LEN_MAX               64U       /**< 固件名称长度*/
#define REGISTER_CODE_LEN_MAX               (4*4U)    /**< 注册码长度Byte*/
#define FRIMWARE_FLAG_PARTITION_NAME        "flag"    /**< 升级标识分区名*/
#define CURRENT_FRIMWARE_SIZE               264*1024U /**< 本固件BIN大小*/

#ifndef SECTION
  #ifdef __CC_ARM                        /* ARM Compiler */
      #define SECTION(x)                 __attribute__((section(x)))
      #define USED                       __attribute__((used))
  #elif defined (__IAR_SYSTEMS_ICC__)    /* for IAR Compiler */
      #define SECTION(x)                 @ x
      #define USED                       __root
  #elif defined (__GNUC__)               /* GNU GCC Compiler */
      #define SECTION(x)                 __attribute__((section(x)))
      #define USED                       __attribute__((used))
  #else
      #error not supported tool chain
  #endif /* __CC_ARM */
#endif

/** Private typedef ----------------------------------------------------------*/
/*升级标识*/
typedef enum
{
  UPDATE_SUCESSFUL  = 0xAD8888,                       /**< 更新成功*/
  UPDATE_WAITTING   = 0xAD6666,                       /**< 等待更新*/
  DOWNLOAD_COMPLETE = 0xAD5555,                       /**< 下载完成*/
}FRIMWARE_UPDATE_FLAG_Typedef_t;

/*固件信息*/
typedef struct      
{     
  char FrimwareName[FRIMWARE_NAME_LEN_MAX];           /**< 固件名称*/
  uint32_t FrimwareSize;                              /**< 固件大小*/
  char BackFrimwareName[FRIMWARE_NAME_LEN_MAX];       /**< facotry固件名称*/
  uint32_t BackFrimwareSize;                          /**< facotry固件大小*/
  FRIMWARE_UPDATE_FLAG_Typedef_t Flag;                /**< 升级标识*/
  uint32_t Retry_Cnt;                                 /**< 等待更新次数累计*/
  uint32_t Register_Code[REGISTER_CODE_LEN_MAX/4];    /**< 注册码*/
}FRIMWARE_INFO_Typedef_t;

/** Private constants --------------------------------------------------------*/

/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static FRIMWARE_INFO_Typedef_t Frimware_Info;
/** Private function prototypes ----------------------------------------------*/
static void Write_Frimware_Info(FRIMWARE_INFO_Typedef_t *Frimware_Info);
static void Get_Frimware_Info(FRIMWARE_INFO_Typedef_t *Frimware_Info);
/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/**
  ******************************************************************
  * @brief   写入固件信息
  * @param   [in]Frimware_Info 固件信息.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/2/12
  ******************************************************************
  */
static void Write_Frimware_Info(FRIMWARE_INFO_Typedef_t *Frimware_Info)
{
  /*擦除信息*/
  printf("erase flag partition ...\r\n");
  Flash_Port_Erase_Partition(FRIMWARE_FLAG_PARTITION_NAME, true);
  Flash_Port_Write_Partition_Data(FRIMWARE_FLAG_PARTITION_NAME,
                              (uint8_t *)Frimware_Info, 0, sizeof(FRIMWARE_INFO_Typedef_t), true);
  printf("write flag partition ok.\r\n");
}

/**
  ******************************************************************
  * @brief   获取固件信息
  * @param   [out]Frimware_Info 信息存储区.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/2/11
  ******************************************************************
  */
static void Get_Frimware_Info(FRIMWARE_INFO_Typedef_t *Frimware_Info)
{
  printf("\r\n***Check APP Flag***\r\n");
  int ret = Flash_Port_Read_Partition_Data(FRIMWARE_FLAG_PARTITION_NAME, 
                              (uint8_t *)Frimware_Info, 0, sizeof(FRIMWARE_INFO_Typedef_t));
  if(ret < 0)
  {
    printf("get frimware info faild.\r\n");
  }
  Frimware_Info->FrimwareName[FRIMWARE_NAME_LEN_MAX-1] = '\0';
  printf("\r\nfirmware name: %s, size : %u Bytes, update flag:%08X ", Frimware_Info->FrimwareName
        ,Frimware_Info->FrimwareSize, Frimware_Info->Flag);
  switch(Frimware_Info->Flag)
  {
    /*更新成功*/
    case UPDATE_SUCESSFUL:
      printf("App Frimware Ok.\r\n");
      break;
    /*等待更新*/
    case UPDATE_WAITTING:
      printf("Update Frimware Ok.\r\n");
      break;
    default:
      printf("Unknow flag.\r\n");
      strncopy(Frimware_Info->FrimwareName, "Factory_APP.bin", FRIMWARE_NAME_LEN_MAX);
      strncopy(Frimware_Info->BackFrimwareName, "Factory_APP.bin", FRIMWARE_NAME_LEN_MAX);
      Frimware_Info->FrimwareSize = Frimware_Info->BackFrimwareSize = CURRENT_FRIMWARE_SIZE;
      break;
  }
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
/**
  ******************************************************************
  * @brief   获取固件名
  * @param   [in]None.
  * @return  固件名.
  * @author  aron566
  * @version v1.0
  * @date    2021/03/01
  ******************************************************************
  */
const char *Update_Port_Get_Frimware_Name(void)
{
  return Frimware_Info.FrimwareName;
}

/**
  ******************************************************************
  * @brief   获取固件大小
  * @param   [in]None.
  * @return  固件大小字节.
  * @author  aron566
  * @version v1.0
  * @date    2021/03/01
  ******************************************************************
  */
uint32_t Update_Port_Get_Frimware_Size(void)
{
  return Frimware_Info.FrimwareSize;
}

/**
  ******************************************************************
  * @brief   写入下载成功标识
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/03/24
  ******************************************************************
  */
void Update_Port_Write_Download_Complete_Flag(void)
{
  /*擦除信息*/
  printf("erase flag partition ...\r\n");
  Flash_Port_Erase_Partition(FRIMWARE_FLAG_PARTITION_NAME, true);
  Frimware_Info.Flag = DOWNLOAD_COMPLETE;
  Flash_Port_Write_Partition_Data(FRIMWARE_FLAG_PARTITION_NAME,
                              (uint8_t *)&Frimware_Info, 0, sizeof(FRIMWARE_INFO_Typedef_t), true);
  printf("write download complete flag partition ok.\r\n");  
}

/**
  ******************************************************************
  * @brief   写入新固件信息
  * @param   [in]Frimware_Name 固件名.
  * @param   [in]Size.
  * @param   [in]Partition_Name 分区名.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/03/01
  ******************************************************************
  */
bool Update_Port_Write_New_Frimware_Info(const char *Frimware_Name, uint32_t Size, const char *Partition_Name)
{
  if(Frimware_Name == NULL || Size == 0)
  {
    return false;
  }
  
  /*检测写入大小*/
  if(Flash_Port_Get_Partition_Size(Partition_Name) < Size)
  {
    return false;
  }
  if(strncmp(Partition_Name, "download", FRIMWARE_NAME_LEN_MAX) == 0)
  {
    strncpy(Frimware_Info.FrimwareName, Frimware_Name, FRIMWARE_NAME_LEN_MAX);
    Frimware_Info.FrimwareSize = Size;
    Frimware_Info.Flag = UPDATE_WAITTING;
    Frimware_Info.Retry_Cnt = 0;
  }
  else if(strncmp(Partition_Name, "factory", FRIMWARE_NAME_LEN_MAX) == 0)
  {
    strncpy(Frimware_Info.BackFrimwareName, Frimware_Name, FRIMWARE_NAME_LEN_MAX);
    Frimware_Info.BackFrimwareSize = Size;
  }
  else
  {
    return false;
  }
  /*写入*/
  Write_Frimware_Info(&Frimware_Info);
  return true;
}

/**
  ******************************************************************
  * @brief   更新初始化
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/2/26
  ******************************************************************
  */
void Update_Port_Init(void)
{
  /*判断标识*/
  Get_Frimware_Info(&Frimware_Info);
  
  if(Frimware_Info.Flag == UPDATE_SUCESSFUL)
  {
    return;
  }
  
  /*写入更新成功标识*/
  Frimware_Info.Flag = UPDATE_SUCESSFUL;
  Write_Frimware_Info(&Frimware_Info);
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
