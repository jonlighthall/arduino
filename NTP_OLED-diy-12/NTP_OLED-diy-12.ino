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
 
  - getNTPtime(struct tm * info, uint32_t ms) where info is a structure which contains time
  information and ms is the time the service waits till it gets a response from NTP.
  Each time you cann this function it calls NTP over the net.

  Showtime is an example on how you can use the time in your sketch

  The functions are inspired by work of G6EJD ( https://www.youtube.com/channel/UCgtlqH_lkMdIa4jZLItcsTg )
*/

#include <ESP8266WiFi.h>
#include <time.h>

// OLED packages
#include <U8x8lib.h>

#include "date_time_fmt.h"

// Wi-Fi settings
#include "credentials.h"
const char* ssid = WIFI_SSID;              //from credentials.h file
const char* password = WIFI_PASSWORD;      //from credentials.h file

// NTP settings
const char* NTP_SERVER = "time.nist.gov";
const char* TZ_INFO    = "CST6CDT";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

// OLED display options
U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

#define PRINT_DELAY 1

void setup() {
  Serial.begin(9600);
  delay(PRINT_DELAY);
  Serial.println();
  Serial.println("NTP Time");

  // display settings
  u8x8.begin();
  u8x8.clear();
  u8x8.home();

  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.println("NTP Time");
  delay(PRINT_DELAY);

  // Wi-Fi settings
  Serial.print("Connecting to ");
  Serial.print(ssid);
  u8x8.clear();
  u8x8.home();
  u8x8.print("Connecting to network");
  WiFi.begin(ssid, password);
  
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);    
    if (++counter > 100) {
      Serial.print("timeout");
      ESP.restart();
    }
    u8x8.print(".");
    Serial.print("."); 
  }
  Serial.println("done");
  u8x8.print("\nWiFi OK");
  delay(PRINT_DELAY);

  configTime(0, 0, NTP_SERVER);
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10)) {  
    // wait up to 10sec to sync
  } 
  else {
    u8x8.println("Time not set");
    ESP.restart();
  }
  u8x8.clear();
  showTime(&timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();
}

bool getNTPtime(int sec) {
  uint32_t start = millis();
  do {
    time(&now);
    localtime_r(&now, &timeinfo);
    delay(1);
  }
  while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    
  if (timeinfo.tm_year <= (2016 - 1900)) 
    return false;  // the NTP call was not successful
    
  u8x8.print("Time Now: ");  
  u8x8.println(now); 
  Serial.print("Time Now: ");  
  Serial.println(now); 
  delay(PRINT_DELAY);
  
  return true;
}

void showTime(tm *localTime) { 
  //display on OLED
  char time_output[30];  
  
  u8x8.home();
/*
  u8x8.setFont(u8x8_font_px437wyse700a_2x2_n);
  sprintf(time_output,"%02d:%02d",localTime->tm_hour,localTime->tm_min);
  u8x8.println(time_output);
  Serial.println(time_output);
*/
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  sprintf(time_output, "%02d:%02d:%02d", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
  u8x8.println(time_output);
  Serial.println(time_output);
  u8x8.println(getDOW(localTime->tm_wday));
  sprintf(time_output, " %s %02d", getMo(localTime->tm_mon + 1), localTime->tm_mday); 
  u8x8.println(time_output);  
  Serial.println(time_output);
  
}

void loop() {
  Serial.println("Get time...");
  getNTPtime(1);
  Serial.println("Show time...");
  showTime(&timeinfo);
  delay(1000);
}
