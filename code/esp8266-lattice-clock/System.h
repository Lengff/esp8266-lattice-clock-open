#ifndef SYSTEM_H
#define SYSTEM_H
#include "EEPROMTool.h"
#include <ESP8266WiFi.h>

class System
{
private:
    /* data */
public:
    System();

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
};


#endif