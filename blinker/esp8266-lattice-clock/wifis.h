#ifndef WIFIS_H
#define WIFIS_H

#include "EEPROMTool.h"
#include "Lattice.h"
#include "PilotLight.h"
#include <ESP8266WiFi.h>
#include <cstring>

// wifi热点的账号密码
#ifndef APSSID
#define APSSID "lattice-clock"
#define APPSK "12345678"
#endif

class Wifis
{
private:
  /**
   * @brief 是否记住wifi密码
   *
   */
  bool rememberWifiPwd = true;

  /**
   * @brief 固定设备热点的SSID
   *
   */
  const char *ssid = APSSID;

  /**
   * @brief 固定设备热点的密码
   *
   */
  const char *password = APPSK;

  /**
   * @brief 初始化wifi
   *
   */
  void initWifi();

  /**
   * @brief 计数器
   *
   */
  int timer;

public:
  /**
   * @brief 构造函数
   *
   */
  Wifis();

  /**
   * @brief wifi模式 0x00: 连接wifi模式(STA)  0x01: wifi热点模式(AP)
   *
   */
  uint8_t wifiMode = 0x00;

  /**
   * @brief 连接wifi
   *
   * @param lattice 点阵显示对象
   * @param pilotLight LED显示对象
   */
  void connWifi(Lattice lattice, PilotLight pilotLight);

  /**
   * @brief 启动热点模式
   *
   */
  void enableApMode();
};
#endif
