#include <hello.h>

void setup() {
  // put your setup code here, to run once:
  serial_init();
  print_debug();
}

void loop() {
  // put your main code here, to run repeatedly:
  hello();
  delay(PRINT_DELAY);
}
