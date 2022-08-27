const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

const uint8_t SEG_SYNC[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G ,          // S
  SEG_B | SEG_C | SEG_D | SEG_F | SEG_G ,          // y
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F                    // C
};

const uint8_t SEG_HEllO[] = {
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G ,          // H
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_B | SEG_C | SEG_E | SEG_F,                   // l l
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F    // O
};

const uint8_t SEG_hEllo[] = {
  SEG_C | SEG_E | SEG_F | SEG_G ,                  // h
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_B | SEG_C | SEG_E | SEG_F,                   // l l
  SEG_C | SEG_D | SEG_E | SEG_G                    // o
};

const uint8_t SEG_CONN[] = {
  SEG_A | SEG_D | SEG_E | SEG_F,                   // C
  SEG_C | SEG_D | SEG_E | SEG_G,                   // o
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_C | SEG_E | SEG_G                            // n
};

const uint8_t SEG_degC[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,                   // Degree Symbol
  SEG_A | SEG_D | SEG_E | SEG_F                    // C
};

const uint8_t SEG_degF[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,                   // Degree Symbol
  SEG_A | SEG_E | SEG_F | SEG_G                    // F
};

const uint8_t SEG_letF[] = {
  SEG_A | SEG_E | SEG_F | SEG_G                    // F
};

const uint8_t SEG_letA[] = {
  SEG_A | SEG_B | SEG_F | SEG_G | SEG_E | SEG_C
};

const uint8_t SEG_letb[] = {
  SEG_F | SEG_E | SEG_G | SEG_D | SEG_C
};

const uint8_t SEG_letC[] = {
  SEG_A | SEG_F | SEG_E | SEG_D 
};

const uint8_t SEG_bad[] = {
  0x00,
  SEG_F | SEG_E | SEG_G | SEG_D | SEG_C,           // B
  SEG_A | SEG_B | SEG_F | SEG_G | SEG_E | SEG_C,   // a
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G            // d
};

const uint8_t SEG_scan[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G ,          // S
  SEG_A | SEG_D | SEG_E | SEG_F,                   // C
  SEG_A | SEG_B | SEG_F | SEG_G | SEG_E | SEG_C,   // a    
  SEG_C | SEG_E | SEG_G                            // n
};
