#ifndef SYSTEM_H
#define SYSTEM_H
#include "EEPROMTool.h"
#include <ESP8266WiFi.h>

static long tmp_value = 1l;

class System
{
private:
    void (*callbackMethod)();

public:
    System();

    void init_callback(void (*callback)());

    /**
     * @brief 重启系统
     *
     */
    static void reset_system();

    /**
     * @brief uint8_t数组转long
     *
     * @param data
     * @return long
     */
    static long uint8t_to_long(uint8_t *data, int length);

    /**
     * @brief 时间延迟函数
     *
     * @param ms 延迟毫秒数
     */
    void delay_time(int ms);

    /**
     * @brief 判断是否超时
     *
     * @param time
     * @return true
     * @return false
     */
    static bool is_overtime(int time);
};

#endif
