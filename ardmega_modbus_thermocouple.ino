#include <SPI.h>
#include "Adafruit_MAX31855.h"
//#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>

const uint8_t dePin = 2;
Adafruit_MAX31855 thermocouple(10);

// NOTE: 3.3v to 5v level-shifting required for MAX485. it will not work on 3.3v, and 5v will burn out the Giga UART.
ModbusRTUSlave modbus(Serial1, dePin); // serial port, driver enable pin for rs-485 (optional)

bool coils[6];
bool discreteInputs[3];
uint16_t holdingRegisters[2];
uint16_t inputRegisters[4];

void setup()
{
    Serial.begin(9600);
    thermocouple.begin();
    modbus.configureCoils(coils, 6);                       // bool array of coil values, number of coils
    modbus.configureDiscreteInputs(discreteInputs, 3);     // bool array of discrete input values, number of discrete inputs
    modbus.configureHoldingRegisters(holdingRegisters, 2); // unsigned 16 bit integer array of holding register values, number of holding registers
    modbus.configureInputRegisters(inputRegisters, 4);     // unsigned 16 bit integer array of input register values, number of input registers
    modbus.begin(3, 9600);                                // slave id, baud rate, config (optional)
}

void loop()
{
    double c = (thermocouple.readCelsius() * 100);
    double f = (thermocouple.readFahrenheit() * 100);
    if (isnan(c))
    {
        uint8_t e = thermocouple.readError();
        if (e & MAX31855_FAULT_OPEN) discreteInputs[0] = 1;
        if (e & MAX31855_FAULT_SHORT_GND) discreteInputs[1] = 1;
        if (e & MAX31855_FAULT_SHORT_VCC) discreteInputs[2] = 1;
        inputRegisters[0] = 0;
        inputRegisters[1] = 0;
    }
    else
    {
        inputRegisters[0] = (uint16_t)(f);
        inputRegisters[1] = (uint16_t)(c);
        discreteInputs[0] = 0;
        discreteInputs[1] = 0;
        discreteInputs[2] = 0;
    }
    inputRegisters[2] = (uint16_t)(map(map(float(analogRead(0)),0.00,1023.00,0.00,5.00),0.50,4.50,0.00,150.00) * 100.00);
    inputRegisters[3] = (uint16_t)(map(map(float(analogRead(1)),0.00,1023.00,0.00,5.00),0.50,4.50,0.00,150.00) * 100.00);

    modbus.poll();
}
