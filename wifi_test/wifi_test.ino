#include <ESP8266WiFi.h>
#include "credentials.h"
const char* ssid = mySSID;          //from credentials.h file
const char* pass = myPASSWORD;      //from credentials.h file
#define PRINT_DELAY 250 // print delay in milliseconds

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);
  // start serial
  Serial.begin(9600);
  while (!Serial) { }
  delay(PRINT_DELAY);
  Serial.println("\nWi-Fi Example");
  Serial.flush();
  wifi_mac();
  // connect to wi-fi
  wifi_con();
  // turn off LED
  digitalWrite(LED_BUILTIN, HIGH);
}

void wifi_mac() {
  Serial.print("       MAC address : ");
  Serial.println(WiFi.macAddress());
}

int wifi_stat() {
  // print status
  int i;
  Serial.print("      Wi-Fi status : ");
  i = WiFi.status();
  Serial.println(i);
  if (i == 3) {
    Serial.print("gateway IP address : ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("  local IP address : ");
    Serial.println(WiFi.localIP());
    Serial.print("   signal strength : ");
    Serial.println(WiFi.RSSI());
  }
  return i;
}

void wifi_con() {
  //wifi_stat();
  Serial.print(" Exit modem sleep mode...");
  WiFi.forceSleepWake(); // Wifi on
  Serial.println("done");
  //wifi_stat();
  WiFi.mode(WIFI_STA); // station mode
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print("...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(PRINT_DELAY);
    Serial.print(".");
  }  
  Serial.print("connected\n");
  wifi_stat();
}
#define STATE_DELAY 4000

void wifi_discon() {
  wifi_stat();
  Serial.print("Disconnecting from ");
  Serial.print(WiFi.SSID());
  Serial.print("...");
  WiFi.disconnect();
  while (WiFi.status() == WL_CONNECTED) {
    delay(PRINT_DELAY);
    Serial.print(".");
  }
  Serial.print("disconnected\n");
  wifi_stat();  
  Serial.print("Enter modem sleep mode...");
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(1); //the modem won't go to sleep unless you do a delay
    while (WiFi.status() == 0) {
    delay(PRINT_DELAY);
    Serial.print(".");
  }
  Serial.println("done");
  wifi_stat();
}

void loop() {
  Serial.println("Begin loop");
  delay(PRINT_DELAY);
  wifi_discon() ;
  delay(STATE_DELAY);
  wifi_con();
  delay(STATE_DELAY);
}
