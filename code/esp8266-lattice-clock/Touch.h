#ifndef TOUCH_H
#define TOUCH_H
#include "EEPROMTool.h"
#include "HttpTool.h"
#include "PilotLight.h"
#include "Udps.h"
#include "Wifis.h"
#include <ESP8266WiFi.h>
#include <OneButton.h>

// UDP数据传输对象
Udps udps;
// wifi对象
Wifis wifis = Wifis();
// 按钮对象
OneButton btnA = OneButton(D8, false, false);
// 控制LED亮灭对象
PilotLight pilotLight = PilotLight();
// 点阵显示对象
Lattice lattice = Lattice(false);
// 点阵显示数,每个点阵应该显示那些数据
unsigned char displayData[4] = {0x00, 0x00, 0x00, 0xff};

const int powerLength = 5;

enum ModeEnum
{
  // 功能0: 显示时间
  POWER0 = 0,
  // 功能1: 显示日期
  POWER1 = 1,
  // 功能2: 显示日期
  POWER2 = 2,
  // 显示bilibili粉丝数量
  BILIFANS = 3,
  // 显示用户自定义数据
  CUSTOM = 4,
  // 显示倒计时
  COUNTDOWN = 5,
  // NTP更新时间
  RESETTIME = 98,
  // 系统重置
  RESET = 99
};
// 每个功能对应多少种模式
uint8_t modePower[6] = {3, 3, 1, 1, 5, 1};
// 默认的功能模式
uint8_t powers[6] = {0, 0, 0, 0, 0, 1};
// 0:显示时间，1:显示日日期，2:显示温度，3：显示倒计时，4：显示自定义内容
uint8_t power = 0;
// 记录按键按下时间
uint32_t clicktime = 0;
// 功能flag
long powerFlag = 0, powerFlag2 = 0;

/**
   初始化状态
*/
void initStatus()
{
  if (power == CUSTOM && powers[power] != 0)
  {
    // 如果显示模式是自定义内容的话,就不重置屏幕显示
    powerFlag = -1;
    return;
  }
  else if (power == COUNTDOWN)
  {

    displayData[0] = 0;
    displayData[1] = 1;
    displayData[2] = 2;
    powerFlag2 = 1; // 这里一定是到大于0的,后续会根据这个值是否为0来判断逻辑
  }

  powerFlag = -1;
  lattice.reset();
}

/**
   处理单击
*/
void singleAClick()
{
  Serial.println("A按键单击");
  // 如果wifi未连接,且当前wifi模式为wifi直连模式,单击则修改wifi模式为热点模式
  if (WiFi.status() != WL_CONNECTED && wifis.wifiMode == 0x0)
  {
    // wifi没连接上的情况
    wifis.enableApMode();
  }
  else
  {
    pilotLight.flashing(100); // 按键单击时先闪一下LED
    power = power == powerLength ? 0 : ++power;
    initStatus();
  }
}

/**
   处理双击
*/
void doubleAClick()
{
  pilotLight.flashing(100); // 按键单击时先闪一下LED
  Serial.println("A按键双击");
  powers[power] = powers[power] == (modePower[power] - 1) ? 0 : ++powers[power];
  initStatus();
}

/**
   按键长按开始做的事情
*/
void longAClickStart()
{
  Serial.println("A按键长按开始");
  pilotLight.bright(); // 按下的时候LED亮起来
  clicktime = millis();
}

/**
   处理按键长按
*/
void longAClick()
{
  Serial.println("A按键长按结束");
  pilotLight.dim(); // 按下的时候LED熄灭
  clicktime = millis() - clicktime + 1000;
  if (clicktime > 2000 && clicktime <= 5000) // 如果长按时间大于3秒,小于六秒则表示重置时间
  {
    power = RESETTIME;
  }
  else if (clicktime >= 5000) // 如果时间大于六秒则重置系统
  {
    power = RESET; // 切换功能模式
  }
  clicktime = 0; // 重置按下时间
}

ICACHE_RAM_ATTR void tickloop()
{
  btnA.tick();
}

/**
   按键初始化
*/
void initTouch();

/**
   按键循环
*/
void touchLoop();

void initTouch()
{
  attachInterrupt(digitalPinToInterrupt(D8), tickloop, CHANGE);
  btnA.attachClick(singleAClick);             // 添加单击事件函数
  btnA.attachDoubleClick(doubleAClick);       // 添加双击事件函数
  btnA.attachLongPressStop(longAClick);       // 添加长按事件函数
  btnA.attachLongPressStart(longAClickStart); // 添加按下事件函数
}

void touchLoop()
{
  btnA.tick();
}
#endif
