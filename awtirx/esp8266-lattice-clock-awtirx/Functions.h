#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "PilotLight.h"

enum ModeEnum
{
    // 功能0: 显示时间
    SHOW_TIME = 0,
    // 功能1: 显示日期
    SHOW_DATE = 1,
    // 功能2: 显示日期
    POWER2 = 2,
    // 显示bilibili粉丝数量
    BILIFANS = 3,
    // 显示用户自定义数据
    CUSTOM = 4,
    // 显示倒计时
    COUNTDOWN = 5,
    // NTP更新时间
    RESETTIME = 98,
    // 系统重置
    RESET = 99
};

#define POWER_SIZE 6

const int modePower[POWER_SIZE] = {3, 3, 1, 1, 5, 1}; // 每个功能对应多少种模式

class Functions
{
private:
    /**
     * @brief 状态指示灯对象
     *
     */
    PilotLight *pilotLight;

    /**
     * @brief 当前功能对应的当前模式，因为用很多不同的功能，每个功能又有不同的模式，所以这里就要列举不同功能对应的不同模式
     *
     */
    unsigned char currMode[POWER_SIZE] = {0, 0, 0, 0, 0, 1};

    /**
     * @brief 当前所处在的功能
     *
     */
    unsigned char currPower = 0;

public:
    Functions();

    /**
     * @brief 带参数的构造函数
     *
     * @param pilotLightobj
     */
    Functions(PilotLight *pilotLightobj);

    /**
     * @brief 重置功能以及模式
     *
     */
    void reset();

    /**
     * @brief 设置功能和模式函数
     *
     * @param power
     * @param mode
     */
    void setPowerAndMode(unsigned char power, unsigned char mode);

    /**
     * @brief 设置功能
     *
     * @param power
     */
    void setPower(unsigned char power);

    /**
     * @brief 设置当前模式
     *
     * @param mode
     */
    void setMode(unsigned char mode);

    /**
     * @brief 功能加 目前基本都只用到了功能加
     *
     */
    void powerUp();

    /**
     * @brief 功能减
     *
     */
    void powerDown();

    /**
     * @brief 模式加 目前基本都只用到了模式加
     *
     * @param power
     */
    void modeUp();

    /**
     * @brief 模式减
     *
     * @param power
     */
    void modeDown();

    /**
     * @brief 获取当前功能
     *
     * @return unsigned char
     */
    unsigned char getCurrPower();

    /**
     * @brief 获取当前模式
     *
     * @return unsigned char
     */
    unsigned char getCurrMode();
};

#endif