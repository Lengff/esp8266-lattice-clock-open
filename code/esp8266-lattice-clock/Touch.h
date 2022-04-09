#ifndef TOUCH_H
#define TOUCH_H
#include "HttpTool.h"
#include "PilotLight.h"
#include "System.h"
#include "Udps.h"
#include "Wifis.h"
#include <ESP8266WiFi.h>
#include <OneButton.h>

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

OneButton btnA = OneButton(D8, false, false);            // 按钮对象
PilotLight pilotLight = PilotLight();                    // 控制LED亮灭对象
Lattice lattice = Lattice(&btnA);                        // 点阵显示对象
Wifis wifis = Wifis(&lattice, &pilotLight);              // wifi对象
unsigned char displayData[4] = {0x00, 0x00, 0x00, 0xff}; // 点阵显示数,每个点阵应该显示那些数据
const int powerLength = 5;                               // 有多少种功能
uint8_t modePower[6] = {3, 3, 1, 1, 5, 1};               // 每个功能对应多少种模式
uint8_t powers[6] = {0, 0, 0, 0, 0, 1};                  // 默认的功能模式
uint8_t power = 0;                                       // 0:显示时间，1:显示日日期，2:显示温度，3：显示倒计时，4：显示自定义内容
uint32_t clicktime = 0;                                  // 记录按键按下时间
long powerFlag = 0, powerFlag2 = 0;                      // 功能flag
uint8_t sleepTime[5] = {0, 0, 8, 0, 1};                  // 这里表示设备休眠时间,默认是凌晨0点到早上八点,所以表示为0:0 ~ 8:0,最后一位是亮度(0:表示息屏,15表示最亮了)
bool isSleepMode = false;                                // 标记当前是否处于睡眠模式
const uint8_t version = 9;                               // OTA 固件版本号

/**
 * @brief 初始化状态
 *
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
 * @brief 处理单击
 *
 */
void singleAClick()
{
  Serial.println("A按键单击");
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  pilotLight.flashing(100);                   // 按键单击时先闪一下LED
  power = power == powerLength ? 0 : ++power; // 功能加加
  initStatus();                               // 初始化状态
}

/**
 * @brief 处理双击
 *
 */
void doubleAClick()
{
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  pilotLight.flashing(100); // 按键单击时先闪一下LED
  Serial.println("A按键双击");
  powers[power] = powers[power] == (modePower[power] - 1) ? 0 : ++powers[power];
  initStatus();
}

/**
 * @brief 按键长按开始做的事情
 *
 */
void longAClickStart()
{
  Serial.println("A按键长按开始");
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  pilotLight.bright(); // 按下的时候LED亮起来
  clicktime = millis();
}

/**
 * @brief 处理按键长按
 *
 */
void longAClick()
{
  Serial.println("A按键长按结束");
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  pilotLight.dim();                          // 按下的时候LED熄灭
  clicktime = millis() - clicktime + 1000;   // 累加按下时间
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

/**
 * @brief 按键初始化
 *
 */
void initTouch();

/**
 * @brief 按键循环
 *
 */
void touchLoop();

void initTouch()
{
  btnA.attachClick(singleAClick);       // 添加单击事件函数
  btnA.attachDoubleClick(doubleAClick); // 添加双击事件函数
  // btnA.attachLongPressStop(longAClick);       // 添加长按事件函数
  // btnA.attachLongPressStart(longAClickStart); // 添加按下事件函数
}

void touchLoop()
{
  btnA.tick();
}
#endif
