#include <NTPClient.h>
#include "wifi_settings.h"

NTPClient timeClient(ntpUDP);

#define BLINK_DELAY 50 // delay in milliseconds

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  Serial.begin(9600);
  while (!Serial) {
    digitalWrite(LED_BUILTIN, LOW); // on
    delay(BLINK_DELAY);
    digitalWrite(LED_BUILTIN, HIGH); // off
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
  delay(1000);
}
