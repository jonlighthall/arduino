/*
   This sketch uses the ESP8266WiFi library
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi settings:
#include "credentials.h"
const char* ssid = mySSID;          //from credentials.h file
const char* pass = myPASSWORD;      //from credentials.h file

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

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
