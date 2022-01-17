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