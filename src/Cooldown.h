#ifndef Cooldown_h
#define Cooldown_h

#include "Particle.h"

class Cooldown
{
    unsigned long previousMillis;
    long interval;
    void (*func)();

public:
    Cooldown(long interval, void (*func)());
    void update();
};

#endif