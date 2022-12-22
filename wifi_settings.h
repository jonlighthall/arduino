#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi settings:
#include "credentials.h"
const char* ssid = mySSID;          //from credentials.h file
const char* pass = myPASSWORD;      //from credentials.h file

WiFiUDP ntpUDP;

void wifi_start();

void wifi_start() {
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
}
