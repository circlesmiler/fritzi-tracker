#include "Cooldown.h"

Cooldown::Cooldown(long interval, void (*func)())
    : interval(interval), func(func), previousMillis(0) {}

void Cooldown::update()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        func();
    }
}