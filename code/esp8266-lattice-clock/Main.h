#ifndef MAIN_H
#define MAIN_H
#include "DateTimes.h"
#include "EEPROMTool.h"
#include "Functions.h"
#include "HttpTool.h"
#include "LatticePlus.h"
#include "Otas.h"
#include "PilotLight.h"
#include "System.h"
#include "Udps.h"
#include "Wifis.h"
#include <DS3231.h>
#include <ESP8266WiFi.h>
#include <OneButton.h>
#include <Ticker.h>
#include <Wire.h>

Ticker timestampticker;                                  // 手动累加时间戳任务
Ticker httptoolticker;                                   // 每五秒钟处理一次http请求标志
System systems;                                          // 系统内置对象
DateTimes datetimes = DateTimes();                       // 时间管理对象
HttpTool httptool;                                       // HTTP 请求对象
LatticePlus lattice = LatticePlus(&systems);             // 点阵显示对象
OneButton btnA = OneButton(D8, false, false);            // 按钮对象
PilotLight pilotLight = PilotLight();                    // 控制LED亮灭对象
Wifis wifis = Wifis(&lattice, &pilotLight);              // wifi对象
Functions functions = Functions(&pilotLight);            // 功能管理对象
Otas otas = Otas(&lattice, &pilotLight);                 // OTA更新处理对象
Udps udps = Udps(&datetimes, &lattice, &pilotLight);     // UDP数据传输对象
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
    powerFlag = -1;
    powerFlag2 = -1;
    if (functions.getCurrPower() == CUSTOM && functions.getCurrMode() != 0) // 如果是自定义显示功能条件下(且模式为显示模式),就不重置屏幕显示
    {
        return;
    }
    else if (functions.getCurrPower() == COUNTDOWN) // 如果是倒计时功能条件下,则进入倒计时相关逻辑操作
    {
        displayData[0] = 0;
        displayData[1] = 1;
        displayData[2] = 2;
        powerFlag2 = 1; // 这里一定是到大于0的,后续会根据这个值是否为0来判断逻辑
    }
    lattice.reset();
}

/**
 * @brief 处理单击
 *
 */
void singleAClickHandler()
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
void doubleAClickHandler()
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
void longAClickStartHandler()
{
    Serial.println("A按键长按开始"); // 打印日志,便于校验
    pilotLight.bright();             // 按下的时候LED亮起来
}

/**
 * @brief 处理按键长按
 *
 */
void longAClickHandler()
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
 * @brief 初始化按键信息
 *
 */
void initTouch()
{
    btnA.attachClick(singleAClickHandler);       // 添加单击事件函数
    btnA.attachDoubleClick(doubleAClickHandler); // 添加双击事件函数
    btnA.attachLongPressStop(longAClickHandler); // 添加长按事件函数
    // btnA.attachLongPressStart(longAClickStartHandler); // 添加按下事件函数
}

/**
 * 订阅bilibili用户ID
 */
void subBili(uint8_t *data)
{
    long uid = System::uint8t_to_long(data, 5); // 先将uint_8转成 long
    httptool.saveBuid(uid);                     // 在将uid保存到存储器中去
    functions.setPowerAndMode(BILIFANS, 0);     // 切换显示模式为bilibili显示
}

/**
 * @brief 设置倒计时信息
 *
 * @param data
 */
void setCountdown(uint8_t *data)
{
    long timestamp = System::uint8t_to_long(data, 5); // 先将uint_8转成 long
    datetimes.saveCountdownTimestamp(timestamp);      // 将倒计时时间戳保存起来
    functions.setPowerAndMode(COUNTDOWN, 0);          // 切换显示模式为bilibili显示
}

/**
 * @brief 初始化休眠时间
 *
 */
void initSleepTime()
{
    uint8_t *t = EEPROMTool.loadData(SLEEP_TIME, 5); // 先从内存中加载
    memcpy(sleepTime, t, 4);
    free(t);
}

/**
 * @brief 设置睡眠时间
 *
 * @param data
 */
void setSleepTime(uint8_t *data)
{
    // 这里的做法目前是比较简单的,data就是一个四位长度的数组,第0和1位表示开始时间的小时和分钟,第2和3位表示结束的小时和分钟
    memcpy(sleepTime, data, 4);
    EEPROMTool.saveData(data, SLEEP_TIME, 5); // 将数据设置EEPROM中去
    // todo 这里为了交互友好,最好还是显示一个config ok 之类的提示
}

void sleepTimeLoop()
{
    Times times = datetimes.getTimes();
    int starttime = sleepTime[0] * 100 + sleepTime[1]; // 开始时间
    int endtime = sleepTime[2] * 100 + sleepTime[3];   // 结束时间
    if (starttime == endtime)                          // 如果开始时间和结束时间是一样的话,就什么都不做
    {
        return;
    }
    int currtime = times.h * 100 + times.m; // 当前时间
    if (starttime < endtime)                // 如果开始时间小于结束时间,则只需要判断当前时间是否在开始时间和结束时间的区间范围内
    {
        if (currtime >= starttime && currtime < endtime) // 如果时间在休眠时间范围内则休眠
        {
            if (!isSleepMode)
            {
                isSleepMode = true;    // 标记进入睡眠模式
                if (sleepTime[4] == 0) // 判断亮度是否为0,如果亮度为0的话,则熄灭屏幕
                {
                    lattice.shutdown(true); // 休眠操作(目前就是把屏幕熄灭)
                }
                else
                {
                    lattice.setBrightness(sleepTime[4], false); // 亮度不为0则将设置屏幕亮度为指定的屏幕亮度
                }
            }
        }
        else
        {
            if (isSleepMode)
            {
                // 这里避免出现误操作,每次都将屏幕点亮,将屏幕亮度设置到预设亮度
                isSleepMode = false;                                             // 标记退出睡眠模式
                lattice.shutdown(false);                                         // 退出休眠操作(目前就是把屏幕点亮)
                lattice.setBrightness(lattice.latticeSetting.brightness, false); // 亮度不为0则将设置屏幕亮度为指定的屏幕亮度
            }
        }
    }
    else // 如果开始时间大于结束时间,表示表示当前时间在反向的范围内则不需要休眠
    {
        if (currtime >= endtime && currtime < starttime) // 如果时间在休眠时间范围内则休眠
        {
            if (isSleepMode)
            {
                // 这里避免出现误操作,每次都将屏幕点亮,将屏幕亮度设置到预设亮度
                isSleepMode = false;                                             // 标记退出睡眠模式
                lattice.shutdown(false);                                         // 退出休眠操作(目前就是把屏幕点亮)
                lattice.setBrightness(lattice.latticeSetting.brightness, false); // 亮度不为0则将设置屏幕亮度为指定的屏幕亮度
            }
        }
        else
        {
            if (!isSleepMode)
            {
                isSleepMode = true;    // 标记进入睡眠模式
                if (sleepTime[4] == 0) // 判断亮度是否为0,如果亮度为0的话,则熄灭屏幕
                {
                    lattice.shutdown(true); // 休眠操作(目前就是把屏幕熄灭)
                }
                else
                {
                    lattice.setBrightness(sleepTime[4], false); // 亮度不为0则将设置屏幕亮度为指定的屏幕亮度
                }
            }
        }
    }
}

/**
 * @brief 初始化光敏电阻配置的值
 *
 */
void initLight()
{
    uint8_t t = EEPROMTool.loadDataOne(LIGHT_VAL); // 先从内存中加载
    lattice.latticeSetting.light = t;
}

/**
 * @brief 设置光敏电阻配置的值
 *
 * @param data
 */
void setLight(uint8_t data)
{
    EEPROMTool.saveDataOne(data, LIGHT_VAL); // 将数据设置EEPROM中去
}

void lightLoop()
{
    if (analogRead(A0) < lattice.latticeSetting.light)
    {
        lattice.shutdown(true); // 休眠操作(目前就是把屏幕熄灭)
    }
    else
    {
        lattice.shutdown(false);                                         // 退出休眠操作(目前就是把屏幕点亮)
        lattice.setBrightness(lattice.latticeSetting.brightness, false); // 亮度不为0则将设置屏幕亮度为指定的屏幕亮度
    }
}

/**
 * 每天早上八点重置时间
 *
 */
void resetTimeLoop()
{
    // 确定是连接wifi的状态，如果不是连接wifi的状态则不进行时间重置
    if (WiFi.status() == WL_CONNECTED)
    {
        Times time = datetimes.getTimes();
        if (time.h == 8 && time.m == 0 && time.s == 0)
        {
            // 直接重启时钟用于校准时间
            Serial.println("校准时间！");
            ESP.restart(); // 重启系统
        }
    }
}

/**
 * @brief 重置时间
 * 重置时间这里有两种方式，一种就是用NTP校准时间，还有一种就是设备没有连接wifi，直接用手机发来的时间戳进行校准时间
 * @param data
 */
void resetTime(uint8_t *data)
{
    if (data != NULL) // 函数中参数如果不为空则使用参数值来作为时间来历
    {
        long timestamp = System::uint8t_to_long(data, 5); // 先将uint_8转成 long
        datetimes.setDateTimes(timestamp + 8 * 60 * 60);  // 将时间戳同步到系统时间中去
    }
    else
    {
        udps.updateTime(); // 校准时间
    }
    functions.reset(); // 重置功能
    initStatus();      // 重置状态
}

/**
 * 显示用户自定义的数据
 */
void setUserData(uint8_t *data)
{
    memcpy(lattice.latticeSetting.userData, data, 32); // 切换用户自定义
    functions.setPowerAndMode(CUSTOM, 0);              // 重置功能
    initStatus();                                      // 重置状态
}

/**
 * @brief 显示倒计时
 *
 */
void showCountDown()
{
    bool showmode = true, minutechange = false;
    long countdown = datetimes.getCountdownTimestamp();   // 根据倒计时时间戳获取截止日期
    long timestamp = datetimes.getTimestamp() - 8 * 3600; // 获取当前日期
    if (countdown - timestamp == powerFlag2 || powerFlag2 <= 0)
    {
        return; // 时间没有发生改变,则跳过
    }
    if ((countdown - timestamp) < (24 * 3600)) // 倒计时时间戳 - 当前时间戳时间小于一天则 按 时分秒 来进行倒计时
    {
        showmode = false;
        minutechange = true;
        if (((countdown - timestamp) / 3600) != (powerFlag2 / 3600)) // 倒计时小于一天,则使用时分秒的显示模式
        {
            lattice.reset(); // 倒计时时钟发生改变
            displayData[0] = 0;
            displayData[1] = 1;
            displayData[2] = 2;
        }
    }
    else
    {
        showmode = true;
        if (((countdown - timestamp) / 3600 / 24) != (powerFlag2 / 3600 / 24)) // 这里判断天数是否发生改变,如果天数发生改变则需要重置一下显示
        {
            lattice.reset(); // 倒计时日发生改变
            displayData[0] = 0;
            displayData[1] = 1;
            displayData[2] = 2;
        }
        if (((countdown - timestamp) / 60) != (powerFlag2 / 60)) // 这里判断分钟数是否发生改变,如果分钟数发生改变,则需要刷新显示
        {
            minutechange = true; // 这里表示分钟数值发生改变
        }
    }
    powerFlag2 = (countdown - timestamp) < 1 ? 0 : (countdown - timestamp);
    lattice.showCountDownTime(powerFlag2, displayData, showmode, minutechange);
    for (int i = 0; i < 3; i++)
    {
        displayData[i] = displayData[i] == 6 ? 1 : ++displayData[i];
    }
}

/**
 * @brief 显示日期
 *
 * @param showmode
 */
void showDate(uint8_t showmode)
{
    Dates dates = datetimes.getDates();
    if (dates.d == powerFlag)
    {
        return; // 如果天数没有发生改变，则不更新时间显示
    }
    powerFlag = dates.d;
    displayData[3] = dates.y / 100;
    displayData[2] = dates.y % 100;
    displayData[1] = dates.m;
    displayData[0] = dates.d;
    if (showmode == 1)
    {
        lattice.showLongNumber(displayData);
    }
    else if (showmode == 0)
    {
        lattice.showDate3(displayData);
    }
    else
    {
        displayData[3] = dates.m / 10;
        displayData[2] = dates.m % 10;
        displayData[1] = dates.d / 10;
        displayData[0] = dates.d % 10;
        lattice.showDate2(displayData);
    }
}

/**
 * @brief 显示温度(由于这个温度显示不是很准确,所以我也就没有花很多心思来搞这个,就简单弄一个显示就完事了)
 *
 */
void showTemperature()
{
    int t = datetimes.getTemperature();
    if (t == powerFlag)
    {
        return; // 温度没有发生改变则忽略
    }
    powerFlag = t;
    lattice.reset();
    displayData[3] = 0;
    displayData[2] = 0x00;
    displayData[1] = t / 100;
    displayData[0] = t % 100;
    lattice.showTemperature(displayData);
}

/**
 * 显示bilibili粉丝数量
 */
void showBiliFans()
{
    httptool.bilibiliFans();            // 每次进来判断是否需要更新bilibili粉丝信息,这里是五分钟更新一次
    Times times = datetimes.getTimes(); // 获取系统时间
    if (displayData[0] != times.s && powerFlag < 99999)
    {
        displayData[0] = times.s; // 这里由于没有缓存数组了,所以就用这个缓存数组
        lattice.lightning(3);
    }
    long fans = httptool.fans;
    if (fans == powerFlag)
    {
        return;
    }
    lattice.reset();
    powerFlag = fans;
    for (int i = 0; i < 4; i++) // 将千万的数字分解成四个数组
    {
        displayData[i] = fans % 100;
        fans = fans / 100;
    }
    if (powerFlag >= 99999)
    {
        lattice.showLongNumber(displayData); // 如果粉丝数量大于这个数量时,就显示全部的粉丝数量
    }
    else
    {
        lattice.showNumAndIcon(2, displayData); // 如果粉丝数量不大于那个数量时,就显示图标加数量
    }
}

/**
 * 显示用户自定数据
 */
void showUserData(uint8_t showmode)
{
    if (showmode == 0)
    {
        lattice.showUserData(showmode); // 优先模式
        return;
    }
    if (System::is_overtime(100 + (lattice.latticeSetting.speed * 10)))
    {
        lattice.showUserData(showmode); // 刷新显示内容
    }
}

/**
 * @brief 按键中断处理
 *
 */
void touchLoop()
{
    btnA.tick();
}
#endif
