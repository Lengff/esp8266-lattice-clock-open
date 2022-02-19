#include "DateTimes.h"

DateTimes::DateTimes() { Wire.begin(); }

Times DateTimes::getTimes()
{
  times.s = ds3231.getSecond();
  times.m = ds3231.getMinute();
  times.h = ds3231.getHour(h12Flag, pmFlag);
  return times;
}

Dates DateTimes::getDates()
{
  dates.y = 2000 + ds3231.getYear();
  dates.m = ds3231.getMonth(century);
  dates.d = ds3231.getDate();
  return dates;
}

int DateTimes::getTemperature() { return ds3231.getTemperature() * 100; }

void DateTimes::setDateTimes(long timestamp)
{
  datetime = DateTime(timestamp);
  ds3231.setYear(datetime.year() % 100);
  ds3231.setMonth(datetime.month());
  ds3231.setDate(datetime.day());
  ds3231.setHour(datetime.hour());
  ds3231.setMinute(datetime.minute());
  ds3231.setSecond(datetime.second());
}

long DateTimes::getTimestamp()
{
  return RTClib::now().unixtime();
}

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
