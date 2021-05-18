/*************************************************************************************************************************************************
 *  TITLE: This sketch connects to a local WiFi network, connects to a network server and obtains the network time. It then converts this to a 
 *  readable format and displays it to the serial terminal and an OLED module. Refer to the video and post for more information. 
 *
 *  By Frenoy Osburn
 *  YouTube Video: https://youtu.be/3LkKYtQqzKo
 *  BnBe Post: https://www.bitsnblobs.com/network-time-using-esp8266
 *************************************************************************************************************************************************/
 
 /*
  This is an example file for using the time function in ESP8266 or ESP32 tu get NTP time
  It offers two functions:

  - getNTPtime(struct tm * info, uint32_t ms) where info is a structure which contains time
  information and ms is the time the service waits till it gets a response from NTP.
  Each time you cann this function it calls NTP over the net.

  If you do not want to call an NTP service every second, you can use
  - getTimeReducedTraffic(int ms) where ms is the the time between two physical NTP server calls. Betwwn these calls,
  the time structure is updated with the (inaccurate) timer. If you call NTP every few minutes you should be ok

  The time structure is called tm and has teh following values:

  Definition of struct tm:
  Member  Type  Meaning Range
  tm_sec  int seconds after the minute  0-61*
  tm_min  int minutes after the hour  0-59
  tm_hour int hours since midnight  0-23
  tm_mday int day of the month  1-31
  tm_mon  int months since January  0-11
  tm_year int years since 1900
  tm_wday int days since Sunday 0-6
  tm_yday int days since January 1  0-365
  tm_isdst  int Daylight Saving Time flag

  because the values are somhow akwardly defined, I introduce a function makeHumanreadable() where all values are adjusted according normal numbering.
  e.g. January is month 1 and not 0 And Sunday or monday is weekday 1 not 0 (according definition of MONDAYFIRST)

  Showtime is an example on how you can use the time in your sketch

  The functions are inspired by work of G6EJD ( https://www.youtube.com/channel/UCgtlqH_lkMdIa4jZLItcsTg )
*/

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <time.h>
#include <U8x8lib.h>
#include "credentials.h"

#define DELAY 250

const char* ssid = mySSID;              //from credentials.h file
const char* password = myPASSWORD;      //from credentials.h file

const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CST6CDT";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)

tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

void setup() 
{
  Serial.begin(115200);

  u8x8.begin();
  u8x8.setFont(u8x8_font_8x13_1x2_f); 
  u8x8.clear();
  u8x8.home();
  u8x8.print("NTP Time");
  delay(DELAY);
  WiFi.begin(ssid, password);

  u8x8.clear();
  u8x8.home();
  u8x8.print("Connecting to network");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);    
    if (++counter > 100) 
      ESP.restart();
    u8x8.print( "." );
  }
  u8x8.clear();
  u8x8.home();
  u8x8.println("WiFi connected");

  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10)) 
  {  
    // wait up to 10sec to sync
  } 
  else 
  {
    u8x8.println("Time not set");
    ESP.restart();
  }
  showTime(&timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();
}

void loop() 
{
  getNTPtime(10);
  showTime(&timeinfo);
  delay(1000);
}

bool getNTPtime(int sec) 
{
  {
    uint32_t start = millis();
    do
    {
      time(&now);
      localtime_r(&now, &timeinfo);
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    
    if (timeinfo.tm_year <= (2016 - 1900)) 
        return false;  // the NTP call was not successful
    
    u8x8.print("Time Now: ");  
    u8x8.println(now); 
  }
  return true;
}

void showTime(tm *localTime) 
{
  //display on OLED
  char time_output[30];
  
  u8x8.setFont(u8x8_font_8x13_1x2_f );
  u8x8.clear();
  u8x8.home();
  sprintf(time_output, "%02d:%02d:%02d", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
  u8x8.println(time_output);
  sprintf(time_output, "%02d/%02d/%02d", localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year - 100);
  u8x8.println(time_output);
  u8x8.println(getDOW(localTime->tm_wday));
}

char * getDOW(uint8_t tm_wday)
{
  switch(tm_wday)
  {
    case 1:
      return "Monday";
      break;
    case 2:
      return "Tuesday";
      break;
    case 3:
      return "Wednesday";
      break;
    case 4:
      return "Thursday";
      break;
    case 5:
      return "Friday";
      break;
    case 6:
      return "Saturday";
      break;
    case 7:
      return "Sunday";
      break;
    default:
      return "Error";
      break;
  }
}
