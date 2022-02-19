#ifndef PILOT_LIGHT
#define PILOT_LIGHT

#include <ESP8266WiFi.h>

#define LED_PIN D0 // LED引脚

class PilotLight
{
private:
    int status = HIGH;

public:
    PilotLight();

    /**
     * LED 闪烁
     */
    void flashing();

    /**
     * @brief  LED 闪烁
     * 
     * @param ms 
     */
    void flashing(int ms);

    /**
     * @brief LED亮
     * 
     */
    void bright();

    /**
     * @brief LED灭
     * 
     */
    void dim();
};
#endif