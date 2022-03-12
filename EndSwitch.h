#ifndef ENDSWITCH_H
#define ENDSWITCH_H

#include <Arduino.h>

class EndSwitch
{
private:
    bool status;
    unsigned short int currentPin;

public:
    EndSwitch();
    ~EndSwitch();

    bool read();
    void attachToPin(unsigned short int pin);

    unsigned short int getCurrentPin();
};
#endif //ENDSWITCH_H
