#ifndef UDPS_H
#define UDPS_H

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DS3231.h>
#include "Lattice.h"
#include "DateTimes.h"

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
  long timeFlag = 0;

  /**
   * NTP服务器网址
   */
  const char *ntpServerName = "cn.ntp.org.cn";

  /**
   * 服务器IP
   */
  IPAddress timeServerIP;

  /**
   * 接收到的ntp数据包
   */
  byte packetBuffer[NTP_PACKET_SIZE];

  /**
   * 返回UDP数据包
   */
  char replyPacket[4];

  /**
   * udp对象
   */
  WiFiUDP udp;

  /**
   * 发送NTP数据包
   */
  void sendNTPpacket();

public:
  /**
   * 是否
   */
  bool isSetTime;
  /**
   * 构造函数
   */
  Udps();

  void initudp();

  /**
   * 开始重置时间
   */
  void startSet();

  /**
   * 设置系统时间
   */
  bool setTimes(DateTimes datetimes);

  /**
   * 接收自定义UPD协议的数据
   */
  Udpdata userLatticeLoop(LatticeSetting latticeSetting, uint8_t power,
                          uint8_t mode, uint8_t version);
};

#endif
