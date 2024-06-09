#ifndef HELLO
#define HELLO

#include "debug.h"

const int PRINT_DELAY=250; // print delay in milliseconds
const char weldiv[] = "-------------"; // serial print divider for welcom message

void hello();
void serial_init();
void print_debug();

#endif
