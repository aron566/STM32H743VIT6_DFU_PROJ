/**
 *  @file UART_Port.c
 *
 *  @date 2021-01-04
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 串口操作
 *
 *  @details 1、在使用不阻塞式发送时(DMA或者中断发送)，缓冲区不可为局部变量.
 *
 *  @version V1.4
 */
#ifdef __cplusplus //use C compiler
extern "C" {
#endif
#include "UART_Port.h"/*外部接口*/
/* External variables --------------------------------------------------------*/
//extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
//extern UART_HandleTypeDef huart3;
//extern UART_HandleTypeDef huart4;
//extern UART_HandleTypeDef huart5;
//extern UART_HandleTypeDef huart6; 
//extern DMA_HandleTypeDef hdma_usart1_rx;
//extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
//extern DMA_HandleTypeDef hdma_usart3_rx;
//extern DMA_HandleTypeDef hdma_usart3_tx;
//extern DMA_HandleTypeDef hdma_usart4_rx;
//extern DMA_HandleTypeDef hdma_usart4_tx;
//extern DMA_HandleTypeDef hdma_usart5_rx;
//extern DMA_HandleTypeDef hdma_usart5_tx;
//extern DMA_HandleTypeDef hdma_usart6_rx;
//extern DMA_HandleTypeDef hdma_usart6_tx;
/** Private typedef ----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
#define USE_LOOPBACK 	      0
#define DEBUG_UART		      &huart2
#define UART_MAX_NUM        7/**< 最大允许串口数量*/
#define USE_IDEL_CALLBACK   1/**< 为1 使用空闲中断不定长接收 为0 使用中断单字节接收*/
#define USE_BLOCK_SEND_MODE	1/**< 使用阻塞式发送*/
/*定义内存管理接口*/
#if !defined(uartport_malloc) && !defined(uartport_free)
  #if USE_FREERTOS
    #define uartport_malloc   pvPortMalloc
    #define uartport_free     vPortFree
  #else
    #define uartport_malloc   malloc
    #define uartport_free     free
  #endif
#endif

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*预定义串口设备信息*/
static Uart_Dev_Handle_t *Uart_pDevice[UART_MAX_NUM+1];
/** Private function prototypes ----------------------------------------------*/
static uint8_t get_uart_index(USART_TypeDef *Instance);
static Uart_Dev_Handle_t *Create_Uart_Dev(UART_NUM_Typedef_t uart_num ,UART_HandleTypeDef *huart 
                                        ,DMA_HandleTypeDef *hdma_rx, DMA_HandleTypeDef *hdma_tx, uint8_t *rx_temp_buf, uint16_t rx_temp_size 
                                        ,uint32_t rxsize ,int work_mode 
#if USE_FREERTOS
                                        ,osSemaphoreId *pRX_Sem
#endif
                                        );
/** Private user code --------------------------------------------------------*/
/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/************************************************************
  * @brief   建立串口设备，为其建立双缓冲区-->使能串口空闲中断
  * @param   [in]uart_num 串口号.
  * @param   [in]huart    串口句柄.
  * @param   [in]hdma_rx  串口dma句柄.
  * @param   [in]hdma_tx  串口dma句柄.
  * @param   [in]rx_temp_buf  临时缓冲区为NULL时自动申请使用堆内存.
  * @param   [in]rx_temp_size  串口临时缓存大小.
  * @param   [in]rxsize   串口接收缓存大小.
  * @param   [in]work_mode 串口工作模式.
  * @param   [in]pRX_Sem  串口接收信号量.  
  * @return  串口操作句柄.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
static Uart_Dev_Handle_t *Create_Uart_Dev(UART_NUM_Typedef_t uart_num ,UART_HandleTypeDef *huart 
                                        ,DMA_HandleTypeDef *hdma_rx, DMA_HandleTypeDef *hdma_tx, uint8_t *rx_temp_buf, uint16_t rx_temp_size 
                                        ,uint32_t rxsize ,int work_mode 
#if USE_FREERTOS
                                        ,osSemaphoreId *pRX_Sem
#endif
                                        )
{
    Uart_Dev_Handle_t *pUart_Dev = (Uart_Dev_Handle_t *)uartport_malloc(sizeof(Uart_Dev_Handle_t));
    pUart_Dev->phuart = huart;
    pUart_Dev->phdma_rx = hdma_rx;
	  pUart_Dev->phdma_tx = hdma_tx;
    pUart_Dev->cb = cb_create(rxsize);
    if(pUart_Dev->cb == NULL)
    {
      while(1);
    }
    pUart_Dev->MAX_RX_Temp = rx_temp_size;
    if(rx_temp_buf == NULL)
    {
      pUart_Dev->RX_Buff_Temp = (uint8_t *)uartport_malloc(sizeof(uint8_t)*rx_temp_size);
    }
    else
    {
      pUart_Dev->RX_Buff_Temp = rx_temp_buf;
    }
    if(NULL == pUart_Dev->RX_Buff_Temp)
    {
      return NULL;
    }
    pUart_Dev->Is_Half_Duplex = work_mode;
#if USE_FREERTOS
    pUart_Dev->pRX_Sem = pRX_Sem;
#endif
    Uart_pDevice[uart_num] = pUart_Dev;
#if USE_IDEL_CALLBACK
    /*打开空闲中断*/
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    /*使能DMA接收*/
    HAL_UART_Receive_DMA(huart, pUart_Dev->RX_Buff_Temp, pUart_Dev->MAX_RX_Temp);
#else
    HAL_UART_Receive_IT(huart, pUart_Dev->RX_Buff_Temp, 1);
#endif
    return pUart_Dev;
}

/************************************************************
  * @brief   获得当前串口信息索引
  * @param   [in]Instance 串口句柄.
  * @return  索引号.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
static uint8_t get_uart_index(USART_TypeDef *Instance)
{
  for(uint8_t index = 0; index < UART_MAX_NUM; index++)
  {
    if(Uart_pDevice[index] == NULL)
    {
      continue;
    }
    if(Uart_pDevice[index]->phuart->Instance == Instance)
    {
      return index;
    }  
  }
  return 0;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/************************************************************
  * @brief   Rx Transfer IRQ
  * @param   huart UART handle.
  * @return  None
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
void USER_UART_IRQHandler(UART_HandleTypeDef *huart)
{
#if USE_IDEL_CALLBACK
    uint8_t index = get_uart_index(huart->Instance);
    if(index != 0)
    {
        if((__HAL_UART_GET_FLAG(Uart_pDevice[index]->phuart ,UART_FLAG_IDLE) != RESET))
        {
          /*
          首先停止DMA传输，
          1.防止后面又有数据接收到，产生干扰，因为此时的数据还未处理。
          2.DMA需要重新配置。
          */
          HAL_UART_AbortReceive(Uart_pDevice[index]->phuart);
          /*清楚空闲中断标志，否则会一直不断进入中断*/
          __HAL_UART_CLEAR_IDLEFLAG(Uart_pDevice[index]->phuart);
          /*计算本次接收数据长度*/
          uint32_t data_length  = Uart_pDevice[index]->MAX_RX_Temp - __HAL_DMA_GET_COUNTER(Uart_pDevice[index]->phdma_rx);
          /*将数据记录至环形区*/
          CQ_putData(Uart_pDevice[index]->cb ,Uart_pDevice[index]->RX_Buff_Temp ,(uint32_t)data_length);
#if USE_LOOPBACK 	
          HAL_UART_Transmit(DEBUG_UART, (uint8_t *)Uart_pDevice[index]->RX_Buff_Temp,(uint16_t)data_length,0xFFFF);
#endif
          /*清空临时缓冲区*/
          memset(Uart_pDevice[index]->RX_Buff_Temp ,0 ,data_length);
          /*打开空闲中断*/
          __HAL_UART_ENABLE_IT(Uart_pDevice[index]->phuart ,UART_IT_IDLE);
          /*重启开始DMA传输*/
          HAL_UART_Receive_DMA(Uart_pDevice[index]->phuart ,Uart_pDevice[index]->RX_Buff_Temp ,Uart_pDevice[index]->MAX_RX_Temp);
        }
    }
#endif ///<! USE_IDEL_CALLBACK
} 

/************************************************************
  * @brief   串口数据发送，使用DMA及中断发送时缓冲区不可为局部变量
  * @param   [in]uart_opt_handle 串口句柄.
  * @param   [in]data 数据.
  * @param   [in]size 数据大小.
  * @param   [in]BlockTime 数据大小.
  * @return  true 发送成功.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
bool Uart_Port_Transmit_Data(Uart_Dev_Handle_t *uart_opt_handle, uint8_t *data, uint16_t size, uint32_t blockTime)
{
  if(uart_opt_handle == NULL)
  {
    return false;
  }
#if USE_IDEL_CALLBACK && (USE_BLOCK_SEND_MODE == 0)///<! 空闲+不阻塞
  /*如用等待DMA 空闲*/
  while(HAL_DMA_GetState(uart_opt_handle->phdma_tx) == HAL_DMA_STATE_BUSY);
  
  if(uart_opt_handle->phuart->gState != HAL_UART_STATE_READY)	
  {
    HAL_UART_AbortTransmit(uart_opt_handle->phuart);
	}
  
  /* 关闭DMA */
  __HAL_DMA_DISABLE(uart_opt_handle->phdma_tx);
  
  if(HAL_UART_Transmit_DMA(uart_opt_handle->phuart, data, size) != HAL_OK)
  {
    return false;
  }
#endif
#if USE_BLOCK_SEND_MODE///<! 阻塞
  if(HAL_UART_Transmit(uart_opt_handle->phuart, data, size, 0xFF) != HAL_OK)
  {
    return false;
  }
#elif (USE_IDEL_CALLBACK == 0) && (USE_BLOCK_SEND_MODE == 0)///<! 不使用空闲+不阻塞
  /*优先使用中断发送，但是当阻塞时间不为0时使用阻塞发送*/
  if(blockTime != 0)
  {
    if(HAL_UART_Transmit(uart_opt_handle->phuart, data, size, blockTime) != HAL_OK)
    {
      return false;
    }
    return true;
  }
  if(HAL_UART_Transmit_IT(uart_opt_handle->phuart, data, size) != HAL_OK)
  {
    return false;
  }
#endif
  return true;
}  

/************************************************************
  * @brief   串口操作句柄获取
  * @param   [in]uart_num 串口号.
  * @return  串口句柄.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
Uart_Dev_Handle_t *Uart_Port_Get_Handle(UART_NUM_Typedef_t uart_num)
{
  if(uart_num >= UART_MAX_NUM)
  {
    return NULL;
  }
  return Uart_pDevice[uart_num];
}

/************************************************************
  * @brief   串口接收中断
  * @param   [in]huart 串口句柄.
  * @return  None.
  * @author  aron566
  * @date    2021/2/3
  * @version v1.0
  * @note    @@
  ***********************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if !USE_IDEL_CALLBACK
  uint8_t index = Get_Uart_Index(huart->Instance);
  if(index != 0)
  {
    CQ_putData(Uart_pDevice[index]->cb, Uart_pDevice[index]->RX_Buff_Temp, 1);
  }
#if USE_LOOPBACK 	
  HAL_UART_Transmit(DEBUG_UART, (uint8_t *)Uart_pDevice[index]->RX_Buff_Temp, 1, 0xFF);
#endif
  HAL_UART_Receive_IT(huart, Uart_pDevice[index]->RX_Buff_Temp, 1);
#endif
}

/**
  ******************************************************************
  * @brief   设置通讯波特率
  * @param   [in]uart_num 串口设备号
  * @param   [in]BaudRate 波特率
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-03-01
  ******************************************************************
  */
void Uart_Port_Set_Baudrate(UART_NUM_Typedef_t uart_num, uint32_t BaudRate)
{
  if(uart_num >= UART_MAX_NUM)
  {
    return;
  }
  if(Uart_pDevice[uart_num] == NULL)
  {
    return;
  }
  if(Uart_pDevice[uart_num]->phuart->Init.BaudRate == BaudRate)
  {
    return;
  }
  /*等待空闲*/
  // while(UART_CheckIdleState(Uart_pDevice[uart_num]->phuart) != HAL_OK);
  // HAL_UART_Abort(Uart_pDevice[uart_num]->phuart);
  // Uart_pDevice[uart_num]->phuart->Init.BaudRate = BaudRate;
  // UART_SetConfig(Uart_pDevice[uart_num]->phuart);

  /*重置*/
  HAL_UART_Abort(Uart_pDevice[uart_num]->phuart);
  HAL_UART_DeInit(Uart_pDevice[uart_num]->phuart);
  Uart_pDevice[uart_num]->phuart->Init.BaudRate = BaudRate;
  HAL_UART_Init(Uart_pDevice[uart_num]->phuart);

  /*重新打开接收*/
#if USE_IDEL_CALLBACK
  /*打开空闲中断*/
  __HAL_UART_ENABLE_IT(Uart_pDevice[uart_num]->phuart, UART_IT_IDLE);
  /*使能DMA接收*/
  HAL_UART_Receive_DMA(Uart_pDevice[uart_num]->phuart, Uart_pDevice[uart_num]->RX_Buff_Temp, Uart_pDevice[uart_num]->MAX_RX_Temp);
#else
  HAL_UART_Receive_IT(Uart_pDevice[uart_num]->phuart, Uart_pDevice[uart_num]->RX_Buff_Temp, 1);
#endif
}

/************************************************************
  * @brief   串口硬件恢复初始化
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
void Uart_Port_Dinit(void)
{
  for(uint8_t index = 0; index < UART_MAX_NUM;index++)
  {
    if(Uart_pDevice[index] == NULL)
    {
      continue;
    }
    HAL_UART_MspDeInit(Uart_pDevice[index]->phuart);
  }
}

/************************************************************
  * @brief   串口操作初始化
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
void Uart_Port_Init(void)
{
	Create_Uart_Dev(UART_NUM_2, &huart2, &hdma_usart2_rx, &hdma_usart2_tx,
										NULL,/**< 备用自定义DMA缓冲区 防止Cache的影响*/
										128, 
										(uint32_t)CQ_BUF_256, 0
#if USE_FREERTOS
  , NULL
#endif
                                                );
    if(Uart_pDevice[UART_NUM_2] == NULL)
    {
      printf("create uart opt handle faild.\n");
    }
}

#if defined (__CC_ARM)
  #pragma import(__use_no_semihosting) // 确保没有从 C 库链接使用半主机的函数
#endif
struct __FILE // 标准库需要的支持函数
{
	int handle;
};
/* FILE is typedef ’ d in stdio.h. */
FILE __stdout;
void _ttywrch(int ch)
{
    ch=ch;
}

void _sys_exit(int x) //定义 _sys_exit() 以避免使用半主机模式
{
    (void)x;
}

#ifndef USE_SMALL_PRINTF
#include "stdio.h"
/************************************************************
  * @brief   重定向c库函数printf到HAL_UART_Transmit
  * @param   [in]ch 输出字符.
  * @param   [in]f 文件指针
  * @return  字符
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&ch, 1, 0xFF);
  ITM_SendChar(ch);
  return ch;
}

/************************************************************
  * @brief   重定向c库函数getchar,scanf
  * @param   [in]f 文件指针
  * @return  字符
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
int fgetc(FILE *f)
{
  uint8_t ch = 0;
  while(HAL_UART_Receive(DEBUG_UART, (uint8_t *)&ch, 1, 0xFF) != HAL_OK);
  return ch;
}

#else
/*small printf */
#include "stdio.h"
int __io_putchar(int ch)
{
    HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&ch ,1 ,0xFF);
    return ch;
}
int __write(int file, char *ptr, int len)
{
    int DataIdx;
    for(DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}
#endif ///<！USE_SMALL_PRINTF

#ifdef __cplusplus ///<! end extern c
}
#endif
