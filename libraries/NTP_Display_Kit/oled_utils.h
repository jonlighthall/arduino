// OLED packages
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// OLED display options
const bool do_RSSI = false;
const bool do_SyncBar = false;
const bool do_BigTime = false;
const bool do_SecondsBar = true;
const bool do_Seconds = true;
void OLEDClockDisplay();
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
int dispwid;
int disphei;

int syncBar = 0;

void OLED_Sync_Bar () {
  // draw sync bar
  for (int i = 0; i < syncBar + 1; i++) {
    u8g2.drawPixel(0, disphei - i);
  }
  u8g2.sendBuffer();
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
