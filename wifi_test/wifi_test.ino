#include "wifi_utils.h"
#define PRINT_DELAY 250 // print delay in milliseconds

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);
  // start serial
  Serial.begin(9600);
  while (!Serial) { }
  delay(PRINT_DELAY);
  Serial.println("\nWi-Fi Example");
  Serial.flush();
  wifi_mac();
  // connect to wi-fi
  wifi_con();
  // turn off LED
  digitalWrite(LED_BUILTIN, HIGH);
}

#define STATE_DELAY PRINT_DELAY*20

void loop() {
  Serial.println("Begin loop");
  delay(PRINT_DELAY);
  wifi_discon() ;
  delay(STATE_DELAY);
  wifi_con();
  delay(STATE_DELAY);
}
