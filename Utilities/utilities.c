/**
 *  @file utilities.c
 *
 *  @date 2021-01-06
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 辅助工具
 *
 *  @details 1、
 *
 *  @version V1.0
 */
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Includes -----------------------------------------------------------------*/
#if USE_MCU_EMBEDED_DEVICE
  #include "main.h"
#endif
/* Private includes ----------------------------------------------------------*/
#include "utilities.h"
/** Private typedef ----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/
/**
 * @name CPU频率
 * @{
 */
#define CPU_FREQUENCY_MHZ 480
/** @}*/

/**
 * @name Base64
 * @{
 */
#define WHITESPACE        64
#define EQUALS            65
#define INVALID           66
/** @}*/
/** Private constants --------------------------------------------------------*/
/**
 * @name Base64表
 * @{
 */
static const unsigned char dec[] =
{
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
  61, 66, 66, 66, 65, 66, 66, 66,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66,
  66, 66, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
  43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66
};
static const char enc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/** @}*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/*进度条打印*/
static char sharp_style_buf[302] = {'\0'};
static char python_progress_style_buf[3*100+4] = {'\0'};

/*计算大小*/
static const char *sizes_str[] = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t exbibytes = 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;
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
 * @brief 16进制格式调试打印
 * 
 * @param msg 
 * @param msg_len 
 */
void debug_print(uint8_t *msg, uint32_t msg_len)
{
  for (uint32_t i = 0; i < msg_len; i++)
  printf("%02X ",
      msg[i]); //打印2位的16进制数，不足位0补齐。
  printf("\n");
}

/**
 * @brief 计算大小输出大小字符串
 * 
 * @param size 
 * @return char* 
 */
char *calculateSize(uint64_t size)
{
  static char result[20] = {0};
  uint64_t  multiplier = exbibytes;
  int i;
  
  for(i = 0; i < GET_ARRAY_SIZE(sizes_str); i++, multiplier /= 1024)
  {
    if (size < multiplier)
    {
      continue;
    }
    if (size % multiplier == 0)
    {
      sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes_str[i]);
    }
    else
    {
      sprintf(result, "%.1f %s", (float) size / multiplier, sizes_str[i]);
    }
    return result;
  }
  strcpy(result, "0");
  return result;
}

/**
 * @brief 安全字符串拷贝
 * 
 * @param dest_str 目标存储区
 * @param src_str 源字符串
 * @param size 最大拷贝字符数目
 * @return char* 目标存储区
 */
char *strncopy(char *dest_str, const char *src_str, size_t size)
{
  if(size == 0 || dest_str == NULL || src_str == NULL)
  {
    return dest_str;
  }
  strncpy(dest_str, src_str, size);
  dest_str[size-1] = '\0';
  return dest_str;
}

/**
	******************************************************************
	* @brief   us级延时
	* @param   [in]nTime 延时us数
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2020-5-18
	******************************************************************
	*/
#if USE_MCU_EMBEDED_DEVICE
void delay_xus(uint32_t nTime)
{
  int old_val,new_val,val;

  if(nTime > 900)
  {
    for(old_val = 0; old_val < nTime/900; old_val++)
    {
      delay_xus(900);
    }
    nTime = nTime%900;
  }

  old_val = SysTick->VAL;
  new_val = old_val - CPU_FREQUENCY_MHZ*nTime;
  if(new_val >= 0)
  {
    do
    {
      val = SysTick->VAL;
    }
    while((val < old_val)&&(val >= new_val));
  }
  else
  {
    new_val += CPU_FREQUENCY_MHZ*1000;
    do
    {
      val = SysTick->VAL;
    }
    while((val <= old_val)||(val > new_val));
  }
}
#endif
/**
	******************************************************************
	* @brief   打印进度条
	* @param   [in]process 进度
	* @param   [in]total 总进度
  * @param   [in]style 显示样式
  * @param   [in]reset_display_flag 重置显示
	* @retval  None
	* @author  aron566
	* @version V1.0
	* @date    2021-2-12
	******************************************************************
	*/
void printf_progress_bar(size_t process, size_t total, PROGRESS_STYLE_Typedef_t style, bool reset_display_flag)
{
  static int last_count = -1;
  const char* stat = {"-\\|/"};
  int current_count = (process * 100) / total;
  if(current_count < 0)
  {
    return;
  }
  if(reset_display_flag == true)
  {
    memset(python_progress_style_buf, 0, 3*100+4);
    memset(sharp_style_buf, 0, 302);
    return;
  }
  /*进度不同则刷新输出*/
  if(last_count != current_count)
  {
    last_count = current_count;
    if(style == PROGRESS_PYTHON_STYLE)
    {
      /*类python进度条，每打印一个字符补偿2个字符否则无法左对齐103等宽*/
//      strcat(python_progress_style_buf, "█");
//      printf("[%-*s][%%%d]%c\r" ,current_count*2+103 ,python_progress_style_buf ,current_count ,stat[current_count%4]);
    }
    
    if(style == SHARP_CHAR_STYLE)
    {
      /*普通进度条，固定左对齐101等宽*/
      sharp_style_buf[current_count] = '#';/*替换█--Alt+9609~9600*/
      printf("[%-101s][%%%d]%c\r" ,sharp_style_buf ,current_count ,stat[current_count%4]);
    }
  }
#if !defined (__CC_ARM)
    fflush(stdout);
#endif
}

/**
  ******************************************************************
  * @brief   获取数值对应的字符串
  * @param   [out]dest_str 字符串存储区
  * @param   [in]data 数值
  * @param   [in]size 存储区长度
  * @param   [in]value_type 数值类型
  * @return  str.
  * @author  aron566
  * @version V1.0
  * @date    2020-12-08
  ******************************************************************
  */
char *get_value_str(char *dest_str, void *data, size_t size, VALUE_Type_t value_type)
{
  if(dest_str == NULL || data == NULL || size <= 0 || value_type == VALUE_TYPE_MAX)
  {
    return dest_str;
  }
  switch(value_type)
  {
    case INT8:
      snprintf(dest_str, size, "%hhd", *(int8_t*)data);
      break;
    case INT16:
      snprintf(dest_str, size, "%hd", *(int16_t*)data);
      break;
    case INT32:
      snprintf(dest_str, size, "%d", *(int32_t*)data);
    case INT64:
      snprintf(dest_str, size, "%lld", *(int64_t*)data);
      break;
    case UINT8:
      snprintf(dest_str, size, "%hhu", *(uint8_t*)data);
      break;
    case UINT16:
      snprintf(dest_str, size, "%hu", *(uint16_t*)data);
      break;
    case UINT32:
      snprintf(dest_str, size, "%u", *(uint32_t*)data);
      break;
    case UINT64:
      snprintf(dest_str, size, "%llu", *(uint64_t*)data);
      break;
    case FLOAT32:
      snprintf(dest_str, size, "%f", *(float*)data);
      break;
    case DOUBLE:      
      snprintf(dest_str, size, "%lf", *(double*)data);
      break;
    case STRING:
      snprintf(dest_str, size, "%s", (const char*)data);
      break;
    default:
      break;
  }
  return dest_str;
}

/**
 * @brief Get the curent time s object
 * 
 * @return uint64_t 时间秒
 */
uint64_t get_current_time_s(UTILITIES_TIME_MODE_Typedef_t mode)
{
  struct tm nowTime;
  time_t sec = 0;
  char str[256];
#if defined(__linux__)
  struct timespec timespe;
  struct timeval tv;
  switch(mode)
  {
    case CURRENT_TIME:
      clock_gettime(CLOCK_REALTIME, &timespe);
      break;
    case RUN_TIME:
      clock_gettime(CLOCK_MONOTONIC, &timespe);
      break;
    case PROCESS_CPUTIME:
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timespe);
      break;
    case THREAD_CPUTIME:
      clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timespe);
      break;
    case CURRENT_TIME_MS:
      gettimeofday(&tv,NULL);
      break;
  }
  sec = timespe.tv_sec;
#elseif defined (__GNUC__) || defined (__IAR_SYSTEMS_ICC__)
	sec = time(NULL);
#endif
  if(mode != CURRENT_TIME_MS)
  {
#if defined (__IAR_SYSTEMS_ICC__)
    memmove(&nowTime, localtime(&sec), sizeof(struct tm));
#else
    localtime_r(&sec, &nowTime);
#endif
    sprintf(str, "GetTime:%04d-%02d-%02d-%02d:%02d:%02d\n"
 ,     nowTime.tm_year + 1900, nowTime.tm_mon+1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    // printf("%s", str);
    return (uint64_t)sec;
  }
  // printf("second:%ld\n", tv.tv_sec);
  // printf("millisecond:%ld\n", tv.tv_sec*1000 + tv.tv_usec/1000);
  // printf("microsecond:%ld\n", tv.tv_sec*1000000 + tv.tv_usec);
  return (uint64_t)(sec*1000);
}

/**
 * @brief Get the time str object
 * 
 * @param sec 
 * @return const char* 
 */
const char *get_time_str(time_t sec)
{
  static char date_str[100] = {0};
  struct tm nowTime;
#if defined (__IAR_SYSTEMS_ICC__)
    memmove(&nowTime, localtime(&sec), sizeof(struct tm));
#else
    localtime_r(&sec, &nowTime);
#endif
  sprintf(date_str, "%04d-%02d-%02d-%02d:%02d:%02d"
 ,     nowTime.tm_year + 1900, nowTime.tm_mon+1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
  return (const char*)date_str;
}

/**
 * @brief 16进制字符转为数值
 * 
 * @param ch 16进制字符
 * @return uint8_t 数值
 */
uint8_t hex_char_to_value(uint8_t ch)
{
  uint8_t result = 0;
  /*获取16进制的高字节位数据*/
  if (ch >= '0' && ch <= '9')
  {
    result = ch - '0';
  }
  else if (ch >= 'a' && ch <= 'z')
  {
    result = ch - 'a' + 10;
  }
  else if (ch >= 'A' && ch <= 'Z')
  {
    result = ch - 'A' + 10;
  }
  else
  {
    result = 0;
  }
  return result;
}

/**
 * @brief 16进制转字符串
 * 
 * @param strbuf 字符串存储区
 * @param hex_data 16进制数据
 * @param len hex数据字节数
 * @return None 
 */
void hex_to_str(char *strbuf, uint8_t *hex_data, uint32_t len)
{
  if(strbuf == NULL || hex_data == NULL)
  {
    return;
  }
  uint32_t str_offset = 0;
  for(uint32_t index = 0;index < len;index++)
  {
    snprintf(strbuf+str_offset, 3, "%02X", hex_data[index]);
    str_offset +=2;
  }
  strbuf[str_offset] = '\0';
}

/**
 * @brief 将大写字母转换成小写字母
 * 
 * @param ch 大写字母
 * @return uint8_t 小写字母
 */
uint8_t ch_tolower(uint8_t ch)
{
  if(ch >= 'A' && ch <= 'Z')
  {
    return ch + 'a' - 'A';
  }
  else
  {
    return ch;
  }
}

/**
 * @brief 16进制的字符串转换成整数
 * 
 * @param s 16进制字符串
 * @return int 数值
 */
int hextoi(char s[])
{
  int i = 0;
  int ret = 0;
  if(s[0] == '0' && (s[1]=='x' || s[1]=='X'))
  {  
    i = 2;
  }
  else
  {
    i = 0;
  }
  for(;(s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)
  {
    if(ch_tolower(s[i]) > '9')
    {
      ret = 16 * ret + (10 + ch_tolower(s[i]) - 'a');
    }
    else
    {
      ret = 16 * ret + (ch_tolower(s[i]) - '0');
    }
  }
  return ret;
}

/**
 ******************************************************************
 * @brief   过滤字符串中指定字符
 * @param   [in]ch 过滤字符
 * @param   [in]str 原字符串
 * @param   [in]out_str 过滤后的字符串
 * @param   [in]size 存储区大小
 * @return  字符串长度，-1失败
 * @author  aron566
 * @version V1.0
 * @date    2020-11-24
 ******************************************************************
 */
int common_filter_special_char(char ch, const char *str, char *out_str, int size)
{
  if(str == NULL || out_str == NULL || size <= 1)
  {
    return -1;
  }
  int len = 0;
  size_t total_len = strlen(str);
  for(size_t index = 0; index < total_len; index++)
  {
    if(str[index] != ch)
    {
      out_str[len] = str[index];
      len++;
      if(len >= size)
      {
        break;
      }
    }
  }
  out_str[len-1] = '\0';
  return len;
}

/**
 ******************************************************************
 * @brief   替换字符串中指定字符
 * @param   [in]int_str 字符串
 * @param   [in]ch 原字符
 * @param   [in]dest_ch 替换后的字符
 * @param   [in]size 存储区大小
 * @return  替换字符数目，-1失败
 * @author  aron566
 * @version V1.0
 * @date    2021-03-01
 ******************************************************************
 */
int common_replace_special_char(char *int_str, char ch, char dest_ch, size_t size)
{
  if(int_str == NULL || size <= 1)
  {
    return -1;
  }
  int len = 0;
  for(size_t index = 0; index < size; index++)
  {
    if(int_str[index] == ch)
    {
      int_str[index] = dest_ch;
      len++;
      if(len >= size)
      {
        break;
      }
    }
  }
  int_str[size-1] = '\0';
  return len;
}

/**
  ******************************************************************
  * @brief   解析浮点数数据
  * @param   [in]data 完整报文，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  float
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
float common_get_float_data(uint8_t *data, int start_index)
{
  static float result_float = 0.0;
  if(data == NULL)
  {
    return result_float;
  }
  uint32_t result_temp = data[start_index];
  result_temp |= (((uint32_t)data[start_index+1])<<8);
  result_temp |= (((uint32_t)data[start_index+2])<<16);
  result_temp |= (((uint32_t)data[start_index+3])<<24);
  result_float = *(float*)&result_temp;
  return result_float;
}

/**
  ******************************************************************
  * @brief   解析整形数据
  * @param   [in]data 完整报文 ，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  uint16_t
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
uint16_t common_get_u16_data(uint8_t *data, int start_index)
{
  static uint16_t result_u16 = 0;
  if(data == NULL)
  {
    return result_u16;
  }
  result_u16 = data[start_index];
  result_u16 |= (((uint16_t)data[start_index+1])<<8);
  return result_u16;
}

/**
  ******************************************************************
  * @brief   解析整形数据
  * @param   [in]data 完整报文 ，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  uint32_t
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
uint32_t common_get_u32_data(uint8_t *data, int start_index)
{
  static uint32_t result_u32 = 0;
  if(data == NULL)
  {
    return result_u32;
  }
  result_u32 = data[start_index];
  result_u32 |= (((uint32_t)data[start_index+1])<<8);
  result_u32 |= (((uint32_t)data[start_index+2])<<16);
  result_u32 |= (((uint32_t)data[start_index+3])<<24);
  return result_u32;
}

/**
  ******************************************************************
  * @brief   解析32位整形数据
  * @param   [in]data 完整报文 ，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  uint32_t
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
uint32_t common_get_modbus_u32_data(uint8_t *data, int start_index)
{
  static uint32_t result_u32 = 0;
  if(data == NULL)
  {
    return result_u32;
  }
  result_u32 = (((uint32_t)data[start_index])<<24);
  result_u32 |= (((uint32_t)data[start_index+1])<<16);
  result_u32 |= (((uint32_t)data[start_index+2])<<8);
  result_u32 |= (((uint32_t)data[start_index+3]));
  return result_u32;
}

/**
  ******************************************************************
  * @brief   解析16位整形数据
  * @param   [in]data 完整报文 ，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  uint16_t
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
uint16_t common_get_modbus_u16_data(uint8_t *data, int start_index)
{
  static uint16_t result_u16 = 0;
  if(data == NULL)
  {
    return result_u16;
  }
  result_u16 = (((uint16_t)data[start_index])<<8);
  result_u16 |= (((uint16_t)data[start_index+1]));
  return result_u16;
}

/**
  ******************************************************************
  * @brief   解析浮点数数据
  * @param   [in]data 完整报文，当为NULL时直接返回，非NULL时解析完成后返回
  * @return  float
  * @author  aron566
  * @version V1.0
  * @date    2020-12-03
  ******************************************************************
  */
float common_get_modbus_float_data(uint8_t *data, int start_index)
{
  static float result_float = 0.0;
  if(data == NULL)
  {
    return result_float;
  }
  uint32_t result_temp = (((uint32_t)data[start_index])<<24);
  result_temp |= (((uint32_t)data[start_index+1])<<16);
  result_temp |= (((uint32_t)data[start_index+2])<<8);
  result_temp |= (((uint32_t)data[start_index+3]));
  result_float = *(float*)&result_temp;
  return result_float;
}

/**
 ******************************************************************
 * @brief   array转为base64,使用完需释放
 * @param   [in]str 原始数据
 * @return  解码后数据
 * @author  aron566
 * @version V1.0
 * @date    2020-12-14
 ******************************************************************
 */
uint8_t *base64_encode(const uint8_t *buf, uint64_t size)
{
    size_t str_len = common_b64_encodesize((size_t)size);
    uint8_t *str = (uint8_t *)calloc(str_len, sizeof(uint8_t));
    if(str == NULL)
    {
        return NULL;
    }
    bool ret = common_b64_encode(buf, (size_t)size, (char *)str, str_len);
    if(ret == false)
    {
        return NULL;
    }
    return str;
}

/**
 ******************************************************************
 * @brief   base64转为array,使用完需释放
 * @param   [in]base64数据
 * @param   [out]size 解码字节长度
 * @return  解码后数据
 * @author  aron566
 * @version V1.0
 * @date    2020-12-14
 ******************************************************************
 */
uint8_t *base64_decode(const char *base64str, uint64_t *size)
{
    size_t data_len = common_b64_maxdecodesize(base64str);
    uint8_t *binary_data = (uint8_t *)calloc(data_len, sizeof(uint8_t));
    if(binary_data == NULL)
    {
        return NULL;
    }

    bool ret = common_b64_decode(base64str, binary_data, &data_len);
    if(ret == false)
    {
        return NULL;
    }
    *size = (uint64_t)data_len;
    return binary_data;
}

/**
 * @brief Get the base64 encode size of the specified binary data
 *
 * @param binsize Size of binary data we wish to encode
 * @return        Size of base64 encoded binary data
 */
size_t common_b64_encodesize (size_t binsize)
{
  size_t result = binsize / 3 * 4;    // Four chars per three bytes
  if (binsize % 3) { result += 4; }   // Another four for trailing one or two
  return result + 1;                  // Plus '\0'
}

/**
 * @brief Get the maximum base64 decode size of the specified base64 encoded string
 *
 * @param in Pointer to base64 encoded string
 * @return   Maximum size of base64 decode
 */
size_t common_b64_maxdecodesize (const char *in)
{
  size_t inLen = strlen (in);
  return (inLen % 4) ? inLen / 4 * 3 + 2 : inLen / 4 * 3;
}

/**
 * @brief Decode base64 (encoded) string
 *
 * @param in     Pointer to base64 encoded string
 * @param out    General purpose pointer to the output of this base64 decode function
 * @param outLen Length of the decoded output
 * @return       'true' if decode successful, 'false' if decode in error
 */
bool common_b64_decode (const char *in, void *outv, size_t *outLen)
{
  uint8_t *out = (uint8_t *) outv;
  int iter = 0;
  uint32_t buf = 0;
  size_t len = 0;

  while (*in)
  {
    unsigned char c = dec[(unsigned char) (*in++)];

    if (c == WHITESPACE) continue;   // skip whitespace
    if (c == INVALID) return false;  // invalid input, return error
    if (c == EQUALS) break;          // pad character, end of data

    buf = buf << 6 | c;

    /* Every four symbols we will have filled the buffer. Split it into bytes */

    if (++iter == 4)
    {
      if ((len += 3) > *outLen) { return false; } // buffer overflow
      *(out++) = (buf >> 16) & 255;
      *(out++) = (buf >> 8) & 255;
      *(out++) = buf & 255;
      buf = 0;
      iter = 0;
    }
  }
  if (iter == 3)
  {
    if ((len += 2) > *outLen) { return false; } // buffer overflow
    *(out++) = (buf >> 10) & 255;
    *(out++) = (buf >> 2) & 255;
  }
  else if (iter == 2)
  {
    if (++len > *outLen) { return false; } // buffer overflow
    *(out++) = (buf >> 4) & 255;
  }

  *outLen = len; // modify outLen to reflect the actual output size
  return true;
}

/**
 * @brief Encode input into base64 encoded string
 *
 * @param in     General purpose pointer to input
 * @param inLen  Size of input
 * @param out    Pointer to output
 * @param outLen Length of the base64 encoded string
 * @return       'true' if encode successful, 'false' if encode in error
 */
bool common_b64_encode (const void *in, size_t inLen, char *out, size_t outLen)
{
  const uint8_t *data = (const uint8_t *) in;
  size_t resultIndex = 0;
  size_t x;
  uint32_t n = 0;
  uint8_t n0, n1, n2, n3;

  if (outLen < common_b64_encodesize (inLen))
  {
    return false;
  }

  /* iterate over the length of the string, three characters at a time */
  for (x = 0; x < inLen; x += 3)
  {
    /* combine up to three bytes into 24 bits */

    n = ((uint32_t) data[x]) << 16;
    if ((x + 1) < inLen)
    {
      n += ((uint32_t) data[x + 1]) << 8;
    }
    if ((x + 2) < inLen)
    {
      n += data[x + 2];
    }

    /* split into four 6-bit numbers */

    n0 = (uint8_t) (n >> 18) & 63;
    n1 = (uint8_t) (n >> 12) & 63;
    n2 = (uint8_t) (n >> 6) & 63;
    n3 = (uint8_t) n & 63;

    /* One byte -> two characters */

    out[resultIndex++] = enc[n0];
    out[resultIndex++] = enc[n1];

    /* Two bytes -> three characters */

    if ((x + 1) < inLen)
    {
      out[resultIndex++] = enc[n2];
    }

    /* Three bytes -> four characters */

    if ((x + 2) < inLen)
    {
      out[resultIndex++] = enc[n3];
    }
  }

  /* Pad to multiple of four characters */

  while (resultIndex % 4)
  {
    out[resultIndex++] = '=';
  }

  /* Terminate string */

  out[resultIndex] = 0;
  return true;
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
