/**
 *  @file Utilities_Multi_Button.h
 *
 *  @date 2021-03-07
 *
 *  @author aron566
 *
 *  @brief 按键接口
 *  
 *  @version V1.0
 */
#ifndef UTILITIES_MULTI_BUTTON_H
#define UTILITIES_MULTI_BUTTON_H
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
#define TICKS_INTERVAL    5	//ms
#define DEBOUNCE_TICKS    3	//MAX 8
#define SHORT_TICKS       (300 /TICKS_INTERVAL)
#define LONG_TICKS        (1000 /TICKS_INTERVAL)
/** Exported typedefines -----------------------------------------------------*/
typedef void (*BtnCallback)(void*);

/*按键事件类型*/
typedef enum {
	PRESS_DOWN = 0,                     /**< 按键按下，每次按下都触发*/
	PRESS_UP,														/**< 按键弹起，每次松开都触发*/
	PRESS_REPEAT,												/**< 重复按下触发，变量repeat计数连击次数*/
	SINGLE_CLICK,												/**< 单击按键事件*/
	DOUBLE_CLICK,												/**< 双击按键事件*/
	LONG_PRESS_START,										/**< 达到长按时间阈值时触发一次*/
	LONG_PRESS_HOLD,										/**< 长按期间一直触发*/
	number_of_event,										/**< 事件数量*/
	NONE_PRESS													/**< 未有事件*/
}PressEvent;

/*按键句柄*/
typedef struct Button {
	uint16_t ticks;
	uint8_t  repeat : 4;
	uint8_t  event : 4;
	uint8_t  state : 3;
	uint8_t  debounce_cnt : 3;
	uint8_t  active_level : 1;
	uint8_t  button_level : 1;
	uint8_t  (*hal_button_Level)(void);
	BtnCallback  cb[number_of_event];
	struct Button* next;
}Button;
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/*初始化按键*/
void button_init(struct Button* handle, uint8_t(*pin_level)(), uint8_t active_level);
/*注册按键事件*/
void button_attach(struct Button* handle, PressEvent event, BtnCallback cb);
/*获取按键事件*/
PressEvent get_button_event(struct Button* handle);
/*启动按键监测*/
int button_start(struct Button* handle);
/*停止按键监测*/
void button_stop(struct Button* handle);
/*按键监测时基*/
void button_ticks(void);

#ifdef __cplusplus ///<end extern c
}
#endif
#endif
/******************************** End of file *********************************/
