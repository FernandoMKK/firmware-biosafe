#include "EndSwitch.h"

EndSwitch::EndSwitch()
{
    //ctor
}

EndSwitch::~EndSwitch()
{
    //dtor
}

void EndSwitch::attachToPin(unsigned short int pin)
{
    pinMode(pin, INPUT);
    this->currentPin = pin;
}

unsigned short int EndSwitch::getCurrentPin()
{
    return this->currentPin;
}

bool EndSwitch::read()
{
    this->status = digitalRead(getCurrentPin());
    return this->status;
}
