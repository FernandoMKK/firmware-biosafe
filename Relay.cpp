#include "Relay.h"

Relay::Relay()
{
    //ctor
}

Relay::~Relay()
{
    //dtor
}

void Relay::attachToPin(unsigned short int pin)
{
    pinMode(pin, OUTPUT);

    this->relayPin = pin;
}

bool Relay::getStatusRelay()
{
    return this->statusRelay;
}

void Relay::setRelay(bool status)
{
    digitalWrite(this->relayPin, status);
    this->statusRelay = status;
}
