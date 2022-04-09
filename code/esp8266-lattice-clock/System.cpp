#include "System.h"

System::System()
{
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
