#include "Udps.h"

Udps::Udps()
{
  // todo
}

Udps::Udps(DateTimes *datetimesobj, LatticePlus *latticeobj, PilotLight *pilotLightobj)
{
  datetimes = datetimesobj;
  lattice = latticeobj;
  pilotLight = pilotLightobj;
}

void Udps::initudp()
{
  // todo 这里需要判断网络状态
  WiFi.hostByName(ntpServerName, timeServerIP); //将域名转换成IP地址
  udp.begin(localPort);                         // 启动监听本地端口
}

/**
 * @brief 发送NTP请求
 *
 */
void Udps::sendNTPpacket()
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);     // 将字节数组的数据全部设置为0
  packetBuffer[0] = 0b11100011;                 // 请求部分其实是有很多数据的,具体的请看参考请求报文说明,这里我们就只设置一个请求头部分即可
  udp.beginPacket(timeServerIP, remoteNtpPort); // 配置远端ip地址和端口
  udp.write(packetBuffer, NTP_PACKET_SIZE);     // 把数据写入发送缓冲区
  udp.endPacket();                              // 发送数据
  Serial.println("send ntp data");              //
}

long Udps::getNtpTimestamp()
{
  int packetSize = udp.parsePacket(); //解析Udp数据包
  if (!packetSize)                    //解析包为空
  {
    return 0;
  }
  else //解析包不为空
  {
    udp.read(packetBuffer, NTP_PACKET_SIZE); // 解析UDP数据包中的数据
    // todo这里获取到的时间其实不是真实的时间,实际上还包含了网络延时的,但是为了方便,这里我们忽略这个因素的存在
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]); // 取出t2时间的高位和低位数据拼凑成以秒为单位的时间戳
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  //
    unsigned long secsSince1900 = highWord << 16 | lowWord;            // 拼凑成以秒为单位的时间戳(时间戳的记录以秒的形式从 1900-01-01 00:00:00算起)
    const unsigned long seventyYears = 2208988800UL;                   //
    unsigned long timestamp = secsSince1900 - seventyYears;            // 前面的32bit是时间戳的秒数(是用1900-01-01 00:00:00开始的秒数,但是我们的是1970年,所以需要减掉2208988800秒)
    timestamp = timestamp + 8 * 60 * 60;                               // 这里加8 是因为时区的问题,如果不加8,得到的结果就会是其他时区的时间
    return timestamp;
  }
}

Udpdata Udps::userLatticeLoop(uint8_t power, uint8_t mode, uint8_t version)
{
  Udpdata udpdata;
  udpdata.lh = 0;
  int packetSize = udp.parsePacket();         //解析Udp数据包
  if (packetSize)                             //解析包不为空
  {                                           //
    pilotLight->flashing(100);                // 每次接收到UDP数据的时候都闪烁一下LED灯
    memset(packetBuffer, 0, NTP_PACKET_SIZE); //每次都先清空掉原有的数据包
    udp.read(packetBuffer, NTP_PACKET_SIZE);  // 读取UDP数据
    // Udp.remoteIP().toString().c_str()用于将获取的远端IP地址转化为字符串
    // Serial.printf("收到来自远程IP：%s（远程端口：%d）的数据包字节数：%d  %X\n", udp.remoteIP().toString().c_str(), udp.remotePort(), packetSize, packetBuffer[8]);
    uint8_t vn = packetBuffer[0] & 0x3; // 数据包版本
    if (vn != 0x1)                      // 如果数据包版本不为1,则直接结束
    {
      udp.beginPacket(udp.remoteIP(), udp.remotePort()); //向udp工具发送消息
      replyPacket[0] = {0x01};                           //
      udp.write(replyPacket);                            //把数据写入发送缓冲区
      udp.endPacket();                                   //发送数据
      return udpdata;
    }
    udpdata.rt = packetBuffer[0] >> 6; // 数据包接收成功返回值
    if (udpdata.rt == 0x0)             // 如果返回类型值为0的时候,则上报状态信息值
    {
      udp.beginPacket(udp.remoteIP(), udp.remotePort());         //向udp工具发送消息 todo 这里的做法是错误的，我们自定义协议，返回的也应该是按协议来返回的，但是我就是想偷个懒
      replyPacket[0] = lattice->latticeSetting.isShutdown << 6;  // 是否显示
      replyPacket[0] += lattice->latticeSetting.brightness << 2; // 显示亮度
      replyPacket[0] += lattice->latticeSetting.direction;       // 显示方向
      replyPacket[1] = power;                                    // 功能
      replyPacket[2] = mode;                                     // 功能模式
      replyPacket[3] = lattice->latticeSetting.speed;            // 动画移动速度
      replyPacket[4] = version;                                  // 系统版本
      udp.write(replyPacket, 5);                                 //把数据写入发送缓冲区
      udp.endPacket();                                           //发送数据
      return udpdata;
    }
    udpdata.te = (packetBuffer[0] & 0x3f) >> 2; // 功能模式
    udpdata.lh = packetBuffer[1];               // 数据长度
    Serial.print("UDP接收到的数据信息为: ");
    Serial.print(udpdata.rt);
    Serial.print(udpdata.te);
    Serial.println(udpdata.lh);
    // 数据包
    for (int i = 0; i < udpdata.lh; i++)
    {
      udpdata.data[i] = packetBuffer[i + 2];
    }
    udp.beginPacket(udp.remoteIP(), udp.remotePort()); //向udp工具发送消息
    replyPacket[0] = {0x01};                           //
    udp.write(replyPacket, 3);                         //把数据写入发送缓冲区
    udp.endPacket();                                   //发送数据
    return udpdata;
  }
  return udpdata;
}

/**
 * @brief 更新系统时间
 *
 */
void Udps::updateTime()
{
  int count = 30;               // 一共尝试五次
  sendNTPpacket();             // 向NTP服务器发请求,先发送更新时间的操作
  int lastSendTime = millis(); // 记录上一次发送更新时间的时间戳
  while (true)                 // 这里用一个死循环来搞定,先发送更新时间的操作
  {
    long timeStamp = getNtpTimestamp(); // 获取当前时间戳
    if (timeStamp != 0)                 // 说明已经获取到了时间信息,这里直接做更新操作
    {
      datetimes->setDateTimes(timeStamp); // 给时钟设置时间
      break;                              // 跳出死循环
    }
    else
    {
      if (millis() - lastSendTime >= 1000) // 如果说发送时间超出上次发送时间1秒钟,则重新发送NTP请求
      {
        sendNTPpacket(); // 向NTP服务器发请求,先发送更新时间的操作
        lastSendTime = millis();
        count--;
      }
    }
    lattice->showLongIcon(2); // 这里延迟两秒是因为过程太快了,交互体验不好
    pilotLight->flashing();   // 校准时间LED闪烁
    delay(100);               // 等待100ms
    if (count < 0)            // 如果说所有机会都用完了
    {
      break; // 跳出死循环
    }
  }
}
