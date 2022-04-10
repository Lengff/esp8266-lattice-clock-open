#ifndef TOUCH_H
#define TOUCH_H
#include "Functions.h"
#include "HttpTool.h"
#include "PilotLight.h"
#include "System.h"
#include "Udps.h"
#include "Wifis.h"
#include <ESP8266WiFi.h>
#include <OneButton.h>

#define LATTICE_CLOCK_VERSION 9 // 点阵时钟代码版本号码

OneButton btnA = OneButton(D8, false, false); // 按钮对象
PilotLight pilotLight = PilotLight();         // 控制LED亮灭对象
Lattice lattice = Lattice(&btnA);             // 点阵显示对象
Wifis wifis = Wifis(&lattice, &pilotLight);   // wifi对象
Functions functions = Functions(&pilotLight); // 功能管理对象

unsigned char displayData[4] = {0x00, 0x00, 0x00, 0xff}; // 点阵显示数,每个点阵应该显示那些数据
long powerFlag = 0, powerFlag2 = 0;                      // 功能flag
uint8_t sleepTime[5] = {0, 0, 8, 0, 1};                  // 这里表示设备休眠时间,默认是凌晨0点到早上八点,所以表示为0:0 ~ 8:0,最后一位是亮度(0:表示息屏,15表示最亮了)
bool isSleepMode = false;                                // 标记当前是否处于睡眠模式

/**
 * @brief 初始化状态
 *
 */
void initStatus()
{
  if (functions.getCurrPower() == CUSTOM && functions.getCurrMode() != 0) // 如果是自定义显示功能条件下(且模式为显示模式),就不重置屏幕显示
  {
    powerFlag = -1;
    return;
  }
  else if (functions.getCurrPower() == COUNTDOWN) // 如果是倒计时功能条件下,则进入倒计时相关逻辑操作
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
  Serial.println("A按键单击");           // 打印日志,便于校验
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  functions.powerUp(); // 功能加一下
  initStatus();        // 初始化一些东西
}

/**
 * @brief 处理双击
 *
 */
void doubleAClick()
{
  Serial.println("A按键双击");           // 打印日志,便于校验
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  pilotLight.flashing(100); // 按键单击时先闪一下LED
  functions.modeUp();       // 功能模式加一下
  initStatus();             // 初始化一些东西
}

/**
 * @brief 按键长按开始做的事情
 *
 */
void longAClickStart()
{
  Serial.println("A按键长按开始"); // 打印日志,便于校验
  pilotLight.bright();             // 按下的时候LED亮起来
}

/**
 * @brief 处理按键长按
 *
 */
void longAClick()
{
  Serial.println("A按键长按结束");       // 打印日志,便于校验
  if (lattice.latticeSetting.isShutdown) // 如果屏幕是熄灭的话,单击操作则是点亮屏幕不做其他操作
  {
    lattice.shutdown(false); // 让点阵屏重新显示
    return;
  }
  lattice.shutdown(true); // 否则关闭点阵屏
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
  btnA.attachLongPressStop(longAClick); // 添加长按事件函数
  // btnA.attachLongPressStart(longAClickStart); // 添加按下事件函数
}

void touchLoop()
{
  btnA.tick();
}
#endif
