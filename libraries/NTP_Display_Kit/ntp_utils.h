#ifndef NTP_UTILS
#define NTP_UTILS

// standard library headers
#include <Arduino.h>
//#include <Print.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

// project library headers
#include <wifi_utils.h>
#include <dst.h>
#include <binary_utils.h>

// ESP8266 Wi-Fi UDP settings
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

// TimeLib settings
time_t getNtpTime();
// set synchronization interval in seconds
#define SYNC_INTERVAL 60 * 10
// set synchronization interval in milliseconds
int syncInterval = SYNC_INTERVAL * 1e3;

// Set Standard time zone
const int timeZone = -6;  // CST
int SetTimeZone = timeZone;

// DST settings
const bool do_DST = true;

// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";

//void sendNTPpacket(IPAddress &address);

time_t prevDisplay = 0;  // when the digital clock was displayed
uint32_t LastSyncTime;

uint32_t NTPfracTime;
uint32_t NTPlocalTime;

// The NTP timestamp packet is 48 bytes long

// define packet size in bytes
const int NTP_PACKET_SIZE = 48;

// create a buffer (an array of bytes) to hold the incoming & outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];

// define the packet size in 32-bit unsigned inegers
// to convert from (8-bit) bytes to 32-bit unsigned integers, divide by 32/8=4
const int NTP_PACKET_LENGTH = NTP_PACKET_SIZE / 4;

// create an array of 32-bit unsigned integers to parse the packets
uint32_t packetWords[NTP_PACKET_LENGTH];

constexpr uint8_t  NTP_UNIX_OFFSET_YEARS = 70;
constexpr uint16_t DAYS_IN_YEAR          = 365;
constexpr uint8_t  NUMBER_OF_LEAP_YEARS  = 17;
constexpr uint32_t SECONDS_IN_DAY        = 86400;
constexpr uint32_t NTP_UNIX_OFFSET_SECONDS =
  (NTP_UNIX_OFFSET_YEARS * DAYS_IN_YEAR + NUMBER_OF_LEAP_YEARS) * SECONDS_IN_DAY;

const char* NTP_header_names[4] = { "header", "root delay", "root dispersion", "reference identifier" };
const char* NTP_names[4] = { "reference", "origin", "receive", "transmit" };

void udp_start() {
  // start UDP
  Serial.println("Starting UDP...");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
}

// parse the NPT packet into 12 32-bit "words"
void readNTP_packet() {
  char buff[64];
  sprintf(buff, "Parsing %i-byte packet into %i 32-bit words... ", NTP_PACKET_SIZE, NTP_PACKET_LENGTH);
  Serial.print(buff);
  // read packet
  for (int i = 0; i < NTP_PACKET_LENGTH; i++) {
    packetWords[i] = getWord(packetBuffer, i * 4);
  }
  Serial.println("done");
}

void parseNTP_header(uint32_t words[]) {
  static int debug=3;
  char buff[64];
  Serial.println("Parsing NTP header...");
  // define raw variables
  byte LI = getBits32(words[0], 0, 2);          // Leap Indicator (2 bits)
  byte VN = getBits32(words[0], 2, 3);          // Version Number (3 bits)
  byte Mode = getBits32(words[0], 5, 3);        // Mode (3 bits)
  byte Stratum = getBits32(words[0], 8, 8);     // Stratum (8 bits)
  byte Poll = getBits32(words[0], 16, 8);       // Poll interval (8 bits)
  byte Precision = getBits32(words[0], 24, 8);  // Precision (8 bits)
  uint32_t rootDelay = words[1];                // Root Delay (32 bits)
  uint32_t rootDispersion = words[2];           // Root Dispersion (32 bits)
  uint32_t referenceIdentifier = words[3];      // Reference Identifier (32 bits)

  // define converted variables
  int8_t poll_interval_log = int8_t(Poll);
  int16_t poll_interval = pow(2, poll_interval_log);
  int8_t precision_log = int8_t(Precision);
  double precision_seconds = pow(2, precision_log);
  uint8_t a = getBits32(referenceIdentifier, 0, 8);
  uint8_t b = getBits32(referenceIdentifier, 8, 8);
  uint8_t c = getBits32(referenceIdentifier, 16, 8);
  uint8_t d = getBits32(referenceIdentifier, 24, 8);
  char RefID[4];
  sprintf(RefID, "%c%c%c%c", a, b, c, d);

  // print header
  if (debug > -1) {
    // print raw packet
    Serial.println();
    if (debug > 0)
      Serial.println("raw 32-bit packet elements");
    Serial.print(" i |  decimal   |  hex     |");
    if (debug > 0)
      Serial.print("  binary");
    Serial.println();
    Serial.print("---+------------+----------+");
    if (debug > 0)
      Serial.print("----------------------------------");
    Serial.println();
    for (int i = 0; i < NTP_PACKET_LENGTH; i++) {
      sprintf(buff, "%2d | %010u | %08X | ", i, packetWords[i], packetWords[i]);
      Serial.print(buff);
      if (debug > 0)
        print_binary(packetWords[i], 32);
      if (i < 4) {
        sprintf(buff, " %s", NTP_header_names[i]);
        Serial.print(buff);
      } else {
        sprintf(buff, " %s ", NTP_names[(i - 4) / 2]);
        Serial.print(buff);
        if (((i - 4) % 2) == 0) {
          Serial.print("seconds");
        } else {
          Serial.print("fraction");
        }
      }
      Serial.println();
    }
  }

  if (debug > 0) {
    Serial.print("\nheader: ");
    print_uint32(words[0]);
  }
  // Print variables
  Serial.print("\n       LI: ");

  if (debug > 0) {
    print_uint8(LI);
    Serial.println("should be dec 0-3 or bin 00-11");
  }

  print_binary(LI, 2);
  sprintf(buff, "       %3d ", LI);
  Serial.print(buff);
  // Serial.print("\t");
  // Serial.print(LI, DEC);
  // Serial.print("\t");
  switch (LI) {
      // Warns of impending leap second adjustments.
    case 0:
      // no warning
      Serial.print("no leap second");
      break;
    case 1:
      // Last minute of the day has 61 seconds
      Serial.print("+1 leap second");
      break;
    case 2:
      // Last minute of the day has 59 seconds
      Serial.print("-1 leap second");
      break;
    case 3:
      // Alarm condition (clock not synchronized)
      Serial.print("unsynced");
      break;
    default:
      Serial.print("UNDEFINED");
      break;
  }
  Serial.println();

  // time.nist.gov uses NTPv3
  // NTPv3 uses 32-bit timestamps
  // This program assumes a 32-bit timestamp
  Serial.print("  Version: ");
  if (debug > 0) {
    print_uint8(VN);
    Serial.println("must be dec 3 or bin 011");
  }
  Serial.print("  ");
  print_binary(VN, 3);
  sprintf(buff, "    %3d\n", VN);
  Serial.print(buff);

  Serial.print("Version number is... ");
  if (VN == 3 ) {
    Serial.println("OK")  ;
  } else {
    Serial.println("INCOMPATIBLE");
  }

  Serial.print("     Mode: ");
  if (debug > 0) {
    print_uint8(Mode);
    Serial.println("should be dec 4 or bin 100");
  }
  Serial.print("     ");
  print_binary(Mode, 3);
  sprintf(buff, " %3d ", Mode);
  Serial.print(buff);
  switch (Mode) {
    // Specifies the operating mode of the sender:
  case 1:
    // Symmetric active
    Serial.print("active");
    break;
  case 2:
    // Symmetric passive
    Serial.print("passive");
    break;
  case 3:
    // Client
    Serial.print("client");
    break;
  case 4:
    // Server
    Serial.print("server");
    break;
  case 5:
    // Broadcast
    Serial.print("broadcast");
    break;
  case 6:
    // NTP control message (reserved for management)
    Serial.print("control");
    break;
  case 7:
    // Reserved for private use
    Serial.print("private");
    break;
  default:
    Serial.print("UNDEFINED");
    break;
  }
  Serial.println();

  Serial.print("  Stratum: ");
  if (debug > 0) {
    print_uint8(Stratum);
    Serial.println("should be dec 0-16, hex 0-F");
    Serial.println("should be dec 1, hex 1");
  }
  print_binary(Stratum, 8);
  sprintf(buff, " %3d ", Stratum);
  Serial.print(buff);
  // Indicates the distance of the device from a primary reference clock.
  if (Stratum == 0)
    Serial.print("unspecified");
  if (Stratum == 1)
    Serial.print("primary");  // reference clock
  if (Stratum > 1 && Stratum < 16)
    Serial.print("secondary");
  if (Stratum == 16)
    Serial.print("unsynched");
  Serial.println();

  Serial.print("     Poll: ");
  if (debug > 0) {
    print_uint8(Poll);
    Serial.println("8-bit signed int");
  }
  print_binary(Poll, 8);
  sprintf(buff, " %3d log2 seconds, 2^%d seconds, %d seconds\n", poll_interval_log, poll_interval_log, poll_interval);
  Serial.print(buff);

  Serial.print("Precision: ");
  if (debug > 0) {
    print_uint8(Precision);
    Serial.println("8-bit signed int");
  }
  print_binary(Precision, 8);
  sprintf(buff, " %3d log2 seconds, 2^%d seconds, %.15f seconds\n", precision_log, precision_log, precision_seconds);
  Serial.print(buff);

  sprintf(buff, "\n          Root Delay: %010u\n", rootDelay);
  Serial.print(buff);
  sprintf(buff, "     Root Dispersion: %010u\n", rootDispersion);
  Serial.print(buff);
  sprintf(buff, "Reference Identifier: %010u", referenceIdentifier);
  Serial.print(buff);

  if (Stratum == 1) {
    sprintf(buff, ", server id: %s\n", RefID);
    Serial.print(buff);
  }
}

void parseNTP_time(uint32_t words[]) {
  char buff[64];

  if (debug > 0) {
    // print raw NTP time
    Serial.println("\nraw 64-bit timestamps");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %1d %010u %010u %s\n", i + 1, words[4 + i * 2], words[5 + i * 2], NTP_names[i]);
      Serial.print(buff);
    }
  }

  // calculate times
  uint32_t secsSince1900[4];
  uint32_t secsSince1970[4];
  uint32_t localTime[4];

  for (int i = 0; i < 4; i++) {
    // extract raw NTP time
    secsSince1900[i] = words[4 + i * 2];
    // convert to unix time
    secsSince1970[i] = secsSince1900[i] - NTP_UNIX_OFFSET_SECONDS;
    // convert to local time
    localTime[i] = secsSince1970[i] + SetTimeZone * SECS_PER_HOUR;
  }

  if (debug > 1) {
    // print raw NTP time
    Serial.println("\nraw 32-bit timestamps (seconds)");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %1d %010u %s\n", i + 1, secsSince1900[i], NTP_names[i]);
      Serial.print(buff);
    }

    Serial.print("epoch offset = ");
    Serial.println(NTP_UNIX_OFFSET_SECONDS);

    // print UTC unix time
    Serial.println("\n32-bit unix timestamps");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %2d %010u %s\n", i, secsSince1970[i], NTP_names[i]);
      Serial.print(buff);
    }

    // print local unix time
    Serial.println("\nlocal 32-bit timestamps");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %2d %010u %s\n", i, localTime[i], NTP_names[i]);
      Serial.print(buff);
    }
  }

  // export time
  NTPlocalTime = localTime[3];
}

void parseNTP_fraction(uint32_t words[]) {
  Serial.println("Parsing 32-bit fractional part of timestamp...");
  char buff[64];
  uint32_t fracSecs[4];
  // print raw NTP time
  Serial.println("\nraw 32-bit timestamps (fraction)");
  for (int i = 0; i < 4; i++) {
    fracSecs[i] = words[5 + i * 2];
    sprintf(buff, "i = %1d %010u %s\n", i + 1, fracSecs[i], NTP_names[i]);
    Serial.print(buff);
  }

  // convert the fractional part to milliseconds
  uint32_t ifrac_secs[4];
  double frac_secs[4];
  for (int i = 0; i < 4; i++) {
    ifrac_secs[i] = words[5 + i * 2];
    frac_secs[i] = ifrac_secs[i]/(0xFFFFFFFF);
  }

  NTPfracTime = ((uint64_t)fracSecs[3] * 1000) >> 32;

  //      if (debug > 1) {
  // print fractional times
  Serial.println("\nfractional times");
  Serial.print("NTPfracTime = ");
  Serial.println(NTPfracTime);
  //    }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  if (debug > 0) {
    Serial.println("initialized packet buffer...");
    readNTP_packet();
  }
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  if (debug > 0) {
    Serial.println("sending packet buffer...");
    readNTP_packet();
    parseNTP_time(packetWords);
  }
  Udp.beginPacket(address, 123);  //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

#endif
