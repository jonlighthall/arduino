/*
   This sketch uses the ESP8266WiFi library
*/

#ifndef WIFI_UTILS
#define WIFI_UTILS

#include <ESP8266WiFi.h>

// Wi-Fi settings:
#include "wifi_credentials.h"
const char* ssid = WIFI_SSID;          //from credentials.h file
const char* pass = WIFI_PASSWORD;      //from credentials.h file

int rssi = 0; // Wifi signal strengh variable

void wifi_start() {
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
}

#endif
