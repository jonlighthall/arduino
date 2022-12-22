/*
   TimeNTP_ESP8266WiFi.ino
   Example showing time sync to NTP time source
*/

#include <TimeLib.h>

// OLED packages
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "dst.h"
#include "wifi_settings.h"
const int debug = 2;
#include "binary_utils.h"

// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";

// Set Standard time zone
const int timeZone = -6; // CST

int SetTimeZone = timeZone;
const bool do_DST = true;

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

void serialClockDisplay();
void OLED_Sync_Bar();

// OLED display options
const bool do_RSSI = false;
const bool do_SyncBar = false;
const bool do_BigTime = false;
const bool do_SecondsBar = true;
const bool do_Seconds = true;
const bool do_milli = true;
void OLEDClockDisplay();
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
int dispwid;
int disphei;

#define PRINT_DELAY 250 // print delay in milliseconds
#define SYNC_INTERVAL 30 // print delay in seconds

int syncBar = 0;

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  //delay(PRINT_DELAY);
  //testDST();
  delay(PRINT_DELAY);
  Serial.println();
  char buff[64];
  sprintf(buff, "\nTimeNTP Example");
  Serial.println(buff);

  // display settings
  u8g2.begin();
  u8g2.clearBuffer();			// clear the internal memory

  // get display dimensions
  dispwid = u8g2.getDisplayWidth();
  disphei = u8g2.getDisplayHeight();

  u8g2.setFont(u8g2_font_timB08_tr);	// choose a suitable font
  sprintf(buff, "NTP Time");
  // get text dimensions
  int textwid = u8g2.getStrWidth(buff);
  int texthei = u8g2.getAscent();

  // set text position
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;

  u8g2.drawStr(xpos, ypos, buff);        	// write something to the internal memory
  u8g2.sendBuffer();			// transfer internal memory to the display
  delay(PRINT_DELAY);

  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d", dispwid, disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  delay(PRINT_DELAY);

  // Wi-Fi settings
  Serial.print("Connecting to ");
  Serial.print(ssid);
  sprintf(buff, "Wi-Fi...");
  xpos = 0;
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  WiFi.begin(ssid, pass);

  // print text throbber
  xpos += u8g2.getStrWidth(buff) + 1;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    sprintf(buff, "X  ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();

    delay(500);

    // draw black background
    u8g2.setDrawColor(0);
    u8g2.drawBox(xpos, ypos - texthei, 9,  texthei);
    u8g2.setDrawColor(1);

    Serial.print(".");
    sprintf(buff, "O   ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();
  }
  Serial.print("connected\n");
  // draw black background
  u8g2.setDrawColor(0);
  u8g2.drawBox(xpos, ypos - texthei, 9,  texthei);
  u8g2.setDrawColor(1);

  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  sprintf(buff, "NTP sync...");
  xpos = 0;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  setSyncProvider(getNtpTime);

  // wait for time to be set
  if (timeStatus() == timeNotSet)
    setSyncInterval(0);
  while (timeStatus() == timeNotSet) {
    ;
  }
  setSyncInterval(1);
  Serial.println("sync complete");
  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  if (do_DST) {
    serialClockDisplay();
    Serial.print("checking DST status... ");
    SetTimeZone = timeZone + isDST(1);
    Serial.println();
    if (isDST() > 0) {
      Serial.println("here");
      delay(1001); // why wait?
      serialClockDisplay();
    }
  } else {
    SetTimeZone = timeZone;
  }

  sprintf(buff, "Timezone = %d", SetTimeZone);
  xpos = dispwid - u8g2.getStrWidth(buff);
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  setSyncInterval(SYNC_INTERVAL); // refresh rate in seconds
  Serial.println("starting...");
}

time_t prevDisplay = 0; // when the digital clock was displayed

uint32_t LastSyncTime;
uint32_t fracTime;
char serdiv[] = "----------------------------"; // serial print divider

int syncInterval = SYNC_INTERVAL * 1e3;
void loop() {
  char buff[64];
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      if (debug > 1) {
        // check time in seconds
        uint32_t tprev = prevDisplay;
        sprintf(buff, "prev = %d\n", tprev);
        Serial.print(buff);
        uint32_t tnow = now();
        sprintf(buff, " now = %d\n", tnow);
        Serial.print(buff);
        uint32_t elap = tnow - prevDisplay;
        sprintf(buff, "elap = %d\n", elap);
        Serial.print(buff);

        sprintf(buff, "%d %d %d\n", tprev, tnow, elap);
        Serial.print(buff);
      }
      prevDisplay = now();

      // check DST
      if (do_DST) {
        if (debug > 0)
          Serial.print("checking DST status... ");
        SetTimeZone = timeZone + isDST(debug);
        if (debug > 0)
          Serial.println();
      } else {
        SetTimeZone = timeZone;
      }

      // check time in milliseconds
      uint32_t printTime = millis();
      // calculate time since/until last/next sync
      int TimeSinceSync = printTime - LastSyncTime;
      int ToSyncTime = syncInterval - TimeSinceSync;
      float syncWait = (float)TimeSinceSync / syncInterval;
      syncBar = syncWait * disphei;
      if (debug > 1) {
        Serial.print("last sync time = ");
        Serial.println(LastSyncTime);
        Serial.print("      time now = ");
        Serial.println(printTime);
        Serial.print("  elapsed time = ");
        Serial.println(TimeSinceSync);
        // print time since/until last/next sync
        sprintf(buff, " Time since last sync = %6d ms or %7.3f s\n", TimeSinceSync, TimeSinceSync / 1e3);
        Serial.print(buff);
        // print time between syncs percentage
        sprintf(buff, "   Time between syncs = %6d ms or %7.3f s\n", syncInterval, syncInterval / 1e3);
        Serial.print(buff);
        sprintf(buff, " Time until next sync = %6d ms or %7.3f s\n", ToSyncTime, ToSyncTime / 1e3);
        Serial.print(buff);
        sprintf(buff, "Sync delay percentage = %7.3f%% or %2d pixels\n", syncWait * 100, syncBar);
        Serial.print(buff);
      }

      // wait until top of second to print time

      sprintf(buff, "fracTime = %d\n", fracTime);
      Serial.print(buff);

      if ((TimeSinceSync < 1000) && (TimeSinceSync > 0)) {
        int totalDelay = fracTime + TimeSinceSync;
        int setDelay = totalDelay % 1000;
        int offsetTime = 1000 - setDelay;
        if (debug > 1) {
          Serial.println(serdiv);
          sprintf(buff, "total delay = %d\n", totalDelay );
          Serial.print(buff);
          sprintf(buff, "  set delay = %d\n", setDelay  );
          Serial.print(buff);
          Serial.print("offset time = ");
          Serial.println(offsetTime);
          sprintf(buff, "delaying display by %d...", offsetTime);
          Serial.print(buff);
        }
        delay(offsetTime);
        if (debug > 1) {
          Serial.println("done");
          Serial.println(serdiv);
        }
      }
      else {
        if (debug > 1) {
          int delayError = TimeSinceSync % 1000;
          int delayDiff = 1000 - delayError;
          Serial.println(serdiv);
          sprintf(buff, "elapsed time since last sync = %d ms\n", TimeSinceSync);
          Serial.print(buff);
          sprintf(buff, "sub-second error = %d ms\n", delayError);
          Serial.print(buff);
          int delayInterval = min(10, delayDiff);
          sprintf(buff, "waiting %d ms...", delayInterval);
          Serial.print(buff);
          delay(delayInterval);
          Serial.println("done");
          Serial.println(serdiv);
        }
      }
      int beforeTime = millis();
      serialClockDisplay();
      int midTime = millis();
      OLEDClockDisplay();
      int afterTime = millis();

      if (debug > 1) {
        sprintf(buff, "serialClockDisplay takes %d\n", midTime - beforeTime);
        Serial.print(buff);

        sprintf(buff, "OLEDClockDisplay takes %d\n", afterTime - midTime);
        Serial.print(buff);

        sprintf(buff, "Clock Displays take %d\n", afterTime - beforeTime);
        Serial.print(buff);

        Serial.print("end of loop, after display: millis = ");
        Serial.println(millis());
      }
    }
  }
}

void serialClockDisplay() {
  // digital clock display of the time
  char buff[128];
  // print time
  sprintf(buff, "%02d:%02d:%02d ", hour(), minute(), second());
  Serial.print(buff);
  // print numperical date
  sprintf(buff, "%02d/%02d/%04d ", month(), day(), year());
  Serial.print(buff);
  // print string date
  sprintf(buff, "%s, ", dayStr(weekday()));
  Serial.print(buff);
  sprintf(buff, "%s ", monthStr(month()));
  Serial.print(buff);
  sprintf(buff, "%d ", day());
  Serial.print(buff);

  // print time zone
  sprintf(buff, "UTC%d (", SetTimeZone);
  Serial.print(buff);
  isDST(1);
  Serial.print(")");

  // print signal strength
  rssi = WiFi.RSSI();
  Serial.print(" RSSI: ");
  Serial.print(rssi);

  Serial.println();
}

void OLEDClockDisplay() {
  // defin OLED variables
  u8g2.clearBuffer();
  int xpos;
  int ypos;
  char buff[dispwid];

  if (do_BigTime) {
    // draw clock display
    u8g2.setFont(u8g2_font_profont22_tn);
    sprintf(buff, "%02d:%02d", hour(), minute());
    if (debug > 0)
      Serial.println(buff);
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos = u8g2.getAscent();
    u8g2.drawStr(xpos, ypos, buff);
  }

  if (do_SecondsBar) {
    // draw seconds bar
    ypos += 2;
    for (int i = 0; i < second() + 1; i++) {
      u8g2.drawPixel(i + 3, ypos);
    }
    ypos += 1;
    for (int i = 0; i < second() + 1; i = i + 5) {
      u8g2.drawPixel(i + 3, ypos);
    }
    ypos += 1;
    for (int i = 0; i < second() + 1; i = i + 15) {
      u8g2.drawPixel(i + 3, ypos);
    }
  }

  if (do_Seconds) {

    // write seconds
    u8g2.setFont(u8g2_font_profont15_tn);
    sprintf(buff, "%02d:%02d:%02d", hour(), minute(), second());
    if (debug > 0)
      Serial.println(buff);
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos += u8g2.getAscent() + 2;
    u8g2.drawStr(xpos, ypos, buff);

    int co_sec = 60 - second();
    int co_min = 60 - minute();
    if (co_sec < 60) {
      co_min--;
    }
    else if (co_sec == 60) {
      co_sec = 0;
    }

    int co_hr = 24 - hour();
    if (co_min < 60) {
      co_hr--;
    }
    else if (co_min == 60) {
      co_min = 0;
    }

    sprintf(buff, "%02d:%02d:%02d", co_hr, co_min, co_sec);
    if (debug > 0)
      Serial.println(buff);
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos += u8g2.getAscent() + 2;
    u8g2.drawStr(xpos, ypos, buff);

  }

  // write day
  u8g2.setFont(u8g2_font_timB08_tr);
  sprintf(buff, "%s", dayStr(weekday()));
  if (debug > 0)
    Serial.println(buff);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 2;
  u8g2.drawStr(xpos, ypos, buff);

  // write date
  //sprintf(buff, "%s %s %d",dayStr(weekday()),monthStr(month()),day());
  sprintf(buff, "%s %d", monthStr(month()), day());
  if (debug > 0)
    Serial.println(buff);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 1;
  u8g2.drawStr(xpos, ypos, buff);

  // set brightness
  if ((hour() >= 20) || (hour() <= 6)) {
    u8g2.setContrast(0);
  }
  else {
    u8g2.setContrast(255);
  }
  u8g2.sendBuffer();
  if (do_SyncBar) OLED_Sync_Bar();
  // if (do_RSSI) OLED_RSSI_Bars();
}
void OLED_Sync_Bar () {
  // draw sync bar
  for (int i = 0; i < syncBar + 1; i++) {
    u8g2.drawPixel(0, disphei - i);
  }
  u8g2.sendBuffer();
}

void OLED_RSSI_Bars () {
  // draw signal bars
  rssi = WiFi.RSSI();

  int bt = 0; // bar top
  int bb = bt + 5 ; // bar bottom
  int bl = 1; // bar left

  // draw black background
  u8g2.setDrawColor(0);
  // bars occupy a box 7 x 5 pixels
  u8g2.drawBox(bl, bt, 9,  7);
  u8g2.setDrawColor(1);

  // draw smallest possible signal strength bars
  if (rssi > -89) u8g2.drawLine(bl + 1, bb, bl + 1, bb - 1);
  if (rssi > -78) u8g2.drawLine(bl + 3, bb, bl + 3, bb - 2);
  if (rssi > -67) u8g2.drawLine(bl + 5, bb, bl + 5, bb - 3);
  if (rssi > -56) u8g2.drawLine(bl + 7, bb, bl + 7, bb - 4);

  u8g2.sendBuffer();
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

constexpr uint8_t  NTP_UNIX_OFFSET_YEARS = 70;
constexpr uint16_t DAYS_IN_YEAR          = 365;
constexpr uint8_t  NUMBER_OF_LEAP_YEARS  = 17;
constexpr uint32_t SECONDS_IN_DAY        = 86400;
constexpr uint32_t NTP_UNIX_OFFSET_SECONDS =
  (NTP_UNIX_OFFSET_YEARS * DAYS_IN_YEAR + NUMBER_OF_LEAP_YEARS) * SECONDS_IN_DAY;


time_t getNtpTime() {
  char buff[64];
  Serial.print("epoch offset = ");
  Serial.println(NTP_UNIX_OFFSET_SECONDS );

  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
  u8g2.drawBox(0, 0, 2, 2);
  u8g2.sendBuffer();
  //digitalWrite(LED_BUILTIN, LOW); // on
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  Serial.print("SetTimeZone = ");
  Serial.println(SetTimeZone);
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      digitalWrite(LED_BUILTIN, HIGH); // off
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      LastSyncTime = millis();

      // read packet
      uint32_t words[12];
      for (int i = 0; i < 12; i++) {
        words[i] = getWord(packetBuffer, i * 4);
      }

      // print raw time
      Serial.println("\nraw 32-bit packet elements");
      Serial.println("          decimal   hex");
      Serial.println("-------------------");
      for (int i = 0; i < 12; i++) {
        sprintf(buff, "i = %2d %010u %08X ", i, words[i], words[i]);
        Serial.print(buff);
        print_binary(words[i],32);
        Serial.println();
      }

      Serial.print("header: ");
      print_uint32(words[0]);
      Serial.print("header: "); print_binary(words[0], 32);
      Serial.println();
      // define variables
      uint32_t  LI = getBits32(words[0], 0, 2);
      Serial.print("  LI: ");
      print_uint8(LI);
      Serial.println("should be dec 0-3 or bin 00-11");
      uint32_t  VN = getBits32(words[0], 2, 3);
      Serial.print("  VN: ");
      print_uint8(VN);
      Serial.println("should be dec 4 or bin 100");
      uint32_t  Mode = getBits32(words[0], 5, 3);
      Serial.print("Mode: ");
      print_uint8(Mode);

      uint32_t  Stratum = getBits32(words[0], 8, 8);
      Serial.print("Stra: ");
      print_uint8(Stratum);
      Serial.println("should be dec 0-16, hex 0-F");

      uint32_t  Poll = getBits32(words[0], 16, 8);
      Serial.print("Poll: ");
      print_uint8(Poll);
      Serial.println("8-bit signed int");

      uint32_t  Precision = getBits32(words[0], 24, 8);
      Serial.print("Prec: ");
      print_uint8(Stratum);
      Serial.println("8-bit signed int");



      uint32_t  rootDelay = words[1];
      uint32_t  rootDispersion = words[2];
      uint32_t  referenceIdentifier = words[3];

      sprintf(buff, "%010u Root Delay\n", rootDelay);
      Serial.print(buff);
      sprintf(buff, "%010u Root Dispersion\n", rootDispersion);
      Serial.print(buff);
      sprintf(buff, "%010u Reference Identifier\n", referenceIdentifier);
      Serial.print(buff);


      const char* names[4] = {"reference", "originate", "receive", "transmit"};

      // print raw NTP time
      Serial.println("\nraw 64-bit timestamps");
      for (int i = 0; i < 4; i++) {
        sprintf(buff, "i = %1d %010u %010u %s\n", i + 1, words[4 + i * 2], words[5 + i * 2], names[i]);
        Serial.print(buff);
      }

      // print raw NTP time
      uint32_t secsSince1900[4];
      Serial.println("\nraw 32-bit timestamps (seconds)");
      for (int i = 0; i < 4; i++) {
        secsSince1900[i] = words[4 + i * 2];
        sprintf(buff, "i = %1d %010u %s\n", i + 1, secsSince1900[i], names[i]);
        Serial.print(buff);
      }

      uint32_t fracSecs[4];
      // print raw NTP time
      Serial.println("\nraw 32-bit timestamps (fraction)");
      for (int i = 0; i < 4; i++) {
        fracSecs[i] = words[5 + i * 2];
        sprintf(buff, "i = %1d %010u %s\n", i + 1, fracSecs[i], names[i]);
        Serial.print(buff);
      }

      // convert to unix time
      uint32_t secsSince1970[4];
      for (int i = 0; i < 4; i++) {
        secsSince1970[i] = secsSince1900[i] - NTP_UNIX_OFFSET_SECONDS;
      }

      // print unix time
      Serial.println("\n32-bit unix timestamps");
      for (int i = 0; i < 4; i++) {
        sprintf(buff, "i = %2d %010u %s\n", i, secsSince1970[i], names[i]);
        Serial.print(buff);
      }

      // convert to local time
      uint32_t localTime[4];
      for (int i = 0; i < 4; i++) {
        localTime[i] = secsSince1970[i] + SetTimeZone * SECS_PER_HOUR;
      }

      // print unix time
      Serial.println("\nlocal 32-bit timestamps");
      for (int i = 0; i < 4; i++) {
        sprintf(buff, "i = %2d %010u %s\n", i, localTime[i], names[i]);
        Serial.print(buff);
      }

      // convert the fractional part to milliseconds
      uint32_t ifrac_secs[4];
      double frac_secs[4];
      for (int i = 0; i < 4; i++) {
        ifrac_secs[i] = words[5 + i * 2];
      }

      fracTime = ((uint64_t) fracSecs[3] * 1000) >> 32;

      //      if (debug > 1) {
      // print fractional times
      Serial.println("\nfractional times");
      Serial.print("fracTime = ");
      Serial.println(fracTime);
      //    }

      Serial.println(serdiv);
      return localTime[3];
    }
  }
  Serial.println("No NTP Response :-(");
  Serial.println(serdiv);
  digitalWrite(LED_BUILTIN, HIGH); // off
  return 0; // return 0 if unable to get the time
}

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
