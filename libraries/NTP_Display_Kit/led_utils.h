#ifndef LED_UTILS
#define LED_UTILS

// LED display options
#include <TM1637Display.h>
const int CLK = D6;               // Set the CLK pin connection to the display
const int DIO = D5;               // Set the DIO pin connection to the display
TM1637Display display(CLK, DIO);  // set up the 4-Digit Display.
#include <seven-segment_text.h>
bool do_mil = false;
bool do_sec_top = false;
bool do_cyc = false;
bool do_sec_mod = false;

#define DISP_DELAY 250  // print delay in milliseconds

void DigitalClockDisplay() {
  // Display Time
  int dig_time;

  if (do_mil) {
    // military time
    dig_time = (hour() * 100) + minute();
    display.showNumberDecEx(dig_time, 0b11100000, true);
  } else {
    // 12-hour time
    dig_time = (hourFormat12() * 100) + minute();
    display.showNumberDecEx(dig_time, 0b11100000, false);
  }

  if (debug > 0) {
    char buff[64];
    sprintf(buff, "   digital time: %d", dig_time);
    Serial.println(buff);
  }
}

// LED Display options
void DigitalClockDisplayOpt() {
  // set brightness
  if ((hour() >= 20) || (hour() <= 6)) {  // night
    // dim display and show time only
    display.setBrightness(0);
    DigitalClockDisplay();
  } else {  // day
    // brighten display and show options
    display.setBrightness(7);
    int dig_time;
    if ((do_cyc) && (second() == 30)) {  // nixie tube cycling
      for (int i = 0; i < 10; i++) {
        dig_time = 1111 * i;
        display.showNumberDecEx(dig_time, 0, true);
        delay(DISP_DELAY);
      }
      DigitalClockDisplay();
    } else if (do_sec_top &&
               ((second() >= 57) ||
                (second() <= 2))) {  // show seconds at the top of the minute
      dig_time = (minute() * 100) + second();
      display.showNumberDecEx(dig_time, 0b11100000, true);
    } else {
      if (do_sec_mod && ((second() % 15) == 0) &&
          (second() > 0)) {  // flash seconds periodically
        dig_time = second();
        display.clear();
        display.showNumberDec(dig_time, true, 2, 1);
        delay(DISP_DELAY);
      }
      DigitalClockDisplay();
    }
  }
}
#endif
