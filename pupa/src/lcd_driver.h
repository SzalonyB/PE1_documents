#pragma once
#include <Arduino.h>

extern const int PIN_LCD_CLK;
extern const int PIN_LCD_DIN;
extern const int PIN_LCD_DC;
extern const int PIN_LCD_CS;
extern const int PIN_LCD_RST;

//small pixel font bitmap, each character is 5x8 pixels
//each charcater is 5 bytes wide and 8 pixels high
//each character uses 6 columns = 5 bytes wide + 1 for spacing
static const uint8_t FONT_SPACE[5] = {0x00,0x00,0x00,0x00,0x00};
static const uint8_t FONT_DOT[5]   = {0x00,0x00,0x60,0x60,0x00};
static const uint8_t FONT_COLON[5] = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t FONT_MINUS[5] = {0x08,0x08,0x08,0x08,0x08};

static const uint8_t FONT_0[5] = {0x3E,0x51,0x49,0x45,0x3E};
static const uint8_t FONT_1[5] = {0x00,0x42,0x7F,0x40,0x00};
static const uint8_t FONT_2[5] = {0x42,0x61,0x51,0x49,0x46};
static const uint8_t FONT_3[5] = {0x21,0x41,0x45,0x4B,0x31};
static const uint8_t FONT_4[5] = {0x18,0x14,0x12,0x7F,0x10};
static const uint8_t FONT_5[5] = {0x27,0x45,0x45,0x45,0x39};
static const uint8_t FONT_6[5] = {0x3C,0x4A,0x49,0x49,0x30};
static const uint8_t FONT_7[5] = {0x01,0x71,0x09,0x05,0x03};
static const uint8_t FONT_8[5] = {0x36,0x49,0x49,0x49,0x36};
static const uint8_t FONT_9[5] = {0x06,0x49,0x49,0x29,0x1E};

static const uint8_t FONT_I[5] = {0x00,0x41,0x7F,0x41,0x00};
static const uint8_t FONT_V[5] = {0x1F,0x20,0x40,0x20,0x1F};

static inline void lcdWrite(bool isData, uint8_t value)
{
  digitalWrite(PIN_LCD_DC, isData ? HIGH : LOW); // we select if we want a command or data to the display
  digitalWrite(PIN_LCD_CS, LOW); // Enabling LCD by pulling CS low

  for (int i = 0; i < 8; i++) //sending 8 bits, startingfrom the most significatnt bit
  {
    digitalWrite(PIN_LCD_DIN, (value & 0x80) ? HIGH : LOW); // anding so that the we put the CURRENT MSB on the data line
    digitalWrite(PIN_LCD_CLK, HIGH);// sending data on the rising edge of the clock, LCD samples
    value <<= 1; // shifting left so that we put the next bit as the MSB
    digitalWrite(PIN_LCD_CLK, LOW); // falling CLK edge, preparing for sending again
  }

  digitalWrite(PIN_LCD_CS, HIGH);// finished sending the byte, deselect LCD
}

static inline void lcdCommand(uint8_t cmd) //additional functions for lcdWrite, lcdcommand sends commands
{
  lcdWrite(false, cmd);
}

static inline void lcdData(uint8_t data)//lcddData sends data, mentioning that it is indeed data to the macro isData?
{
  lcdWrite(true, data);
}
// look up table referencing the bitmap, returns space if nothing from the font is selected
static inline const uint8_t* getCharBitmap(char c)
{
  switch (c)
  {
    case ' ': return FONT_SPACE;
    case '.': return FONT_DOT;
    case ':': return FONT_COLON;
    case '-': return FONT_MINUS;
    case '0': return FONT_0;
    case '1': return FONT_1;
    case '2': return FONT_2;
    case '3': return FONT_3;
    case '4': return FONT_4;
    case '5': return FONT_5;
    case '6': return FONT_6;
    case '7': return FONT_7;
    case '8': return FONT_8;
    case '9': return FONT_9;
    case 'I': return FONT_I;
    case 'V': return FONT_V;
    default:  return FONT_SPACE;
  }
}

static inline void lcdInit(void) // initalization of the LCD
{ //initializing all of the pins connected to it as outputs
  pinMode(PIN_LCD_CLK, OUTPUT);
  pinMode(PIN_LCD_DIN, OUTPUT);
  pinMode(PIN_LCD_DC, OUTPUT);
  pinMode(PIN_LCD_CS, OUTPUT);
  pinMode(PIN_LCD_RST, OUTPUT);

  digitalWrite(PIN_LCD_CS, HIGH);  //default idle start of the initialization
  digitalWrite(PIN_LCD_CLK, LOW);

  digitalWrite(PIN_LCD_RST, LOW); // quick reset pulse, briging the reset low and then high after 10 ms
  delay(10);
  digitalWrite(PIN_LCD_RST, HIGH);

  lcdCommand(0x21); //enter extended instruction set
  lcdCommand(0xBF); // sets contrast
  lcdCommand(0x04); // sets the temperature coefficient
  lcdCommand(0x14); // sets LCD bias mode
  lcdCommand(0x20); // lest the lcd back to the instruction set after all of these
  lcdCommand(0x0C); // normal display mode (not inverted)
}

static inline void lcdSetXY(uint8_t x, uint8_t y) // sets the cursor
{
  lcdCommand(0x80 | x); //sets column
  lcdCommand(0x40 | y); // sets row  
}

static inline void lcdClear(void) /// clears the entire display
{
  lcdSetXY(0, 0); // sets the cursor at the top left courner

  for (int i = 0; i < 84 * 6; i++) // for the entire display ram
  {
    lcdData(0x00);  // writes 0x00 bytes to clear the entire ram and turn all of the pixels off
  }

  lcdSetXY(0, 0);// sets cursor back at the top left again
}

static inline void lcdPrintChar(char c) // prints one character to the screen
{
  const uint8_t* bmp = getCharBitmap(c); // pointer to the bitmap

  for (int i = 0; i < 5; i++) // draw the columns of 5 characters wide
  {
    lcdData(bmp[i]);
  }
  lcdData(0x00); // one column for spacing
}

static inline void lcdPrint(const char* s) // prints a string to the lcd screen
{
  while (*s) // as long as the string isn't done (NULL byte)
  {
    lcdPrintChar(*s++); // prints one character from the bitmap and caries on the loop
  }
}

static inline void lcdPrintValue(uint8_t row, const char* label, float value) // allows us to print strings
{
  char buf[12]; // temporary variable 
  dtostrf(value, 0, 2, buf); // converts floats to strings, 0 no minimum width, 2 -> two points after decimal, stores them into the buf

  lcdSetXY(0, row); // sets the cursor back to the beginning of the row
  lcdPrint(label); // prints the label
  lcdPrint(buf); // prints the value
}

static inline void showValues(float volt, float i1, float i2, float i3, float i4) // uses the earlier functions to print the values we want
{
  lcdClear(); // clears the LCD
  lcdPrintValue(0, "V:",  volt); // prints voltage
  lcdPrintValue(1, "I1:", i1); // prints current1
  lcdPrintValue(2, "I2:", i2); // prints current2
  lcdPrintValue(3, "I3:", i3); // prints current3
  lcdPrintValue(4, "I4:", i4); //prints current4
}