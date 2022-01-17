#ifndef TOUCH_H
#define TOUCH_H
#include <ESP8266WiFi.h>
#include <OneButton.h>
#include "Wifis.h"
#include "HttpTool.h"
#include "EEPROMTool.h"

OneButton btnA = OneButton(D8, false, false);
OneButton btnB = OneButton(D3, false, false);

Lattice lattice = Lattice(false);

const int powerLength = 4;

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
  // NTP更新时间
  RESETTIME = 98,
  // 系统重置
  RESET = 99
};
// 每个功能对应多少种模式
uint8_t modePower[5] = {2, 3, 1, 1, 5};
// 默认的功能模式
uint8_t powers[5] = {0, 0, 0, 0, 0};
// 0:显示时间，1:显示日日期，2:显示温度，3：显示倒计时，4：显示自定义内容
uint8_t power = 0;
// 是否是编辑模式(此功能没开发完)
bool isedit = false;
// 记录按键按下时间
uint32_t clicktime = 0, isadd = 0;
// 功能flag
long powerFlag = 0;

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
  powerFlag = -1;
  lattice.reset();
}

/**
   处理单击
*/
void singleAClick()
{
  if (!isedit)
  {
    // 非编辑状态，切换显示模式
    power = power == powerLength ? 0 : ++power;
    initStatus();
  }
  isadd = 1;
  Serial.println("A按键单击");
}

/**
   处理双击
*/
void doubleAClick()
{
  powers[power] = powers[power] == (modePower[power] - 1) ? 0 : ++powers[power];
  initStatus();
  Serial.println("A按键双击");
}

/**
   按键长按开始做的事情
*/
void longAClickStart()
{
  Serial.println("A按键长按开始");
  clicktime = millis();
}

/**
   处理按键长按
*/
void longAClick()
{
  Serial.println("A按键长按结束");
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
