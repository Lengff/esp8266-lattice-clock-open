#ifndef UDPS_H
#define UDPS_H

#include "DateTimes.h"
#include "LatticePlus.h"
#include "PilotLight.h"
#include <DS3231.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define localPort 1234     // 监听本地UDP数据包端口
#define remoteNtpPort 123  // 远程服务端口
#define NTP_PACKET_SIZE 48 // NTP数据包数据长度

struct Udpdata
{
  uint8_t rt;       // 返回类型
  uint8_t te;       // 数据类型
  uint8_t lh;       // 数据长度
  uint8_t data[64]; // 数据包
};

class Udps
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
   * @brief 操作时间的对象
   *
   */
  DateTimes *datetimes;
  /**
   * @brief NTP服务器网址
   */
  const char *ntpServerName = "cn.ntp.org.cn";

  /**
   * @brief 服务器IP
   */
  IPAddress timeServerIP;

  /**
   * @brief 接收到的ntp数据包
   */
  byte packetBuffer[NTP_PACKET_SIZE];

  /**
   * @brief 返回UDP数据包
   */
  char replyPacket[4];

  /**
   * @brief udp对象
   */
  WiFiUDP udp;

  /**
   * @brief 发送NTP数据包
   */
  void sendNTPpacket();

public:
  /**
   * @brief构造函数
   */
  Udps();

  /**
   * @brief 构造函数
   *
   * @param datetimes
   */
  Udps(DateTimes *datetimesobj, LatticePlus *latticeobj, PilotLight *pilotLightobj);

  /**
   * @brief 初始化UDP信息
   *
   */
  void initudp();

  /**
   * @brief设置系统时间
   */
  long getNtpTimestamp();

  /**
   * @brief 更新时间
   *
   */
  void updateTime();

  /**
   * @brief接收自定义UPD协议的数据
   */
  Udpdata userLatticeLoop(uint8_t power, uint8_t mode, uint8_t version);
};

#endif
