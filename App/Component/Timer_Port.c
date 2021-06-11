/**
 *  @file Timer_Port.c
 *
 *  @date 2021-01-01
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 定时器接口
 *
 *  @details 1、
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#include <time.h>
/* Private includes ----------------------------------------------------------*/
#include "Timer_Port.h"
#include "main.h"
#include "User_Main.h"
#include "Utilities_Multi_Timer.h"
#include "Utilities_Multi_Button.h"  
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
#define ENABLE_SYSTICK_COUNT  1/**< 是否启用Systick计数,在中断中添加调用HAL_SYSTICK_IRQHandler();方能正确启用*/
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*定时器任务句柄*/
static struct Timer timer1;
static struct Timer timer2;
static struct Timer timer3;
static struct Timer timer4;

static uint32_t Timer_Port_TimeMS = 0;
static uint32_t Timer_Port_TimeSec = 0;
/** Private function prototypes ----------------------------------------------*/
static inline void Timer_Port_IRQHandler(void);
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
  * @brief   定时器中断回调
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-13
  ******************************************************************
  */
static inline void Timer_Port_IRQHandler(void)
{
  Timer_Port_TimeMS++;
  if(Timer_Port_TimeMS == 1000)
  {
    Timer_Port_TimeMS = 0;
    Timer_Port_TimeSec++;
  }
  timer_ticks();
  ymodem_update_time_ms_Port();
}

/**
  ******************************************************************
  * @brief   定时1任务
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-02-25
  ******************************************************************
  */
static void timer1_callback(void)
{

}

/**
  ******************************************************************
  * @brief   延时任务
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2020-02-25
  ******************************************************************
  */
static void timer2_callback(void)
{
  Update_Port_Init();
}

/**
  ******************************************************************
  * @brief   定时3任务
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-04-19
  ******************************************************************
  */
static void timer3_callback(void)
{
  /*按键检测时基*/
  button_ticks();
}

/**
  ******************************************************************
  * @brief   定时4任务
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-05-10
  ******************************************************************
  */
static void timer4_callback(void)
{

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
  * @brief   上电运行时间
  * @param   [in]None
  * @return  clock_t 运行时间
  * @author  aron566
  * @version v1.0
  * @date    2021/2/1
  ******************************************************************
  */
clock_t clock(void) 
{
  return Timer_Port_TimeSec;
}

/**
  ******************************************************************
  * @brief   经过的秒数
  * @param   [in]None
  * @return  time_t 秒数
  * @author  aron566
  * @version v1.0
  * @date    2021/2/1
  ******************************************************************
  */
#if _DLIB_TIME_USES_64

time_t __time64(time_t *p) 
{
  return Timer_Port_TimeSec;
}
#else
time_t __time32(time_t *p) 
{
  return Timer_Port_TimeSec;
}
#endif

/**
  ******************************************************************
  * @brief   获取时区
  * @param   [in]None
  * @return  时区
  * @author  aron566
  * @version v1.0
  * @date    2021/2/1
  ******************************************************************
  */
const char *__getzone(void)
{
  return ": GMT+8:GMT+9:+0800";
}

/**
  ******************************************************************
  * @brief   设置时间
  * @param   [in]year
  * @param   [in]month
  * @param   [in]day
  * @param   [in]hour
  * @param   [in]min
  * @param   [in]sec
  * @return  None.
  * @author  aron566
  * @version v1.0
  * @date    2021/2/1
  ******************************************************************
  */
void Timer_Port_Set_Time(int year, int month, int day, int hour, int min, int sec)
{
  struct tm set_time;
  set_time.tm_sec = sec;
  set_time.tm_min = min;
  set_time.tm_hour = hour;
  set_time.tm_mday = day;
  set_time.tm_mon = month-1;
  set_time.tm_year = year-1900;
  //set_time.tm_wday = 1;
  //set_time.tm_yday = 2;
  set_time.tm_isdst = -1;
  Timer_Port_TimeSec = mktime(&set_time);
}

/**
  ******************************************************************
  * @brief   获取当前运行累计时间
  * @param   [in]time_base 单位
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-13
  ******************************************************************
  */
uint32_t Timer_Port_Get_Current_Time(TIMER_TIME_UNIT_Typedef_t time_unit)
{
  return (time_unit == TIMER_MS)?Timer_Port_TimeMS:Timer_Port_TimeSec;
}

/**
  ******************************************************************
  * @brief   定时器周期时间到回调
  * @param   [in]htim 定时器句柄
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2020-02-25
  ******************************************************************
  */
#if !ENABLE_SYSTICK_COUNT
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM3)
  {
    Timer_Port_IRQHandler();
  }
}
#endif

/**
  ******************************************************************
  * @brief   滴答计时回调,在中断中添加调用HAL_SYSTICK_IRQHandler()
  * @param   [in]None.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-02-24
  ******************************************************************
  */
void HAL_SYSTICK_Callback(void)
{
#if ENABLE_SYSTICK_COUNT
  Timer_Port_IRQHandler();
#endif
}

/**
  ******************************************************************
  * @brief   定时器接口启动
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-03-13
  ******************************************************************
  */
void Timer_Port_Start(void)
{
  timer_loop();
}

/**
  ******************************************************************
  * @brief   初始化定时器
  * @param   [in]None
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-01-13
  ******************************************************************
  */
void Timer_Port_Init(void)
{
  /*初始化定时任务 5s loop*/
	timer_init(&timer1, timer1_callback, 1000, 5000);
	timer_start(&timer1);
	
  /*初始化延时任务 50ms delay*/
	timer_init(&timer2, timer2_callback, 50, 0);
	timer_start(&timer2);

  /*初始化定时任务 5ms delay*/
	timer_init(&timer3, timer3_callback, 1000, 5);
	timer_start(&timer3);
  
  /*初始化定时任务 50ms delay*/
	timer_init(&timer4, timer4_callback, 1000, 50);
	timer_start(&timer4);
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
