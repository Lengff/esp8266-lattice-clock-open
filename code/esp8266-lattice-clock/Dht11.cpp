#include "Dht11.h"

Dht11::Dht11()
{
    pinMode(DHT11_PIN, OUTPUT);
}

byte Dht11::read_data()
{
    byte data = 0; // 初始化数据，不然可能出错
    for (int i = 0; i < 8; i++)
    {
        if (digitalRead(DHT11_PIN) == LOW) // 一开始要 LOW 才表示要传过来
        {
            while (digitalRead(DHT11_PIN) == LOW) // 现在已经变成 HIGH 了
            {
                // Do nothing
            }
            delayMicroseconds(30);              //等待 50us；判断高电平持续时间，以判定资料是‘0’还是‘1’；
            if (digitalRead(DHT11_PIN) == HIGH) // 持续了 30 us 以上就是 1
            {
                data |= (1 << (7 - i)); //高位在前，低位元在后；
            }
            // 如果这时已经是 LOW, 表示这 bit 是 0, 不必塞入 data
            //..而且以下的 while 也会立即结束(因为 LOW), 准备接收下一个 bit
            while (digitalRead(DHT11_PIN) == HIGH) // 等待下一bit的接收；
            {
                // Do nothing
            }
        }
    }
    return data; // 收完 8 bit 等于 1 byte 等于 1 char
}

void Dht11::set_data()
{
    digitalWrite(DHT11_PIN, LOW);          //拉低到 LOW，发送表示要开始沟通的信号；
    delay(30);                             //延时要大于 18ms，以便 DHT11 能检测到开始信号；我们用30ms
    digitalWrite(DHT11_PIN, HIGH);         // 拉高HIGH, 让 DHT11 拉低到 LOW 告诉我们要传送
    delayMicroseconds(40);                 // 给40us等待 DHT11 响应；
    pinMode(DHT11_PIN, INPUT);             // 改为输入 mode 准备 digitalRead( )
    while (digitalRead(DHT11_PIN) == HIGH) // 必须等到 LOW
    {
        // Do nothing
    }
    delayMicroseconds(80);                // DHT11 发出响应，会拉低 80us；所以至少等80us
    while (digitalRead(DHT11_PIN) == LOW) // 继续等到变 HIGH
    {
        // Do nothing
    }
    delayMicroseconds(80);      // DHT11 会拉高到HIGH 80us 后开始发送数据；
    for (int i = 0; i < 5; i++) // 以下连续读入 5 bytes (40 bits), 最后的 byte 是 checksum 校验值
    {
        data[i] = read_data(); //接收温湿度资料，校验位元；
    }
    //  根据datasheet规定, dat[4] 要 == (dat[0]+dat[1]+dat[2]+dat[3]) %256 否则表示沟通有错误 !!
    pinMode(DHT11_PIN, OUTPUT);    // 改为 Output mode, 准备拉高HIGH
    digitalWrite(DHT11_PIN, HIGH); //发送完一次资料后释放bus，等待下一次开始信号；
}

/**
 * @brief 将二进制编码的十进制数转换为普通十进制数
 *
 * @param val
 * @return byte
 */
byte Dht11::bcdToDec(byte val)
{
    // 将二进制编码的十进制数转换为普通十进制数
    return ((val / 16 * 10) + (val % 16));
}

/**
 * @brief 获取温度湿度结构体
 *
 * @return Tah
 */
Tah Dht11::get_tah()
{
    if (millis() - lasttime < 1985 * 10)
    {
        return tah;
    }
    lasttime = millis();
    set_data();
    tah.humdity = data[0] + (data[1] / 10.00);
    tah.temperature = data[2] + (data[3] / 10.00);
    return tah;
}