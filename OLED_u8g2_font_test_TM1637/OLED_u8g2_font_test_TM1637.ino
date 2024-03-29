/*
  HelloWorld.ino
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.
*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/*
  U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
*/

// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

int dispwid;
int disphei;

#include <TM1637Display.h>

const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display

int numCounter = 0;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.


void setup(void) {
  u8g2.begin();
  u8g2.clearBuffer();			// clear the internal memory
  u8g2.setFont(u8g2_font_timB08_tr);	// choose a suitable font
  char buff[64];
  sprintf(buff, "Hello World!");
  Serial.begin(9600);
  Serial.println(buff);

  // get display dimensions
  dispwid = u8g2.getDisplayWidth();
  disphei = u8g2.getDisplayHeight();

  // get text dimensions
  int textwid = u8g2.getStrWidth(buff);
  int texthei = u8g2.getAscent();

  // set text position
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;

  u8g2.drawStr(xpos, ypos, buff);        	// write something to the internal memory
  u8g2.sendBuffer();			// transfer internal memory to the display
  delay(1000);

  sprintf(buff, "text dimensions are %d x %d", textwid, texthei);
  Serial.println(buff);
  sprintf(buff, "text position is %d x %d", xpos, ypos);
  Serial.println(buff);
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d", dispwid, disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos = 2 * texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  delay(1000);

  display.clear();
  display.setBrightness(0x0a); //set the diplay to maximum brightness
}

void loop(void) {
  // write minutes only
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont22_tn);
  char buff[dispwid];
  sprintf(buff, "12:00");
  Serial.println(buff);
  int textwid = u8g2.getStrWidth(buff);
  int texthei =  u8g2.getAscent();
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  //display.showNumberDecEx(1200,1);
  // Display the current time in 24 hour format with leading zeros enabled and a center colon:
  display.showNumberDecEx(1200, 0b11100000, true);
  
  delay(1000);
  sprintf(buff, "text dimensions are %d x %d", textwid, texthei);
  Serial.println(buff);
  sprintf(buff, "text position is %d x %d", xpos, ypos);
  Serial.println(buff);

  // write seconds
  u8g2.setFont(u8g2_font_profont15_tn);
  sprintf(buff, "12:00:00");
  Serial.println(buff);
  textwid = u8g2.getStrWidth(buff);
  texthei =  u8g2.getAscent();
  xpos = (dispwid - textwid) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  delay(1000);

  // write milliseconds
  u8g2.setFont(u8g2_font_profont10_tn);
  sprintf(buff, "12:00:00.000");
  Serial.println(buff);
  textwid = u8g2.getStrWidth(buff);
  texthei =  u8g2.getAscent();
  xpos = (dispwid - textwid) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  display.showNumberDec(numCounter); //Display the numCounter value;
  numCounter++;

    delay(1000);

}
