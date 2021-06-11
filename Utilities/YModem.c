/**                                                                             
 *  @file YModem.c                                                   
 *                                                                              
 *  @date 2021年01月11日 23:19:12 星期一
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief Ymodem 传输协议.
 *
 *  @details None.
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "YModem.h"
/** Private typedef ----------------------------------------------------------*/
/*YModem 标识符*/
typedef enum
{
    YMODEM_SOH = 0x01,
    YMODEM_STX = 0x02,
    YMODEM_ACK = 0x06,
    YMODEM_NAK = 0x15,
    YMODEM_CAN = 0x18,/**< 传输中止*/
    YMODEM_EOT = 0x04,
    YMODEM_C   = 0x43,
}YMODEM_CHAR_FLAG_Typedef_t;                                              
/** Private macros -----------------------------------------------------------*/
#define YMODEM_DATA_FRAME_SIZE_MIN      128U
#define YMODEM_DATA_FRAME_SIZE_MAX      1024U
#define YMODEM_RECEIVER_FRAME_SIZE_MIN  1U
#define YMODEM_CAN_NUMBER               5U
#define YMODEM_DATA_PACKAGE_SIZE_MIN    (YMODEM_DATA_FRAME_SIZE_MIN+5)
#define YMODEM_DATA_PACKAGE_SIZE_MAX    (YMODEM_DATA_FRAME_SIZE_MAX+5)
#define YMODEM_RECEIVER_MIN_DATA_SIZE_CHECK(size)   ((size >= YMODEM_DATA_PACKAGE_SIZE_MIN)?1:0)
#define YMODEM_RECEIVER_MAX_DATA_SIZE_CHECK(size)   ((size >= YMODEM_DATA_PACKAGE_SIZE_MAX)?1:0)    
/*判断是否是起始帧*/
#define YMODEM_IS_START_FRAME(buf)      ((buf[0] == (uint8_t)YMODEM_SOH) && \
                                         (buf[1] == (uint8_t)(~buf[2])) && (buf[3] != '\0'))
/*判断是否是结束帧*/
#define YMODEM_IS_END_FRAME(buf)        ((buf[0] == (uint8_t)YMODEM_SOH) && \
                                        (buf[1] == (uint8_t)(~buf[2])) && (buf[3] == 0))
/*判断是否是数据帧头*/
#define YMODEM_IS_HEADER(buf)           ((buf[0] == (uint8_t)YMODEM_SOH) || (buf[0] == (uint8_t)YMODEM_STX) \
                                        && (buf[1] == (uint8_t)(~buf[2])))
/*获得完整帧大小*/
#define YMODEM_GET_FRAME_SIZE(buf)      ((buf[0] == (uint8_t)YMODEM_SOH)? \
                                        YMODEM_DATA_PACKAGE_SIZE_MIN:YMODEM_DATA_PACKAGE_SIZE_MAX)
/*获得帧数据大小*/
#define YMODEM_GET_FRAME_DATA_SIZE(buf) ((buf[0] == (uint8_t)YMODEM_SOH)? \
                                        YMODEM_DATA_FRAME_SIZE_MIN:YMODEM_DATA_FRAME_SIZE_MAX)
/*原始数据帧crc计算起始偏移*/                                                
#define YMODEM_CRC_DATA_OFFSET(buf)     ((uint8_t *)buf+3)
/*校验帧头部标识*/
#define YMODEM_CRC_CHECK_HEADER(buf)    (buf[0] == (uint8_t)YMODEM_SOH || buf[0] == (uint8_t)YMODEM_STX)
/*获得短报文crc值*/
#define YMODEM_GET_SOH_CRC_VALUE(buf)   ((uint16_t)buf[131]<<8|buf[132])
/*获得长报文crc值*/
#define YMODEM_GET_STX_CRC_VALUE(buf)   ((uint16_t)buf[1027]<<8|buf[1028])
/*获得原始数据crc值*/
#define YMODEM_GET_FRAME_CRC_VALUE(buf) ((buf[0] == (uint8_t)YMODEM_SOH)? \
                                        YMODEM_GET_SOH_CRC_VALUE(buf):YMODEM_GET_STX_CRC_VALUE(buf))
#define POLY                            0x1021
#define YMODEM_FILL_CHAR                0x1A
#define UNUSED(x)                       (void)(x)
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
static uint32_t ymodem_current_time_sec = 0;
static uint32_t ymodem_current_time_ms = 0;                                                                        
/** Private function prototypes ----------------------------------------------*/
static bool ymodem_crc_check(uint8_t *data, uint16_t len);
static uint16_t ymodem_crc16(uint8_t *addr, uint16_t num);
static size_t get_ch_index_of_str(const char *str, char ch ,size_t size);

static YMODEM_RUN_RESULT_Typedef_t ymodem_send_task(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static YMODEM_RUN_RESULT_Typedef_t ymodem_rec_task(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_start_SOH(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_stop_SOH(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static YMODEM_RUN_RESULT_Typedef_t ymodem_get_data_frame(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_EOT(const uint8_t *data);
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_start_SOH(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len);
static void ymodem_get_file_name(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data);
static void ymodem_send_flag_step(YMODEM_HANDLE_Typedef_t *handle, YMODEM_CHAR_FLAG_Typedef_t flag, bool froce);
static void ymodem_update_progress(YMODEM_HANDLE_Typedef_t *handle);       
static void ymodem_update_run_step(YMODEM_HANDLE_Typedef_t *handle, YMODEM_TRANSFER_STEP_Typedef_t step);

static void ymodem_send_start_frame(YMODEM_HANDLE_Typedef_t *handle);
static YMODEM_RUN_RESULT_Typedef_t ymodem_req_send_file_data(YMODEM_HANDLE_Typedef_t *handle);
static void ymodem_send_end_frame(YMODEM_HANDLE_Typedef_t *handle);
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
  * @brief   ymodem crc16数据校验
  * @param   [in]data 数据
  * @param   [in]num 数据长度
  * @return  true 校验正确
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */ 
static uint16_t ymodem_crc16(uint8_t *addr, uint16_t num)  
{  
    int i;
    uint16_t crc = 0;
    for(; num > 0; num--)					/* Step through bytes in memory */  
    {  
        crc = crc ^ (*addr++ << 8);			/* Fetch byte from memory, XOR into CRC top byte*/  
        for(i = 0; i < 8; i++)				/* Prepare to rotate 8 bits */  
        {
            if(crc & 0x8000)				/* b15 is set... */  
                crc = (crc << 1) ^ POLY;  	/* rotate and XOR with polynomic */  
            else                          	/* b15 is clear... */  
                crc <<= 1;					/* just rotate */  
        }									/* Loop for 8 bits */  
        crc &= 0xFFFF;						/* Ensure CRC remains 16-bit value */  
    }										/* Loop until num=0 */  
    return(crc);							/* Return updated CRC */  
}

/**
  ******************************************************************
  * @brief   ymodem 数据校验
  * @param   [in]data 原始帧数据
  * @param   [in]len 数据总长度
  * @return  true 校验正确
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */                                                                          
static bool ymodem_crc_check(uint8_t *data, uint16_t len)
{
    ASSERT_PAR(data, NULL, return false);
    ASSERT_PAR(len, YMODEM_DATA_PACKAGE_SIZE_MIN-1 || len < YMODEM_DATA_PACKAGE_SIZE_MIN, return false);
    uint16_t crc = 0;
    uint16_t crc_data_len = YMODEM_GET_FRAME_DATA_SIZE(data);
    /*进行CRC校验*/
    if(YMODEM_CRC_CHECK_HEADER(data) && (len >= YMODEM_GET_FRAME_DATA_SIZE(data)))
    {
        crc = ymodem_crc16(YMODEM_CRC_DATA_OFFSET(data), crc_data_len);
        if(crc != YMODEM_GET_FRAME_CRC_VALUE(data))
        {
            return false;///< 没通过校验
        }
        /*通过校验*/
        return true;
    }
    return false;
}  

/**
  ******************************************************************
  * @brief   返回首次字符出现的位置
  * @param   [in]str
  * @param   [in]ch 字符
  * @param   [in]size 字符串长度
  * @return  地址号
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static size_t get_ch_index_of_str(const char *str, char ch ,size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        if(str[i] == ch)
        {
            return i;
        }
    }
    return 0;
}

/**
  ******************************************************************
  * @brief   ymodem 发送停止帧
  * @param   [in]handle Ymodem句柄
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */      
static void ymodem_send_end_frame(YMODEM_HANDLE_Typedef_t *handle)
{
    uint8_t buf[YMODEM_DATA_PACKAGE_SIZE_MIN] = {0};
    buf[0] = (uint8_t)YMODEM_SOH;
    buf[1] = 0;
    buf[2] = ~buf[1];
    uint16_t crc = ymodem_crc16(YMODEM_CRC_DATA_OFFSET(buf), YMODEM_DATA_FRAME_SIZE_MIN);
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-2] = (uint8_t)crc>>8;
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-1] = (uint8_t)crc&0x00FF;
    handle->set_reply_data_func(buf, YMODEM_DATA_PACKAGE_SIZE_MIN);
}

/**
  ******************************************************************
  * @brief   ymodem 发送数据填充
  * @param   [in]handle Ymodem句柄
  * @return  YMODEM_SUCCESSFUL 发送数据完成 .
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */              
static YMODEM_RUN_RESULT_Typedef_t ymodem_req_send_file_data(YMODEM_HANDLE_Typedef_t *handle)
{
    ASSERT_PAR(handle->set_send_data_func, NULL, return YMODEM_ABORT_ERROR);
    YMODEM_RUN_RESULT_Typedef_t ret = YMODEM_IS_RUNNING;
    uint8_t buf[YMODEM_DATA_PACKAGE_SIZE_MIN] = {0};
    uint8_t packet_num = 0;
    uint32_t fill_size = handle->set_send_data_func(buf+3, YMODEM_DATA_FRAME_SIZE_MIN, &packet_num);
    if(fill_size == 0)
    {
        return YMODEM_SUCCESSFUL;
    }
    else if(fill_size < YMODEM_DATA_FRAME_SIZE_MIN && fill_size != 0)
    {
        memset(buf+fill_size, YMODEM_FILL_CHAR, YMODEM_DATA_FRAME_SIZE_MIN-fill_size);
        ret = YMODEM_SUCCESSFUL;
    }
    else
    {
        ret = YMODEM_IS_RUNNING;
    }
    
    buf[0] = (uint8_t)YMODEM_SOH;
    buf[1] = packet_num;
    buf[2] = ~buf[1];
    uint16_t crc = ymodem_crc16(YMODEM_CRC_DATA_OFFSET(buf), YMODEM_DATA_FRAME_SIZE_MIN);
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-2] = (uint8_t)(crc>>8);
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-1] = (uint8_t)crc&0x00FF;
    ASSERT_PAR(handle->set_reply_data_func, NULL, return YMODEM_ABORT_ERROR);
    handle->set_reply_data_func(buf, YMODEM_DATA_PACKAGE_SIZE_MIN);
    handle->file_size_cnt += fill_size;
    ymodem_update_progress(handle);
    return ret;
}

/**
  ******************************************************************
  * @brief   ymodem 发送起始帧
  * @param   [in]handle Ymodem句柄
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */ 
static void ymodem_send_start_frame(YMODEM_HANDLE_Typedef_t *handle)
{
    uint8_t buf[YMODEM_DATA_PACKAGE_SIZE_MIN] = {0};
    buf[0] = (uint8_t)YMODEM_SOH;
    buf[1] = 0;
    buf[2] = ~buf[1];
    snprintf((char *)buf+3, YMODEM_DATA_FRAME_SIZE_MIN, "%s %u", handle->file_name, handle->file_size);
    buf[3+strlen(handle->file_name)] = '\0';
    uint16_t crc = ymodem_crc16(YMODEM_CRC_DATA_OFFSET(buf), YMODEM_DATA_FRAME_SIZE_MIN);
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-2] = (uint8_t)(crc>>8);
    buf[YMODEM_DATA_PACKAGE_SIZE_MIN-1] = (uint8_t)crc&0x00FF;
    ASSERT_PAR(handle->set_reply_data_func, NULL, return);
    handle->set_reply_data_func(buf, YMODEM_DATA_PACKAGE_SIZE_MIN);
}

/**
  ******************************************************************
  * @brief   ymodem 更新进度
  * @param   [in]handle Ymodem句柄
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */ 
static void ymodem_update_progress(YMODEM_HANDLE_Typedef_t *handle)
{
    ASSERT_PAR(handle, NULL, return);
    ASSERT_PAR(handle->file_size, 0, return);
    handle->file_size_cnt = (handle->file_size_cnt > handle->file_size)?handle->file_size:handle->file_size_cnt;
    handle->current_progress = (float)handle->file_size_cnt/handle->file_size;
}

/**
  ******************************************************************
  * @brief   ymodem 更新运行步骤
  * @param   [in]handle Ymodem句柄
  * @param   [in]step 步骤
  * @author  aron566
  * @version V1.0
  * @date    2021-01-15
  ******************************************************************
  */
static inline void ymodem_update_run_step(YMODEM_HANDLE_Typedef_t *handle, YMODEM_TRANSFER_STEP_Typedef_t step)
{
    handle->run_step = step;
}

/**
  ******************************************************************
  * @brief   ymodem 检测是否EOT帧
  * @param   [in]data 数据
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static inline YMODEM_RUN_RESULT_Typedef_t ymodem_check_EOT(const uint8_t *data)
{
    if(data[0] == (uint8_t)YMODEM_EOT)
    {
        return YMODEM_IS_RUNNING;
    }
    return YMODEM_FRAME_NOT_MATCH;
}

/**
  ******************************************************************
  * @brief   ymodem 获得数据帧
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 数据
  * @param   [in]len 数据长度
  * @return  YMODEM_SUCCESSFUL 获取成功.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static YMODEM_RUN_RESULT_Typedef_t ymodem_get_data_frame(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    if(len < YMODEM_DATA_PACKAGE_SIZE_MIN)
    {
        return YMODEM_FRAME_NOT_FULL;
    }
    uint16_t size =  YMODEM_GET_FRAME_SIZE(data);
    uint16_t data_len = YMODEM_GET_FRAME_DATA_SIZE(data);
    if(YMODEM_IS_HEADER(data))
    {
        if(len < size)
        {
            return YMODEM_FRAME_NOT_FULL;
        }
        /*crc verify*/
        if(ymodem_crc_check(data, size) == true)
        {
            ASSERT_PAR(handle->set_rec_data_func, NULL, return YMODEM_FRAME_NOT_MATCH);
            handle->set_rec_data_func(data+3, (uint32_t)data_len, data[1]);
            handle->file_size_cnt += data_len;
            ymodem_update_progress(handle);
            return YMODEM_IS_RUNNING;
        }
    }
    return YMODEM_FRAME_NOT_MATCH;
}

/**
  ******************************************************************
  * @brief   ymodem 解析起始帧文件名等信息
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 帧数据
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-15
  ******************************************************************
  */
static inline void ymodem_get_file_name(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data)
{
    strncpy(handle->file_name, (const char *)data+3, FILE_NAME_LEN_MAX);
    size_t pos = get_ch_index_of_str((const char *)data+3, ' ' ,FILE_NAME_LEN_MAX*2);
    if(pos != 0)
    {
        data[pos+3] = '\0';
    }
    for(uint8_t index = 0; index < YMODEM_DATA_FRAME_SIZE_MIN; index++)
    {
        if(data[3+index] == '\0')
        {
            handle->file_name[index] = '\0';
            
            handle->file_size = (uint32_t)atoi((const char *)data+4+index);
            return;
        }
    }
    handle->file_name[FILE_NAME_LEN_MAX-1] = '\0';
    handle->file_size = 0;
}

/**
  ******************************************************************
  * @brief   ymodem 检测结束帧
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 帧数据
  * @param   [in]len 数据大小
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_stop_SOH(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    UNUSED(handle);
    if(YMODEM_RECEIVER_MIN_DATA_SIZE_CHECK(len) == 0)
    {
        return YMODEM_FRAME_NOT_FULL;
    }
    uint16_t size = YMODEM_GET_FRAME_SIZE(data);
    /*检测是否是ymodem帧数据*/
    if(YMODEM_IS_END_FRAME(data))
    {
        if(ymodem_crc_check(data, size) == false)
        {
            return YMODEM_FRAME_NOT_MATCH;
        }
        return YMODEM_IS_RUNNING;
    }
    return YMODEM_FRAME_NOT_MATCH;
}

/**
  ******************************************************************
  * @brief   ymodem 检测起始帧
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 帧数据
  * @param   [in]len 数据大小
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static YMODEM_RUN_RESULT_Typedef_t ymodem_check_start_SOH(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    uint32_t index = 0;
    for(; index < len; index++)
    {
        if(data[index] == (uint8_t)YMODEM_SOH)
        {
            break;
        }
    }
    uint16_t size = len - index;
    if(YMODEM_RECEIVER_MIN_DATA_SIZE_CHECK(size) == 0)
    {
        return YMODEM_FRAME_NOT_FULL;
    }
    /*检测是否是ymodem帧数据*/
    uint8_t *data_offset = data+index;
    if(YMODEM_IS_START_FRAME(data_offset))
    {
        if(ymodem_crc_check(data_offset, YMODEM_DATA_PACKAGE_SIZE_MIN) == false)
        {
            return YMODEM_FRAME_NOT_MATCH;
        }
        ymodem_get_file_name(handle, data_offset);
        return YMODEM_IS_RUNNING;
    }
    return YMODEM_FRAME_NOT_MATCH;
}

/**
  ******************************************************************
  * @brief   ymodem 发送标识
  * @param   [in]handle Ymodem句柄
  * @param   [in]flag 标识数据
  * @param   [in]froce 强制发送
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-15
  ******************************************************************
  */
static void ymodem_send_flag_step(YMODEM_HANDLE_Typedef_t *handle, YMODEM_CHAR_FLAG_Typedef_t flag, bool froce)
{
    if(ymodem_current_time_sec != handle->time_cnt || froce == true)
    {
        handle->set_reply_data_func((uint8_t *)&flag, 1);
        handle->time_cnt = ymodem_current_time_sec;
    }
}

/**
  ******************************************************************
  * @brief   ymodem 检测中止帧
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 帧数据
  * @param   [in]len 数据大小
  * @return  false 不是中止帧.
  * @author  aron566
  * @version V1.0
  * @date    2021-02-17
  ******************************************************************
  */
static bool ymodem_is_abort_frame(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
  if(len < YMODEM_CAN_NUMBER)
  {
    return false;
  }
  for(uint16_t i = 0; i < YMODEM_CAN_NUMBER; i++)
  {
    if(data[i] != YMODEM_CAN)
    {
      return false;
    }
  }
  return true;
}

/**
  ******************************************************************
  * @brief   ymodem 接收数据任务
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 数据
  * @param   [in]len 长度
  * @return  运行状态
  * @author  aron566
  * @version V1.0
  * @date    2021-01-15
  ******************************************************************
  */
static YMODEM_RUN_RESULT_Typedef_t ymodem_rec_task(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    YMODEM_RUN_RESULT_Typedef_t ret = YMODEM_IS_RUNNING;
    if(ymodem_is_abort_frame(handle, data, len) == true)
    {
      ymodem_update_run_step(handle, YMODEM_NO_RUNNING);
      return YMODEM_ABORT_ERROR;
    }
    switch(handle->run_step)
    {
        case YMODEM_NO_RUNNING:
            handle->last_time = ymodem_current_time_sec;
            ymodem_update_run_step(handle, YMODEM_START_SEND_C_LOOP);
            break;
        case YMODEM_START_SEND_C_LOOP:
        case YMODEM_WAIT_START_SOH:
            ASSERT_PAR(handle->set_reply_data_func, NULL, return YMODEM_ABORT_ERROR);
            ymodem_send_flag_step(handle, YMODEM_C, false);
            if((ret = ymodem_check_start_SOH(handle, data, len)) == YMODEM_IS_RUNNING)
            {
                ymodem_update_run_step(handle, YMODEM_REPLY_START_SOH_ACK);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_REPLY_START_SOH_ACK:
            ymodem_send_flag_step(handle, YMODEM_ACK, true);
            ymodem_update_run_step(handle, YMODEM_SEND_SIGNAL_ONE_C);
            break;
        case YMODEM_SEND_SIGNAL_ONE_C:
            ymodem_send_flag_step(handle, YMODEM_C, true);
            ymodem_update_run_step(handle, YMODEM_WAIT_DATA_FRAME);
            break;
        case YMODEM_WAIT_DATA_FRAME:
        case YMODEM_REPLY_DATA_ACK:
        case YMODEM_WAIT_START_EOT_FLAG:
        case YMODEM_REPLY_START_EOT_NAK:
            if((ret = ymodem_check_EOT(data)) == YMODEM_IS_RUNNING)
            {
                ymodem_send_flag_step(handle, YMODEM_NAK, true);
                ymodem_update_run_step(handle, YMODEM_WAIT_END_EOT_FLAG);
                handle->last_time = ymodem_current_time_sec;
            }
            else if((ret = ymodem_get_data_frame(handle, data, len)) == YMODEM_IS_RUNNING)
            {
                ymodem_send_flag_step(handle, YMODEM_ACK, true);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_WAIT_END_EOT_FLAG:
        case YMODEM_REPLY_END_EOT_ACK:
            if((ret = ymodem_check_EOT(data)) == YMODEM_IS_RUNNING)
            {
                ymodem_send_flag_step(handle, YMODEM_ACK, true);
                ymodem_update_run_step(handle, YMODEM_SEND_ONE_C_END);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_SEND_ONE_C_END:
            ymodem_send_flag_step(handle, YMODEM_C, true);
            ymodem_update_run_step(handle, YMODEM_WAIT_END_SOH);
            handle->last_time = ymodem_current_time_sec;
            break;
        case YMODEM_WAIT_END_SOH:
        case YMODEM_REPLY_END_SOH_ACK:
            if((ret = ymodem_check_stop_SOH(handle, data, len)) == YMODEM_IS_RUNNING)
            {
                ymodem_update_run_step(handle, YMODEM_COMPLETE);
                ymodem_send_flag_step(handle, YMODEM_ACK, true);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_COMPLETE:
            return YMODEM_SUCCESSFUL;
        default:
            break;
    }
    /*进度未变则检测超时*/
    if(handle->run_step == handle->last_run_step)
    {
        if((ymodem_current_time_sec - handle->last_time) > handle->set_time_out)
        {
            ymodem_update_run_step(handle, YMODEM_COMPLETE);
            return YMODEM_RUN_TIMEOUT;
        }
    }
    handle->last_run_step = handle->run_step;
    return ret;
}

/**
  ******************************************************************
  * @brief   ymodem 发送数据任务
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 数据
  * @param   [in]len 长度
  * @return  运行状态
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */
static YMODEM_RUN_RESULT_Typedef_t ymodem_send_task(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    switch(handle->run_step)
    {
        case YMODEM_NO_RUNNING:
            handle->last_time = ymodem_current_time_sec;
            ymodem_update_run_step(handle, YMODEM_WAIT_START_C);
            break;
        case YMODEM_WAIT_START_C:      /**< 等待起始 C字符*/
            if(data[0] == (uint8_t)YMODEM_C)
            {
                handle->last_time = ymodem_current_time_sec;
                ymodem_update_run_step(handle, YMODEM_SEND_START_FRAME);
            }
            break;
        case YMODEM_SEND_START_FRAME:  /**< 发送起始帧*/
            ymodem_send_start_frame(handle);
            ymodem_update_run_step(handle, YMODEM_WAIT_START_ACK);
            handle->last_time = ymodem_current_time_sec;
            break;
        case YMODEM_WAIT_START_ACK:    /**< 等待ACK*/
            if(data[0] == (uint8_t)YMODEM_ACK)
            {
                ymodem_update_run_step(handle, YMODEM_WAIT_START_REC);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_WAIT_START_REC:    /**< 等待接收C字符发送数据起始*/
            if(data[0] == (uint8_t)YMODEM_C)
            {
                ymodem_update_run_step(handle, YMODEM_SEND_DATA_FRAME);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_SEND_DATA_FRAME:
            if(ymodem_req_send_file_data(handle) == YMODEM_SUCCESSFUL)
            {
                ymodem_update_run_step(handle, YMOMDE_SEND_EOT_FRAME);
                break;
            }
            ymodem_update_run_step(handle, YMODEM_WAIT_DATA_ACK);
            handle->last_time = ymodem_current_time_sec;
            break;
        case YMODEM_WAIT_DATA_ACK:
            if(data[0] == (uint8_t)YMODEM_ACK)
            {
                ymodem_update_run_step(handle, YMODEM_SEND_DATA_FRAME);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMOMDE_SEND_EOT_FRAME:
            ASSERT_PAR(handle->set_reply_data_func, NULL, return YMODEM_ABORT_ERROR);
            ymodem_update_run_step(handle, YMODEM_WAIT_NAK);
            ymodem_send_flag_step(handle, YMODEM_EOT, true);
            handle->last_time = ymodem_current_time_sec;
            break;
        case YMODEM_WAIT_NAK:
        case YMODEM_SEND_EOT_END:
            if(data[0] == (uint8_t)YMODEM_NAK)
            {
                ymodem_update_run_step(handle, YMODEM_WAIT_EOT_ACK);
                ymodem_send_flag_step(handle, YMODEM_EOT, true);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_WAIT_EOT_ACK:
            if(data[0] == (uint8_t)YMODEM_ACK)
            {
                ymodem_update_run_step(handle, YMODEM_WAIT_REC_END_C);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_WAIT_REC_END_C:    /**< 等待接收结束帧信号*/
        case YMODEM_SEND_END_FRAME:
            if(data[0] == (uint8_t)YMODEM_C)
            {
                ymodem_update_run_step(handle, YMODEM_WAIT_END_ACK);
                ymodem_send_end_frame(handle);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_WAIT_END_ACK:
            if(data[0] == (uint8_t)YMODEM_ACK)
            {
                ymodem_update_run_step(handle, YMODEM_COMPLETE);
                handle->last_time = ymodem_current_time_sec;
            }
            break;
        case YMODEM_COMPLETE:
            return YMODEM_SUCCESSFUL;
        default:
            break;
    }
    /*进度未变则检测超时*/
    if(handle->run_step == handle->last_run_step)
    {
        if((ymodem_current_time_sec - handle->last_time) > handle->set_time_out)
        {
            ymodem_update_run_step(handle, YMODEM_COMPLETE);
            return YMODEM_RUN_TIMEOUT;
        }
    }
    handle->last_run_step = handle->run_step;
    return YMODEM_IS_RUNNING;
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
  * @brief   ymodem ms计时更新
  * @param   [in]None.
  * @return  ms
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */ 
void ymodem_update_time_ms_Port(void)
{
    ymodem_current_time_ms++;  
    if(ymodem_current_time_ms == 1000)
    {
        ymodem_current_time_ms = 0;
        ymodem_current_time_sec++;
    }
}

/**
  ******************************************************************
  * @brief   ymodem 发送信息设置
  * @param   [in]handle Ymodem句柄
  * @param   [in]file_name 文件名
  * @param   [in]file_size 文件大小
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-16
  ******************************************************************
  */ 
void ymodem_set_send_file_info(YMODEM_HANDLE_Typedef_t *handle, const char *file_name, uint32_t file_size)
{
    ASSERT_PAR(handle, NULL || file_name == NULL, return);
    strncpy(handle->file_name, file_name, FILE_NAME_LEN_MAX);
    handle->file_name[FILE_NAME_LEN_MAX-1] = '\0';
    handle->file_size = file_size;
}

/**
  ******************************************************************
  * @brief   ymodem 获得当前发送或接收进度
  * @param   [in]handle Ymodem句柄
  * @return  进度.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */ 
float ymodem_get_progress(YMODEM_HANDLE_Typedef_t *handle)
{
    ASSERT_PAR(handle, NULL, return -0.1);
    return handle->current_progress;
}

/**
  ******************************************************************
  * @brief   ymodem 获取文件名及大小
  * @param   [in]handle Ymodem句柄
  * @param   [out]file_size 协议帧
  * @return  文件名.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */  
const char *ymodem_get_file_info(YMODEM_HANDLE_Typedef_t *handle, uint32_t *file_size)
{
    ASSERT_PAR(handle, NULL || file_size == NULL, return NULL);
    *file_size = handle->file_size;
    return (const char*)handle->file_name;
}

/**
  ******************************************************************
  * @brief   ymodem 任务启动
  * @param   [in]handle Ymodem句柄
  * @param   [in]data 协议帧
  * @param   [in]len 数据长度
  * @return  YMODEM_SUCCESSFUL 成功.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */  
YMODEM_RUN_RESULT_Typedef_t ymodem_task_run(YMODEM_HANDLE_Typedef_t *handle, uint8_t *data, uint32_t len)
{
    YMODEM_RUN_RESULT_Typedef_t ret = YMODEM_SUCCESSFUL;
    ASSERT_PAR(handle, NULL, return YMODEM_UNKNOW_ERROR);

    /*判断运行模式*/
    switch(handle->set_mode)
    {
        case YMODEM_REC_MODE:
            ret = ymodem_rec_task(handle, data, len);
            break;
        case YMODEM_SEND_MODE:
            ret = ymodem_send_task(handle, data, len);
            break;
        default:
            return YMODEM_UNKNOW_ERROR;
    }
    return ret;
}

/**
  ******************************************************************
  * @brief   ymodem 接收者接收数据
  * @param   [in]handle Ymodem句柄
  * @param   [in]mode 模式功能选择
  * @param   [in]rec_func 接收回调
  * @param   [in]send_func 发送回调
  * @param   [in]time_out 超时时间Sec
  * @return  true 成功.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-12
  ******************************************************************
  */   
bool ymodem_init(YMODEM_HANDLE_Typedef_t *handle, YMODEM_MODE_Typedef_t mode, YMODEM_REC_DATA_CALLBACK rec_func
                  ,YMODEM_SEND_DATA_CALLBACK send_func, YMODEM_REPLY_DATA_CALLBACK reply_func, uint32_t time_out)
{
    ASSERT_PAR(handle, NULL || (rec_func == NULL && send_func == NULL), return false);
    memset(handle, 0, sizeof(YMODEM_HANDLE_Typedef_t));
    handle->set_time_out = time_out;
    handle->set_mode = mode;
    handle->run_step = YMODEM_NO_RUNNING;
    handle->set_rec_data_func = rec_func;
    handle->set_send_data_func = send_func;
    handle->set_reply_data_func = reply_func;
    return true;
}

#ifdef __cplusplus ///<end extern c                                             
}                                                                               
#endif                                                                          
/******************************** End of file *********************************/
