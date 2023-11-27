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
  | 3V3 | 3.3V   |       | OLED VCC
  +-----+--------+-------+----------------+
  | TX  | GPIO1  | TX    |
  +-----+--------+-------+----------------+
  | RX  | GPIO3  | RX    |
  +-----+--------+-------+----------------+
  | D1  | GPIO5  | SCL   | OLED SCL
  +-----+--------+-------+----------------+
  | D2  | GPIO4  | SDA   | OLED SDA
  +-----+--------+-------+----------------+
  | D3  | GPIO0  | FLASH | 
  +-----+--------+-------+----------------+
  | D4  | GPIO2  | LED   | sync cue
  +-----+--------+-------+----------------+
  | G   | GND    | GND   | OLED GND
  +-----+--------+-------+----------------+
  | 5V  | N/A    | VCC   | 
  +-----+--------+-------+----------------+

*/

//-------------------------------
const int debug = 0;
//-------------------------------

// custom library headers
#include <TimeLib.h>

// project library headers
#include <wifi_utils.h>
#include <dst.h>
#include <oled_utils.h>
#include <ntp_utils.h>

// Serial display settings
void serialClockDisplay();
#define PRINT_DELAY 250 // print delay in milliseconds
const bool do_milliseconds = true;

// Christmas countdown options
const bool do_Christmas = true;
tmElements_t xmas_elem;  // time elements structure
time_t xmas_time[3]; // a timestamp
int diff_DAYS = 0;

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize Serial
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
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
  u8g2.clearBuffer(); // clear the internal memory

  // get OLED display dimensions
  dispwid = u8g2.getDisplayWidth();
  disphei = u8g2.getDisplayHeight();

  // print OLED welcome message
  u8g2.setFont(u8g2_font_timB08_tr); // choose a suitable font
  sprintf(buff, "NTP Time");

  // get OLED text dimensions
  int textwid = u8g2.getStrWidth(buff);
  int texthei = u8g2.getAscent();

  // set OLED text position
  int xpos = (dispwid - textwid) / 2;
  int ypos = texthei;
  u8g2.drawStr(xpos, ypos, buff); // write something to the internal memory
  u8g2.sendBuffer(); // transfer internal memory to the display

  // print OLED display dimensions
  sprintf(buff, "display dimensions are %d x %d", dispwid, disphei);
  Serial.println(buff);
  sprintf(buff, "disp is %d x %d", dispwid, disphei);
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += texthei + 1;
  u8g2.drawStr(xpos, ypos, buff);
  u8g2.sendBuffer();

  // pause for readability
  delay(PRINT_DELAY);

  /* print connecting message */
  // Serial
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // OLED connect message
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

  /* print connected message */
  // Serial
  Serial.print("connected\n");
  // OLED
  u8g2.setDrawColor(0);
  u8g2.drawBox(xpos, ypos - texthei, 9,  texthei);
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
      delay(1001); // why wait?
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

  if (do_Christmas) {
    // define Christmas Time
    // convert a date and time into unix time, offset 1970
    xmas_elem.Second = 0;
    xmas_elem.Hour = 0;
    xmas_elem.Minute = 0;
    xmas_elem.Day = 25;
    xmas_elem.Month = 12;
  }

  setSyncInterval(SYNC_INTERVAL); // refresh rate in seconds
  Serial.println("done with setup");
  Serial.println("starting loop...");
}

char serdiv[] = "----------------------------"; // serial print divider

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
	// define OLED sync bar
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
        if (debug > 0) {
          sprintf(buff, "   NTPfracTime = %d\n", NTPfracTime);
          Serial.print(buff);
        }

        if ((TimeSinceSync < 1000) && (TimeSinceSync > 0)) {
          int totalDelay = NTPfracTime + TimeSinceSync;
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
      }
      // Display time, serial
      int beforeTime = millis();
      serialClockDisplay();
      if (do_Christmas)
	calcChristmas();
      int midTime = millis();
      // Display time, OLED
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
    } // end prevDisplay
  } // end timeNotSet
} // end loop

void serialClockDisplay() {
  // send date/time to Serial Monitor
  char buff[128];
  // print time
  sprintf(buff, "%02d:%02d:%02d ", hour(), minute(), second());
  Serial.print(buff);
  // print numeric date
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

  if (do_RSSI) {
    // print signal strength
    rssi = WiFi.RSSI();
    Serial.print(" RSSI: ");
    Serial.print(rssi);
  }

  if (do_Christmas)
    calcChristmas();
  
  Serial.println();
}

int dday;
int xmasDay = 0;
int monthDays [12] = {31, 28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31};
int last_DAYS = 0;

void calcChristmas() {
  char buff[128];
  int dhour = 24 - hour();
  int dmonth = 12 - month();
  time_t time_now = now();
  time_t time_diff[3] = {0};
  int diff_sec[3];
  float diff_min[3];
  float diff_hr[3];
  float diff_day[3];

  if (debug > 0) {
    Serial.println("\nCalculating time until christmas...");
  }

  // calculate time relative to xmas last year, this year, and next year
  for (int i = 0; i < 3; i++) {

    int xmas_year = year() + i - 1;
    xmas_elem.Year = xmas_year - 1970; // years since 1970, so deduct 1970
    xmas_time[i] =  makeTime(xmas_elem);
    time_diff[i] = xmas_time[i] - time_now;

    if (debug > 0) {
      Serial.print("year is: ");
      Serial.println(xmas_year);
      Serial.print("   Christmas time is:");
      Serial.println(xmas_time[i]);
      Serial.print("Time until Christmas is :");
      Serial.println(time_diff[i]);
    }

    diff_sec[i] = (int)time_diff[i];
    diff_min[i] = (float)diff_sec[i] / 60;
    diff_hr[i] = diff_min[i] / 60;
    diff_day[i] = diff_hr[i] / 24;
  }

  // find the appropriate Christmas
  for (int i = 0; i < 3; i++) {
    if (time_diff[i] > 0) {
      last_DAYS = floor(diff_day[i - 1]);
      if (debug > 0) {
        sprintf(buff, "   Time since last Christmas: %d sec or %.1f days\n", time_diff[i - 1], diff_day[i - 1]);
        Serial.print(buff);
        Serial.print("   in days:");
        Serial.println(last_DAYS);
      }
      diff_DAYS = floor(diff_day[i]);
      
      sprintf(buff, "   Time until next Christmas: %d sec or %.1f days\n", time_diff[i], diff_day[i]);
      Serial.print(buff);
      if (debug > 0) {  
        Serial.print("   Time until next Christmas is:");
        Serial.println(time_diff[i]);
        Serial.print("   in days:");
        Serial.println(diff_day[i]);
      }
      if ((diff_DAYS - last_DAYS) < 365) {
        exit;
      }
      if (debug > 0) {
        Serial.print("   Sum seconds :");
        Serial.println(time_diff[i] - time_diff[i - 1]);
        Serial.print("   Sum days :");
        Serial.println((time_diff[i] - time_diff[i - 1]) / (24 * 60 * 60));
      }
      break;
    }
  }
}

void OLEDClockDisplay() {
  // define OLED variables
  int xpos, ypos;
  char buff[dispwid];

  u8g2.clearBuffer();
  if (do_Christmas) {
    // print days
    if (xmasDay > 0) {
      u8g2.setFont(u8g2_font_timB14_tr);
      //u8g2.setFont(u8g2_font_profont15_tn);
      switch (xmasDay) {
        case 1:
          sprintf(buff, "%dst Day", xmasDay);
          break;
        case 2:
          sprintf(buff, "%dnd Day", xmasDay);
          break;
        case 3:
          sprintf(buff, "%drd Day", xmasDay);
          break;
        default:
          sprintf(buff, "%dth Day", xmasDay);
          break;
      }

      if (xmasDay < 13) {
        if (u8g2.getStrWidth(buff) > dispwid)
          xpos = 0;
        else
          xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
        ypos = u8g2.getAscent();
        u8g2.drawStr(xpos, ypos, buff);

        u8g2.setFont(u8g2_font_timB08_tr);
        sprintf(buff, "of Xmas", dday);
        xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
        ypos += u8g2.getAscent() + 4;
        u8g2.drawStr(xpos, ypos, buff);
      }
      else {
        sprintf(buff, "King's");
        xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
        ypos = u8g2.getAscent();
        u8g2.drawStr(xpos, ypos, buff);
        sprintf(buff, "Day");
        xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
        ypos += u8g2.getAscent() + 1;
        u8g2.drawStr(xpos, ypos, buff);
      }
    }
    else {
      dday = diff_DAYS;

      u8g2.setFont(u8g2_font_timB14_tr);
      //u8g2.setFont(u8g2_font_profont15_tn);
      sprintf(buff, "%d Days", dday);
      if (u8g2.getStrWidth(buff) > dispwid)
        xpos = 0;
      else
        xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
      ypos = u8g2.getAscent();
      u8g2.drawStr(xpos, ypos, buff);

      u8g2.setFont(u8g2_font_timB08_tr);
      sprintf(buff, "Until Xmas", dday);
      xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
      ypos += u8g2.getAscent() + 4;
      u8g2.drawStr(xpos, ypos, buff);
    }
  }

  if (do_BigTime) {
    // draw OLED clock display
    u8g2.setFont(u8g2_font_profont22_tn);
    sprintf(buff, "%02d:%02d", hour(), minute());
    if (debug > 0) {
      Serial.print("   ");
      Serial.println(buff);
    }
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos += u8g2.getAscent() + 2;
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
    // set font
    u8g2.setFont(u8g2_font_profont15_tn);
    // create time buffer
    sprintf(buff, "%02d:%02d:%02d", hour(), minute(), second());
    // print time to serial
    if (debug > 0) {
      Serial.print("   ");
      Serial.println(buff);
    }
    // calculate OLED display position
    xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
    ypos += u8g2.getAscent() + 2;
    // display time
    u8g2.drawStr(xpos, ypos, buff);

    // calculate "co-time"
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

    // print "co-time"
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
  if (debug > 0) {
    Serial.print("   ");
    Serial.println(buff);
  }
  xpos = (dispwid - u8g2.getStrWidth(buff)) / 2;
  ypos += u8g2.getAscent() + 2;
  u8g2.drawStr(xpos, ypos, buff);

  // write date
  //sprintf(buff, "%s %s %d",dayStr(weekday()),monthStr(month()),day());
  sprintf(buff, "%s %d", monthStr(month()), day());
  if (debug > 0) {
    Serial.print("   ");
    Serial.println(buff);
  }
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

/*-------- NTP code ----------*/

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
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
  return 0; // return 0 if unable to get the time
}
