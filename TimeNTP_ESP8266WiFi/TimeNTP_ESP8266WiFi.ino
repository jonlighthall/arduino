/*
   TimeNTP_ESP8266WiFi.ino
   Example showing time sync to NTP time source

   This sketch uses the ESP8266WiFi library
*/

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "credentials.h"
#include "dst.h"

const char* ssid = mySSID;          //from credentials.h file
const char* pass = myPASSWORD;      //from credentials.h file

// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

// Set Standard time zone
//const int timeZone = 0;  // GMT
//const int timeZone = -5; // EST
const int timeZone = -6; // CST
//const int timeZone = -7; // MST
//const int timeZone = -8; // PST

int SetTimeZone = timeZone;
const bool do_DST = true;

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void serialClockDisplay();
void OLEDClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
int dispwid;
int disphei;

#define DELAY 250

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  //delay(DELAY);
  //testDST();
  delay(DELAY);
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
  
  u8g2.drawStr(xpos,ypos,buff);        	// write something to the internal memory
  u8g2.sendBuffer();			// transfer internal memory to the display
  delay(DELAY);
  
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d",dispwid,disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += texthei+1;
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();      
  delay(DELAY);
 
  // Wi-Fi settings
  Serial.print("Connecting to ");
  Serial.print(ssid);
  sprintf(buff, "Wi-Fi...");
  xpos = 0;
  ypos += texthei+2;
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();      
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("connected\n");
  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();      
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  sprintf(buff, "NTP sync...");
  xpos = 0;
  ypos += texthei+1;
  u8g2.drawStr(xpos,ypos,buff);
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
  u8g2.drawStr(xpos,ypos,buff);
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
  
  sprintf(buff, "Timezone = %d",SetTimeZone);
  xpos = dispwid - u8g2.getStrWidth(buff);
  ypos += texthei+2;
  u8g2.drawStr(xpos,ypos,buff);
  u8g2.sendBuffer();      
  
  setSyncInterval(10); // refresh rate in seconds
  Serial.println("starting...");
}

time_t prevDisplay = 0; // when the digital clock was displayed

uint32_t bufferTime;
uint32_t fracTime;
char serdiv[] = "----------------------------"; // serial print divider
int debug = 0;
void loop() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      char buff[50];

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
      int delayTime = printTime - bufferTime;
      if (debug > 1) {
        Serial.print("buffer time = ");
        Serial.println(bufferTime);
        Serial.print(" print time = ");
        Serial.println(printTime);
        Serial.print(" delay time = ");
        Serial.println(delayTime);
        sprintf(buff, "Time since last sync = %.3fs\n", delayTime / 1e3);
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
      if (debug > 1) {
        Serial.print("end of loop, after deiplay: millis = ");
        Serial.println(millis());
      }
    }
  }
}

void serialClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  if (isAM() == 1) {
    Serial.print("am");
  }
  else {
    Serial.print("pm");
  }
  Serial.print(" am:");
  Serial.print(isAM());
  Serial.print(" pm:");
  Serial.print(isPM());
  Serial.print(" weekday:");
  Serial.print(weekday());
  Serial.print(" daystr:");
  Serial.print(dayStr(weekday()));
  Serial.print(dayShortStr(weekday()));
  Serial.print(monthStr(month()));
  Serial.print(monthShortStr(month()));
  isDST(1);
  Serial.println();
}

void printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void OLEDClockDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont22_tn);
  char buff[dispwid];
  sprintf(buff, "%02d:%02d", hour(), minute());
  Serial.println(buff);
  int xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  int ypos = u8g2.getAscent();
  u8g2.drawStr(xpos, ypos, buff);

  // draw seconds bar
  ypos+=2;
  for (int i=0;i<second()+1;i++) {
    u8g2.drawPixel(i+3,ypos);
  }
  ypos+=1;
  for (int i=0;i<second()+1;i=i+5) {
    u8g2.drawPixel(i+3,ypos);
  }
  ypos+=1;
  for (int i=0;i<second()+1;i=i+15) {
    u8g2.drawPixel(i+3,ypos);
  }

  // write seconds
  u8g2.setFont(u8g2_font_profont15_tn);
  sprintf(buff, "%02d:%02d:%02d", hour(), minute(),second());
  Serial.println(buff);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 2;
  u8g2.drawStr(xpos, ypos, buff);
  
  // write day
  u8g2.setFont(u8g2_font_timB08_tr);
  sprintf(buff, "%s",dayStr(weekday()));
  Serial.println(buff);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 2;
  u8g2.drawStr(xpos, ypos, buff);
  
  // write date
  //sprintf(buff, "%s %s %d",dayStr(weekday()),monthStr(month()),day());
  sprintf(buff, "%s %d",monthStr(month()),day());
  Serial.println(buff);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 1;
  u8g2.drawStr(xpos, ypos, buff);
  
  u8g2.sendBuffer();
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
  u8g2.drawBox(0,0,2,2);
  u8g2.sendBuffer();
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
      u8g2.drawBox(0,dispwid-2,2,2);
      u8g2.sendBuffer();
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
