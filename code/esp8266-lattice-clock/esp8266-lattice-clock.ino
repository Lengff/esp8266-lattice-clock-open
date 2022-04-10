#include "Main.h"
// 这里加了一个点灯科技的代码支持，假如说你想用点灯科技，则可以注释掉下面这段代码
// #include "BlinkerSupport.h"
// 然后在step函数中注释掉那段initBlinker();和Blinker.run();代码即可
// 关于这段的说明请参考：https://gitee.com/lengff/esp8266-lattice-clock-open/tree/master/blinker

#define LATTICE_CLOCK_VERSION 9 // 点阵时钟代码版本号码

/**
 * 处理接受到的UDP数据
 */
void handleUdpData()
{
  Udpdata udpdata = udps.userLatticeLoop(functions.getCurrPower(), functions.getCurrMode(), LATTICE_CLOCK_VERSION);
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
    functions.setPowerAndMode(udpdata.data[0], 0); // 切换功能
    initStatus();
    break;
  case 3:
    functions.setMode(udpdata.data[0]); // 切换功能模式
    initStatus();
    break;
  case 4:
    httptool.updateBilibiliFlag(); // 更新bilibili粉丝数量前,需要重置一下flag
    subBili(udpdata.data);         // 订阅BIlibiliUID
    initStatus();
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
    functions.setPower(CUSTOM);
    break;
  case 9:
    otas.updateOta(udpdata.data[0]); // OTA 升级
    break;
  case 10:
    setCountdown(udpdata.data); // 设置倒计时
    initStatus();
    break;
  case 11:
    setSleepTime(udpdata.data); // 设置睡眠时间
    break;
  default:
    break;
  }
}

/**
 * @brief 显示时间回调处理
 *
 */
void showTimeCallback()
{
  handleUdpData();
  touchLoop();
  // Blinker.run();
}

/**
 * @brief 显示时间
 *
 * @param showmode
 */
void showTime(uint8_t showmode)
{
  Times times = datetimes.getTimes();
  if (times.s == powerFlag)
  {
    return; // 如果秒钟数没有改变,则不执行方法
  }
  powerFlag = times.s;
  displayData[0] = times.s;
  displayData[1] = times.m;
  displayData[2] = times.h;
  if (showmode == 0)
  {
    lattice.showTime3(displayData, showTimeCallback);
  }
  else if (showmode == 1)
  {
    lattice.showTime(displayData, showTimeCallback);
  }
  else
  {
    if (times.s == 0 || powerFlag == -1)
    {
      displayData[0] = times.m % 10;
      displayData[1] = times.m / 10;
      displayData[2] = times.h % 10;
      displayData[3] = times.h / 10;
      lattice.showTime2(displayData, showTimeCallback);
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
 * 功能处理
 */
void handlePower()
{
  switch (functions.getCurrPower()) // 显示数据模式
  {
  case SHOW_TIME:
    showTime(functions.getCurrMode()); // 显示时间
    break;
  case SHOW_DATE:
    showDate(functions.getCurrMode()); // 显示日期
    break;
  case POWER2:
    showTemperature(); // 显示温度
    break;
  case BILIFANS:
    showBiliFans(); // 显示bilibili粉丝数量
    break;
  case CUSTOM:
    showUserData(functions.getCurrMode()); // 显示用户自定义的数据
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
  Serial.begin(115200);                                             // 初始化串口波特率
  WiFi.hostname("lattice-clock");                                   //设置ESP8266设备名
  initTouch();                                                      // 初始化按键信息
  wifis.connWifi();                                                 // 连接wifi
  udps.initudp();                                                   // 初始化UDP客户端
  httptoolticker.attach(5 * 6 * 1000, httptool.updateBilibiliFlag); // 每五分分钟更新一次更新bilibili粉丝flag
  timestampticker.attach(1, DateTimes::timestampAdd);               // 每一秒叠加一次秒数
  if (!wifis.isApMode())                                            // 如果wifi模式为连接wifi的模式则联网矫正时间
  {
    resetTime(NULL);               // 每次初始化的时候都校准一下时间,这里是随便传的一个参数,不想重新声明参数
    httptool.updateBilibiliFlag(); // 更新bilibili粉丝数量前,需要重置一下flag
    httptool.bilibiliFans();       // 刷新bilibili粉丝数量
    // initBlinker();
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
  // Blinker.run();
}
