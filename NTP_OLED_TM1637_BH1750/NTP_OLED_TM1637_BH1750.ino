/*
  TimeNTP_ESP8266WiFi.ino
  Example showing time sync to NTP time source
*/

/*

  Wemos D1 Mini Pin Connections

  +-----+--------+-------+----------------+
  | Pin | ESP    | Use   |
  +-----+--------+-------+----------------+
  | RST | RST    | Reset |
  +-----+--------+-------+----------------+
  | A0  | A0     | ADC   |
  +-----+--------+-------+----------------+
  | D0  | GPIO16 | WAKE  |
  +-----+--------+-------+----------------+
  | D5  | GPIO14 | SCLK  | LED DIO (display if LED1 defined)
  +-----+--------+-------+----------------+
  | D6  | GPIO12 | MISO  | LED CLK (display if LED1 defined)
  +-----+--------+-------+----------------+
  | D7  | GPIO13 | MOSI  |
  +-----+--------+-------+----------------+
  | D8  | GPIO15 | CS    |
  +-----+--------+-------+----------------+
  | 3V3 | 3.3V   |       |
  +-----+--------+-------+----------------+
  | TX  | GPIO1  | TX    |
  +-----+--------+-------+----------------+
  | RX  | GPIO3  | RX    |
  +-----+--------+-------+----------------+
  | D1  | GPIO5  | SCL   |
  +-----+--------+-------+----------------+
  | D2  | GPIO4  | SDA   |
  +-----+--------+-------+----------------+
  | D3  | GPIO0  | FLASH |
  +-----+--------+-------+----------------+
  | D4  | GPIO2  | LED   | sync cue
  +-----+--------+-------+----------------+
  | G   | GND    | GND   |
  +-----+--------+-------+----------------+
  | 5V  | N/A    | VCC   |
  +-----+--------+-------+----------------+

*/

// custom library headers
#include <TimeLib.h>

// LED display options
// Possible options are:
//   LED1 - enable clock-like time display on 4-bit 7-segment LED
//   LED2 - enable seconds and fractional seconds on second LED
//   LDR  - use LDR to adjust brightness level
//   WEMOS_V4 - use I2C port (requires LED1, conflicts with LED2)
//   CLOCK_BUTTONS - use the buttons on the clock to set options

#define LED1
//#define LDR
//#define LED2

// project library headers
// project library headers
#include "debug.h"
#include "dst.h"
#include "led_utils.h"
#include "ntp_utils.h"
#include "oled_utils.h"
#include "serial_utils.h"
#include "wifi_utils.h"

// desk light
#include <BH1750.h>
BH1750 lightMeter(0x23);
const int relayPin = D0;
int relayState;
int tlast = -1;
#define READ_DELAY 1000 * 5
const int lux_low_thresh = 30;
const int lux_high_thresh = 100;

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize Serial
  serial_init();
  Serial.println("---------------");
  char buff[64];
  sprintf(buff, "TimeNTP Example");
  Serial.println(buff);
  Serial.println("---------------");

  OLED_init();
  LED_init();

  // initialize I2C bus
  // the BH1750 library doesn't do this automatically
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  Wire.begin();

  // initialize BH1750
  // begin() returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  // initialize relay
  pinMode(relayPin, OUTPUT);
  relayState = LOW;
  Serial.println("Initalizing: turning light OFF...");
  digitalWrite(relayPin, relayState);

  // pause for readability
  delay(PRINT_DELAY);

  /* print connecting message */
  // Serial
  Serial.print("Connecting to ");
  Serial.print(ssid);
#ifdef OLED
  // OLED
  sprintf(buff, "Wi-Fi...");
  xpos = 0;
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  xpos += u8g2.getStrWidth(buff) + 1;
#endif
  // LED
  LED_conn();

  WiFi.begin(ssid, pass);

  // print text throbber
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
#ifdef OLED
    sprintf(buff, "X  ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();
#endif

    delay(500);

    Serial.print(".");
#ifdef OLED
    // draw black background
    u8g2.setDrawColor(0);
    u8g2.drawBox(xpos, ypos - texthei, 9, texthei);
    u8g2.setDrawColor(1);
    sprintf(buff, "O   ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();
#endif
  }

  /* print connected message */
  // Serial
  Serial.print("connected\n");
  // OLED
#ifdef OLED
  u8g2.setDrawColor(0);
  u8g2.drawBox(xpos, ypos - texthei, 9, texthei);
  u8g2.setDrawColor(1);
  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
#endif

  /* print Wi-Fi connection status */
  rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());

  udp_start();

  /* print sync message */
  // Serial
  Serial.println("waiting for sync...");
  // OLED
#ifdef OLED
  sprintf(buff, "NTP sync...");
  xpos = 0;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
#endif
  // LED
  LED_sync();

  // wait for time to be set
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeNotSet)
    setSyncInterval(0);
  while (timeStatus() == timeNotSet) {
    Serial.print(".");
  }
  setSyncInterval(1);

  /* print sync complete message */
  // Serial
  Serial.println("sync complete");
  // OLED
#ifdef OLED
  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
#endif

  if (do_DST) {
    serialClockDisplay();
    Serial.print("checking DST status... ");
    SetTimeZone = timeZone + isDST(1);
    Serial.println();
    if (isDST() > 0) {
      Serial.println("refreshing time...");
      delay(1001);  // why wait?
      serialClockDisplay();
    }
  } else {
    SetTimeZone = timeZone;
  }

  OLED_TZ();

  setSyncInterval(SYNC_INTERVAL);  // refresh rate in seconds
  Serial.println("done with setup");
  Serial.println("starting loop...");
}

void loop() {
  char buff[64];
  // ---------------------------------
  // Light controls
  // ---------------------------------
  int tstart = millis();
  if ((hour() > 23) || ((hour() == 22) && (minute() >= 30)) || (hour() < 6) ||
      ((hour() == 5) && (minute() <= 30))) {
    // override light controls at night
    relayState = LOW;
    Serial.println(" Override: turning light OFF...");
    digitalWrite(relayPin, relayState);
  } else {
    if (((tstart - tlast) >= READ_DELAY) || (tlast == -1)) {
      uint16_t lux = lightMeter.readLightLevel();
      tlast = millis();
      sprintf(buff, "Light: %d lx\n", lux);
      Serial.print(buff);

      if (lux < lux_low_thresh) {
        relayState = HIGH;
        sprintf(buff, " It is too dark: %d < %d\n", lux, lux_low_thresh);
        Serial.print(buff);
        Serial.println(" Turning light ON...");
        digitalWrite(relayPin, relayState);
      } else if (lux > lux_high_thresh) {
        relayState = LOW;
        sprintf(buff, " It is too bright: %d > %d\n", lux, lux_high_thresh);
        Serial.println(" Turning light OFF...");
        digitalWrite(relayPin, relayState);
      }
    } else {
      // do nothing and continue
      //    sprintf(buff, "waiting : %d %d %d\n", tstart, tlast, (tstart -
      //    tlast)); Serial.print(buff);
    }
  }
  // ---------------------------------
  // Time
  // ---------------------------------
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {  // update the display only if time has changed
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
      // save previous display time
      prevDisplay = now();

      // check DST
      if (do_DST) {
        if (debug > 0)
          Serial.print("   checking DST status... ");
        SetTimeZone = timeZone + isDST(debug);
        if (debug > 0)
          Serial.println();
      } else {
        SetTimeZone = timeZone;
      }
      if (do_milliseconds) {
        // check time in milliseconds
        uint32_t printTime = millis();
        // calculate time since/until last/next sync
        int TimeSinceSync = printTime - LastSyncTime;
        int ToSyncTime = syncInterval - TimeSinceSync;
        float syncWait = (float)TimeSinceSync / syncInterval;
        if (debug > 1) {
          Serial.print("last sync time = ");
          Serial.println(LastSyncTime);
          Serial.print("      time now = ");
          Serial.println(printTime);
          Serial.print("  elapsed time = ");
          Serial.println(TimeSinceSync);
          // print time since/until last/next sync
          sprintf(buff, " Time since last sync = %6d ms or %7.3f s\n",
                  TimeSinceSync, TimeSinceSync / 1e3);
          Serial.print(buff);
          // print time between syncs percentage
          sprintf(buff, "   Time between syncs = %6d ms or %7.3f s\n",
                  syncInterval, syncInterval / 1e3);
          Serial.print(buff);
          sprintf(buff, " Time until next sync = %6d ms or %7.3f s\n",
                  ToSyncTime, ToSyncTime / 1e3);
          Serial.print(buff);
          sprintf(buff, "Sync delay percentage = %7.3f%%", syncWait * 100);
          Serial.print(buff);
          // define OLED sync bar
          syncBar = syncWait * disphei;
          sprintf(buff, " or %2d pixels", syncBar);
          Serial.println(buff);
        }

        // wait until top of second to print time
        if (debug > 0) {
          sprintf(buff, "   NTPfracTime = %d\n", NTPfracTime);
          Serial.print(buff);
        }

        // wait until top of second to print time
        if ((TimeSinceSync < 1000) && (TimeSinceSync > 0)) {
          int totalDelay = NTPfracTime + TimeSinceSync;
          int setDelay = totalDelay % 1000;
          int offsetTime = 1000 - setDelay;
          if (debug > 1) {
            Serial.println(serdiv);
            sprintf(buff, "total delay = %d\n", totalDelay);
            Serial.print(buff);
            sprintf(buff, "  set delay = %d\n", setDelay);
            Serial.print(buff);
            Serial.print("offset time = ");
            Serial.println(offsetTime);
            sprintf(buff, "delaying display by %d...", offsetTime);
            Serial.print(buff);
          }
#ifndef LED2
          delay(offsetTime);
#endif
          if (debug > 1) {
            Serial.println("done");
            Serial.println(serdiv);
          }
        } else {
          if (debug > 1) {
            int delayError = TimeSinceSync % 1000;
            int delayDiff = 1000 - delayError;
            Serial.println(serdiv);
            sprintf(buff, "elapsed time since last sync = %d ms\n",
                    TimeSinceSync);
            Serial.print(buff);
            sprintf(buff, "sub-second error = %d ms\n", delayError);
            Serial.print(buff);
            int delayInterval = min(10, delayDiff);
            sprintf(buff, "waiting %d ms...", delayInterval);
            Serial.print(buff);
#ifndef LED2
            delay(delayInterval);
#endif
            Serial.println("done");
            Serial.println(serdiv);
          }
        }
      }  // end do_milliseconds
     
      check_clock_buttons();
      
      //-------------------------------
      // Output updated time
      //-------------------------------

      // Display time, serial
      int beforeTime = millis();
      serialClockDisplay();
      int midTime = millis();
      // Display time, OLED
      OLEDClockDisplay();
      int afterTime = millis();
      // Display time, LED
      DigitalClockDisplayOpt();

#ifdef LED2
      // save previous display time
      prev_disp_ms = millis();  // milliseconds
#endif
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
    }  // end prevDisplay
#ifdef LED2
    else {
      // caclculate fractional seconds between displaying whole seconds
      int t_diff_ms = millis() - prev_disp_ms;
      float sec_dec = (float(t_diff_ms) / 1000.0);
      sec_frac = float(second()) + sec_dec;
      int dig_sec = sec_frac * 100;
      if (debug > 1) {
        Serial.print("t diff ms = ");
        Serial.print(t_diff_ms);
        Serial.print(", dig sec = ");
        Serial.println(sec_frac);
      }
      display2.showNumberDecEx(dig_sec, 0b01000000, true);
    }
#endif
  }  // end timeNotSet
}  // end loop

/*-------- NTP code ----------*/

time_t getNtpTime() {

  while (Udp.parsePacket() > 0)
    ;  // discard any previously received packets
  serial_sync();
  OLED_sync();
  LED_sync();

  // send packet
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  const uint32_t udp_timeout = 1500;
  uint32_t udp_time;

  // wait for response
  while (millis() - beginWait < udp_timeout) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      // print status
      Serial.print("Received NTP response after ");
      udp_time = millis() - beginWait;
      Serial.print(udp_time);
      Serial.println(" ms");

      // read packet
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      LastSyncTime = millis();
      readNTP_packet();

      // parse packet
      parseNTP_time(packetWords);
      parseNTP_header(packetWords);

      Serial.println(serdiv);

      // return time
      return NTPlocalTime;
    }
  }
  Serial.println("No NTP response after ");
  Serial.print(udp_timeout);
  Serial.println(" ms :-(");
  Serial.println(serdiv);
  return 0;  // return 0 if unable to get the time
}
