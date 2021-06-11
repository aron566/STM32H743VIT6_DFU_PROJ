/**                                                                             
 *  @file YModem.h                                                    
 *                                                                              
 *  @date 2021年01月11日 23:18:49 星期一
 *                                                                              
 *  @author aron566                                                             
 *                                                                              
 *  @brief Ymodem 传输协议.                                                                
 *                                                                              
 *  @version V1.0                                                               
 */                                                                             
#ifndef YMODEM_H                                                          
#define YMODEM_H                                                          
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
                                                                              
/*
接收开始流程：
接收者1HZ发送接收状态：
      /-----------/
    /     C     /
  /-----------/
C：代表字符'C',进入接收状态

发送者发送起始帧：
      /-----------/-----------/-----------/-----------/-----------/-----------/-----------/-----------/
    /    SOH    /    00     /     FF    / FILE_NAME / FILE_SIZE /    NULL   /   CRC_H   /   CRC_L   /
  /-----------/-----------/-----------/-----------/-----------/-----------/-----------/-----------/
/                                   /     C            R           C    /
SOH：代表起始帧，十六进制：0x01，也代表传输数据128字节
CRC：丢弃前三字节，后面为CRC数据区域
帧大小：128+5=133Bytes
NULL：0

接收应答发送：
      /-----------/
    /    ACK    /
  /-----------/
ACK：0x06

接收者发送开始接收标识ONCE：
      /-----------/
    /     C     /
  /-----------/
C：代表字符'C',进入接收状态

发送者发送数据帧：
      /-----------/-----------/-----------/-----------/-----------/-----------/-----------/-----------/
    /  SOH/STX  /  帧编号   / 帧编号反码  /      D       A       T       A    /   CRC_H   /   CRC_L   /
  /-----------/-----------/-------------/-----------/-----------/-----------/-----------/-----------/
/                                      /     C            R           C    /
SOH：十六进制：0x01，也代表传输数据128字节，STX 表示有1024个字节
CRC：丢弃前三字节，后面为CRC数据区域
帧大小：最小128+5=133Bytes 最大1024+5=1029Bytes
帧数据不足：使用0X1A填充

接收应答发送：
      /-----------/
    /    ACK    /
  /-----------/
ACK：0x06

发送结束流程：
发送者发送结束标识：
      /-----------/
    /    EOT    /
  /-----------/
EOT：代表0x04

接收者应答：
      /-----------/
    /    NAK    /
  /-----------/
NAK：代表0x15

发送者再次发送结束标识：
      /-----------/
    /    EOT    /
  /-----------/
EOT：代表0x04

接收应答发送：
      /-----------/
    /    ACK    /
  /-----------/
ACK：0x06

接收者发送开始接收结束帧：
      /-----------/
    /     C     /
  /-----------/
C：代表字符'C',进入接收状态

发送者发送结束帧：
      /-----------/-----------/-----------/-----------/-----------/-----------/-----------/-----------/
    /    SOH    /    00     /     FF    /         N   U   L   L[128]        /   CRC_H   /   CRC_L   /
  /-----------/-----------/-----------/-----------/-----------/-----------/-----------/-----------/
/                                   /     C            R           C    /
SOH：代表结束帧，十六进制：0x01，也代表传输数据128字节
CRC：丢弃前三字节，后面为CRC数据区域
帧大小：128+5=133Bytes
NULL：0

接收应答发送：
      /-----------/
    /    ACK    /
  /-----------/
ACK：0x06

流程结束！

发送者发送中止传输标识：
      /-----------/
    /    CAN    /
  /-----------/
CA：代表0x18

*/
/** Private defines ----------------------------------------------------------*/
#define FILE_NAME_LEN_MAX       64
#define FILE_SIZE_LEN_MAX       64
/** Exported typedefines -----------------------------------------------------*/
/*Ymodem 帧检测结果*/                                                                
typedef enum                                                               
{                                                                               
  YMODEM_IS_YMODEM_FRAME = 0,  /**< 是Ymodem数据帧 */ 
  YMODEM_IS_NOT_YMODEM_FRAME,                                                
}YMODEM_FRAME_CHECK_STATE_Typedef_t;

/*Ymodem 帧CRC校验结果*/
typedef enum                                                               
{                                                                               
  YMODEM_CRC_OK = 0,          /**< Ymodem数据帧校验正确 */ 
  YMODEM_CRC_ERROR,                                                
}YMODEM_FRAME_CRC_STATE_Typedef_t;

/*Ymodem 运行状态*/
typedef enum
{
  YMODEM_NO_RUNNING = 0,
  /*接收方*/
  YMODEM_START_SEND_C_LOOP,    /**< 启动发 C字符*/
  YMODEM_WAIT_START_SOH,
  YMODEM_REPLY_START_SOH_ACK,
  YMODEM_SEND_SIGNAL_ONE_C,
  YMODEM_WAIT_DATA_FRAME,
  YMODEM_REPLY_DATA_ACK,
  YMODEM_WAIT_START_EOT_FLAG,
  YMODEM_REPLY_START_EOT_NAK,
  YMODEM_WAIT_END_EOT_FLAG,
  YMODEM_REPLY_END_EOT_ACK,
  YMODEM_SEND_ONE_C_END,
  YMODEM_WAIT_END_SOH,
  YMODEM_REPLY_END_SOH_ACK,

  /*发送方*/
  YMODEM_WAIT_START_C,      /**< 等待起始 C字符*/
  YMODEM_SEND_START_FRAME,  /**< 发送起始帧*/
  YMODEM_WAIT_START_ACK,    /**< 等待ACK*/
  YMODEM_WAIT_START_REC,    /**< 等待接收C字符发送数据起始*/
  YMODEM_SEND_DATA_FRAME,
  YMODEM_WAIT_DATA_ACK,
  YMOMDE_SEND_EOT_FRAME,
  YMODEM_WAIT_NAK,
  YMODEM_SEND_EOT_END,
  YMODEM_WAIT_EOT_ACK,
  YMODEM_WAIT_REC_END_C,  /**< 等待接收结束帧信号*/
  YMODEM_SEND_END_FRAME,
  YMODEM_WAIT_END_ACK,

  YMODEM_COMPLETE
}YMODEM_TRANSFER_STEP_Typedef_t;

/*Ymodem 运行结果*/
typedef enum
{
  YMODEM_SUCCESSFUL = 0,      /**< 运行成功*/
  YMODEM_RUN_TIMEOUT,         /**< 运行超时*/
  YMODEM_IS_RUNNING,          /**< 运行中*/
  YMODEM_ABORT_ERROR,         /**< 运行中止*/
  YMODEM_FRAME_NOT_MATCH,     /**< 帧不匹配*/
  YMODEM_FRAME_NOT_FULL,      /**< 帧不全*/
  YMODEM_UNKNOW_ERROR,        /**< 未知错误*/
}YMODEM_RUN_RESULT_Typedef_t;

/*Ymodem 句柄*/
/*Ymodem 模式选择状态*/
typedef enum
{
  YMODEM_REC_MODE = 0,
  YMODEM_SEND_MODE,
}YMODEM_MODE_Typedef_t;
/*Ymodem 数据接收回调*/
/**
 * @brief 接收数据回调
 * @param data 数据
 * @param size 数据长度
 * @param packet_num 第几包数据
 * @return 接收数据大小
 */
typedef uint32_t (*YMODEM_REC_DATA_CALLBACK)(const uint8_t *data, uint32_t size, uint8_t packet_num);
/*Ymodem 数据发送回调*/
/**
 * @brief 填充发送数据回调
 * @param dest_buf 数据存储区
 * @param size 存储区大小
 * @param packet_num 填充本次数据包号
 * @return 本次填充数据大小
 */
typedef uint32_t (*YMODEM_SEND_DATA_CALLBACK)(uint8_t *dest_buf, uint32_t size, uint8_t *packet_num);
/*Ymodem 答复接口回调*/
/**
 * @brief 发送数据接口
 * @param data 数据
 * @param size 数据长度
 * @return None
 */
typedef void (*YMODEM_REPLY_DATA_CALLBACK)(const uint8_t *data, uint32_t size);

typedef struct 
{
  char file_name[FILE_NAME_LEN_MAX];              /**< 发送/接收文件名*/
  uint32_t file_size;                             /**< 发送/接收文件大小*/
  float current_progress;                         /**< 当前文件发送/接收进度*/
  uint32_t time_cnt;                              /**< 时间计时*/
  uint32_t set_time_out;                          /**< 任务超时*/
  uint32_t last_time;                             /**< 上次运行时间*/
  uint32_t file_size_cnt;                         /**< 当前发送/接收报总大小*/
  YMODEM_REC_DATA_CALLBACK set_rec_data_func;     /**< 接收数据回调*/
  YMODEM_SEND_DATA_CALLBACK set_send_data_func;   /**< 发送数据回调*/
  YMODEM_REPLY_DATA_CALLBACK set_reply_data_func; /**< 答复数据回调*/
  YMODEM_MODE_Typedef_t set_mode;                 /**< Ymodem模式状态*/
  YMODEM_TRANSFER_STEP_Typedef_t run_step;        /**< Ymodem运行状态*/
  YMODEM_TRANSFER_STEP_Typedef_t last_run_step;   /**< Ymodem上次运行状态*/
}YMODEM_HANDLE_Typedef_t;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
#ifndef ASSERT_PAR
#define ASSERT_PAR(par, value, out) do{ \
                                        if(par == value) \
                                        { \
                                          out; \
                                        }}while(0)
#endif
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*Ymodem ms计时接口回调-放入systick或者1ms定时器中中断中*/
void ymodem_update_time_ms_Port(void);

/*Ymodem 初始化*/
/**
 * @brief Ymodem 初始化
 * 
 * @param handle Ymodem句柄
 * @param mode 运行模式：YMODEM_REC_MODE接收模式/YMODEM_SEND_MODE发送模式
 * @param rec_func 接收数据回调
 * @param send_func 发送数据时的填充数据回调
 * @param reply_func 回复数据回调
 * @param time_out 超时时间
 * @return true 
 * @return false 
 */
bool ymodem_init(YMODEM_HANDLE_Typedef_t *handle, YMODEM_MODE_Typedef_t mode, YMODEM_REC_DATA_CALLBACK rec_func
                  ,YMODEM_SEND_DATA_CALLBACK send_func, YMODEM_REPLY_DATA_CALLBACK reply_func, uint32_t time_out);
/*Ymodem 启动*/
/**
 * @brief Ymodem 启动,循环调用，注意数据一帧大小接收缓冲区应大于133
 * 
 * @param handle Ymodem句柄
 * @param data 将接收到的数据帧传递
 * @param len 数据帧大小字节数
 * @return YMODEM_RUN_RESULT_Typedef_t 
 */
YMODEM_RUN_RESULT_Typedef_t ymodem_task_run(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);

/*Ymodem 获取文件名及大小*/
/**
 * @brief Ymodem 获取文件名及大小
 * 
 * @param handle Ymodem句柄
 * @param file_size 获得文件大小
 * @return const char* 
 */
const char *ymodem_get_file_info(YMODEM_HANDLE_Typedef_t *handle, uint32_t *file_size);

/*Ymodem 获得当前发送或接收进度*/
/**
 * @brief Ymodem 获得当前发送或接收进度
 * 
 * @param handle Ymodem句柄
 * @return float 当前进度
 */
float ymodem_get_progress(YMODEM_HANDLE_Typedef_t *handle);

/*Ymodem 发送文件信息设置*/
/**
 * @brief Ymodem 发送文件信息设置
 * 
 * @param handle Ymodem句柄
 * @param file_name 设置发送的文件名
 * @param file_size 设置发送的文件大小
 */
void ymodem_set_send_file_info(YMODEM_HANDLE_Typedef_t *handle, const char *file_name, uint32_t file_size);

#ifdef __cplusplus ///<end extern c                                             
}                                                                               
#endif                                                                          
#endif                                                                          
/******************************** End of file *********************************/
