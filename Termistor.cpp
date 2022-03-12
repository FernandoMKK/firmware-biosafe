#include "Termistor.h"
Termistor::Termistor()
{
    //ctor
}

Termistor::~Termistor()
{
    //dtor
}

void Termistor::attachToPin(unsigned short int ADC_pin)
{
    this->ADC_pin = ADC_pin;
}
void Termistor::setScale(float temp1, float temp2)
{
    this->tempMin = temp1;
    this->tempMax = temp2;
}
float Termistor::getTemperature()
{
    int digitalValue = analogRead(this->ADC_pin);
    this->currentTemperature = map(digitalValue, 0, 4095, this->tempMin, this->tempMax);
    return this->currentTemperature;
}
