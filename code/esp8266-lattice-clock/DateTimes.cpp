#include "DateTimes.h"

DateTimes::DateTimes() { Wire.begin(); }

/**
 * @brief 获取时间信息
 *
 * @return Times
 */
Times DateTimes::getTimes()
{
  times.s = ds3231.getSecond();
  times.m = ds3231.getMinute();
  times.h = ds3231.getHour(h12Flag, pmFlag);
  // 如果从芯片取到的小时数据为25的时候,表示这个数据不是从芯片里面取到的,所以就需要换成我们手动的
  if (times.h == 25)
  {
    datetime = DateTime(currtimestamp);
    times.h = datetime.hour();
    times.m = datetime.minute();
    times.s = datetime.second();
  }
  return times;
}

/**
 * @brief 获取日期信息
 *
 * @return Dates
 */
Dates DateTimes::getDates()
{
  dates.y = 2000 + ds3231.getYear();
  dates.m = ds3231.getMonth(century);
  dates.d = ds3231.getDate();
  // 如果从芯片取到的年数据为85的时候,表示这个数据不是从芯片里面取到的,所以就需要换成我们手动的
  if (dates.m == 85)
  {
    datetime = DateTime(currtimestamp);
    dates.y = datetime.year();
    dates.m = datetime.month();
    dates.d = datetime.day();
  }
  return dates;
}

/**
 * @brief 获取温度信息
 *
 * @return int
 */
int DateTimes::getTemperature()
{
  // 如果从芯片取到的年数据为85的时候,表示这个数据不是从芯片里面取到的,所以就需要换成我们手动的
  if (ds3231.getMonth(century) == 85)
  {
    return 8888; // 假如你读取到的温度信息为88.88的时候,说明你设备没有用到时钟芯片提供的时间信息
  }
  return ds3231.getTemperature() * 100;
}

/**
 * @brief 给时钟芯片和系统时间设置时间信息
 *
 * @param timestamp
 */
void DateTimes::setDateTimes(long timestamp)
{
  currtimestamp = timestamp;
  datetime = DateTime(timestamp);
  ds3231.setYear(datetime.year() % 100);
  ds3231.setMonth(datetime.month());
  ds3231.setDate(datetime.day());
  ds3231.setHour(datetime.hour());
  ds3231.setMinute(datetime.minute());
  ds3231.setSecond(datetime.second());
}

/**
 * @brief 获取时间戳信息
 *
 * @return long
 */
long DateTimes::getTimestamp()
{
  // 如果从芯片取到的年数据为85的时候,表示这个数据不是从芯片里面取到的,所以就需要换成我们手动的
  if (ds3231.getMonth(century) == 85)
  {
    return currtimestamp;
  }
  return RTClib::now().unixtime();
}

/**
 * @brief 系统时间戳++
 *
 */
void DateTimes::timestampAdd()
{
  currtimestamp++;
}

/**
 * @brief 保存倒计时时间戳信息
 *
 * @param timestamp
 */
void DateTimes::saveCountdownTimestamp(long timestamp)
{
  uint8_t arr[5];
  for (int i = 0; i < 5; i++)
  {
    arr[i] = (timestamp & 0xff);
    timestamp >>= 8;
  }
  EEPROMTool.saveData(arr, 103, 5);
}

/**
 * @brief 获取倒计时时间戳信息
 *
 * @return long
 */
long DateTimes::getCountdownTimestamp()
{
  long timestamp = 0;
  uint8_t *temp = EEPROMTool.loadData(103, 5); // 这里的103处理的不得当,后续优化,但是不影响实际功能
  for (int i = 0; i < 5; i++)
  {
    timestamp += temp[i] << (i * 8);
  }
  // 用完以后删除内存
  free(temp);
  return timestamp;
}
