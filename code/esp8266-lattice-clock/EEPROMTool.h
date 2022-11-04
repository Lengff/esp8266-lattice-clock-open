#ifndef EEPROM_TOOL_H
#define EEPROM_TOOL_H

#include <EEPROM.h>

#define EEPROM_BEGIN 1536
#define EEPROM_LENGTH 50

/**
 * @brief todo 这里可以优化，直接改成结构体
 *
 */
enum EEPROM_ADDRESS_ENUM
{

  REMEMBER_WIFI = EEPROM_BEGIN + 1,   // 记住wifi内存地址
  WIFI_MODE = EEPROM_BEGIN + 2,       // wifi模式内存地址
  DIRECTION = EEPROM_BEGIN + 3,       // 屏幕显示方向内存地址
  BRIGHTNESS = EEPROM_BEGIN + 4,      // 屏幕亮度内存地址
  SLEEP_TIME = EEPROM_BEGIN + 5,      // 从第5位到第9位来存睡眠时间相关
  BILIBILI_UID = EEPROM_BEGIN + 10,   // 从10到14来存储bilibilii的UID
  COUNTDOWN_TIME = EEPROM_BEGIN + 15, // 从15到19来存储倒计时时间戳
  H12_ADDR = EEPROM_BEGIN + 20,       // 12位存放H12标记 \\\ 这里占位,暂无任何意义
  ANNI_DAY = EEPROM_BEGIN + 21,       // 10,11存放纪念日 月和 日 \\\ 这里占位,暂无任何意义
  LIGHT_VAL = EEPROM_BEGIN + 22      // 存放亮度的
};

struct EEPROMTOOLS
{
  /**
   * @brief 保存数据
   */
  void saveData(uint8_t *data, int offset, int length)
  {
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
    unsigned char arr = EEPROM.read(offset);
    return arr;
  }

  /**
   * @brief 删除数据
   */
  void clearData(int offset, int length)
  {
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
    EEPROM.begin(4096);
    for (int i = 0; i < 4096; i++)
    {
      EEPROM.write(i, 0x0);
    }
    EEPROM.end();
  }

  /**
   * @brief 备份自用数据到内存
   */
  uint8_t *backupAll()
  {
    uint8_t *arr = new uint8_t[EEPROM_LENGTH];
    EEPROM.begin(2048);
    for (int i = 0; i < EEPROM_LENGTH; i++)
    {
      arr[i] = EEPROM.read(EEPROM_BEGIN + i);
    }
    EEPROM.end();
    return arr;
  }

  /**
   * @brief 恢复内存数据到EEPROM
   */
  void restoreAll(uint8_t *arr)
  {
    EEPROM.begin(2048);
    for (int i = 0; i < EEPROM_LENGTH; i++)
    {
      EEPROM.write(EEPROM_BEGIN + i, arr[i]);
    }
    EEPROM.end();
  }
};

static EEPROMTOOLS EEPROMTool;

#endif
