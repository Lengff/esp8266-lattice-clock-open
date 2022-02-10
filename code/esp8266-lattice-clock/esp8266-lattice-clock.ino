#include <ESP8266WiFi.h>

#include <DS3231.h>
#include <Wire.h>
#include <Ticker.h>

#include "EEPROMTool.h"
#include "Wifis.h"
#include "Lattice.h"
#include "Udps.h"
#include "Otas.h"
#include "DateTimes.h"
#include "HttpTool.h"

Ticker httptoolticker;
HttpTool httptool;
Wifis wifis;
Udps udps;
DateTimes datetimes = DateTimes();

bool updateFansIf = false;

void updateBiliFstatus() { updateFansIf = true; }

/**
 * 订阅bilibili用户ID
 */
void subBili(uint8_t *data)
{
  // 先将uint_8转成 long
  long uid = 0;
  for (int i = 0; i < 5; i++)
  {
    uid += data[i] << (i * 8);
  }
  // 在将uid保存到存储器中去
  httptool.saveBuid(uid);
  // 切换显示模式为bilibili显示
  power = BILIFANS;
  initStatus();
}

void setCountdown(uint8_t *data)
{
  // 先将uint_8转成 long
  long timestamp = 0;
  for (int i = 0; i < 5; i++)
  {
    timestamp += data[i] << (i * 8);
  }
  // 将倒计时时间戳保存起来
  datetimes.saveCountdownTimestamp(timestamp);
  // 切换显示模式为倒计时显示
  power = COUNTDOWN;
  initStatus();
}

/**
   重置时间
*/
bool resetTime()
{
  udps.startSet(); // 开始设置时间
  while (!udps.isSetTime)
  {
    udps.setTimes(datetimes);
    delay(100);
    lattice.showLongIcon(2); // 这里延迟两秒是因为过程太快了,交互体验不好
  }
  isedit = false;
  isadd = 0;
  initStatus();
  power = POWER0;
  powers[power] = 0;
  ESP.wdtFeed();
}

/**
 * 显示用户自定义的数据
 */
void setUserData(uint8_t *data)
{
  for (int i = 0; i < 32; i++) // 切换用户自定义
  {
    lattice.latticeSetting.userData[i] = data[i];
  }
  power = CUSTOM;
  powers[power] = 0;
  initStatus();
}

/**
   显示时间
*/
void showTime(uint8_t showmode)
{
  Times times = datetimes.getTimes();
  if (times.s == powerFlag)
  {
    return; // 如果秒钟数没有改变,则不执行方法
  }
  if (showmode == 0)
  {
    powerFlag = times.s;
    displayData[0] = times.s;
    displayData[1] = times.m;
    displayData[2] = times.h;
    lattice.showTime3(displayData);
  }
  else if (showmode == 1)
  {
    powerFlag = times.s;
    displayData[0] = times.s;
    displayData[1] = times.m;
    displayData[2] = times.h;
    lattice.showTime(displayData);
  }
  else
  {
    if (times.s == 0 || powerFlag == -1)
    {
      displayData[0] = times.m % 10;
      displayData[1] = times.m / 10;
      displayData[2] = times.h % 10;
      displayData[3] = times.h / 10;
      lattice.showTime2(displayData);
    }
    powerFlag = times.s;
    if (times.s % 2 == 0)
    {
      lattice.reversalLR(3);
    }
    else
    {
      lattice.reversalUD(3);
    }
  }
}

/**
 * 显示倒计时
 */
void showCountDown()
{
  bool showmode = true, minutechange = false;
  long countdown = datetimes.getCountdownTimestamp();
  long timestamp = datetimes.getTimestamp() - 8 * 3600;
  if (countdown - timestamp == powerFlag2 || powerFlag2 <= 0)
  {
    // 时间没有发生改变,则跳过
    return;
  }
  // 倒计时时间戳 - 当前时间戳时间小于一天则 按 时分秒 来进行倒计时
  if ((countdown - timestamp) < (24 * 3600))
  {
    showmode = false;
    minutechange = true;
    // 倒计时小于一天,则使用时分秒的显示模式
    if ((countdown - timestamp) == powerFlag2)
    {
      // 这里表示秒钟数没有发生改变
      return;
    }
    if (((countdown - timestamp) / 3600) != (powerFlag2 / 3600))
    {
      // 倒计时时钟发生改变
      lattice.reset();
      displayData[0] = 0;
      displayData[1] = 1;
      displayData[2] = 2;
    }
  }
  else
  {
    showmode = true;
    // 这里判断天数是否发生改变,如果天数发生改变则需要重置一下显示
    if (((countdown - timestamp) / 3600 / 24) != (powerFlag2 / 3600 / 24))
    {
      lattice.reset();
      // 倒计时日发生改变
      displayData[0] = 0;
      displayData[1] = 1;
      displayData[2] = 2;
    }
    // 这里判断分钟数是否发生改变,如果分钟数发生改变,则需要刷新显示
    if (((countdown - timestamp) / 60) != (powerFlag2 / 60))
    {
      // 这里表示分钟数值发生改变
      minutechange = true;
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
 * 手动编辑时间 --> 这个功能只做到了一半, 看看就好了
 */
void editTime(uint8_t addit, uint8_t showmode)
{
  if (showmode == 0)
  {
    displayData[0] = addit == 1 ? (displayData[0] == 59 ? 0 : ++displayData[0])
                                : (displayData[0] == 0 ? 59 : --displayData[0]);
  }
  else if (showmode == 1)
  {
    displayData[1] = addit == 1 ? (displayData[1] == 59 ? 0 : ++displayData[1])
                                : (displayData[1] == 0 ? 59 : --displayData[1]);
  }
  else if (showmode == 2)
  {
    displayData[2] = addit == 1 ? (displayData[2] == 23 ? 0 : ++displayData[2])
                                : (displayData[2] == 0 ? 23 : --displayData[2]);
  }
  else
  {

    resetTime(); // 提前结束
    powers[power] = 0;
    return;
  }
  lattice.showTime(displayData);
}

/**
   显示日期
*/
void showDate(uint8_t showmode)
{
  Dates dates = datetimes.getDates();
  if (dates.d == powerFlag)
  {
    return; // 如果天数没有发生改变，则不更新时间显示
  }
  powerFlag = dates.d;
  if (showmode == 0)
  {
    displayData[3] = dates.y / 100;
    displayData[2] = dates.y % 100;
    displayData[1] = dates.m;
    displayData[0] = dates.d;
    lattice.showLongNumber(displayData);
  }
  else if (showmode == 1)
  {
    displayData[3] = dates.y / 100;
    displayData[2] = dates.y % 100;
    displayData[1] = dates.m;
    displayData[0] = dates.d;
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
   显示温度
   由于这个温度显示不是很准确,所以我也就没有花很多心思来搞这个,就简单弄一个显示就完事了
*/
void showTemperature()
{
  int t = datetimes.getTemperature();
  if (t == powerFlag)
  {
    // 温度没有发生改变则忽略
    return;
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
  if (updateFansIf) // 判断是否需要更新bilibili粉丝数量
  {
    Serial.println(datetimes.getTimestamp());
    httptool.bilibiliFans(); // 每五秒获取一次bilibili粉丝信息
    updateFansIf = false;    // 重置状态
  }
  Times times = datetimes.getTimes();
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
  if (millis() - powerFlag < 100 + (lattice.latticeSetting.speed * 10)) // 如果当前时间减上次刷新时间小于用户设置的速度,则不刷新
  {
    return; // 时间间隔小于100ms就不执行
  }
  powerFlag = millis();
  lattice.showUserData(showmode); // 刷新显示内容
}

/**
 * 重置系统，删除flash中的信息
 */
void resetsystem()
{
  EEPROMTool.clearAll(); // 删除EEPRON信息
  ESP.restart();         // 重启系统
}

/**
 * 处理接受到的UDP数据
 */
void handleUdpData()
{
  if (!updateFansIf)
  {
    // 五秒种打印一次数据信息 todo 这里后期可以改成MQTT协议向服务器上报状态信息
  }
  Udpdata udpdata = udps.userLatticeLoop(lattice.latticeSetting, power, powers[power], version);
  if (udpdata.lh < 1) // 数据长度小于1则表示没有接收到任何数据
  {
    return; // 没有收到任何UDP数据
  }
  switch (udpdata.te) // 判断UDP数据类型
  {
  case 0:
    resetTime(); // 重置时间
    break;
  case 1:
    lattice.setBrightness(udpdata.data[0]); // 设置亮度
    break;
  case 2:
    power = udpdata.data[0]; // 切换功能
    initStatus();
    break;
  case 3:
    powers[power] = udpdata.data[0]; // 切换功能显示样式
    initStatus();
    break;
  case 4:
    subBili(udpdata.data); // 订阅BIlibiliUID
    break;
  case 5:
    lattice.shutdown(udpdata.data[0]); // 是否启用点阵屏幕
    break;
  case 6:
    lattice.setDirection(udpdata.data[0]); // 切换显示方向
    break;
  case 7:
    setUserData(udpdata.data); // 设置用户数据
    break;
  case 8:
    lattice.latticeSetting.speed = udpdata.data[0]; // 设置动画速度
    power = CUSTOM;
    break;
  case 9:
    updateOta((int)udpdata.data[0]); // OTA 升级
    break;
  case 10:
    setCountdown(udpdata.data); // 设置倒计时
    break;
  default:
    break;
  }
}

/**
 * 功能处理
 */
void handlePower()
{
  if (!isedit)
  {
    switch (power) // 显示数据模式
    {
    case POWER0:
      showTime(powers[power]); // 显示时间
      break;
    case POWER1:
      showDate(powers[power]); // 显示日期
      break;
    case POWER2:
      showTemperature(); // 显示温度
      break;
    case BILIFANS:
      showBiliFans(); // 显示bilibili粉丝数量
      break;
    case CUSTOM:
      showUserData(powers[power]); // 显示用户自定义的数据
      break;
    case COUNTDOWN:
      showCountDown(); // 显示倒计时
      break;
    case RESET:
      resetsystem(); // 重置系统
      break;
    case RESETTIME:
      resetTime(); // 重置时间
      break;
    default:
      break; // 默认不做任何处理
    }
  }
  else if (isedit && isadd != 0)
  {
    switch (power) // 编辑数据模式 （此功能待商榷）
    {
    case POWER0:
      editTime(isadd, powers[power]);
      break;
    default:
      break;
    }
    isadd = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  initTouch();                                 // 初始化按键信息
  wifis.connWifi(lattice);                     // 连接wifi
  udps.initudp();                              // 初始化UDP客户端
  httptoolticker.attach(5, updateBiliFstatus); // 每分钟更新一次bilibili粉丝数量
  httptool.bilibiliFans();                     // 刷新bilibili粉丝数量
}

void loop()
{
  handleUdpData();
  touchLoop();
  handlePower();
}
