#include <Arduino.h>
#include "lcd_driver.h"

// LCD
const int PIN_LCD_CLK = 18;
const int PIN_LCD_DIN = 5;
const int PIN_LCD_DC  = 6;
const int PIN_LCD_CS  = 7;
const int PIN_LCD_RST = 15;

// ADC
const int PIN_I1 = 1;
const int PIN_I2 = 2;
const int PIN_I3 = 12;
const int PIN_I4 = 11;
const int PIN_V  = 3;
//MOSFET CONTROL
const int PIN_GATE1 = 44;
const int PIN_GATE2 = 43;
const int PIN_GATE3 = 17;
const int PIN_GATE4 = 16;
//Current offsets, due to some leakage current of the mosfets and uncalibrated ADC I've had some weird readings on my current sensing pins
//We measure the current and substract the offsets from actual readings
float offsetI1 = 0.0f;
float offsetI2 = 0.0f;
float offsetI3 = 0.0f;
float offsetI4 = 0.0f;

const float REF_V = 3.3f;
const float DIVISIONS = 4095.0f; //12 bit adc 
const float DIVIDER = 98.0f / 30.0f; //on the board I have a 68k and 100k ohm resistor but the 100k is "broken" and I get a reading of 30K so I adjusted the values from 168k/100 to this
const float SHUNT = 0.040f; //RdsON of the mosfet that I estimated, it should be 20-50% higher than the datasheet provides so 40-45 mOhms is close enough
const int ADC_SAMPLES = 64; //Number of analog samples to average together for a more stable reading, the readings are slower but it reduces the noise figure

//voltage functions
float adcToVoltage(int raw); // ADC value to voltage
float readVoltage(void); //reads the voltage from the divider (5V rail which gets us 2.9 volts from the 68k / 100k, we can multiply it by 1.7 to get 5V [the actual trace's voltage])

//current sensing functions
void calibrateCurrentOffsets(void); // measures the 0 current at startup
float readAveragedVoltage(int pin, int samples); //reads average voltage on the Current sensing pin
float readSenseVoltageWithOffset(int pin, float offset); // reads Current sense pin's voltage and substracts the offset
float VoltageToCurrent(float v_sense); // converts the voltage to currents with the shunt's value

//mosfet control
void setChannelOn(int channel, bool on);

void setup()
{
  lcdInit();
  lcdClear();
  //configures the mosfet control pins as outputs
  pinMode(PIN_GATE1, OUTPUT);
  pinMode(PIN_GATE2, OUTPUT);
  pinMode(PIN_GATE3, OUTPUT);
  pinMode(PIN_GATE4, OUTPUT);
  //turns all MOSFETs OFF
  setChannelOn(1, false);
  setChannelOn(2, false);
  setChannelOn(3, false);
  setChannelOn(4, false);
  //configures the ADC pins as inputs
  pinMode(PIN_I1, INPUT);
  pinMode(PIN_I2, INPUT);
  pinMode(PIN_I3, INPUT);
  pinMode(PIN_I4, INPUT);
  pinMode(PIN_V, INPUT);

  delay(300);
  calibrateCurrentOffsets(); // sets up the offset (effectively clears the current sensing)
}

void loop()
{
  float volt = readVoltage();//reads voltage on the divider
  float i1 = VoltageToCurrent(readSenseVoltageWithOffset(PIN_I1, offsetI1)); // they all read the average voltage
  float i2 = VoltageToCurrent(readSenseVoltageWithOffset(PIN_I2, offsetI2)); // then they substract the startup offset
  float i3 = VoltageToCurrent(readSenseVoltageWithOffset(PIN_I3, offsetI3)); // then they convert the corrected voltage into current I = V / R
  float i4 = VoltageToCurrent(readSenseVoltageWithOffset(PIN_I4, offsetI4));

  showValues(volt, i1, i2, i3, i4); // prints the values to the screen
  delay(500);
}

// Voltage sense
float adcToVoltage(int raw) // makes sure the ADC works properly 
{
  return (REF_V * raw) / DIVISIONS;
}

float readVoltage(void) // reads the voltage of PIN_V then converts it the ADC value to an actual Voltage number 
{
  int raw = analogRead(PIN_V); // reads PIN_V and stores it in raw
  float vSense = adcToVoltage(raw); // converts the ADC value into a real value
  return vSense * DIVIDER * 1.68;  // vSense * Divider only gives us around 2.9 volts the true volatge measurement from the divider
  // but multiplying it by 1.69 gives us the actual power trace's voltage we are measuring, which is 5V
}

// Current sense
float readAveragedVoltage(int pin, int samples)
{
  unsigned long total = 0;
  //unsigned long is used so that this variable can hold multiple added values
  for (int i = 0; i < samples; i++)
  { // reads the pin I1-I4 and adds the values to total
    total += analogRead(pin);
    delayMicroseconds(200);
  }

  float Avg = (float)total / (float)samples; // calculates the average ADC value
  return adcToVoltage((int)Avg); // returns the average already converted into volts
}

void calibrateCurrentOffsets(void)
{ // this is done to mitigate the noise that appeared on my system and the leakage current of the MOSFET
  offsetI1 = readAveragedVoltage(PIN_I1, 200);
  offsetI2 = readAveragedVoltage(PIN_I2, 200);
  offsetI3 = readAveragedVoltage(PIN_I3, 200);
  offsetI4 = readAveragedVoltage(PIN_I4, 200);
}

float readSenseVoltageWithOffset(int pin, float offset) //Reads the corrected voltage on the Current-Sense pins 
{
  float v = readAveragedVoltage(pin, ADC_SAMPLES); // reads the average voltage on the I-sense pin
  float corrected = v - offset; // substracts the startup offset from the 
  if (corrected < 0.0f) // gets rid of negative values
  {
    corrected = 0.0f;
  }
  return corrected;
}

float VoltageToCurrent(float v_sense)
{
  const float margin = 0.12f; // ads a margin of error to the ADC 
  if (v_sense < margin)
  {
    return 0.0f;
  }
  return v_sense / SHUNT; // returns the proper current on the shunt (which is a mosfet)
}

// MOSFET control
void setChannelOn(int channel, bool on) // the function is self explanatory
{
  int pin = -1; // temporary value for initialization

  switch (channel) // if we input any of the mosfet numbers (from the top letf to right they go 4 - 3 on the bottom  1 - 2)
  {
    case 1: pin = PIN_GATE1; break;
    case 2: pin = PIN_GATE2; break;
    case 3: pin = PIN_GATE3; break;
    case 4: pin = PIN_GATE4; break;
  }

  if (pin >= 0) // if the pin value is valid 
  { // write to the mosfet either turning it ON with HIGH or of with LOW (the input should be true of false)
    digitalWrite(pin, on ? HIGH : LOW);
  }
}