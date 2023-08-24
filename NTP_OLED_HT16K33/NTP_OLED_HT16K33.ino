/*
  TimeNTP_ESP8266WiFi.ino
  Example showing time sync to NTP time source
*/

// standard library headers
#include <TimeLib.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// custom library headers
#include <wifi_utils.h>
#include <dst.h>
#include <oled_utils.h>

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70

// Create display object.  This is a global variable that
// can be accessed from both the setup and loop function below.
Adafruit_7segment matrix = Adafruit_7segment();

// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";

// Set Standard time zone
//const int timeZone = 0;  // GMT
//const int timeZone = -5; // EST
const int timeZone = -6; // CST
//const int timeZone = -7; // MST
//const int timeZone = -8; // PST

int SetTimeZone = timeZone;
const bool do_DST = true;

// LED display options
bool do_mil = false;
bool do_sec_top = false;
bool do_cyc = false;
bool do_sec_mod = true;

time_t getNtpTime();
void serialClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

#define PRINT_DELAY 250 // print delay in milliseconds
#define SYNC_DELAY 300 // print delay in seconds

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize LED display
  matrix.begin(DISPLAY_ADDRESS);
  matrix.clear();
  matrix.setBrightness(15);
  //display.setSegments(SEG_hEllo);
  matrix.writeDigitRaw(0, 0b01110100);//h
  matrix.writeDigitRaw(1, 0b01111001);//E
  matrix.writeDigitRaw(3, 0b00110110);//ll
  matrix.writeDigitRaw(4, 0b01011100);//o
  //  matrix.print("HELO");
  matrix.writeDisplay();
  Serial.begin(9600);
  while (!Serial) { }
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
  //display.setSegments(SEG_CONN);
  matrix.clear();
  matrix.writeDigitRaw(0, 0b01011000);//c
  matrix.writeDigitRaw(1, 0b01011100);//o
  matrix.writeDigitRaw(3, 0b01010100);//n
  matrix.writeDigitRaw(4, 0b01010100);//n
  matrix.writeDisplay();

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
    sprintf(buff, "X  ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();

    delay(500);
    Serial.print(".");
    sprintf(buff, "O   ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();
  }
  Serial.print("connected\n");
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
  //display.setSegments(SEG_SYNC);
  matrix.clear();
  matrix.writeDigitRaw(0, 0b01101101);//S
  matrix.writeDigitRaw(1, 0b01101110);//y
  matrix.writeDigitRaw(3, 0b01010100);//n
  matrix.writeDigitRaw(4, 0b01011000);//c
  matrix.writeDisplay();

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
      delay(1001);
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

  setSyncInterval(SYNC_DELAY); // refresh rate in seconds
  Serial.println("starting...");
}

time_t prevDisplay = 0; // when the digital clock was displayed

uint32_t bufferTime;
uint32_t fracTime;
char serdiv[] = "----------------------------"; // serial print divider
int debug = 0;
int syncTime = SYNC_DELAY * 1e3;
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
      int delayTime = printTime - bufferTime;
      int ToSyncTime = syncTime - delayTime;
      float syncWait = (float)delayTime / syncTime;
      syncBar = syncWait * disphei;
      if (debug > 1) {
        Serial.print("buffer time = ");
        Serial.println(bufferTime);
        Serial.print(" print time = ");
        Serial.println(printTime);
        Serial.print(" delay time = ");
        Serial.println(delayTime);
        // print time since/until last/next sync
        sprintf(buff, "Time since last sync = %.3fs\n", delayTime / 1e3);
        Serial.print(buff);
        sprintf(buff, "Time since last sync = %dms\n", delayTime);
        Serial.print(buff);
        sprintf(buff, "Time since last sync = %.3fs\n", delayTime / 1e3);
        Serial.print(buff);
        // print time between syns percentage
        sprintf(buff, "Time between syncs = %dms\n", syncTime);
        Serial.print(buff);
        sprintf(buff, "Time between syncs = %.3fs\n", syncTime / 1e3);
        Serial.print(buff);

        sprintf(buff, "Time until next sync = %dms\n", ToSyncTime);
        Serial.print(buff);
        sprintf(buff, "Time until next sync = %.3fs\n", ToSyncTime / 1e3);
        Serial.print(buff);

        sprintf(buff, "Sync delay percentage = %.3f%%\n", syncWait * 100);
        Serial.print(buff);

        sprintf(buff, "Sync delay percentage = %d\n", syncBar);
        Serial.print(buff);
      }

      // wait until top of second to print time
      if ((delayTime < 1000) && (delayTime > 0)) {
        int totalDelay = fracTime + delayTime;
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
          sprintf(buff, "delaying display by %d...\n", offsetTime);
          Serial.print(buff);
          Serial.println(serdiv);
        }
        delay(offsetTime);
      }
      else {
        if (debug > 1) {
          int delayError = delayTime % 1000;
          Serial.println(serdiv);
          sprintf(buff, "delay error = %d\n", delayError);
          Serial.print(buff);
          Serial.println(serdiv);
        }
      }
      serialClockDisplay();
      OLEDClockDisplay();
      DigitalClockDisplayOpt();
      //OLEDBarDisplay();

      if (debug > 1) {
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
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont22_tn);
  char buff[dispwid];
  sprintf(buff, "%02d:%02d", hour(), minute());
  if (debug > 0)
    Serial.println(buff);
  int xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  int ypos = u8g2.getAscent();
  u8g2.drawStr(xpos, ypos, buff);

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

    // write seconds
    u8g2.setFont(u8g2_font_profont15_tn);
    sprintf(buff, "%02d:%02d:%02d", hour(), minute(), second());
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
  if (do_RSSI) OLED_RSSI_Bars();
}

void DigitalClockDisplay() {
  int dig_time ;

  if (do_mil) {
    dig_time = (hour() * 100) + minute();
    matrix.print(dig_time, DEC);
  }
  else {
    dig_time = (hourFormat12() * 100) + minute();
    matrix.print(dig_time, DEC);
  }
  matrix.drawColon(true);
  matrix.writeDisplay();

  if (debug > 0) {
    char buff[dispwid];
    sprintf(buff, "digital time: %d", dig_time);
    Serial.println(buff);
  }
}

void DigitalClockDisplayOpt() {
  // set brightness
  if ((hour() >= 20) || (hour() <= 6)) { // night
    // dim display and show time only
    matrix.setBrightness(0);
    DigitalClockDisplay();
  }
  else { // day
    // brighten display and show options
    matrix.setBrightness(15);
    int dig_time;
    if ((do_cyc) && (second() == 30)) { // nixie tube cycling
      for (int i = 0; i < 10; i++) {
        dig_time = 1111 * i;
        matrix.print(dig_time, DEC);
        matrix.writeDisplay();
        delay(PRINT_DELAY);
      }
      DigitalClockDisplay();
    }
    else if (do_sec_top && ((second() >= 57) || (second() <= 2))) { // show seconds at the top of the minute
      dig_time = (minute() * 100) + second();
      matrix.print(dig_time, DEC);
      matrix.writeDisplay();
    }
    else {
      if (do_sec_mod && ((second() % 15) == 0) && (second() > 0)) { // flash seconds periodically
        dig_time = second();
        matrix.clear();
        matrix.print(dig_time, DEC);
        matrix.writeDisplay();
        delay(PRINT_DELAY);
      }
      DigitalClockDisplay();
    }
  }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
  u8g2.drawBox(0, 0, 2, 2);
  u8g2.sendBuffer();
  //digitalWrite(LED_BUILTIN, LOW); // on
  //  display.setSegments(SEG_SYNC);
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
      bufferTime = millis();
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      // convert four bytes starting at location 44 to a long integer
      unsigned long frac;
      frac  = (unsigned long) packetBuffer[44] << 24;
      frac |= (unsigned long) packetBuffer[45] << 16;
      frac |= (unsigned long) packetBuffer[46] <<  8;
      frac |= (unsigned long) packetBuffer[47];

      // convert the fractional part to milliseconds
      fracTime = ((uint64_t) frac * 1000) >> 32;
      if (debug > 1) {
        Serial.print("fracTime = ");
        Serial.println(fracTime);
      }
      Serial.println(serdiv);
      return secsSince1900 - 2208988800UL + SetTimeZone * SECS_PER_HOUR;
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
