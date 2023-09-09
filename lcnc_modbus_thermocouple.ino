#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>
#define PIN_RED 3
#define PIN_AMBER 4
#define PIN_GREEN 5
#define PIN_BLUE 6
#define PIN_WHITE 7
#define PIN_BUZZER 8
int sigTowerPins[6] = {PIN_RED, PIN_AMBER, PIN_GREEN, PIN_BLUE, PIN_WHITE, PIN_BUZZER};
const uint8_t dePin = 2;
Adafruit_MAX31855 thermocouple(10);
ModbusRTUSlave modbus(Serial, dePin); // serial port, driver enable pin for rs-485 (optional)

bool coils[6];
bool discreteInputs[3];
uint16_t holdingRegisters[2];
uint16_t inputRegisters[2];

void setup() {
  for (int i=0; i<6; i++) {
    pinMode(sigTowerPins[i],OUTPUT); digitalWrite(sigTowerPins[i],LOW);
  }
  thermocouple.begin();
  modbus.configureCoils(coils, 2);                       // bool array of coil values, number of coils
  modbus.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs
  modbus.configureHoldingRegisters(holdingRegisters, 2); // unsigned 16 bit integer array of holding register values, number of holding registers
  modbus.configureInputRegisters(inputRegisters, 2);     // unsigned 16 bit integer array of input register values, number of input registers
  modbus.begin(1, 9600);                                // slave id, baud rate, config (optional)
}

void loop() {
  double c = thermocouple.readCelsius();
  double f = thermocouple.readFahrenheit();
  if (isnan(f)) {
    uint8_t e = thermocouple.readError();
    if (e & MAX31855_FAULT_OPEN) discreteInputs[0] = 1;
    if (e & MAX31855_FAULT_SHORT_GND) discreteInputs[1] = 1;
    if (e & MAX31855_FAULT_SHORT_VCC) discreteInputs[2] = 1;
    inputRegisters[0] = 0;
    inputRegisters[1] = 0;
  } else {
    inputRegisters[0] = (uint16_t)(f * 100);
    inputRegisters[1] = (uint16_t)(c * 100);
    discreteInputs[0] = 0;
    discreteInputs[1] = 0;
    discreteInputs[2] = 0;
  }

  modbus.poll();

  digitalWrite(sigTowerPins[0],coils[0]);
  digitalWrite(sigTowerPins[1],coils[1]);
  digitalWrite(sigTowerPins[2],coils[2]);
  digitalWrite(sigTowerPins[3],coils[3]);
  digitalWrite(sigTowerPins[4],coils[4]);
  digitalWrite(sigTowerPins[5],coils[5]);

}
