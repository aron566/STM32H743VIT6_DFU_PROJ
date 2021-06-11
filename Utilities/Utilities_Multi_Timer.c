/**
 *  @file Utilities_Multi_Timer.c
 *
 *  @date 2021-02-22
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 定时器接口
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
#include "Utilities_Multi_Timer.h"
/** Private typedef ----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*timer handle list head*/
static struct Timer* head_handle = NULL;
/*Timer ticks*/
static uint32_t _timer_ticks = 0;

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
  * @brief  Initializes the timer struct handle.
  * @param  handle: the timer handle strcut.
  * @param  timeout_cb: timeout callback.
  * @param  repeat: repeat interval time.
  * @retval None
  */
void timer_init(struct Timer* handle, void (*timeout_cb)(), uint32_t timeout, uint32_t repeat)
{
    // memset(handle, sizeof(struct Timer), 0);
    handle->timeout_cb = timeout_cb;
    handle->timeout    = _timer_ticks + timeout;
    handle->repeat     = repeat;
}

/**
  * @brief  Start the timer work, add the handle into work list.
  * @param  btn: target handle strcut.
  * @retval 0: succeed. -1: already exist.
  */
int timer_start(struct Timer* handle)
{
    struct Timer* target = head_handle;
    while(target)
    {
        if(target == handle)
            return -1;  //already exist.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle  = handle;
    return 0;
}

/**
  * @brief  Stop the timer work, remove the handle off work list.
  * @param  handle: target handle strcut.
  * @retval None
  */
void timer_stop(struct Timer* handle)
{
    struct Timer** curr;
    for(curr = &head_handle; *curr;)
    {
        struct Timer* entry = *curr;
        if(entry == handle)
        {
            *curr = entry->next;
            //			free(entry);
        }
        else
            curr = &entry->next;
    }
}

/**
  * @brief  main loop.
  * @param  None.
  * @retval None
  */
void timer_loop()
{
    struct Timer* target;
    for(target = head_handle; target; target = target->next)
    {
        if(_timer_ticks >= target->timeout)
        {
            if(target->repeat == 0)
            {
                timer_stop(target);
            }
            else
            {
                target->timeout = _timer_ticks + target->repeat;
            }
            target->timeout_cb();
        }
    }
}

/**
  * @brief  background ticks, timer repeat invoking interval 1ms.
  * @param  None.
  * @retval None.
  */
void timer_ticks()
{
    _timer_ticks++;
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
