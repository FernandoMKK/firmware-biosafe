#ifndef RELAYSHIELD_H
#define RELAYSHIELD_H

#include <Arduino.h>

class Relay
{
private:
   unsigned short int relayPin;
   bool statusRelay;

public:
    Relay();
    ~Relay();

    void attachToPin(unsigned short int pin);
    bool getStatusRelay();
    void setRelay(bool status);
};
#endif //RELAYSHIELD_H
