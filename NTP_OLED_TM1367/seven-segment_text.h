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
