#include "Cooldown.h"

Cooldown::Cooldown(long intervalMillis, void (*func)())
    : interval(intervalMillis), func(func), previousMillis(0) {}

void Cooldown::update()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        func();
    }
}

long Cooldown::getInterval() {
    return interval;
}

void Cooldown::setInterval(long intervalMillis) {
    interval = intervalMillis;
}