#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED
{
private:
    bool status;
    unsigned short int currentPin;

public:
    LED();
    ~LED();

    //getters
    bool getStatus();
    unsigned short int getCurrentPin();

    //Methods
    void attachToPin(unsigned short int pin);
    void setOff();
    void setOn();
};
#endif //LED_H
