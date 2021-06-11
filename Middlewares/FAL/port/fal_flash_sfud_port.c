/**
 *  @file fal_flash_sfud_port.c
 *
 *  @date 2021-05-14
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 外部Flash FAL分区实现
 *
 *  @details 1、提供对FAL组件操作接口
 *
 *  @version V1.0
 */
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#define FAL_USING_SFUD_PORT
#ifdef FAL_USING_SFUD_PORT
#include <fal.h>
#include <sfud.h>
#ifdef RT_USING_SFUD
  #include <spi_flash_sfud.h>
#endif
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Private typedef ----------------------------------------------------------*/
/** Private macros -----------------------------------------------------------*/
#ifndef FAL_USING_NOR_FLASH_DEV_NAME
  #define FAL_USING_NOR_FLASH_DEV_NAME             "nor_flash0"
#endif
/** Private constants --------------------------------------------------------*/

/** Private variables --------------------------------------------------------*/
static sfud_flash_t sfud_dev = NULL;
/** Private function prototypes ----------------------------------------------*/
static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);
/** Public variables ---------------------------------------------------------*/
struct fal_flash_dev nor_flash0 =
{
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 16 * 1024 * 1024,
    .blk_size   = 4096,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};
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
  * @brief   Flash初始化
  * @param   [in]None
  * @return  0成功
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int init(void)
{

#ifdef RT_USING_SFUD
    /* RT-Thread RTOS platform */
    sfud_dev = rt_sfud_flash_find_by_dev_name(FAL_USING_NOR_FLASH_DEV_NAME);
#else
    /* bare metal platform */
    sfud_dev = sfud_get_device(SFUD_GD25Q64C_DEVICE_INDEX);
#endif

    if (NULL == sfud_dev)
    {
        return -1;
    }

    /* update the flash chip information */
    nor_flash0.blk_size = sfud_dev->chip.erase_gran;
    nor_flash0.len = sfud_dev->chip.capacity;

    return 0;
}

/**
  ******************************************************************
  * @brief   Flash读取
  * @param   [in]None
  * @return  读取大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int read(long offset, uint8_t *buf, size_t size)
{
  assert(sfud_dev);
  assert(sfud_dev->init_ok);
  if(sfud_read(sfud_dev, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
  {
    return -1;
  }
  
  return size;
}

/**
  ******************************************************************
  * @brief   Flash写入
  * @param   [in]None
  * @return  写入大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int write(long offset, const uint8_t *buf, size_t size)
{
  assert(sfud_dev);
  assert(sfud_dev->init_ok);
  if(sfud_write(sfud_dev, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
  {
    return -1;
  }

  return size;
}

/**
  ******************************************************************
  * @brief   Flash擦除
  * @param   [in]None
  * @return  擦除大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int erase(long offset, size_t size)
{
  assert(sfud_dev);
  assert(sfud_dev->init_ok);
  if (sfud_erase(sfud_dev, nor_flash0.addr + offset, size) != SFUD_SUCCESS)
  {
    return -1;
  }

  return size;
}

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
#ifdef __cplusplus ///<end extern c
}
#endif
#endif /* FAL_USING_SFUD_PORT */
/******************************** End of file *********************************/
