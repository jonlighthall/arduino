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
  | D5  | GPIO14 | SCLK  |
  +-----+--------+-------+----------------+
  | D6  | GPIO12 | MISO  |
  +-----+--------+-------+----------------+
  | D7  | GPIO13 | MOSI  |
  +-----+--------+-------+----------------+
  | D8  | GPIO15 | CS    |
  +-----+--------+-------+----------------+
  | 3V3 | 3.3V   |       | LED V_IO
  +-----+--------+-------+----------------+
  | TX  | GPIO1  | TX    |
  +-----+--------+-------+----------------+
  | RX  | GPIO3  | RX    |
  +-----+--------+-------+----------------+
  | D1  | GPIO5  | SCL   | LED matrix SCL
  +-----+--------+-------+----------------+
  | D2  | GPIO4  | SDA   | LED matrix SDA
  +-----+--------+-------+----------------+
  | D3  | GPIO0  | FLASH |
  +-----+--------+-------+----------------+
  | D4  | GPIO2  | LED   | sync cue
  +-----+--------+-------+----------------+
  | G   | GND    | GND   | LED GND
  +-----+--------+-------+----------------+
  | 5V  | N/A    | VCC   | LED +5V
  +-----+--------+-------+----------------+

*/

//-------------------------------
const int debug = 1;
//-------------------------------

// custom library headers
#include <TimeLib.h>

// project library headers
#include <dst.h>
#include <ntp_utils.h>
#include <oled_utils.h>
#include <serial_utils.h>
#include <wifi_utils.h>

// LED display options
#include <Adafruit_GFX.h>

#include "Adafruit_LEDBackpack.h"
// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS 0x70
// Create display object.  This is a global variable that
// can be accessed from both the setup and loop function below.
Adafruit_7segment matrix = Adafruit_7segment();
bool do_mil = false;
bool do_sec_top = false;
bool do_cyc = false;
bool do_sec_mod = false;

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN,
               HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize Serial
  Serial.begin(9600);
  while (!Serial)
    ;  // Needed for Leonardo only
  delay(PRINT_DELAY);
  // Serial welcome message
  Serial.println();
  Serial.println("---------------");
  char buff[64];
  sprintf(buff, "TimeNTP Example");
  Serial.println(buff);
  Serial.println("---------------");

  // initialize OLED display
  u8g2.begin();
  u8g2.clearBuffer();  // clear the internal memory

  // get OLED display dimensions
  dispwid = u8g2.getDisplayWidth();
  disphei = u8g2.getDisplayHeight();

  // print OLED welcome message
  u8g2.setFont(u8g2_font_timB08_tr);  // choose a suitable font
  sprintf(buff, "NTP Time");

  // get OLED text dimensions
  int textwid = u8g2.getStrWidth(buff);
  int texthei = u8g2.getAscent();

  // set OLED text position
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;
  u8g2.drawStr(xpos, ypos, buff);  // write something to the internal memory
  u8g2.sendBuffer();               // transfer internal memory to the display

  // print OLED display dimensions
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d", dispwid, disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  // initialize LED display
  matrix.begin(DISPLAY_ADDRESS);
  matrix.clear();
  matrix.setBrightness(15);

  // LED welcome message
  matrix.writeDigitRaw(0, 0b01110100);  // h
  matrix.writeDigitRaw(1, 0b01111001);  // E
  matrix.writeDigitRaw(3, 0b00110110);  // ll
  matrix.writeDigitRaw(4, 0b01011100);  // o
  matrix.writeDisplay();

  // pause for readability
  delay(PRINT_DELAY);

  /* print connecting message */
  // Serial
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // OLED connecting message
  sprintf(buff, "Wi-Fi...");
  xpos = 0;
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  // LED connecting message
  matrix.clear();
  matrix.writeDigitRaw(0, 0b01011000);  // c
  matrix.writeDigitRaw(1, 0b01011100);  // o
  matrix.writeDigitRaw(3, 0b01010100);  // n
  matrix.writeDigitRaw(4, 0b01010100);  // n
  matrix.writeDisplay();

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
    u8g2.drawBox(xpos, ypos - texthei, 9, texthei);
    u8g2.setDrawColor(1);

    Serial.print(".");
    sprintf(buff, "O   ");
    u8g2.drawStr(xpos, ypos, buff);
    u8g2.sendBuffer();
  }

  /* print connected message */
  // Serial
  Serial.print("connected\n");
  // OLED
  u8g2.setDrawColor(0);
  u8g2.drawBox(xpos, ypos - texthei, 9, texthei);
  u8g2.setDrawColor(1);
  sprintf(buff, "OK");
  xpos = dispwid - u8g2.getStrWidth(buff);
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

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
  // OLED sync message
  sprintf(buff, "NTP sync...");
  xpos = 0;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();
  // LED sync message
  matrix.clear();
  matrix.writeDigitRaw(0, 0b01101101);  // S
  matrix.writeDigitRaw(1, 0b01101110);  // y
  matrix.writeDigitRaw(3, 0b01010100);  // n
  matrix.writeDigitRaw(4, 0b01011000);  // c
  matrix.writeDisplay();

  // wait for time to be set
  setSyncProvider(getNtpTime);
  if (timeStatus() == timeNotSet) setSyncInterval(0);
  while (timeStatus() == timeNotSet) {
    Serial.print(".");
  }
  setSyncInterval(1);

  /* print sync complete message */
  // Serial
  Serial.println("sync complete");
  // OLED
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
      Serial.println("refreshing time...");
      delay(1001);  // why wait?
      serialClockDisplay();
    }
  } else {
    SetTimeZone = timeZone;
  }

  // OLED timezone message
  sprintf(buff, "Timezone = %d", SetTimeZone);
  xpos = dispwid - u8g2.getStrWidth(buff);
  ypos += texthei + 2;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  setSyncInterval(SYNC_INTERVAL);  // refresh rate in seconds
  Serial.println("done with setup");
  Serial.println("starting loop...");
}

char serdiv[] = "----------------------------";  // serial print divider

void loop() {
  char buff[64];
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
        if (debug > 0) Serial.print("   checking DST status... ");
        SetTimeZone = timeZone + isDST(debug);
        if (debug > 0) Serial.println();
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
          // define OLED sync bar
          syncBar = syncWait * disphei;
          Serial.print(buff);
          sprintf(buff, " or %2d pixels", syncBar);
          Serial.println(buff);
        }

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
          delay(offsetTime);
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
            // delay(delayInterval);
            Serial.println("done");
            Serial.println(serdiv);
          }
        }
      }  // end do_milliseconds

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
  }    // end timeNotSet
}  // end loop

// LED Display
void DigitalClockDisplay() {
  // Display Time
  int dig_time;

  if (do_mil) {
    // military time
    dig_time = (hour() * 100) + minute();
    matrix.print(dig_time, DEC);
  } else {
    // 12-hour time
    dig_time = (hourFormat12() * 100) + minute();
    matrix.print(dig_time, DEC);
  }
  matrix.drawColon(true);
  matrix.writeDisplay();

  if (debug > 0) {
    char buff[dispwid];
    sprintf(buff, "   digital time: %d", dig_time);
    Serial.println(buff);
  }
}

// LED Display options
void DigitalClockDisplayOpt() {
  // set brightness
  if ((hour() >= 20) || (hour() <= 6)) {  // night
    // dim display and show time only
    matrix.setBrightness(0);
    DigitalClockDisplay();
  } else {  // day
    // brighten display and show options
    matrix.setBrightness(15);
    int dig_time;
    if ((do_cyc) && (second() == 30)) {  // nixie tube cycling
      for (int i = 0; i < 10; i++) {
        dig_time = 1111 * i;
        matrix.print(dig_time, DEC);
        matrix.writeDisplay();
        delay(PRINT_DELAY);
      }
      DigitalClockDisplay();
    } else if (do_sec_top &&
               ((second() >= 57) ||
                (second() <= 2))) {  // show seconds at the top of the minute
      dig_time = (minute() * 100) + second();
      matrix.print(dig_time, DEC);
      matrix.writeDisplay();
    } else {
      if (do_sec_mod && ((second() % 15) == 0) &&
          (second() > 0)) {  // flash seconds periodically
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

time_t getNtpTime() {
  IPAddress ntpServerIP;  // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ;  // discard any previously received packets
  // Serial sync message
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  // OLED sync message (cue light)
  u8g2.drawBox(0, 0, 2, 2);
  u8g2.sendBuffer();

  // send packet
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();

  // wait for response
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      // print status
      Serial.println("Receive NTP Response");

      // read packet
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      LastSyncTime = millis();
      readNTP_packet();

      // parse packet
      parseNTP_time(packetWords);

      Serial.println(serdiv);

      // return time
      return NTPlocalTime;
    }
  }
  Serial.println("No NTP Response :-(");
  Serial.println(serdiv);
  return 0;  // return 0 if unable to get the time
}
