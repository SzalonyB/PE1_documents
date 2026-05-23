#pragma once
#include <Arduino.h>

extern const int PIN_LCD_CLK;
extern const int PIN_LCD_DIN;
extern const int PIN_LCD_DC;
extern const int PIN_LCD_CS;
extern const int PIN_LCD_RST;

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
  digitalWrite(PIN_LCD_DC, isData ? HIGH : LOW);
  digitalWrite(PIN_LCD_CS, LOW);

  for (int i = 0; i < 8; i++)
  {
    digitalWrite(PIN_LCD_DIN, (value & 0x80) ? HIGH : LOW);
    digitalWrite(PIN_LCD_CLK, HIGH);
    value <<= 1;
    digitalWrite(PIN_LCD_CLK, LOW);
  }

  digitalWrite(PIN_LCD_CS, HIGH);
}

static inline void lcdCommand(uint8_t cmd)
{
  lcdWrite(false, cmd);
}

static inline void lcdData(uint8_t data)
{
  lcdWrite(true, data);
}

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

static inline void lcdInit(void)
{
  pinMode(PIN_LCD_CLK, OUTPUT);
  pinMode(PIN_LCD_DIN, OUTPUT);
  pinMode(PIN_LCD_DC, OUTPUT);
  pinMode(PIN_LCD_CS, OUTPUT);
  pinMode(PIN_LCD_RST, OUTPUT);

  digitalWrite(PIN_LCD_CS, HIGH);
  digitalWrite(PIN_LCD_CLK, LOW);

  digitalWrite(PIN_LCD_RST, LOW);
  delay(10);
  digitalWrite(PIN_LCD_RST, HIGH);

  lcdCommand(0x21);
  lcdCommand(0xBF);
  lcdCommand(0x04);
  lcdCommand(0x14);
  lcdCommand(0x20);
  lcdCommand(0x0C);
}

static inline void lcdSetXY(uint8_t x, uint8_t y)
{
  lcdCommand(0x80 | x);
  lcdCommand(0x40 | y);
}

static inline void lcdClear(void)
{
  lcdSetXY(0, 0);

  for (int i = 0; i < 84 * 6; i++)
  {
    lcdData(0x00);
  }

  lcdSetXY(0, 0);
}

static inline void lcdPrintChar(char c)
{
  const uint8_t* bmp = getCharBitmap(c);

  for (int i = 0; i < 5; i++)
  {
    lcdData(bmp[i]);
  }

  lcdData(0x00);
}

static inline void lcdPrint(const char* s)
{
  while (*s)
  {
    lcdPrintChar(*s++);
  }
}

static inline void lcdPrintValue(uint8_t row, const char* label, float value)
{
  char buf[12];
  dtostrf(value, 0, 2, buf);

  lcdSetXY(0, row);
  lcdPrint(label);
  lcdPrint(buf);
}

static inline void showValues(float vbat, float i1, float i2, float i3, float i4)
{
  lcdClear();
  lcdPrintValue(0, "V:",  vbat);
  lcdPrintValue(1, "I1:", i1);
  lcdPrintValue(2, "I2:", i2);
  lcdPrintValue(3, "I3:", i3);
  lcdPrintValue(4, "I4:", i4);
}