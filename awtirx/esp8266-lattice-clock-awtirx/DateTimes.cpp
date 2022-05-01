#include "DateTimes.h"

DateTimes::DateTimes()
{
  Wire.begin(D3, D1);
  sensor.begin();
}

/**
 * @brief 获取时间信息
 *
 * @return Times
 */
Times DateTimes::getTimes()
{
  datetime = DateTime(currtimestamp);
  times.h = datetime.hour();
  times.m = datetime.minute();
  times.s = datetime.second();
  return times;
}

/**
 * @brief 获取日期信息
 *
 * @return Dates
 */
Dates DateTimes::getDates()
{
  datetime = DateTime(currtimestamp);
  dates.y = datetime.year();
  dates.m = datetime.month();
  dates.d = datetime.day();
  return dates;
}

/**
 * @brief 获取温度信息
 *
 * @return int
 */
int DateTimes::getTemperature()
{
  if (sensor.measure())
  {
    float temperature = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    Serial.print("获取到的温度为 (°C): ");
    Serial.println(temperature);
    Serial.print("获取到的湿度为 (%RH): ");
    Serial.println(humidity);
    return (int)temperature * 100;
  }
  else
  {
    return 88 * 100;
  }
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
}

/**
 * @brief 获取时间戳信息
 *
 * @return long
 */
long DateTimes::getTimestamp() { return currtimestamp; }

/**
 * @brief 系统时间戳++
 *
 */
void DateTimes::timestampAdd() { currtimestamp++; }

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
  EEPROMTool.saveData(arr, COUNTDOWN_TIME, 5);
}

/**
 * @brief 获取倒计时时间戳信息
 *
 * @return long
 */
long DateTimes::getCountdownTimestamp()
{
  long timestamp = 0;
  uint8_t *temp = EEPROMTool.loadData(COUNTDOWN_TIME, 5);
  for (int i = 0; i < 5; i++)
  {
    timestamp += temp[i] << (i * 8);
  }
  // 用完以后删除内存
  free(temp);
  return timestamp;
}
