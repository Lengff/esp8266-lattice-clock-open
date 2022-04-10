#include "Wifis.h"
#include "ConfigWifiHtml.h"

Wifis::Wifis()
{
}

Wifis::Wifis(Lattice *latticeobj, PilotLight *pilotLightobj)
{
  lattice = latticeobj;
  pilotLight = pilotLightobj;
}

void Wifis::wifiloop()
{
  if (isApMode()) // 这里只有在热点模式下才进行此操作
  {
    server.handleClient();
    dnsServer.processNextRequest();
  }
}

void Wifis::initWifi()
{
  if (EEPROMTool.loadDataOne(WIFI_MODE) == 0x01) // 如果取到的数据为1时表示启动了热点模式
  {
    EEPROMTool.saveDataOne(0x00, WIFI_MODE); // 处理完热点模式以后随机将默认模式改为wifi模式
    wifiMode = 0x01;                         // 标记当前wifi模式
    initWebServer();                         // 初始化web服务
  }
  else
  {
    WiFi.mode(WIFI_STA);
    wifiMode = 0x00;
  }
}

/**
 * @brief 处理主页请求
 *
 */
void Wifis::handleIndex()
{
  server.send(200, "text/html", page_html);
}

/**
 * @brief 处理配网请求
 *
 */
void Wifis::handleConfigWifi()
{
  WiFi.persistent(true);                                             //首次在flash中保存WiFi配置-可选
  WiFi.begin(server.arg("ssid").c_str(), server.arg("pwd").c_str()); // 使用配网获取的wifi信息
  WiFi.setAutoConnect(true);                                         // 设置自动连接
  EEPROMTool.saveDataOne(0XFE, REMEMBER_WIFI);                       // 记住wifi密码
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    count++;
    if (count > 40) //如果20秒内没有连上，就开启Web配网 可适当调整这个时间
    {
      server.send(200, "text/html", "wifi连接失败,请检查密码后重试。"); //返回保存成功页面
      break;                                                            //跳出 防止无限初始化
    }
    Serial.println(".");
  }
  if (WiFi.status() == WL_CONNECTED) //如果连接上 就输出IP信息
  {
    Serial.print("WIFI Connected:");                              //打印esp8266的IP地址
    Serial.println(WiFi.localIP());                               //打印esp8266的IP地址
    server.send(200, "text/html", "wifi连接成功,即将重启设备。"); //返回保存成功页面
    delay(3000);                                                  // 等待三秒
    ESP.reset();                                                  // 重启设备
  }
}

/**
 * @brief 处理扫描wifi请求
 *
 */
void Wifis::handleWifiList()
{
  int n = WiFi.scanNetworks(); //开始同步扫描，将返回值存放在变量n中
  if (n > 0)                   // 只有有数据的时候才处理
  {
    char wifilist[640] = {0}; // 返回给网页的数据
    Serial.println("sacn wifi.");
    for (int i = 0; i < 20; ++i) //开始逐个打印扫描到的
    {
      sprintf(wifilist, "%s%s%s", wifilist, WiFi.SSID(i).c_str(), ","); // 组装信息返回给接口
    }
    Serial.print(wifilist);                  // 打印一下日志
    server.send(200, "text/html", wifilist); //返回保存成功页面
    return;                                  // 结束这里的操作
  }
  Serial.println("no any wifi.");           // 打印没有任何wifi日志
  server.send(200, "text/html", ".nodata"); //返回保存成功页面
}

void Wifis::initWebServer()
{
  WiFi.mode(WIFI_AP_STA);                                     // 设置模式为wifi热点模式
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //初始化AP模式
  WiFi.softAP(AP_NAME, NULL, 1, 0, 4);                        //初始化AP模式
  server.on("/", HTTP_GET, handleIndex);                      //设置主页回调函数
  server.on("/configwifi", HTTP_GET, handleConfigWifi);       //设置Post请求回调函数
  server.on("/wifilist", HTTP_GET, handleWifiList);           // 设置获取wifi列表回调函数
  server.onNotFound(handleIndex);                             //设置无法响应的http请求的回调函数
  server.begin();                                             //启动WebServer
  Serial.println("WebServer started!");
  dnsServer.start(53, "*", apIP);
}

void Wifis::connWifi()
{
  initWifi();     // 初始化wifi信息
  int timer = 0;  // 清零计数器
  if (isApMode()) // 模式为热点模式就不走连接wifi和wifi配网了
  {
    return; // 如果wifi模式为热点模式,则不进wifi连接和配网
  }
  if (EEPROMTool.loadDataOne(REMEMBER_WIFI) == 0XFE) // 如果记住wifi值不为0xfe表示存在WiFi账号密码等信息
  {
    Serial.println("start connect wifi ");
    Serial.println(WiFi.SSID().c_str());
    Serial.println(WiFi.psk().c_str());
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // 使用配网获取的wifi信息
    // WiFi.begin("你的wifiSSID", "你的wifi密码"); // 使用固定的wifi信息
    while (WiFi.status() != WL_CONNECTED)
    {
      timer++;
      pilotLight->flashing();   // 闪烁LED灯
      lattice->showLongIcon(0); // 显示连接wifi图案
      delay(100);
      if (timer >= 300) // 如果计数器大于60次,表示超过一分钟,则说明一分钟都没有连接上wifi,就不连了
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
    enableApMode();
    // WiFi.beginSmartConfig(); // 没有账号密码则进入到配网模式
    // Serial.println("smart config wifi ");
    // while (1)
    // {
    // timer++;
    // lattice.showLongIcon(1); // 显示配网中图案信息
    // pilotLight.flashing();   // 闪烁LED灯
    // delay(500);              // 等待半秒钟
    // if (timer >= 20)         // 如果配网次数超过20此,则重启系统,重新配网
    // {
    //   ESP.restart(); // 重启系统
    //   return;
    // }
    // if (WiFi.smartConfigDone()) // 配网成功
    // {
    //   WiFi.setAutoConnect(true);                   // 设置自动连接
    //   EEPROMTool.saveDataOne(true, REMEMBER_WIFI); // 记住wifi密码
    //   break;
    // }
    // wifiloop();
    // }
  }
  Serial.println("conn wifi successful"); // 记录一下日志,避免一点都不知道有没有连上wifi
  delay(500);                             // 等几秒再进入系统
}

void Wifis::enableApMode()
{
  EEPROMTool.saveDataOne(0x01, WIFI_MODE); // 修改wifi模式,随后重启ESP
  ESP.restart();                           // 重启系统
}

/**
 * @brief 获取当前wifi状态,判断是否是热点模式
 *
 * @return true
 * @return false
 */
bool Wifis::isApMode()
{
  // 只有wifi模式 == 0x01时,表示wifi为热点模式
  return wifiMode == 0x01;
}