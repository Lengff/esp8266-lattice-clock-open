#ifndef OTAS_H
#define OTAS_H
#include "Touch.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

/**
 * OTA 固件版本号
 */
const uint8_t version = 8;
int process = 0;

void update_started()
{
  Serial.println("callback: ota is start!");
  lattice.reset(); // 重置显示内容
}

void update_finished()
{
  Serial.println("callback:  ota is finish!");
  lattice.reset();         // 重置显示内容
  lattice.showLongIcon(3); // 显示OTA更新成功图案
}

void update_progress(long cur, long total)
{
  process = (int)(((cur * 0.1) / (total * 0.1)) * 100);
  Serial.printf("callback:  updateing %d of %d bytes...\n", cur, total);
  pilotLight.flashing(20);        // 固件升级的时候LED闪
  lattice.showOtaUpdate(process); // OTA显示当前进度图案
}

void update_error(int err)
{
  Serial.printf("callback:  ota is error:  %d\n", err);
  lattice.reset();         // 重置显示内容
  lattice.showLongIcon(4); // 显示OTA更新失败图案
}

void updateOta(int version)
{
  WiFiClient client;

  if (WiFi.status() == WL_CONNECTED) // 确保有网络
  {
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    ESPhttpUpdate.onStart(update_started);     // 添加可选的回调通知程序
    ESPhttpUpdate.onEnd(update_finished);      // 添加更新完成回调方法
    ESPhttpUpdate.onProgress(update_progress); // 添加更新中回调方法
    ESPhttpUpdate.onError(update_error);       // 添加更新失败回调方法
    char *updateUrl = new char[50];
    sprintf(updateUrl, "%s%d%s%d", "http://oss.lengff.com/iot/lattice/", version, ".bin?t=", millis() % 100); // 后面的对100取余就是为了解除文件CDN缓存
    Serial.println(updateUrl);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, updateUrl);
    free(updateUrl);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("ote is fail, err code is：(%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      lattice.reset();         // 重置显示内容
      lattice.showLongIcon(4); // 显示OTA更新失败图案
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("not ota update!");
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
