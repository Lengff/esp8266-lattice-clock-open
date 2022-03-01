#ifndef EEPROM_TOOL_H
#define EEPROM_TOOL_H

#include <EEPROM.h>

#define EEPROM_BEGIN 1024

enum EEPROM_ADDRESS_ENUM
{

  REMEMBER_WIFI = 0x01, // 记住wifi内存地址
  WIFI_MODE = 0x02,     // wifi模式内存地址
  DIRECTION = 0x03,     // 屏幕显示方向内存地址
  BRIGHTNESS = 0x04     // 屏幕亮度内存地址
};

struct EEPROMTOOLS
{
  /**
   * @brief 保存数据
   */
  void saveData(uint8_t *data, int offset, int length)
  {
    EEPROM.begin(EEPROM_BEGIN);
    for (int i = 0; i < length; i++)
    {
      EEPROM.write(offset + i, data[i] & 0xff);
    }
    EEPROM.commit();
  }
  /**
   * @brief 只保存一位数据
   *
   * @param data
   * @param offset
   */
  void saveDataOne(uint8_t data, int offset)
  {
    EEPROM.begin(EEPROM_BEGIN);
    EEPROM.write(offset, data & 0xff);
    EEPROM.commit();
  }

  /**
   * @brief 获取数据
   *
   * @param offset
   * @param length
   * @return uint8_t*
   */
  uint8_t *loadData(int offset, int length)
  {
    unsigned char *arr = new uint8_t[length];
    EEPROM.begin(EEPROM_BEGIN);
    for (int i = 0; i < length; i++)
    {
      arr[i] = EEPROM.read(offset + i);
    }
    return arr;
  }

  /**
   * @brief 获取一位数据
   */
  uint8_t loadDataOne(int offset)
  {
    unsigned char *arr = new uint8_t[1];
    EEPROM.begin(EEPROM_BEGIN);
    return EEPROM.read(offset);
  }

  /**
   * @brief 删除数据
   */
  void clearData(int offset, int length)
  {
    EEPROM.begin(EEPROM_BEGIN);
    for (int i = 0; i < length; i++)
    {
      EEPROM.write(offset + i, 0x0);
    }
    EEPROM.commit();
  }

  /**
   * @brief 删除全部数据
   */
  void clearAll()
  {
    EEPROM.begin(EEPROM_BEGIN);
    for (int i = 0; i < EEPROM_BEGIN + 300; i++)
    {
      EEPROM.write(i, 0x0);
    }
    EEPROM.commit();
  }
};

static EEPROMTOOLS EEPROMTool;

#endif