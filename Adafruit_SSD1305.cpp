/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1305 drivers

  Pick one up today in the adafruit shop!
  ------> https://www.adafruit.com/products/2675

These displays use I2C or SPI to communicate

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#ifdef __AVR__
 #include <avr/pgmspace.h>
 #include <util/delay.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#include <stdlib.h>
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1305.h"

#include "glcdfont.c"

#ifdef SPI_HAS_TRANSACTION
SPISettings oledspi = SPISettings(4000000, MSBFIRST, SPI_MODE0);
#else
#define ADAFRUIT_SSD1305_SPI SPI_CLOCK_DIV2
#endif

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// a 5x7 font table
extern const uint8_t PROGMEM font[];

// the memory buffer for the LCD

static uint8_t buffer[SSD1305_LCDHEIGHT * SSD1305_LCDWIDTH / 8] = { 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x0C, 0x3E, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF0,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xE0, 0xE0, 0xE0,
0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFD,
0xFB, 0x7B, 0xBF, 0xFF, 0xFF, 0xFC, 0x7F, 0xFF, 0xF7, 0xF7, 0xF7, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x3F, 0x1F, 0x0F, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#if (SSD1305_LCDHEIGHT == 64)	//wcp - this line moved up one to fix compile issue for SSD1305_128_32 only
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD,
0xFE, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0xFC, 0xF3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xF1,
0xC1, 0x01, 0x00, 0x00, 0x70, 0x78, 0x7C, 0x7C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0xFC, 0xFC, 0xF8,
0xF0, 0x00, 0x00, 0xF0, 0xF8, 0xFC, 0xFC, 0x3C, 0x3C, 0x3C, 0x3C, 0x78, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x70, 0x78, 0x7C, 0x7C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0xFC, 0xFC, 0xF8, 0xF0, 0x00,
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x3D, 0x3D, 0x3D, 0x00, 0x00, 0xFC, 0xFC, 0xFC, 0xFC, 0x70, 0x38,
0x3C, 0x3C, 0x3C, 0x00, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC, 0xFC,
0xFC, 0x00, 0x00, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x3C, 0x3C, 0x3C,
0x00, 0x00, 0x00, 0x00, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x03,
0x03, 0x01, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF,
0xFF, 0x00, 0x00, 0x00, 0xFC, 0xFE, 0xFF, 0xFF, 0xE7, 0xE7, 0xE7, 0xE7, 0xF7, 0xFF, 0xFF, 0xFF,
0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0xFC, 0xFE, 0xFF, 0xFF, 0xE7, 0xE7, 0xE7, 0xE7, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0xFF, 0xFF, 0xFF,
0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF8, 0xF9, 0xF9, 0xF9,
0xF9, 0xF8, 0xF8, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9,
0xF8, 0xF8, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF9, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9, 0xF8,
0xF8, 0xF9, 0x09, 0xF9, 0x09, 0xD8, 0xB8, 0x78, 0x08, 0xF8, 0x09, 0xE9, 0xE9, 0x19, 0xF8, 0x08,
0xF8, 0xF8, 0x08, 0xF8, 0xD8, 0xA8, 0xA9, 0x69, 0xF9, 0xE9, 0x09, 0xE9, 0xF8, 0x09, 0xA9, 0x29,
0xD9, 0xF8, 0x08, 0xF9, 0x09, 0xA9, 0xA9, 0xF8, 0xD8, 0xA8, 0xA9, 0x69, 0xF9, 0xF9, 0xF9, 0xF9,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01,
0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01
#endif
};



// the most basic function, set a single pixel
void Adafruit_SSD1305::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= width()) || (y >= height()) || (x < 0) || (y < 0))
    return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    adagfx_swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    adagfx_swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }  

  // x is which column
  if (color == WHITE) 
    buffer[x+ (y/8)*SSD1305_LCDWIDTH] |= _BV((y%8));  
  else
    buffer[x+ (y/8)*SSD1305_LCDWIDTH] &= ~_BV((y%8)); 
}

void Adafruit_SSD1305::begin(uint8_t i2caddr) {
  _i2caddr = i2caddr;

  // set pin directions
  if (sclk != -1) {
    pinMode(sid, OUTPUT);
    pinMode(sclk, OUTPUT);
#ifdef __AVR__
    clkport     = portOutputRegister(digitalPinToPort(sclk));
    clkpinmask  = digitalPinToBitMask(sclk);
    mosiport    = portOutputRegister(digitalPinToPort(sid));
    mosipinmask = digitalPinToBitMask(sid);
#endif
  } else if (cs != -1) {
    // hardware SPI
    SPI.begin();
  } else {
    // I2C
    Wire.begin();
  }

  if (cs != -1) {
    pinMode(dc, OUTPUT);
    pinMode(cs, OUTPUT);
  }
  
  if (rst != -1) {
    pinMode(rst, OUTPUT);
    
    digitalWrite(rst, HIGH);
    // VDD (3.3V) goes high at start, lets just chill for a ms
    delay(1);
    // bring reset low
    digitalWrite(rst, LOW);
    // wait 10ms
    delay(10);
    // bring out of reset
    digitalWrite(rst, HIGH);
  }

#if defined SSD1305_128_32 //wcp - Typo omits init code. Was: #if defined SSD1306_128_32
  // Init sequence for 128x32 OLED module
  command(SSD1305_DISPLAYOFF);                    // 0xAE
  command(SSD1305_SETLOWCOLUMN | 0x0);  // low col = 0
  command(SSD1305_SETHIGHCOLUMN | 0x0);  // hi col = 0
  command(SSD1305_SETSTARTLINE | 0x0); // line #0
  command(0x2E); //??
  command(SSD1305_SETCONTRAST);                   // 0x81
  command(0x32);
  command(SSD1305_SETBRIGHTNESS);                 // 0x82
  command(0x80);
  command(SSD1305_SEGREMAP | 0x01);
  command(SSD1305_NORMALDISPLAY);                 // 0xA6
  command(SSD1305_SETMULTIPLEX);                  // 0xA8
  command(0x3F); // 1/64
  command(SSD1305_MASTERCONFIG);
  command(0x8e); /* external vcc supply */
  command(SSD1305_COMSCANDEC);
  command(SSD1305_SETDISPLAYOFFSET);              // 0xD3
  command(0x40); 
  command(SSD1305_SETDISPLAYCLOCKDIV);            // 0xD5
  command(0xf0); 
  command(SSD1305_SETAREACOLOR);  
  command(0x05);
  command(SSD1305_SETPRECHARGE);                  // 0xd9
  command(0xF1);
  command(SSD1305_SETCOMPINS);                    // 0xDA
  command(0x12);

  command(SSD1305_SETLUT);
  command(0x3F);
  command(0x3F);
  command(0x3F);
  command(0x3F);

  #endif

  #if defined SSD1305_128_64
  // Init sequence for 128x64 OLED module
  command(SSD1305_DISPLAYOFF);                    // 0xAE
  command(SSD1305_SETLOWCOLUMN | 0x4);  // low col = 0
  command(SSD1305_SETHIGHCOLUMN | 0x4);  // hi col = 0
  command(SSD1305_SETSTARTLINE | 0x0); // line #0
  command(0x2E); //??
  command(SSD1305_SETCONTRAST);                   // 0x81
  command(0x32);
  command(SSD1305_SETBRIGHTNESS);                 // 0x82
  command(0x80);
  command(SSD1305_SEGREMAP | 0x01);
  command(SSD1305_NORMALDISPLAY);                 // 0xA6
  command(SSD1305_SETMULTIPLEX);                  // 0xA8
  command(0x3F); // 1/64
  command(SSD1305_MASTERCONFIG);
  command(0x8e); /* external vcc supply */
  command(SSD1305_COMSCANDEC);
  command(SSD1305_SETDISPLAYOFFSET);              // 0xD3
  command(0x40); 
  command(SSD1305_SETDISPLAYCLOCKDIV);            // 0xD5
  command(0xf0); 
  command(SSD1305_SETAREACOLOR);  
  command(0x05);
  command(SSD1305_SETPRECHARGE);                  // 0xd9
  command(0xF1);
  command(SSD1305_SETCOMPINS);                    // 0xDA
  command(0x12);

  command(SSD1305_SETLUT);
  command(0x3F);
  command(0x3F);
  command(0x3F);
  command(0x3F);
  #endif

  command(SSD1305_DISPLAYON);//--turn on oled panel
}

void Adafruit_SSD1305::invertDisplay(uint8_t i) {
  if (i) {
    command(SSD1305_INVERTDISPLAY);
  } else {
    command(SSD1305_NORMALDISPLAY);
  }
}

void Adafruit_SSD1305::command(uint8_t c) { 

  if (cs != -1) {
    // SPI of sorts

    digitalWrite(cs, HIGH);
    digitalWrite(dc, LOW);
    delay(1);
    if (sclk == -1) {
#ifdef SPI_HAS_TRANSACTION
      SPI.beginTransaction(oledspi);
#else
      SPI.setDataMode(SPI_MODE0);
      SPI.setClockDivider(ADAFRUIT_SSD1305_SPI);
#endif
    } 
    digitalWrite(cs, LOW);
    spixfer(c);
    digitalWrite(cs, HIGH);

#ifdef SPI_HAS_TRANSACTION
    if (sclk == -1)
      SPI.endTransaction();              // release the SPI bus
#endif
  } else {
    // I2C
    uint8_t control = 0x00;   // Co = 0, D/C = 0
    Wire.beginTransmission(_i2caddr);
    Wire.write(control);
    Wire.write(c);
    Wire.endTransmission();
  }
}

void Adafruit_SSD1305::data(uint8_t c) {
  if (cs != -1) {
    // SPI of sorts
    digitalWrite(cs, HIGH);
    digitalWrite(dc, HIGH);
    
    if (sclk == -1) {
#ifdef SPI_HAS_TRANSACTION
      SPI.beginTransaction(oledspi);
#else
      SPI.setDataMode(SPI_MODE0);
      SPI.setClockDivider(ADAFRUIT_SSD1305_SPI);
#endif
    }
    
    digitalWrite(cs, LOW);
    spixfer(c);
    digitalWrite(cs, HIGH);
    
#ifdef SPI_HAS_TRANSACTION
    if (sclk == -1)
      SPI.endTransaction();              // release the SPI bus
#endif
  } else {
    // I2C
    uint8_t control = 0x40;   // Co = 0, D/C = 1
    Wire.beginTransmission(_i2caddr);
    Wire.write(control);
    Wire.write(c);
    Wire.endTransmission();
  }  
}

void Adafruit_SSD1305::display(void) {
  uint16_t i=0;
  uint8_t page;
  if (SSD1305_LCDHEIGHT == 64) page = 0;
  if (SSD1305_LCDHEIGHT == 32) page = 4;

  for(; page<8; page++)
  {
    command(SSD1305_SETPAGESTART + page);
    command(0x00);
    command(0x10);

    if (cs == -1) {
      // save I2C bitrate
#ifdef __AVR__
      uint8_t twbrbackup = TWBR;
      TWBR = 12; // upgrade to 400KHz!
#endif
      
      //Serial.println(TWBR, DEC);
      //Serial.println(TWSR & 0x3, DEC);
      
      // I2C has max 16 bytes per xmision
      // send a bunch of data in one xmission
      for (uint8_t w=0; w<128/16; w++) {
	Wire.beginTransmission(_i2caddr);
	Wire.write(0x40);
	for (uint8_t x=0; x<16; x++) {
	  Wire.write(buffer[i++]);
	}
	Wire.endTransmission();
      }

#ifdef __AVR__
      TWBR = twbrbackup;
#endif
    } else {
      // SPI
      if (sclk == -1) {
#ifdef SPI_HAS_TRANSACTION
	SPI.beginTransaction(oledspi);
#else
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(ADAFRUIT_SSD1305_SPI);
#endif
      }
      
      digitalWrite(cs, HIGH);
      digitalWrite(dc, HIGH);
      digitalWrite(cs, LOW);
      
      for(uint8_t x=0; x<128; x++) {
	spixfer(buffer[i++]);
      }
      
      digitalWrite(cs, HIGH);
#ifdef SPI_HAS_TRANSACTION
      if (sclk == -1)
	SPI.endTransaction();              // release the SPI bus
#endif
    }
  }
}

// clear everything
void Adafruit_SSD1305::clearDisplay(void) {
  memset(buffer, 0, (SSD1305_LCDWIDTH*SSD1305_LCDHEIGHT/8));
}


void Adafruit_SSD1305::spixfer(uint8_t x) {
  if (sclk == -1) {
    SPI.transfer(x);
    return;
  }
  // software spi
  //Serial.println("Software SPI");

  for(uint8_t bit = 0x80; bit; bit >>= 1) {
#if defined(AVR)
    *clkport &= ~clkpinmask;
    if(x & bit) *mosiport |=  mosipinmask;
    else        *mosiport &= ~mosipinmask;
    *clkport |=  clkpinmask;
#else
    digitalWrite(sclk, LOW);
    if(x & bit) digitalWrite(sid, HIGH);
    else        digitalWrite(sid, LOW);
    digitalWrite(sclk, HIGH);
#endif
  }
}
