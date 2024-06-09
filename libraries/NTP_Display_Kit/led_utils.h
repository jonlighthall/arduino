#ifndef LED_UTILS
#define LED_UTILS

// project library headers
#include <debug.h>

// load LED libraries
#include <TM1637Display.h>
#include <seven-segment_text.h>
#include <TimeLib.h>

// set LED connection pins
#ifdef LED1
#ifdef WEMOS_V4
const int CLK = D1;  // Set the CLK pin connection to the display
const int DIO = D2;  // Set the DIO pin connection to the display
#else
const int CLK = D6;  // Set the CLK pin connection to the display
const int DIO = D5;  // Set the DIO pin connection to the display
#endif
TM1637Display display(CLK, DIO);  // set up the 4-Digit Display.
#endif

#ifdef LED2
// LED display2 settings
const int CLK2 = D1;  // Set the CLK pin connection to the display
const int DIO2 = D2;  // Set the DIO pin connection to the display
TM1637Display display2(CLK2, DIO2);  // set up the 4-Digit Display.

// fractional seconds
int prev_disp_ms;
float sec_frac;
#endif

// Set LED display options
// use military time (24 hour clock)
const bool do_mil = false;
// show seconds at the top of the minute (for setting/synchronizing watches)
const bool do_sec_top = false;
// nixie tube cycling
const bool do_cyc = false;
const bool do_sec_mod = false;

// print delay in milliseconds
const int DISP_DELAY = 250;

// LDR variables
#ifdef LDR
const bool do_ldr = true;
#else
const bool do_ldr = false;
#endif
int brite;

void LED_init() {
  // initialize LED display
#ifdef LED1
  display.clear();
  display.setBrightness(7);
#endif

#ifdef LED2
  display2.clear();
  display2.setBrightness(7);
#endif

// print LED welcome message
#ifdef LED1
  display.setSegments(SEG_hEllo);
#endif
#ifdef LED2
  display2.setSegments(SEG_hEllo);
#endif
}

void LED_conn() {
  // LED connecting message
#ifdef LED1
  display.setSegments(SEG_CONN);
#endif
#ifdef LED2
  display2.setSegments(SEG_ecti);
#endif
}

void LED_sync() {
  // LED sync message
#ifdef LED1
  display.setSegments(SEG_SYNC);
#endif
#ifdef LED2
  display2.setSegments(SEG_hron);
#endif
}

// update LED display with the current time
void DigitalClockDisplay() {
  // Display Time
  int dig_time;

  if (do_mil) {
    // military time
    dig_time = (hour() * 100) + minute();
#ifdef LED1
    display.showNumberDecEx(dig_time, 0b11100000, true);
#endif
  } else {
    // 12-hour time
    dig_time = (hourFormat12() * 100) + minute();
#ifdef LED1
    display.showNumberDecEx(dig_time, 0b11100000, false);
#endif
  }

  if (debug > 0) {
    char buff[64];
    sprintf(buff, "   digital time: %d", dig_time);
    Serial.println(buff);
  }

#ifdef LED2
  // display seconds
  int dig_sec;
  dig_sec = second() * 100;
  display2.showNumberDecEx(dig_sec, 0b01000000, true);
#endif
}

// Apply LED display options
void DigitalClockDisplayOpt() {
  // set brightness
  if (do_ldr) {
    // measure brightness
    // read sensor
    int sensorValue = analogRead(A0);
    float scale = sensorValue / 1023.;
    float vmax = 3.3;
    float voltage = (scale) * vmax;
    float bstep = (scale) * 7;
    brite = (int(floor(bstep)));

    char buff[64];
    if (debug > 0) {
      Serial.println("calculating brightness...");
      sprintf(buff, "ADC = %04d\n", sensorValue);
      Serial.print(buff);
      sprintf(buff, "percent = %7.3f%%\n", scale * 100.);
      Serial.print(buff);
      sprintf(buff, "voltage = %7.3f V\n", voltage);
      Serial.print(buff);
      sprintf(buff, "bstep = %7.3f\n", bstep);
      Serial.print(buff);
      sprintf(buff, "brite = %d\n", brite);
      Serial.print(buff);
    }
  } else {
    Serial.println("using default brightness");
    if ((hour() >= 20) || (hour() <= 6)) {  // night
      // dim display and show time only
      brite = 0;
    } else {  // day
      // brighten display and show options
      brite = 7;
    }
  }
  if (debug > 0) {
    Serial.print("setting brightness to ");
    Serial.println(brite);
  }
#ifdef LED1
  display.setBrightness(brite);
#endif
#ifdef LED2
  display2.setBrightness(brite);
#endif

  if ((hour() < 20) && (hour() > 6)) {  // day
    // show options
    int dig_time;
    if ((do_cyc) && (second() == 30)) {  // nixie tube cycling
      for (int i = 0; i < 10; i++) {
        dig_time = 1111 * i;
#ifdef LED1
        display.showNumberDecEx(dig_time, 0, true);
#endif
#ifdef LED2
        display2.showNumberDecEx(dig_time, 0, true);
#endif
        delay(DISP_DELAY);
      } // nixie loop
    } else if (do_sec_top &&
               ((second() >= 57) ||
                (second() <= 2))) {  // show seconds at the top of the minute
#ifndef LED2
      dig_time = (minute() * 100) + second();
#ifdef LED1
      display.showNumberDecEx(dig_time, 0b11100000, true);
#endif
#endif
    } else {
#ifndef LED2
      if (do_sec_mod && ((second() % 15) == 0) &&
          (second() > 0)) {  // flash seconds periodically
        dig_time = second();
#ifdef LED1
        display.clear();
        display.showNumberDec(dig_time, true, 2, 1);
#endif
        delay(DISP_DELAY);
      }
#endif
    }
  } // end day/night
  DigitalClockDisplay();
}
#endif
