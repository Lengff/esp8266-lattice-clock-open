#include "Wifis.h"

// 连接您的WIFI SSID和密码 (调试模式可使用此方式连接wifi)
#define WIFI_SSID "wifiname"
#define WIFI_PASSWD "password"

bool usePass = false; // 如果需要使用上面的密码,请将值  `false` 改成 `true`

Wifis::Wifis()
{
}

void Wifis::initWifi()
{
  if (EEPROMTool.loadDataOne(WIFI_MODE) == 0x01) // 如果取到的数据为1时表示启动了热点模式
  {
    WiFi.mode(WIFI_AP);                      // 设置wifi模式为热点模式
    wifiMode = 0x01;                         // 标记当前wifi模式
    WiFi.softAP(ssid, password);             // 设置wifi热点账号密码
    EEPROMTool.saveDataOne(0x00, WIFI_MODE); // 处理完热点模式以后随机将默认模式改为wifi模式
  }
  else
  {
    WiFi.mode(WIFI_STA);
    wifiMode = 0x00;
  }
}

void Wifis::connWifi(Lattice lattice, PilotLight pilotLight)
{
  initWifi();           // 初始化wifi信息
  timer = 0;            // 清零计数器
  if (wifiMode == 0x01) // 模式为热点模式就不走连接wifi和wifi配网了
  {
    // 如果wifi模式为热点模式,则不进wifi连接和配网
    return;
  }
  if (EEPROMTool.loadDataOne(REMEMBER_WIFI) || usePass) // 如果记住wifi值不为0xfe表示存在WiFi账号密码等信息
  {
    Serial.println("start connect wifi ");
    if (usePass) // 判断是否使用代码中的wifi信息
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWD); // 使用固定的wifi信息
    }
    else
    {
      WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // 使用配网获取的wifi信息
    }
    while (WiFi.status() != WL_CONNECTED)
    {
      timer++;
      pilotLight.flashing();   // 闪烁LED灯
      lattice.showLongIcon(0); // 显示连接wifi图案
      delay(100);
      if (timer >= 600) // 如果计数器大于60次,表示超过一分钟,则说明一分钟都没有连接上wifi,就不连了
      {
        timer = 0;      // 清零计数器
        enableApMode(); // 联网失败进入到热点模式
        break;          // 防止出问题还是break一下
      }
    }
    WiFi.setAutoConnect(true); // 设置自动连接
  }
  else
  {
    WiFi.beginSmartConfig(); // 没有账号密码则进入到配网模式
    Serial.println("smart config wifi ");
    while (1)
    {
      timer++;
      lattice.showLongIcon(1); // 显示配网中图案信息
      pilotLight.flashing();   // 闪烁LED灯
      delay(500);              // 等待半秒钟
      if (timer >= 20)         // 如果配网次数超过20此,则重启系统,重新配网
      {
        ESP.restart(); // 重启系统
        return;
      }
      if (WiFi.smartConfigDone()) // 配网成功
      {
        WiFi.setAutoConnect(true);                   // 设置自动连接
        EEPROMTool.saveDataOne(true, REMEMBER_WIFI); // 记住wifi密码
        break;
      }
    }
  }
  Serial.println("conn wifi successful"); // 记录一下日志,避免一点都不知道有没有连上wifi
  delay(500);                             // 等几秒再进入系统
  timer = 0;                              // 清零计数器
}

void Wifis::enableApMode()
{
  EEPROMTool.saveDataOne(0x01, WIFI_MODE); // 修改wifi模式,随后重启ESP
  ESP.restart();                           // 重启系统
}
