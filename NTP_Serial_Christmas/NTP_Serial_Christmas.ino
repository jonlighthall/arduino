/*
   TimeNTP_ESP8266WiFi.ino
   Example showing time sync to NTP time source

   This sketch uses the ESP8266WiFi library
*/

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "dst.h"

// Wi-Fi settings:
#include "credentials.h"
const char* ssid = mySSID;          //from credentials.h file
const char* pass = myPASSWORD;      //from credentials.h file

// NTP Servers:
static const char ntpServerName[] = "time.nist.gov";

// Set Standard time zone
const int timeZone = -6; // CST

int SetTimeZone = timeZone;
const bool do_DST = true;

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

int rssi = 0; // Wifi signal strengh variable

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
void serialClockDisplay();

const bool do_Christmas = true;
int isLeapYear (int input_year, int default_debugLY = 0); // set default function value
void serialChristmas();

#define PRINT_DELAY 250 // print delay in milliseconds
#define SYNC_DELAY 300 // print delay in seconds

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
  delay(PRINT_DELAY);

  // Wi-Fi settings
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
    Serial.print(".");
  }
  Serial.print("connected\n");
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
  setSyncProvider(getNtpTime);

  // wait for time to be set
  if (timeStatus() == timeNotSet)
    setSyncInterval(0);
  while (timeStatus() == timeNotSet) {
    ;
  }
  setSyncInterval(1);
  Serial.println("sync complete");

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
  Serial.println(rssi);

  if (do_Christmas) serialChristmas();
}


int isLeapYear(int in_year, int debugLY) {
  char buff[128];
  sprintf(buff, "%d input\n", in_year);
  Serial.print(buff);
  sprintf(buff, "leap year debug = %d\n", debugLY);
  Serial.print(buff);
}

void serialChristmas() {
  // check month
  char buff[128];
  int dmonth = 12 - month();
  int dday = 0;
  sprintf(buff, "month = %d, dmonth = %d\n", month(), dmonth);
  Serial.print(buff);
  if (dmonth > 0) {
    sprintf(buff, "must do more programming\n");
    Serial.print(buff);
    sprintf(buff, "leap year = %d\n", isLeapYear(year()));
    Serial.print(buff);
  }
  else {
 dday = 25 - day();
 sprintf(buff, "day = %d, dday = %d\n", day(), dday);
  Serial.print(buff); 
  }

  int dhour = 24 - hour();
 sprintf(buff, "hour = %d, dhour = %d\n", hour(), dhour);
  Serial.print(buff); 
  

}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
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
