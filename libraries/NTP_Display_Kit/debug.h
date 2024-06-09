#ifndef DEBUG_LIB
#define DEBUG_LIB

//-------------------------------
const int debug = 0;
//-------------------------------

void print_debug() {
  Serial.print("debug = ");
  Serial.println(debug);
}

#endif
