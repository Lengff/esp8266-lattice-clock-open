#ifndef DATETIMES_H
#define DATETIMES_H

#include <DS3231.h>
#include <Wire.h>

struct Times
{
  uint8_t h;
  uint8_t m;
  uint8_t s;
};

struct Dates
{
  uint16_t y;
  uint8_t m;
  uint8_t d;
};

class DateTimes
{
private:
  bool h12Flag, pmFlag, century = false;
  DS3231 ds3231;

  DateTime datetime;

  /**
   * @brief 时间结构体
   *
   */
  Times times;

  /**
   * @brief 日期结构体
   *
   */
  Dates dates;

public:
  /**
   * 构造函数
   */
  DateTimes();

  void initDateTimes();

  /**
   * 获取时间
   */
  Times getTimes();

  /**
   * 获取日期
   */
  Dates getDates();

  /**
   * 获取温度信息
   */
  int getTemperature();

  /**
   * 根据datetime设置时间
   */
  void setDateTimes(long timestamp);
};

#endif
