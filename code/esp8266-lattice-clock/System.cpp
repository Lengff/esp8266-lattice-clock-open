#include "System.h"

System::System()
{
}

void System::init_callback(void (*callback)())
{
    callbackMethod = callback;
}

void System::reset_system()
{
    EEPROMTool.clearAll(); // 删除EEPRON信息
    ESP.restart();         // 重启系统
}

/**
 * @brief uint8_t数组转long
 *
 * @param data
 * @return long
 */
long System::uint8t_to_long(uint8_t *data, int length)
{
    long temp = 0;
    for (int i = 0; i < length; i++)
    {
        temp += data[i] << (i * 8);
    }
    return temp;
}

/**
 * @brief 重新定义延迟函数
 *
 * @param ms
 * @param callback
 */
void System::delay_time(int ms)
{
    int timeFlag = millis();
    while (true)
    {
        if (millis() - timeFlag >= ms)
        {
            break;
        }
        if (callbackMethod != NULL)
        {
            callbackMethod();
        }
    }
}

bool System::is_overtime(int time)
{
    if (millis() - tmp_value < time)
    {
        return false;
    }
    tmp_value = millis();
    return true;
}