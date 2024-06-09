// standard library headers
#include <Arduino.h>
// project library headers
#include "hello.h"

void hello()
{
  Serial.println("Hello, World!");
}

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

void print_debug() {
  Serial.print("debug = ");
  Serial.println(debug);
}
