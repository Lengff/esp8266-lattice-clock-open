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
   * @brief 记录wifi账号密码的结构体 2022-2-17 今天才发现这里有可能多次一举，但是也先放着，后续有时间改改
   *
   */
  struct WifiPwd
  {
    uint8_t rememberPwd = 0x00; // 记住wifi密码,如果这个值为0xfe时,表示为记住wifi密码
  };

  /**
   * @brief 声明变量
   *
   */
  WifiPwd wifipwd;

  /**
   * @brief 保存WiFi密码信息
   *
   */
  void saveConfig();

  /**
   * @brief 加载WiFi账号密码
   *
   */
  void loadConfigs();

  /* Set these to your desired credentials. */
  const char *ssid = APSSID;

  const char *password = APPSK;

  /**
   * @brief 初始化wifi
   * 
   */
  void initWifi();

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
   * @brief 将wifi模式修改为STA模式
   *
   */
  void enableApMode();
};
#endif
