#include "LED.h"

LED::LED()
{
    //ctor
}

LED::~LED()
{
    //dtor
}

bool LED::getStatus()
{
    return this->status;
}
unsigned short int LED::getCurrentPin()
{
    return this->currentPin;
}

void LED::attachToPin(unsigned short int pin)
{
    pinMode(pin, OUTPUT);
    this-> currentPin = pin;
}
void LED::setOff()
{
    digitalWrite(getCurrentPin(), LOW);
    this->status = LOW;
}
void LED::setOn()
{
    digitalWrite(getCurrentPin(), HIGH);
    this->status = HIGH;
}
