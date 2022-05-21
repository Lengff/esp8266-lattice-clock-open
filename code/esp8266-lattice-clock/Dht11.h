/**
 * @file Dht11.h
 * @author Lengff
 * @brief 本段代码主要参考于这里，大家可以前往这里查阅：https://zhuanlan.zhihu.com/p/163727848
 * @version 0.1
 * @date 2022-04-21
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef DHT11_H
#define DHT11_H

#include <ESP8266WiFi.h>

#define DHT11_PIN 16 // 指定DHT的引脚

/**
 * @brief 温度和湿度的结构体
 *
 */
struct Tah
{
    /**
     * @brief 温度
     *
     */
    double temperature;
    /**
     * @brief 湿度
     *
     */
    double humdity;
};

class Dht11
{
private:
    Tah tah;

    /**
     * @brief 上一次获取温度的时间
     *
     */
    long lasttime = 0;

    /**
     * @brief dht11读取到的温度数据
     *
     */
    byte data[4];

    /**
     * @brief 读取温度的函数
     *
     * @return byte
     */
    byte read_data();

    /**
     * @brief 将数据设置到data中去
     *
     */
    void set_data();

    /**
     * @brief 将二进制编码的十进制数转换为普通十进制数
     *
     * @param val
     * @return byte
     */
    byte bcdToDec(byte val);

public:
    Dht11();

    /**
     * @brief 获取温度湿度结构体
     *
     * @return Tah
     */
    Tah get_tah();
};
#endif