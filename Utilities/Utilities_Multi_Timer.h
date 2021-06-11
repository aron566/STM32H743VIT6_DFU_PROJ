/**
 *  @file Utilities_Multi_Timer.h
 *
 *  @date 2021-02-22
 *
 *  @author aron566
 *
 *  @brief 定时器操作
 *  
 *  @version V1.0
 */
#ifndef UTILITIES_MULTI_TIMER_H
#define UTILITIES_MULTI_TIMER_H
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< nedd definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
#include <limits.h> /**< need variable max value    */
/** Private includes ---------------------------------------------------------*/

/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/
/** 定时器句柄*/
typedef struct Timer 
{
	uint32_t timeout;
	uint32_t repeat;
	void (*timeout_cb)(void);
	struct Timer* next;
}Timer;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*初始化定时任务*/
void timer_init(struct Timer* handle, void(*timeout_cb)(), uint32_t timeout, uint32_t repeat);
/*启动定时任务*/
int  timer_start(struct Timer* handle);
/*停止定时任务*/
void timer_stop(struct Timer* handle);
/*定时器计时*/
void timer_ticks(void);
/*定时器模块运行*/
void timer_loop(void);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
