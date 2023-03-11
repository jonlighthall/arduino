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
  for (int i = 0; i < NTP_PACKET_LENGTH; i++) {
    packetWords[i] = getWord(packetBuffer, i * 4);
  }

  if (debug > 0) {
    // print raw packet
    Serial.println("\nraw 32-bit packet elements");
    Serial.println(" i |  decimal  |  hex     |  binary");
    Serial.println("---------------------------------------------------------------------");
    for (int i = 0; i < NTP_PACKET_LENGTH; i++) {
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
