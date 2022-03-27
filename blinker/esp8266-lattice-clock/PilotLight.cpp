#include "PilotLight.h"

PilotLight::PilotLight()
{
    pinMode(LED_PIN, OUTPUT);
}

void PilotLight::flashing()
{
    status = status == HIGH ? LOW : HIGH;
    digitalWrite(LED_PIN, status);
}

void PilotLight::flashing(int ms)
{
    bright();
    delay(ms);
    dim();
}

void PilotLight::bright(){
    digitalWrite(LED_PIN, LOW);
}

void PilotLight::dim(){
    digitalWrite(LED_PIN, HIGH);
}