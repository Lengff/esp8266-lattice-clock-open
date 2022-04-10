#include "Functions.h"

Functions::Functions()
{
}

Functions::Functions(PilotLight *pilotLightobj)
{
    pilotLight = pilotLightobj;
}

/**
 * @brief 重置功能和模式
 *
 */
void Functions::reset()
{
    currPower = 0;
    currMode[currPower] = 0;
}

/**
 * @brief 设置功能和模式
 *
 * @param power
 * @param mode
 */
void Functions::setPowerAndMode(unsigned char power, unsigned char mode)
{
    currPower = power;
    currMode[currPower] = mode;
}

/**
 * @brief 设置功能
 *
 * @param power
 */
void Functions::setPower(unsigned char power)
{
    currPower = power;
}

/**
 * @brief 设置当前模式
 *
 * @param mode
 */
void Functions::setMode(unsigned char mode)
{
    currMode[currPower] = mode;
}

/**
 * @brief 功能加 目前基本都只用到了功能加
 *
 */
void Functions::powerUp()
{
    // 当前功能加到超过最大值时,就重置为默认的功能
    if (currPower == POWER_SIZE - 1)
    {
        currPower = 0;
    }
    else
    {
        ++currPower; // 功能加加
    }
    pilotLight->flashing(50); // 功能处理完闪烁一下状态指示灯
}

/**
 * @brief 功能减
 *
 */
void Functions::powerDown()
{
    // todo 这个函数先不写
}

/**
 * @brief 模式加 目前基本都只用到了模式加
 *
 * @param power
 */
void Functions::modeUp()
{
    // 如果说当前模式超过最大模式,则设置为初始模式
    if (currMode[currPower] == (modePower[currPower] - 1))
    {
        currMode[currPower] = 0;
    }
    else
    {
        ++currMode[currPower];
    }
    pilotLight->flashing(50); // 功能处理完闪烁一下状态指示灯
}

/**
 * @brief 模式减
 *
 * @param power
 */
void Functions::modeDown()
{
    // todo 这个函数先不写
}

/**
 * @brief 获取当前功能
 *
 * @return unsigned char
 */
unsigned char Functions::getCurrPower()
{
    return currPower;
}

/**
 * @brief 获取当前模式
 *
 * @return unsigned char
 */
unsigned char Functions::getCurrMode()
{
    return currMode[currPower];
}