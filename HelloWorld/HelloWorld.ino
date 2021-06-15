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

// Please UNCOMMENT one of the contructor lines below
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

int dispwid;
int disphei;

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
  int texthei = u8g2.getMaxCharHeight();

  // set text position
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;
  
  u8g2.drawStr(xpos,ypos,buff);        	// write something to the internal memory
  u8g2.sendBuffer();			// transfer internal memory to the display
  delay(1000);
  
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d",dispwid,disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos = 2*texthei+1;
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();      
  delay(1000);
  }

void loop(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont22_tn);
  char buff[dispwid];
  sprintf(buff, "12:00");
  Serial.println(buff);
  int textwid = u8g2.getStrWidth(buff);
  int texthei = u8g2.getMaxCharHeight();
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();         
  delay(1000);  
  u8g2.clearBuffer();        
  sprintf(buff, "12 00");
  Serial.println(buff);
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();         
  delay(1000);    
}
