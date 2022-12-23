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
uint32_t packetWords[NTP_PACKET_SIZE / 4];

constexpr uint8_t  NTP_UNIX_OFFSET_YEARS = 70;
constexpr uint16_t DAYS_IN_YEAR          = 365;
constexpr uint8_t  NUMBER_OF_LEAP_YEARS  = 17;
constexpr uint32_t SECONDS_IN_DAY        = 86400;
constexpr uint32_t NTP_UNIX_OFFSET_SECONDS =
  (NTP_UNIX_OFFSET_YEARS * DAYS_IN_YEAR + NUMBER_OF_LEAP_YEARS) * SECONDS_IN_DAY;

const char* NTP_names[4] = {"reference", "originate", "receive", "transmit"};

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress & address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void readNTP_packet () {
  char buff[64];
  // read packet
  for (int i = 0; i < 12; i++) {
    packetWords[i] = getWord(packetBuffer, i * 4);
  }

  if (debug > 0) {
    // print raw packet
    Serial.println("\nraw 32-bit packet elements");
    Serial.println(" i |  decimal  |  hex     |  binary");
    Serial.println("---------------------------------------------------------------------");
    for (int i = 0; i < 12; i++) {
      sprintf(buff, "%2d | %010u | %08X | ", i, packetWords[i], packetWords[i]);
      Serial.print(buff);
      print_binary(packetWords[i], 32);
      Serial.println();
    }
  }
}

void parseNTP_header (uint32_t words[]) {
  char buff[64];
  // define variables
  uint32_t  rootDelay = words[2];
  uint32_t  rootDispersion = words[3];
  uint32_t  referenceIdentifier = words[4];

  Serial.print("header: ");
  print_uint32(words[0]);
  Serial.println();
  // define variables
  uint32_t  LI = getBits32(words[0], 0, 2);
  Serial.print("\nLI: ");
  print_uint8(LI);
  Serial.println("should be dec 0-3 or bin 00-11");
  uint32_t  VN = getBits32(words[0], 2, 3);
  Serial.print("\nVN: ");
  print_uint8(VN);
  Serial.println("should be dec 4 or bin 100");
  uint32_t  Mode = getBits32(words[0], 5, 3);
  Serial.print("\nMode: ");
  print_uint8(Mode);

  uint32_t  Stratum = getBits32(words[0], 8, 8);
  Serial.print("\nStra: ");
  print_uint8(Stratum);
  Serial.println("should be dec 0-16, hex 0-F");

  uint32_t  Poll = getBits32(words[0], 16, 8);
  Serial.print("\nPoll: ");
  print_uint8(Poll);
  Serial.print("int: ");
  int8_t pinterval = int8_t(Poll);
  Serial.print(pinterval);
  Serial.print(" seconds\n");
  Serial.println("8-bit signed int");

  uint32_t  Precision = getBits32(words[0], 24, 8);
  Serial.print("\nPrec: ");
  print_uint8(Precision);
  Serial.print("int: ");
  int8_t ppower = int8_t(Precision);
  Serial.print(ppower);
  Serial.print(", seconds ");
  double sprec = pow(2, ppower);

  Serial.print(sprec);
  sprintf(buff, "precision: %.30f seconds\n", sprec);
  Serial.print(buff);
  Serial.println("\n8-bit signed int");

  sprintf(buff, "%010u Root Delay\n", rootDelay);
  Serial.print(buff);
  sprintf(buff, "%010u Root Dispersion\n", rootDispersion);
  Serial.print(buff);
  sprintf(buff, "%010u Reference Identifier\n", referenceIdentifier);
  Serial.print(buff);

  if (Stratum == 1) {
    print_binary(referenceIdentifier, 32);
    Serial.println();
    uint8_t a = getBits32(referenceIdentifier, 0, 8) ;
    uint8_t b = getBits32(referenceIdentifier, 8, 8) ;
    uint8_t c = getBits32(referenceIdentifier, 16, 8) ;
    uint8_t d = getBits32(referenceIdentifier, 24, 8) ;

    sprintf(buff, "here\n");
    Serial.print(buff);

    sprintf(buff, "server id: %c%c%c%c\n", a, b, c, d);
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

    // print unix time
    Serial.println("\n32-bit unix timestamps");
    for (int i = 0; i < 4; i++) {
      sprintf(buff, "i = %2d %010u %s\n", i, secsSince1970[i], NTP_names[i]);
      Serial.print(buff);
    }

    // print unix time
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
