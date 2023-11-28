#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi settings:
#include <credentials.h>
const char* ssid = WIFI_SSID;      // from credentials.h file
const char* pass = WIFI_PASSWORD;  // from credentials.h file

#define PRINT_DELAY 250  // print delay in milliseconds

WiFiUDP ntpUDP;

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
  Serial.println();
  wifi_stat();
  Serial.print(" Exit modem sleep mode...");
  WiFi.forceSleepWake();  // Wifi on
  Serial.println("done");
  wifi_stat();
  WiFi.mode(WIFI_STA);  // station mode
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
  //  Serial.println("set sleep type none...");
  //  wifi_set_sleep_type(NONE_SLEEP_T);
  //  wifi_fpm_close(); //disable force sleep function
}

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
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);  // the modem won't go to sleep unless you do a delay
  Serial.println("done");
  wifi_stat();
}
