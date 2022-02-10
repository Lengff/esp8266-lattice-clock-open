#ifndef OTAS_H
#define OTAS_H
#include "Touch.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

/**
 * OTA 固件版本号
 */
const uint8_t version = 5;

void update_started()
{
  Serial.println("回调:  HTTP更新进程已启动");
  lattice.reset();
}

void update_finished()
{
  Serial.println("回调:  HTTP更新过程已完成");
  lattice.reset();
  lattice.showLongIcon(3); // 显示OTA更新成功图案
}

void update_progress(int cur, int total)
{
  Serial.printf("回调:  HTTP更新过程位于 %d of %d bytes...\n", cur, total);
  lattice.showOtaUpdate((int)((cur / (double)total) * 100)); // OTA显示当前进度图案
}

void update_error(int err)
{
  Serial.printf("回调BACK:  HTTP更新致命错误代码 %d\n", err);
  lattice.reset();         // 重置显示内容
  lattice.showLongIcon(4); // 显示OTA更新失败图案
}

void updateOta(int version)
{
  WiFiClient client;

  if (WiFi.status() == WL_CONNECTED) // 确保有网络
  {
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    ESPhttpUpdate.onStart(update_started); // 添加可选的回调通知程序
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
    char *updateUrl = new char[50];
    sprintf(updateUrl, "%s%d%s%d", "http://oss.lengff.com/iot/lattice/", version, ".bin?t=", millis() % 100); // 后面的对100取余就是为了解除文件CDN缓存
    Serial.println(updateUrl);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, updateUrl);
    free(updateUrl);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP更新失败 错误代码：(%d): %s\n",
                    ESPhttpUpdate.getLastError(),
                    ESPhttpUpdate.getLastErrorString().c_str());
      lattice.reset();         // 重置显示内容
      lattice.showLongIcon(4); // 显示OTA更新失败图案
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("无HTTP更新");
      lattice.showLongIcon(3); // 显示OTA更新成功图案
      break;
    case HTTP_UPDATE_OK:
      lattice.reset();
      lattice.showLongIcon(3); // 显示OTA更新成功图案
      break;
    }
  }
}

#endif