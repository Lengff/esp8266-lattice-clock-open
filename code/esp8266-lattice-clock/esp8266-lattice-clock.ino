#include "DateTimes.h"
#include "EEPROMTool.h"
#include "HttpTool.h"
#include "Otas.h"
#include "Touch.h"
#include <DS3231.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Wire.h>

Ticker timestampticker;                              // 手动累加时间戳任务
Ticker httptoolticker;                               // 每五秒钟处理一次http请求标志
HttpTool httptool;                                   //
Otas otas = Otas(&lattice, &pilotLight);             // OTA更新处理对象
DateTimes datetimes;                                 // 时间管理对象
Udps udps = Udps(&datetimes, &lattice, &pilotLight); // UDP数据传输对象
bool updateFansIf = false;                           // 判断是否需要更新B站粉丝

void updateBiliFstatus() { updateFansIf = true; }

/**
 * 订阅bilibili用户ID
 */
void subBili(uint8_t *data)
{
  long uid = System::uint8t_to_long(data, 5); // 先将uint_8转成 long
  httptool.saveBuid(uid);                     // 在将uid保存到存储器中去
  power = BILIFANS;                           // 切换显示模式为bilibili显示
  initStatus();
}

void setCountdown(uint8_t *data)
{
  long timestamp = System::uint8t_to_long(data, 5); // 先将uint_8转成 long
  datetimes.saveCountdownTimestamp(timestamp);      // 将倒计时时间戳保存起来
  power = COUNTDOWN;                                // 切换显示模式为倒计时显示
  initStatus();
}

/**
 * @brief 初始化休眠时间
 *
 */
void initSleepTime()
{
  // 先从内存中加载
  uint8_t *t = EEPROMTool.loadData(SLEEP_TIME, 5);
  for (int i = 0; i < 5; i++)
  {
    sleepTime[i] = t[i];
  }
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
  for (int i = 0; i < 5; i++)
  {
    sleepTime[i] = data[i];
  }
  EEPROMTool.saveData(data, SLEEP_TIME, 5); // 将数据设置EEPROM中去
  // todo 这里为了交互友好,最好还是显示一个config ok 之类的提示
}

void sleepTimeLoop()
{
  Times times = datetimes.getTimes();
  uint8_t starttime = sleepTime[0] * 100 + sleepTime[1]; // 开始时间
  uint8_t endtime = sleepTime[2] * 100 + sleepTime[3];   // 结束时间
  if (starttime == endtime)                              // 如果开始时间和结束时间是一样的话,就什么都不做
  {
    return;
  }
  uint8_t currtime = times.h * 100 + times.m; // 当前时间
  if (starttime < endtime)                    // 如果开始时间小于结束时间,则只需要判断当前时间是否在开始时间和结束时间的区间范围内
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
    udps.updateTime();
  }
  initStatus();
  power = POWER0;
  powers[power] = 0;
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
  if (showmode == 1)
  {
    displayData[3] = dates.y / 100;
    displayData[2] = dates.y % 100;
    displayData[1] = dates.m;
    displayData[0] = dates.d;
    lattice.showLongNumber(displayData);
  }
  else if (showmode == 0)
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
 * 处理接受到的UDP数据
 */
void handleUdpData()
{
  Udpdata udpdata = udps.userLatticeLoop(power, powers[power], version);
  if (udpdata.lh < 1) // 数据长度小于1则表示没有接收到任何数据
  {
    // 没有收到任何UDP数据
    return;
  }
  switch (udpdata.te) // 判断UDP数据类型
  {
  case 0:
    resetTime(udpdata.data); // 重置时间
    break;
  case 1:
    lattice.setBrightness(udpdata.data[0], true); // 设置亮度
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
    otas.updateOta(udpdata.data[0]); // OTA 升级
    break;
  case 10:
    setCountdown(udpdata.data); // 设置倒计时
    break;
  case 11:
    setSleepTime(udpdata.data); // 设置睡眠时间
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
    System::reset_system(); // 重置系统
    break;
  case RESETTIME:
    resetTime(displayData); // 重置时间,这里是随便传的一个参数,不想重新声明参数
    break;
  default:
    break; // 默认不做任何处理
  }
}

void setup()
{
  Serial.begin(115200);                               // 初始化串口波特率
  WiFi.hostname("lattice-clock");                     //设置ESP8266设备名
  initTouch();                                        // 初始化按键信息
  wifis.connWifi();                                   // 连接wifi
  udps.initudp();                                     // 初始化UDP客户端
  pilotLight.dim();                                   //正常进操作就熄灭指示灯
  httptoolticker.attach(5, updateBiliFstatus);        // 每分钟更新一次bilibili粉丝数量
  timestampticker.attach(1, DateTimes::timestampAdd); // 每一秒叠加一次秒数
  if (wifis.wifiMode == 0x00)                         // 如果wifi模式为连接wifi的模式则联网矫正时间
  {
    resetTime(NULL);         // 每次初始化的时候都校准一下时间,这里是随便传的一个参数,不想重新声明参数
    httptool.bilibiliFans(); // 刷新bilibili粉丝数量
  }
  initSleepTime(); // 初始化休眠时间
}

void loop()
{
  wifis.wifiloop();
  handleUdpData();
  touchLoop();
  handlePower();
  sleepTimeLoop();
}
