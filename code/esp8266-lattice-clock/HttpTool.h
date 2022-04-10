#ifndef HTTP_TOOL_H
#define HTTP_TOOL_H

#include "EEPROMTool.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

static bool is_need_update_bilibili = false; // 判断是否需要更新bilibili粉丝数flag

class HttpTool
{
private:
  /**
   * @brief wifi客户端
   *
   */
  WiFiClient wifiClient;
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
  const String bilibiliFansApi = "http://api.lengff.com/openapi/bili/f/";

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
