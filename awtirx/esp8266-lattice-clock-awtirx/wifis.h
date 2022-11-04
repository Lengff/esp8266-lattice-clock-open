#ifndef WIFIS_H
#define WIFIS_H

#include "EEPROMTool.h"
#include "LatticePlus.h"
#include "PilotLight.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <cstring>

#ifndef APSSID
#define APSSID "lattice-clock-ap" // wifi热点的账号密码
#endif

static ESP8266WebServer server(80);    // 创建dnsServer实例
static DNSServer dnsServer;            // dnsServer
static IPAddress apIP(192, 168, 4, 1); // esp8266-AP-IP地址

class Wifis
{
private:
  /**
   * @brief 显示对象
   *
   */
  LatticePlus *lattice;

  /**
   * @brief 状态指示灯对象
   *
   */
  PilotLight *pilotLight;

  /**
   * @brief wifi名字
   *
   */
  const char *AP_NAME = APSSID;

  /**
   * @brief 是否记住wifi密码
   *
   */
  bool rememberWifiPwd = 0X00;

  /**
   * @brief 初始化wifi
   *
   */
  void initWifi();

  /**
   * @brief 初始化web服务
   *
   */
  void initWebServer();

  /**
   * @brief wifi模式 0x00: 连接wifi模式(STA)  0x01: wifi热点模式(AP)
   *
   */
  uint8_t wifiMode = 0x00;

public:
  /**
   * @brief 构造函数
   *
   */
  Wifis();

  /**
   * @brief 构造函数
   *
   * @param lattice
   * @param pilotLight
   */
  Wifis(LatticePlus *latticeobj, PilotLight *pilotLightobj);

  /**
   * @brief 连接wifi
   *
   */
  void connWifi();

  /**
   * @brief 启动热点模式
   *
   */
  void enableApMode();

  /**
   * @brief 获取当前wifi状态,判断是否是热点模式
   *
   * @return true
   * @return false
   */
  bool isApMode();

  /**
   * @brief wifi轮循方法
   *
   */
  void wifiloop();

  /**
   * @brief 处理主页请求
   *
   */
  static void handleIndex();

  /**
   * @brief 处理配网请求
   *
   */
  static void handleConfigWifi();

  /**
   * @brief 处理扫描wifi请求
   *
   */
  static void handleWifiList();
};
#endif
