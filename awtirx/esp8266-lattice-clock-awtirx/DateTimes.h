#ifndef DATETIMES_H
#define DATETIMES_H

#include "EEPROMTool.h"
#include "System.h"
#include <DS3231.h>
#include <HTU21D.h>
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

/**
 * 当前时间戳，冗余做法，即时钟芯片没有或者不生效时，使用系统的时间戳来反馈时间
 */
static long currtimestamp = 0;

class DateTimes
{
private:
  bool h12Flag, pmFlag, century = false;

  /**
   * @brief 初始化DS3231
   *
   */
  DS3231 ds3231;
  /**
   * @brief
   *
   */
  DateTime datetime;

  /**
   * @brief 温湿度传感器
   *
   */
  HTU21D sensor;

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
   * @brief 温度
   *
   */
  byte temperature = 0;
  /**
   * @brief 湿度
   *
   */
  byte humidity = 0;
  /**
   * 构造函数
   */
  DateTimes();

  /**
   * @brief 初始化
   *
   */
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

  /**
   * 获取时间戳信息
   */
  long getTimestamp();

  /**
   * 获取倒计时的时间戳
   */
  long getCountdownTimestamp();

  /**
   * 保存倒计时的时间戳
   */
  void saveCountdownTimestamp(long timestamp);

  /**
   * @brief 时间戳++
   *
   */
  static void timestampAdd();
};

#endif
