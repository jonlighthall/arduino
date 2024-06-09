#ifndef BIN_UTILS
#define BIN_UTILS

// project library headers
#include "debug.h"

uint32_t getWord(byte packet[48], int idx) {
  // extract a 32-bit segment of a 48-byte packet
  uint32_t bit_word;
  // shift and OR 4 8-bit bytes to construct a 32-bit integer
  bit_word =  (unsigned long)packet[idx] << 24;
  bit_word |= (unsigned long)packet[idx + 1] << 16;
  bit_word |= (unsigned long)packet[idx + 2] << 8;
  bit_word |= (unsigned long)packet[idx + 3];
  return bit_word;
}

// print a binary number with leading zeros
void print_binary_spc(uint32_t number, uint8_t Length, uint8_t block) {
  // length is the number to print from right
  if (Length > 32)
    return;  
  static int Bits;
  static int bits_written;
  const char* spc = " ";

  if (number) { //The remaining bits have a value greater than zero continue
    Bits++; // Count the number of bits so we know how many leading zeros to print first
    print_binary_spc(number >> 1, Length,block); // Remove a bit and do a recursive call this function.
    if (Bits)
      for (byte x = (Length - Bits); x; x--) {
        Serial.write('0'); // Add the leading zeros
        bits_written++;
        if  ( ( (bits_written % block) == 0) && (bits_written < Length)) {
          Serial.print(spc);
        }
        if (bits_written == Length) bits_written = 0;
      }

    Bits = 0; // clear no need for this any more
    Serial.write((number & 1) ? '1' : '0'); // print the bits in reverse order as we depart the recursive function
    bits_written++;
    if  ( ( (bits_written % block) == 0) && (bits_written < Length) ) {
      Serial.print(spc);
    }
    if (bits_written == Length) bits_written = 0;
  }
  else { // if the value of number is zero, print zeros
    if (Bits == 0)
      for (byte x = Length; x; x--) {
        Serial.write('0'); // Add the leading zeros
        bits_written++;
        if  (((bits_written % block) == 0) && (bits_written < Length)) {
          Serial.print(spc);
        }
        if (bits_written == Length) bits_written = 0;
      }
  }
}

void print_binary(uint32_t number, uint8_t Length) {
  // length is the number to print from right
  if (Length > 32)
    return;
  static uint8_t Bits;
  if (number) { //The remaining bits have a value greater than zero continue
    Bits++; // Count the number of bits so we know how many leading zeros to print first
    print_binary(number >> 1, Length); // Remove a bit and do a recursive call this function.
    if (Bits) {
      //Serial.print("length = "); Serial.print(Length); Serial.print(", bits = "); Serial.print(Bits);
      if (Length > Bits)
        for (uint8_t x = (Length - Bits); x; x--)
          Serial.write('0'); // Add the leading zeros
    }
    Bits = 0; // clear no need for this any more
    if (Length > Bits)
      Serial.write((number & 1) ? '1' : '0'); // print the bits in reverse order as we depart the recursive function
  }
  else  // if the value of number is zero, print zeros
    if (Bits == 0)
      for (uint8_t x = Length; x; x--)
        Serial.print("0"); // Add the leading zeros
}

void print_uint32(uint32_t dword32) {
  char buff[64];
  sprintf(buff, "dec: %10u, hex: %08X, oct: %011o", dword32, dword32, dword32);
  Serial.print(buff);
  print_binary(dword32, 32);  Serial.print(", 32bin: ");
  Serial.println(dword32, BIN); // print as an ASCII-encoded binary
}

void print_uint16(uint16_t word16) {
  char buff[64];
  sprintf(buff, "dec: %5u, hex: %04X, oct: %06o", word16, word16, word16);
  Serial.print(buff);
  print_binary(word16, 16); Serial.print(", bin16: ");
  Serial.println(word16, BIN); // print as an ASCII-encoded binary
}

void print_uint8(byte byte8) {
  char buff[64];
  sprintf(buff, "dec: %3u, hex: %02X, oct: %03o", byte8, byte8, byte8);
  Serial.print(buff);
  print_binary(byte8, 8); Serial.print(", bin08: ");
  Serial.println(byte8, BIN); // print as an ASCII-encoded binary
}

void getBits16(uint16_t word16) {
  if (debug > 2) {
    uint8_t high, low;
    high = (word16 >> 8);
    Serial.print("high byte: ");
    print_uint8(high);

    low = (word16 << 8) >> 8;
    Serial.print(" low byte: ");
    print_uint8(low);
  }
}

uint32_t mask32(uint32_t dword32, uint8_t bit_start, uint8_t bit_len) {
  uint32_t mask, out;
  mask = ((1 << bit_len) - 1) << (32 - bit_start - bit_len);
  out = dword32 & mask;
  if (debug > 2) {
    char buff[64];
    sprintf(buff, "masking at position %u, length %u\n", bit_start, bit_len);
    Serial.print(buff);
    Serial.print("word: ");
    print_uint32(dword32);
    Serial.print("mask: ");
    print_uint32(mask);
    Serial.print(" out: ");
    print_uint32(out);
  }
  return out;
}

uint32_t shift32(uint32_t dword32, uint8_t bit_start, uint8_t bit_len) {
  uint32_t out;
  out = dword32 >> (32 - bit_start - bit_len);
  if (debug > 2) {
    char buff[64];
    sprintf(buff, "shifting at position %u, length %u\n", bit_start, bit_len);
    Serial.print(buff);
    Serial.print("word: ");
    print_uint32(dword32);
    Serial.print(" out: ");
    print_uint32(out);
  }
  return out;
}

uint32_t getBits32(uint32_t dword32, uint8_t bit_start, uint8_t bit_len) {
  uint32_t out, out2, out3;
  out2 = mask32(dword32, bit_start, bit_len);
  out3 = shift32(out2, bit_start, bit_len);

  if (debug > 2) {
    uint16_t high, low;
    //equivalent to start 0, length 16
    high = (dword32 << 0) >> 16;
    Serial.print("high 16: ");
    print_uint16(high);
    getBits16(high);

    //equivalent to start 16, length 16
    low = (dword32 << 16) >> 16;
    Serial.print(" low 16: ");
    print_uint16(low);
    getBits16(low);
    Serial.println("shift dword to starting point");
    out = (dword32 << bit_start);
    print_binary(out, bit_len); Serial.print(", full: "); print_binary(out, 32); Serial.print(", bin: ");
    Serial.println(out, BIN);

    Serial.println("shift back for length");
    out = (dword32 << bit_start) >> (32 - bit_len );
    print_binary(out, 32); Serial.print(", bin: ");
    Serial.println(out, BIN);
    Serial.print(" out: ");
    print_uint32(out);

    if (out != out3) {
      Serial.print("something didn't work...\n");
      return -1;
    }
    else
      Serial.print("shifting methods work!\n");
    return out;
  }
  else
    return out3;
}

#endif
