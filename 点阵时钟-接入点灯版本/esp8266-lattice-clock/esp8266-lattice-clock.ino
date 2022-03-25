#include "Main.h"
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#include <Blinker.h>

char auth[] = "0b6921f916bd";

// 新建组件对象
BlinkerButton powerBtn("btn-power");         // 开关按键
BlinkerButton directBtn("btn-direction");    // 方向按键
BlinkerNumber temperNum("num-temperature");  // 温度回调
BlinkerSlider brightSlider("slider_bright"); //亮度滑动条
BlinkerSlider powerSlider("slider_power");   //功能滑动条

/**
 * @brief 设置方向回调
 *
 * @param state
 */
void directionCallback(const String &state)
{
  Serial.print("设置时钟方向:");
  Serial.println(state);
  uint8_t direction = 0;
  if (state == BLINKER_CMD_ON)
  {
    direction = 0;
    directBtn.print("on"); // 反馈开关状态
  }
  else if (state == BLINKER_CMD_OFF)
  {
    direction = 1;
    directBtn.print("off"); // 反馈开关状态
  }
  lattice.setDirection(direction); // 是否启用点阵屏幕
}

/**
 * @brief 设置亮度的回调
 *
 * @param value
 */
void brightCallback(int32_t value)
{
  lattice.setBrightness(value, true); // 设置亮度
  brightSlider.print(value);          // 将数据回调给组件
}

/**
 * @brief 功能切换的回调
 *
 * @param value
 */
void powerCallback(int32_t value)
{
  power = value; // 切换功能
  initStatus();
  powerSlider.print(value); // 将数据回调给组件
}

/**
   小爱同学接入方法 - 开关功能回调
*/
void miotPowerState(const String &state)
{
  Serial.print("小爱语音设置的功能状态为:");
  Serial.println(state);
  uint8_t power = 0;
  if (state == BLINKER_CMD_ON)
  {
    power = 0;
    BlinkerMIOT.powerState("on");
    powerBtn.print("on"); // 反馈开关状态
    BlinkerMIOT.print();
  }
  else if (state == BLINKER_CMD_OFF)
  {
    power = 1;
    BlinkerMIOT.powerState("off");
    powerBtn.print("off"); // 反馈开关状态
    BlinkerMIOT.print();
  }
  lattice.shutdown(power); // 是否启用点阵屏幕
}

/**
   小爱同学接入方法 - 设置亮度
*/
void miotBright(const String &bright)
{
  Serial.print("小爱语音设置的亮度为:");
  Serial.println(bright.toInt());
  lattice.setBrightness(bright.toInt() / 100 * 16, true); // 设置亮度
  BlinkerMIOT.brightness(bright.toInt());
  brightSlider.print((int)bright.toInt() / 100 * 16); // 将数据回调给组件
  BlinkerMIOT.print();
}

/**
   小爱同学接入方法 - 查询状态
*/
void miotQuery(int32_t queryCode)
{
  Serial.print("小爱语音查询状态代码:");
  Serial.println(queryCode);
}

void setup()
{
  Serial.begin(115200);                        // 初始化串口波特率
  initTouch();                                 // 初始化按键信息
  wifis.connWifi(lattice, pilotLight);         // 连接wifi
  udps.initudp();                              // 初始化UDP客户端
  pilotLight.dim();                            //正常进操作就熄灭指示灯
  httptoolticker.attach(5, updateBiliFstatus); // 每分钟更新一次bilibili粉丝数量
  if (wifis.wifiMode == 0x00)                  // 如果wifi模式为连接wifi的模式则联网矫正时间
  {
    Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str()); // 初始化Blinker 这里只有在连接wifi成功的情况下才能用
    powerBtn.attach(miotPowerState);                              // 注册开关按键
    brightSlider.attach(brightCallback);                          // 注册亮度调节
    directBtn.attach(directionCallback);                          // 注册方向按钮回到
    powerSlider.attach(powerCallback); // 注册功能切换
    BlinkerMIOT.attachPowerState(miotPowerState); // 注册屏幕开关的回调
    BlinkerMIOT.attachBrightness(miotBright);     // 注册亮度控制的回调
    BlinkerMIOT.attachQuery(miotQuery);           // 注册小爱同学语音状态查询
    resetTime(displayData);                       // 每次初始化的时候都校准一下时间,这里是随便传的一个参数,不想重新声明参数
    httptool.bilibiliFans();                      // 刷新bilibili粉丝数量
  }
  else
  {
    pilotLight.bright(); // 如果是热点模式的话,指示的LED灯常亮
  }
  initSleepTime(); // 初始化休眠时间
}

void loop()
{
  handleUdpData();
  touchLoop();
  handlePower();
  sleepTimeLoop();
  // temperNum.print(datetimes.getTemperature());
  Blinker.run();
}
