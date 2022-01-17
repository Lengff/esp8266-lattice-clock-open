#ifndef WIFIS_H
#define WIFIS_H

#include <ESP8266WiFi.h>
#include <cstring>
#include "Lattice.h"
#include "EEPROMTool.h"

/**
 * 记录wifi账号密码的结构体
 */

class Wifis
{
private:
  struct WifiPwd
  {
    char ssid[32];// wifi名称
    char pwd[64];// wifi密码
    uint8_t rememberPwd = 0x00;// 记住wifi密码,如果这个值为0xfe时,表示为记住wifi密码
  };
  /**
   *声明变量
   */
  WifiPwd wifipwd;

  /**
   * 保存WiFi密码信息
   */
  void saveConfig();

  /**
   * 加载WiFi账号密码
   */
  void loadConfigs();

public:
  /**
   * 构造函数
   */
  Wifis();

  /**
   * 连接WiFi
   */
  void connWifi(Lattice lattice);
};
#endif
