// This example sketch gets current time from NTP server, and prints it to Serial.

// standard library headers
#include <Arduino.h>
#include <TZ.h>
using ssid_char_t = const char;

// custom library headers
#include <wifi_utils.h>

// project headers
#include <ESPPerfectTime.h>

// NTP server
const char *ntpServer = "time.nist.gov";

void connectWiFi() {
  WiFi.begin(ssid, pass);

  Serial.print("\nconnecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nconnected. ");
}

void printTime(struct tm *tm, suseconds_t usec) {
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d.%06ld\n",
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                usec);
}

void setup() {
  Serial.begin(9600);
  connectWiFi();

  // Configure SNTP client in the same way as built-in one
  pftime::configTzTime(TZ_Asia_Tokyo, ntpServer);

  // This is deprecated:
  //pftime::configTime(9 * 3600, 0, ntpServer);

  // NOTE: ESP8266 Arduino core (2.7.0 to 2.7.1) has a sign-reversal bug for configTime(),
  //       so using configTzTime() is recommended
}

void loop() {

  // Get current local time as struct tm
  // by calling pftime::localtime(nullptr)
  struct tm *tm = pftime::localtime(nullptr);

  // You can get microseconds at the same time
  // by passing suseconds_t* as 2nd argument
  suseconds_t usec;
  tm = pftime::localtime(nullptr, &usec);

  // Print them to serial
  printTime(tm, usec);

  // Get current time as UNIX time
  time_t t = pftime::time(nullptr);

  // If time_t is passed as 1st argument,
  // pftime::localtime() behaves as with built-in localtime() function
  // This is for backward compatibility
  tm = pftime::localtime(&t);

  delay(1000);
}
