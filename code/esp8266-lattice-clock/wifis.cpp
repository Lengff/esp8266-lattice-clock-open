#include "Wifis.h"

// 连接您的WIFI SSID和密码 (调试模式可使用此方式连接wifi)
#define WIFI_SSID "wifiname"
#define WIFI_PASSWD "password"

bool usePass = false; // 如果需要使用上面的密码,请将值  `false` 改成 `true`

Wifis::Wifis() {}

void Wifis::connWifi(Lattice lattice)
{

  loadConfigs(); // 先从flash中加载账号密码

  if (wifipwd.rememberPwd == 0xfe || usePass) // 如果记住wifi值不为0xfe表示存在WiFi账号密码等信息
  {
    // 如果说有账号密码信息,那就直接用账号密码连接wifi
    Serial.println("start conn wifi ");
    WiFi.mode(WIFI_STA);
    if (usePass)
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWD); // 使用固定的wifi信息
    }
    else
    {
      WiFi.begin(wifipwd.ssid, wifipwd.pwd); // 使用配网获取的wifi信息
    }
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      lattice.showLongIcon(0); // 显示连接wifi图案
      delay(100);
    }
    WiFi.setAutoConnect(true); // 设置自动连接
  }
  else
  {
    WiFi.beginSmartConfig(); // 没有账号密码则进入到配网模式
    Serial.println("smart config wifi ");
    while (1)
    {
      Serial.print("-");
      lattice.showLongIcon(1); // 显示配网中图案信息
      delay(100);
      if (WiFi.smartConfigDone()) // 配网成功
      {
        strcpy(wifipwd.ssid, WiFi.SSID().c_str());
        strcpy(wifipwd.pwd, WiFi.psk().c_str());
        wifipwd.rememberPwd = 0xfe; // 记住wifi密码
        WiFi.setAutoConnect(true);  // 设置自动连接
        saveConfig();
        break;
      }
    }
  }
  Serial.println("conn wifi successful");
  Serial.println(WiFi.SSID().c_str());
  Serial.println(WiFi.psk().c_str());
  delay(500); // 等几秒再进入系统
}

void Wifis::saveConfig()
{
  EEPROMTool.saveData((uint8_t *)(&wifipwd), 0, sizeof(wifipwd));
}

void Wifis::loadConfigs()
{
  uint8_t *p = (uint8_t *)(&wifipwd);
  uint8_t *temp = EEPROMTool.loadData(0, sizeof(wifipwd));
  for (int i = 0; i < sizeof(wifipwd); i++)
  {
    p[i] = temp[i];
  }
  free(temp); // 用完以后删除内存
}