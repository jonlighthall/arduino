/*
  TimeNTP_ESP8266WiFi.ino
  Example showing time sync to NTP time source
*/

#include <TimeLib.h>

#include "wifi_utils.h"
#include "oled_utils.h"
#include "dst.h"

//-------------------------------
const int debug = 0;
//-------------------------------
#include "binary_utils.h"
#include "ntp_utils.h"

void serialClockDisplay();

const bool do_milliseconds = true;
const bool do_rssi = false;

const bool do_Christmas = true;
int isLeapYear (int input_year, int default_debugLY = 0); // set default function value

#define PRINT_DELAY 250 // print delay in milliseconds

tmElements_t xmas_elem;  // time elements structure
time_t xmas_time[3]; // a timestamp
int diff_DAYS = 0;

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

  // test leap year function
  for (int i = 1700; i < 2101; i = i + 50) {
    if (isLeapYear(i, 1)) {
      Serial.println("   YES");
    }
    else {
      Serial.println("   NO");
    }
  }

  for (int i = 2000; i < 2101; i = i + 1) {
    if (isLeapYear(i, 0)) {
      Serial.println(i);
    }
  }

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

  if (do_Christmas) {
    // convert a date and time into unix time, offset 1970
    xmas_elem.Second = 0;
    xmas_elem.Hour = 0;
    xmas_elem.Minute = 0;
    xmas_elem.Day = 25;
    xmas_elem.Month = 12;

    time_t time_now = now();
    time_t time_diff[3] = {0};
    int diff_sec[3];
    float diff_min[3];
    float diff_hr[3];
    float diff_day[3];

    // calculate time relative to xmas last year, this year, and next year
    Serial.println();
    for (int i = 0; i < 3; i++) {
      Serial.print("year is:");
      int xmas_year = year() + i - 1;
      Serial.println(xmas_year);
      xmas_elem.Year = xmas_year - 1970; // years since 1970, so deduct 1970
      xmas_time[i] =  makeTime(xmas_elem);
      Serial.print("   Christmas time is:");
      Serial.println(xmas_time[i]);

      //setTime(xmas_time[i]);
      //serialClockDisplay();

      time_diff[i] = xmas_time[i] - time_now;
      Serial.print("Time until Christmas is:");
      Serial.println(time_diff[i]);

      diff_sec[i] = (int)time_diff[i];
      diff_min[i] = (float)diff_sec[i] / 60;
      diff_hr[i] = diff_min[i] / 60;
      diff_day[i] = diff_hr[i] / 24;
    }



    for (int i = 0; i < 3; i++) {
      if (time_diff[i] > 0) {
        Serial.print("Time since last Christmas is:");
        Serial.println(time_diff[i - 1]);
        Serial.print("   in days:");
        Serial.println(diff_day[i - 1]);

        Serial.print("Time until next Christmas is:");
        Serial.println(time_diff[i]);
        Serial.print("   in days:");
        Serial.println(diff_day[i]);
        diff_DAYS = floor(diff_day[i]);
        break;
      }
    }
  }
}

time_t prevDisplay = 0; // when the digital clock was displayed
uint32_t LastSyncTime;
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
          Serial.print("checking DST status... ");
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
          sprintf(buff, "NTPfracTime = %d\n", NTPfracTime);
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
  } // end timeNotSet
} // end loop

void serialClockDisplay() {
  // digital clock display of the time
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

void calcChristmas() {
  char buff[128];
  int dhour = 24 - hour();
  int dmonth = 12 - month();

  // is it December?
  if (dmonth == 0) {
    dday = 25 - day();
    // is it before or after Christmas?
    if (dday <= 0) {
      Serial.println("Maybe Christmas: December");
      xmasDay = -dday + 1;
      sprintf(buff, " it's the %d day of Christmas!", xmasDay);
      Serial.println(buff);
    }
    // is it January ?
  } else if (month() == 1) {
    Serial.println("Maybe Christmas: January");
    // calculate the Xth day of Christmas
    xmasDay = day() + 7;
    if (xmasDay <= 12) {
      sprintf(buff, " it's the %d day of Christmas!", xmasDay);
      Serial.println(buff);
    }
    else if (day() == 6) {
      Serial.println(" it's King's Day!");
    }
  }
  else { // Not Christmas, not january or december
    Serial.println("\nNot Christmas: not December or January");
    xmasDay = 0;
    int idxMonth = month() - 1;
    dday = monthDays[idxMonth ] - day();
    sprintf(buff, "   adding %2d days left in month", dday);
    Serial.println(buff);

    for (int i = month() ; i < 11; i++) {
      dday += monthDays[i] ;
      sprintf(buff, "   adding %2d days in month %d = %d", monthDays[i], i+1,dday);
      Serial.println(buff);

      if (i == 1) { // add Leap Year
        dday += isLeapYear(year(), debug) ;
        sprintf(buff, "   adding  1 days for leap year %d = %d", year(),dday);
        Serial.println(buff);
      }
    }
    // add the month of December
    dday += 25 ;
    sprintf(buff, "  adding 25 days for December = %d",dday);
    Serial.println(buff);
    sprintf(buff, " %d days until Christmas", dday);
    Serial.println(buff);
  }

  Serial.println("calculating time until christmas:");
  time_t time_now = now();
  time_t time_diff[3] = {0};
  int diff_sec[3];
  float diff_min[3];
  float diff_hr[3];
  float diff_day[3];

  // calculate time relative to xmas last year, this year, and next year
  for (int i = 0; i < 3; i++) {
    Serial.print("year is: ");
    int xmas_year = year() + i - 1;
    Serial.println(xmas_year);
    xmas_elem.Year = xmas_year - 1970; // years since 1970, so deduct 1970
    xmas_time[i] =  makeTime(xmas_elem);
    Serial.print("   Christmas time is:");
    Serial.println(xmas_time[i]);

    time_diff[i] = xmas_time[i] - time_now;
    Serial.print("   Time until Christmas is:");
    Serial.println(time_diff[i]);

    diff_sec[i] = (int)time_diff[i];
    diff_min[i] = (float)diff_sec[i] / 60;
    diff_hr[i] = diff_min[i] / 60;
    diff_day[i] = diff_hr[i] / 24;
  }

  int diff_DAYS = 0;
  int last_DAYS = 0;

  for (int i = 0; i < 3; i++) {
    if (time_diff[i] > 0) {
      Serial.print("Time since last Christmas is:");
      Serial.println(time_diff[i - 1]);
      Serial.print("   in days:");
      Serial.println(diff_day[i - 1]);
      last_DAYS = floor(diff_day[i - 1]);

      Serial.print("Time until next Christmas is:");
      Serial.println(time_diff[i]);
      Serial.print("   in days:");
      Serial.println(diff_day[i]);
      diff_DAYS = floor(diff_day[i]);
      break;

      if ((diff_DAYS - last_DAYS)<365) {
        exit;
      }
     Serial.print("Sum time:");
      Serial.println(time_diff[i]-time_diff[i - 1]);
      Serial.print("Sum days:");
      Serial.println((time_diff[i]-time_diff[i - 1])/(24*60*60));
      
    }
  }
  Serial.print("   in days:");
  Serial.println(last_DAYS);
  Serial.print("   in days:");
  Serial.println(diff_DAYS);
  Serial.print("   in days:");
  Serial.println(diff_DAYS - last_DAYS);

}

int isLeapYear(int in_year, int debugLY) {
  char buff[128];
  if (debugLY > 0) {
    sprintf(buff, "%d input\n", in_year);
    Serial.print(buff);
    sprintf(buff, "   leap year debug = %d\n", debugLY);
    Serial.print(buff);
  }

  // leap year if perfectly divisible by 400
  if (in_year % 400 == 0) {
    if (debugLY > 0) {
      sprintf(buff, "   %d is a leap year.\n", in_year);
      sprintf(buff, "   %d is divisible by 400.\n", in_year);
      Serial.print(buff);
    }
    return 1;
  }
  // not a leap year if divisible by 100
  // but not divisible by 400
  else if (in_year % 100 == 0) {
    if (debugLY > 0) {
      sprintf(buff, "   %d is NOT a leap year.\n", in_year);
      sprintf(buff, "   %d is divisible by 100, but not 400.\n", in_year);
      Serial.print(buff);
    }
    return 0;
  }
  // leap year if not divisible by 100
  // but divisible by 4
  else if (in_year % 4 == 0) {
    if (debugLY > 0) {
      sprintf(buff, "   %d is a leap year.\n", in_year);
      sprintf(buff, "   %d is divisible by 4, but not 100.\n", in_year);
      Serial.print(buff);
    }
    return 1;
  }
  // all other years are not leap years
  else {
    if (debugLY > 0) {
      sprintf(buff, "   %d is NOT a leap year.\n", in_year);
      sprintf(buff, "   %d is not divisible by 4 or 400.\n", in_year);
      Serial.print(buff);
    }
    return 0;
  }
}

void OLEDClockDisplay() {
  // defin OLED variables
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
    // draw clock display
    u8g2.setFont(u8g2_font_profont22_tn);
    sprintf(buff, "%02d:%02d", hour(), minute());
    if (debug > 0)
      Serial.println(buff);
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
    if (debug > 0)
      Serial.println(buff);
    // calculate display position
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
  if (do_RSSI) OLED_RSSI_Bars();
}

/*-------- NTP code ----------*/

time_t getNtpTime() {
  char buff[64];
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println(serdiv);
  Serial.println("Transmit NTP Request");
  u8g2.drawBox(0, 0, 2, 2); // cue light for sync status
  u8g2.sendBuffer();
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      LastSyncTime = millis();

      readNTP_packet();

      parseNTP_time(packetWords);

      Serial.println(serdiv);
      return NTPlocalTime;
    }
  }
  Serial.println("No NTP Response :-(");
  Serial.println(serdiv);
  return 0; // return 0 if unable to get the time
}
