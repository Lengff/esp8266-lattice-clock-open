#include "HttpTool.h"

HttpTool::HttpTool() {}

void HttpTool::initHttptool() {}

void HttpTool::bilibiliFans()
{
  if (!is_need_update_bilibili)
  {
    return;
  }
  fans = 0;             // 每次都先重置粉丝数量,避免出现问题
  biliUid = loadBuid(); // 每次都重新加载bilibili用户ID
  Serial.println(biliUid);

  if (WiFi.status() != WL_CONNECTED) // 确保wifi网络是可用的,不可用则忽略
  {
    Serial.println("no wifi");
    is_need_update_bilibili = false;
    return;
  }
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setFingerprint(fingerprint);
  client->setInsecure();
  // 直接调用B站接口获取粉丝数量
  espClient.begin(*client, bilibiliFansApi + biliUid);
  int httpCode = espClient.GET();
  Serial.println(httpCode);
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
    {
      String json = espClient.getString();
      Serial.println(json);
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, json);
      // 获取bilibili粉丝数量
      fans = doc["data"]["follower"];
      Serial.println(fans);
      is_need_update_bilibili = false;
    }
  }
  else
  {
    Serial.println("调用BIlibili的接口失败！");
  }
  espClient.end();
}
/**
 * @brief 保存bilibili的UID到EEPROM
 *
 * @param uid
 */
void HttpTool::saveBuid(long uid)
{
  uint8_t arr[5];
  for (int i = 0; i < 5; i++)
  {
    arr[i] = (uid & 0xff);
    uid >>= 8;
  }
  EEPROMTool.saveData(arr, BILIBILI_UID, 5);
}

/**
 * @brief 从EEPROM中加载bilibili的UID
 *
 * @return long
 */
long HttpTool::loadBuid()
{
  long uid = 0;
  uint8_t *temp = EEPROMTool.loadData(BILIBILI_UID, 5); // 这里的97处理的不得当,后续优化,但是不影响实际功能
  for (int i = 0; i < 5; i++)
  {
    uid += temp[i] << (i * 8);
  }
  // 用完以后删除内存
  free(temp);
  return uid;
}

void HttpTool::updateBilibiliFlag()
{
  is_need_update_bilibili = true;
}