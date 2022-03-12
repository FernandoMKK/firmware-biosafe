#ifndef TERMISTOR_H
#define TERMISTOR_H

#include <Arduino.h>

class Termistor
{
private:
    unsigned short int ADC_pin;
    float currentTemperature;
    float tempMax, tempMin;
public:
    Termistor();
    ~Termistor();

    void attachToPin(unsigned short int ADC_pin);
    void setScale(float temp1, float temp2);
    float getTemperature();
};
#endif //TERMISTOR_H