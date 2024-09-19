#ifndef VoltageStatistics_H
#define VoltageStatistics_H

#include "ATMega32_utility_bib.h"
#include "avr-can-lib/can.h"
#include <stdio.h>
#include <string.h>

#define valueSize 10
#define refVoltage 5
#define adc_steps 1024

// VoltageStatistics
class VoltageStatistics{
public:
VoltageStatistics();
uint16_t readValue(uint8_t kanal);
uint16_t convertMilliVoltage(uint16_t value);
void addValue(uint16_t voltageInMillivolt);
void showStatistics();
bool sendCanMessages(bool activ, can_t* send,uint16_t lastValue, uint32_t Zeitstempel_100ms);
private:
uint16_t values[valueSize];
};






#endif