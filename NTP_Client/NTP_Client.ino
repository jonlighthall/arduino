#include <NTPClient.h>
#include "wifi_settings.h"

NTPClient timeClient(ntpUDP);

void setup() {
  Serial.begin(9600);
  wifi_start();
  timeClient.begin();
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  delay(1000);
}
