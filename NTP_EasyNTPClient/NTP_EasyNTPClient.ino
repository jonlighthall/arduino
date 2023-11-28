/*

EasyNTPClient example: ArduinoEspWifiShield

This example shows the basic usage of the EasyNTPClient on an Arduino UNO with
an ESP-01 (ESP8266) WiFi module. The output is visible in the Serial Monitor at
9600 baud rate.

For more details see: https://github.com/aharshac/EasyNTPClient

An example by Claran Martis
https://www.collaborizm.com/profile/SJne7FcMg

*/

/*

Pin Connectiions

+--------+-----------------------------+
| ESP-01 | Connection                  |
+--------+-----------------------------+
| TXD    | Arduino D3                  |
+--------+-----------------------------+
| CH_PD  | External Power Supply +3.3V |
+--------+-----------------------------+
| RST    | Arduino Reset               |
+--------+-----------------------------+
| VCC    | External Power Supply +3.3V |
+--------+-----------------------------+
| RXD    | Arduino D2                  |
+--------+-----------------------------+
| GPIO0  | {None}                      |
+--------+-----------------------------+
| GPIO2  | {None}                      |
+--------+-----------------------------+
| GND    | Common GND                  |
+--------+-----------------------------+

*/

// custom library headers
#include <wifi_utils.h>

// project headers
#include <EasyNTPClient.h>

EasyNTPClient ntpClient(Udp, "pool.ntp.org",
                        ((5 * 60 * 60) + (30 * 60)));  // IST = GMT + 5:30

void setup() {
  Serial.begin(9600);

  wifi_start();
  Udp.begin(123);
}

void loop() {
  Serial.println(ntpClient.getUnixTime());
  delay(20000);  // wait for 20 seconds before refreshing.
}
