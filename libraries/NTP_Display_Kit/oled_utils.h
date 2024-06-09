#ifndef OLED_UTILS
#define OLED_UTILS

// OLED packages
#include <U8g2lib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

// project library headers
#include <debug.h>

// OLED display options
const bool do_RSSI_Bars = false;
const bool do_SyncBar = false;
const bool do_BigTime = false;
const bool do_SecondsBar = true;
const bool do_Seconds = true;
void OLEDClockDisplay();
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
int dispwid;
int disphei;

 // OLED text dimensions
int textwid; 
int texthei;

  // OLED text position
int xpos;
int ypos;

int syncBar = 0;

void OLED_init () {
  char buff[64];
  // initialize OLED display
  u8g2.begin();
  u8g2.clearBuffer();  // clear the internal memory

  // get OLED display dimensions
  dispwid = u8g2.getDisplayWidth();
  disphei = u8g2.getDisplayHeight();

  // print OLED welcome message
  u8g2.setFont(u8g2_font_timB08_tr);  // choose a suitable font
  sprintf(buff, "NTP Time");

  // get OLED text dimensions
  textwid = u8g2.getStrWidth(buff);
  texthei = u8g2.getAscent();

  // set OLED text position
  xpos = (dispwid - textwid) / 2;
  ypos = texthei;
  u8g2.drawStr(xpos, ypos, buff);  // write something to the internal memory
  u8g2.sendBuffer();               // transfer internal memory to the display

  // print OLED display dimensions
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d", dispwid, disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();  
}

void OLED_TZ () {
#ifdef OLED
  char buff[64];
  // OLED timezone message
  sprintf(buff, "Timezone = %d", SetTimeZone);
  xpos = dispwid - u8g2.getStrWidth(buff);
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
#endif
}

void OLED_Sync_Bar () {
#ifdef OLED
  // draw sync bar
  for (int i = 0; i < syncBar + 1; i++) {
    u8g2.drawPixel(0, disphei - i);
  }
  u8g2.sendBuffer();
#endif
}

void OLED_sync () {
#ifdef OLED
  // OLED sync message (cue light)
  u8g2.drawBox(0, 0, 2, 2);
  u8g2.sendBuffer();
#endif
}

void OLED_RSSI_Bars () {
  // draw signal bars
  int rssi_oled = WiFi.RSSI();

  int bt = 0; // bar top
  int bb = bt + 5 ; // bar bottom
  int bl = 1; // bar left

  // draw black background
  u8g2.setDrawColor(0);
  // bars occupy a box 7 x 5 pixels
  u8g2.drawBox(bl, bt, 9,  7);
  u8g2.setDrawColor(1);

  // draw smallest possible signal strength bars
  if (rssi_oled > -89) u8g2.drawLine(bl + 1, bb, bl + 1, bb - 1);
  if (rssi_oled > -78) u8g2.drawLine(bl + 3, bb, bl + 3, bb - 2);
  if (rssi_oled > -67) u8g2.drawLine(bl + 5, bb, bl + 5, bb - 3);
  if (rssi_oled > -56) u8g2.drawLine(bl + 7, bb, bl + 7, bb - 4);

  u8g2.sendBuffer();
}

void OLEDClockDisplay() {
  // define OLED variables
  int xpos;
  int ypos;
  char buff[dispwid];

  u8g2.clearBuffer();

  if (do_BigTime) {
    // draw OLED clock display
    u8g2.setFont(u8g2_font_profont22_tn);
    sprintf(buff, "%02d:%02d", hour(), minute());
    if (debug > 0) {
      Serial.print("   ");
      Serial.println(buff);
    }
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos = u8g2.getAscent();
    u8g2.drawStr(xpos, ypos, buff);
  }

  if (do_SecondsBar) {
    // draw seconds bar
    ypos += 2;
    for (int i = 0; i < second() + 1; i++) {
      u8g2.drawPixel(i + 3, ypos);
    }
    ypos += 1;
    for (int i = 0; i < second() + 1; i = i + 5) {
      u8g2.drawPixel(i + 3, ypos);
    }
    ypos += 1;
    for (int i = 0; i < second() + 1; i = i + 15) {
      u8g2.drawPixel(i + 3, ypos);
    }
  }

  if (do_Seconds) {
    // write seconds
    // set font
    u8g2.setFont(u8g2_font_profont15_tn);
    // create time buffer
    sprintf(buff, "%02d:%02d:%02d", hour(), minute(), second());
    // print time to serial
    if (debug > 0) {
      Serial.print("   ");
      Serial.println(buff);
    }
    // calculate OLED display position
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos += u8g2.getAscent() + 2;
    // display time
    u8g2.drawStr(xpos, ypos, buff);
  }

  // write day
  u8g2.setFont(u8g2_font_timB08_tr);
  sprintf(buff, "%s", dayStr(weekday()));
  if (debug > 0) {
    Serial.print("   ");
    Serial.println(buff);
  }
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 2;
  u8g2.drawStr(xpos, ypos, buff);

  // write date
  //sprintf(buff, "%s %s %d",dayStr(weekday()),monthStr(month()),day());
  sprintf(buff, "%s %d", monthStr(month()), day());
  if (debug > 0) {
    Serial.print("   ");
    Serial.println(buff);
  }
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 1;
  u8g2.drawStr(xpos, ypos, buff);

  // set brightness
  if ((hour() >= 20) || (hour() <= 6)) {
    u8g2.setContrast(0);
  }
  else {
    u8g2.setContrast(255);
  }
  u8g2.sendBuffer();
  if (do_SyncBar) OLED_Sync_Bar();
  if (do_RSSI_Bars) OLED_RSSI_Bars();
}

#endif
