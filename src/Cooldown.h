#ifndef Cooldown_h
#define Cooldown_h

#include "Particle.h"

/**
 * The code defines a class called `Cooldown`. This class represents a cooldown timer that can be used to 
 * execute a specified function at regular intervals.
 */
class Cooldown
{
    unsigned long previousMillis;
    long interval;
    void (*func)();

public:
    /**
     * The above function is a constructor for a Cooldown object that takes an interval in milliseconds and
     * a function pointer as parameters.
     *
     * @param intervalMillis The interval in milliseconds between each execution of the function.
     * @param func The "func" parameter is a pointer to a function that takes no arguments and returns
     * nothing. It is used to specify the function that will be called when the cooldown interval has
     * elapsed.
     */
    Cooldown(long intervalMillis, void (*func)());

    /**
     * The function updates a cooldown timer and calls a specified function when the cooldown interval has
     * elapsed.
     */
    void update();
    long getInterval();
    void setInterval(long intervalMillis);
};

#endif