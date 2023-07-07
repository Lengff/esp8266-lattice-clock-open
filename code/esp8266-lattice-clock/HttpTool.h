#ifndef HTTP_TOOL_H
#define HTTP_TOOL_H

#include "EEPROMTool.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecureBearSSL.h>

static bool is_need_update_bilibili = false; // 判断是否需要更新bilibili粉丝数flag

const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};


class HttpTool
{
private:
  /**
   * WiFiMulti
   *
   */
  ESP8266WiFiMulti WiFiMulti;
  /**
   * http客户端
   */
  HTTPClient espClient;

  /**
   * 加载BiliBili_Uid
   */
  long loadBuid();

  /**
   * bilibili用户ID
   */
  long biliUid = 0;

  /**
   * 获取bilibili粉丝数量的接口 （此处欠妥，先这用着，后续优化）
   */
  const String bilibiliFansApi = "https://api.bilibili.com/x/relation/stat?vmid=";

public:
  HttpTool();

  /**
   * bilibili粉丝数
   */
  long fans = 0;

  /**
   * 初始化
   */
  void initHttptool();

  /**
   * 获取bilibili粉丝数量
   */
  void bilibiliFans();

  /**
   * 保存BiliBili_Uid
   */
  void saveBuid(long uid);

  /**
   * @brief 时间戳++
   *
   */
  static void updateBilibiliFlag();
};

#endif
