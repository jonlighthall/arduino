#ifndef SERIAL_UTILS
#define SERIAL_UTILS

// standard library headers
#include <TimeLib.h>

// project library headers
#include "hello.h"
#include "ntp_utils.h"

// Serial display settings
const int PRINT_DELAY=250; // print delay in milliseconds
const bool do_milliseconds = true;
const bool do_RSSI = false;
const char weldiv[] = "-------------"; // serial print divider for welcom message
const char serdiv[] = "----------------------------"; // serial print divider

void serial_init() {
  // initialize Serial
  Serial.begin(9600);
  while (!Serial)
    ;  // Needed for Leonardo only
  // pause for Serial Monitor
  delay(PRINT_DELAY);
  // Serial welcome message
  Serial.println();
  Serial.println(weldiv);
  hello();
  Serial.println(weldiv);
}

void serialClockDisplay() {
  // send date/time to Serial Monitor
  char buff[128];
  // print time
  sprintf(buff, "%02d:%02d:%02d ", hour(), minute(), second());
  Serial.print(buff);
  // print numeric date
  sprintf(buff, "%02d/%02d/%04d ", month(), day(), year());
  Serial.print(buff);
  // print string date
  sprintf(buff, "%s, ", dayStr(weekday()));
  Serial.print(buff);
  sprintf(buff, "%s ", monthStr(month()));
  Serial.print(buff);
  sprintf(buff, "%d ", day());
  Serial.print(buff);

  // print time zone
  sprintf(buff, "UTC%d (", SetTimeZone);
  Serial.print(buff);
  isDST(1);
  Serial.print(")");

  if (do_RSSI) {
    // print signal strength
    rssi = WiFi.RSSI();
    Serial.print(" RSSI: ");
    Serial.print(rssi);
  }

  Serial.println();
}

void serial_sync() {
  // Serial sync message
  Serial.println(serdiv);
  Serial.println("Transmiting NTP request...");
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
}

#endif
