#include <NTPClient.h>

#include "wifi_settings.h"

// NTP
static const char ntpServerName[] = "time.nist.gov";
const long nptTimeOffset = 0;                 // seconds
const unsigned long nptUpdateInterval = 300;  // millisecons

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, ntpServerName);

#define BLINK_DELAY 50  // delay in milliseconds

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN,
               HIGH);  // Turn the LED off by making the voltage HIGH

  Serial.begin(9600);
  while (!Serial) {
    digitalWrite(LED_BUILTIN, LOW);  // on
    delay(BLINK_DELAY);
    digitalWrite(LED_BUILTIN, HIGH);  // off
    delay(BLINK_DELAY);
  }
  delay(128);
  Serial.println("NTP Client");

  wifi_con();
  timeClient.begin();
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getEpochTime());
  Serial.println(timeClient.getDay());
  Serial.println(timeClient.getHours());
  Serial.println(timeClient.getMinutes());
  Serial.println(timeClient.getSeconds());
  delay(1000);
}
