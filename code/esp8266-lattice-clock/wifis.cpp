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
  if (EEPROMTool.loadDataOne(1) == 0x01) // 如果取到的数据为1时表示启动了热点模式
  {
    WiFi.mode(WIFI_AP);              // 设置wifi模式为热点模式
    wifiMode = 0x01;                 // 标记当前wifi模式
    WiFi.softAP(ssid, password);     // 设置wifi热点账号密码
    EEPROMTool.saveDataOne(0x00,1); // 处理完热点模式以后随机将默认模式改为wifi模式
  }
  else
  {
    WiFi.mode(WIFI_STA);
    wifiMode = 0x00;
  }
}

void Wifis::connWifi(Lattice lattice, PilotLight pilotLight)
{
  initWifi();
  if (wifiMode == 0x01)
  {
    // 如果wifi模式为热点模式,则不进wifi连接和配网
    return;
  }
  loadConfigs();                              // 先从flash中加载账号密码
  if (wifipwd.rememberPwd == 0xfe || usePass) // 如果记住wifi值不为0xfe表示存在WiFi账号密码等信息
  {
    // 如果说有账号密码信息,那就直接用账号密码连接wifi
    Serial.println("start connect wifi ");
    if (usePass)
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWD); // 使用固定的wifi信息
    }
    else
    {
      WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // 使用配网获取的wifi信息
    }
    while (WiFi.status() != WL_CONNECTED)
    {
      pilotLight.flashing();              // 闪烁LED灯
      lattice.showLongIcon(0);            // 显示连接wifi图案
      if (WiFi.softAPgetStationNum() > 0) // 有手机连接上了设备热点的话,跳出循环
      {
        wifiMode = 0x01;
        pilotLight.bright(); // 开启wifi热点LED灯常亮
        break;
      }
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
      lattice.showLongIcon(1);            // 显示配网中图案信息
      pilotLight.flashing();              // 闪烁LED灯
      if (WiFi.softAPgetStationNum() > 0) // 有手机连接上了设备热点的话,跳出循环
      {
        wifiMode = 0x01;
        pilotLight.bright(); // 开启wifi热点LED灯常亮
        break;
      }
      delay(500);
      if (WiFi.smartConfigDone()) // 配网成功
      {
        wifipwd.rememberPwd = 0xfe; // 记住wifi密码
        WiFi.setAutoConnect(true);  // 设置自动连接
        saveConfig();
        break;
      }
    }
  }
  Serial.println("conn wifi successful");
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

void Wifis::enableApMode()
{
  EEPROMTool.saveDataOne(0x01,1); // 修改wifi模式,随后重启ESP
  ESP.restart();                   // 重启系统
}