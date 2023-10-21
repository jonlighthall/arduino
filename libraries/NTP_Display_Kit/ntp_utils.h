#include <TimeLib.h>

#include <WiFiUdp.h>
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";

// Set Standard time zone
const int timeZone = -6; // CST

int SetTimeZone = timeZone;
const bool do_DST = true;

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

#define SYNC_INTERVAL 30 // print delay in seconds
int syncInterval = SYNC_INTERVAL * 1e3;

uint32_t NTPfracTime;
uint32_t NTPlocalTime;

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
const int NTP_PACKET_LENGTH = NTP_PACKET_SIZE / 4;
uint32_t packetWords[NTP_PACKET_LENGTH];

constexpr uint8_t  NTP_UNIX_OFFSET_YEARS = 70;
constexpr uint16_t DAYS_IN_YEAR          = 365;
constexpr uint8_t  NUMBER_OF_LEAP_YEARS  = 17;
constexpr uint32_t SECONDS_IN_DAY        = 86400;
constexpr uint32_t NTP_UNIX_OFFSET_SECONDS =
  (NTP_UNIX_OFFSET_YEARS * DAYS_IN_YEAR + NUMBER_OF_LEAP_YEARS) * SECONDS_IN_DAY;

const char* NTP_header_names[4] = {"header", "root delay", "root dispersion", "reference identifier"};
const char* NTP_names[4] = {"reference", "origin", "receive", "transmit"};

void readNTP_packet () {
  char buff[64];
  // read packet
  for (int i = 0; i < NTP_PACKET_LENGTH; i++) {
    packetWords[i] = getWord(packetBuffer, i * 4);
  }  
}

void parseNTP_header (uint32_t words[]) {
  char buff[64];
  // define raw variables
  byte LI = getBits32(words[0], 0, 2);
  byte VN = getBits32(words[0], 2, 3);
  byte Mode = getBits32(words[0], 5, 3);
  byte Stratum = getBits32(words[0], 8, 8);
  byte Poll = getBits32(words[0], 16, 8);
  byte Precision = getBits32(words[0], 24, 8);
  uint32_t rootDelay = words[1];
  uint32_t rootDispersion = words[2];
  uint32_t referenceIdentifier = words[3];

  // define converted variables
  int8_t Poll_interval = int8_t(Poll);
  int8_t ppower = int8_t(Precision);
  double sprec = pow(2, ppower);
  uint8_t a = getBits32(referenceIdentifier, 0, 8) ;
  uint8_t b = getBits32(referenceIdentifier, 8, 8) ;
  uint8_t c = getBits32(referenceIdentifier, 16, 8) ;
  uint8_t d = getBits32(referenceIdentifier, 24, 8) ;
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
      }
      else {
        sprintf(buff, " %s ", NTP_names[(i - 4) / 2]);
        Serial.print(buff);
        if (((i - 4) % 2) == 0) {
          Serial.print("seconds");
        }
        else {
          Serial.print("fraction");
        }
      }
      Serial.println();
    }
  }
  
  if (debug > 0) {
    Serial.print("\nheader: ");
    print_uint32(words[0]);
    Serial.println();
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
  //Serial.print("\t");
  //Serial.print(LI, DEC);
  //Serial.print("\t");
  switch (LI) {
    case 0:
      Serial.print("no leap second");
      break;
    case 1:
      Serial.print("+1 leap second");
      break;
    case 2:
      Serial.print("-1 leap second");
      break;
    case 3:
      Serial.print("unsynced");
      break;
    default:
      Serial.print("UNDEFINED");
      break;
  }
  Serial.println();

  Serial.print("  Version: ");
  if (debug > 0) {
    print_uint8(VN);
    Serial.println("should be dec 4 or bin 100");
  }
  Serial.print("  ");
  print_binary(VN, 3);
  sprintf(buff, "    %3d\n", VN);
  Serial.print(buff);

  Serial.print("     Mode: ");
  if (debug > 0) {
    print_uint8(Mode);
    Serial.println("should be dec 3 or bin 011");
  }
  Serial.print("     ");
  print_binary(Mode, 3);
  sprintf(buff, " %3d ", Mode);
  Serial.print(buff);
  if (Mode == 4)
    Serial.print("server");
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
  if (Stratum == 1)
    Serial.print("primary");
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
  sprintf(buff, " %3d seconds\n", Poll_interval);
  Serial.print(buff);

  Serial.print("Precision: ");
  if (debug > 0) {
    print_uint8(Precision);
    Serial.println("8-bit signed int");
  }
  print_binary(Precision, 8);
  sprintf(buff, " %3d log2(seconds), %.15f seconds\n", ppower, sprec);
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

void parseNTP_time (uint32_t words[]) {
  char buff[64];
  if (debug > 0 ) {
    // print raw NTP time
    Serial.println("\nraw 64-bit timestamps");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %1d %010u %010u %s\n", i + 1, words[4 + i * 2], words[5 + i * 2], NTP_names[i]);
      Serial.print(buff);
    }
  }

  //calculate times
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
    Serial.println(NTP_UNIX_OFFSET_SECONDS );

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

void parseNTP_fraction (uint32_t words[]) {
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
  }

  NTPfracTime = ((uint64_t) fracSecs[3] * 1000) >> 32;

  //      if (debug > 1) {
  // print fractional times
  Serial.println("\nfractional times");
  Serial.print("NTPfracTime = ");
  Serial.println(NTPfracTime);
  //    }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress & address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  if (debug > 0) {
    Serial.println("initialized packet buffer...");
    readNTP_packet();
  }
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
