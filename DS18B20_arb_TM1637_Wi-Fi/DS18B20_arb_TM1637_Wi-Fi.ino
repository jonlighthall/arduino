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
  | D5  | GPIO14 | SCLK  | LED display DIO
  +-----+--------+-------+----------------+
  | D6  | GPIO12 | MISO  | LED display CLK
  +-----+--------+-------+----------------+
  | D7  | GPIO13 | MOSI  |
  +-----+--------+-------+----------------+
  | D8  | GPIO15 | CS    |
  +-----+--------+-------+----------------+
  | 3V3 | 3.3V   |       | LED VCC
  +-----+--------+-------+----------------+
  | TX  | GPIO1  | TX    |
  +-----+--------+-------+----------------+
  | RX  | GPIO3  | RX    |
  +-----+--------+-------+----------------+
  | D1  | GPIO5  | SCL   |
  +-----+--------+-------+----------------+
  | D2  | GPIO4  | SDA   |
  +-----+--------+-------+----------------+
  | D3  | GPIO0  | FLASH |
  +-----+--------+-------+----------------+
  | D4  | GPIO2  | LED   | sync cue
  +-----+--------+-------+----------------+
  | G   | GND    | GND   | LED GND
  +-----+--------+-------+----------------+
  | 5V  | N/A    | VCC   |
  +-----+--------+-------+----------------+

*/

// standard library headers
#include <OneWire.h>
#include <DallasTemperature.h>  // Dallas Temperature Requires OneWire

// define options
#define LED1

// project library headers
#include "debug.h"
#include "led_utils.h"
#include "serial_utils.h"
#include "wifi_utils.h"

bool ssid_found = false;

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

const int array_size = 5;
DeviceAddress therm[array_size];
int no_therm;
int no_therm_sum;
int idx_probe = -1;

// define ADC
const int ADC = A0;
int ADCvalue = -1;

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);

  serial_init();
  LED_init();
  
  Serial.println();
  Serial.println("---------------");
  Serial.println("Dallas Temperature IC Control Library Demo");
  Serial.println("---------------");

  // Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  no_therm = sensors.getDeviceCount();
  no_therm_sum = no_therm;
  Serial.print(no_therm);
  Serial.println(" devices.");

  Serial.print("Checking number of sensors vs array length: ");
  if (no_therm > array_size) {
    Serial.println("ERROR, make array larger!");
    exit;
  } else {
    Serial.print("OK ");
    Serial.print(no_therm);
    Serial.print(" < ");
    Serial.println(array_size);
  }

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  oneWire.reset_search();
  for (int i = 0; i < no_therm; i++) {
    if (!oneWire.search(therm[i])) {
      Serial.print("Unable to find address for sensor ");
      Serial.println(i);
    } else {
      Serial.print("Device ");
      Serial.print(i);
      Serial.print(" Address: ");
      printAddress(therm[i]);
      Serial.println();
    }
  }

  for (int i = 0; i < no_therm; i++) {
    // set the resolution to 9 bit per device
    sensors.setResolution(therm[i], TEMPERATURE_PRECISION);
    Serial.print("Device ");
    Serial.print(i);
    Serial.print(" Resolution: ");
    Serial.print(sensors.getResolution(therm[i]), DEC);
    Serial.println();
  }

  wifi_scan();

  // clear displays
  display.clear();
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress) {
  Serial.print(" Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

void loop() {
  float atempC[array_size];
  float atempF[array_size];
  float temp_sum = 0;
  float temp_mean = 0;
  float temp_std = 0;
  float temp_var_sum = 0;
  float temp_var_mean = 0;

  // read the value from the sensor:
  ADCvalue = analogRead(ADC);
  Serial.print("ADC value = ");
  Serial.println(ADCvalue);
  const int ADCmax = 1024;

  if (true) {
    float ADCscale = ADCvalue / ADCmax;
    Serial.print("ADC scale = ");
    Serial.println(ADCscale);

    // connect to wi-fi if connected to power
    if (ADCvalue == ADCmax) {
      Serial.println("Battery connected to power.");
      if (ssid_found) {
        Serial.print(" Connecting to Wi-Fi...");
        if (WiFi.status() != WL_CONNECTED) {
          display.setSegments(SEG_CONN);
          wifi_con();
        } else {
          Serial.println("Wi-Fi already connected. Continuing...");
        }
      } else {
        Serial.print(ssid);
        Serial.println(" not found");
      }

    }
    // otherwise turn off modem
    else if (ADCvalue < 100) {
      Serial.println("Battery removed from power. Disconnecting from  Wi-Fi...");
      if (WiFi.status() == WL_CONNECTED) {
        wifi_discon();
      } else {
        Serial.println("Wi-Fi already disconnected. Continuing...");
      }
    }
  }

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("OK");

  // print the device information
  for (int i = 0; i < no_therm; i++) {
    printData(therm[i]);
    atempC[i] = sensors.getTempC(therm[i]);
    atempF[i] = sensors.getTempF(therm[i]);
    if (i != idx_probe) temp_sum += atempF[i];
  }
  temp_mean = temp_sum / no_therm_sum;

  // print average temperature and calculate standard deviation
  if (no_therm_sum > 1) {
    Serial.print("Average temperature = ");
    Serial.print(temp_mean);
    for (int i = 0; i < no_therm; i++) {
      if (i != idx_probe) temp_var_sum += sq(atempF[i] - temp_mean);
    }
    temp_var_mean = temp_var_sum / no_therm_sum;
    temp_std = sqrt(temp_var_mean);
    Serial.print(" +/- ");
    Serial.print(temp_std);
    Serial.print(" °F");
    Serial.print(" (N = ");
    Serial.print(no_therm_sum);
    Serial.println(")");
  }
  if (no_therm_sum > 0) {
    // update LED display
    if ((temp_mean >= 0) && (temp_mean < 100)) {
      display.setSegments(SEG_degF, 2, 2);
      display.showNumberDec(temp_mean, false, 2, 0);
    }

    if ((temp_mean >= 100) || ((temp_mean < 0) && (temp_mean > -100))) {
      display.setSegments(SEG_letF, 1, 3);
      display.showNumberDec(temp_mean, false, 3, 0);
    }

    if (temp_mean <= -100) {
      display.showNumberDec(temp_mean, false, 4, 0);
    }
  } else {
    display.setSegments(SEG_bad, 4, 0);
    Serial.println(" No temperature sensors found!");
  }

  delay(PRINT_DELAY);
}

void wifi_scan() {
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());
  Serial.print("scaning networks...");
  display.setSegments(SEG_scan, 4, 0);
  int n = WiFi.scanNetworks();
  Serial.println("done");
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());
  if (n == 0)
    Serial.println("no networks found");
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
      if (WiFi.SSID(i) == ssid) {  //enter the ssid which you want to search
        Serial.print(ssid);
        Serial.println(" is available");
        ssid_found = true;
        break;
      }
    }
  }
}

void wifi_con() {
  WiFi.forceSleepWake();  // Wifi on
  Serial.println("Exit modem sleep mode");
  Serial.print("MAC address : ");
  Serial.println(WiFi.macAddress());
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());
  WiFi.mode(WIFI_STA);  // station mode
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print("...");
  WiFi.begin(ssid, pass);
  int counter = 0;
  const int wait_lim = 10000 / PRINT_DELAY;
  while ((WiFi.status() != WL_CONNECTED) && (counter < wait_lim)) {
    delay(PRINT_DELAY);
    Serial.print(".");
    counter++;
    Serial.print("Wi-Fi status : ");
    Serial.println(WiFi.status());
    if (counter == wait_lim) {
      Serial.println("timeout");
      display.setSegments(SEG_fail, 4, 0);
      delay(PRINT_DELAY);
    }
  }

  // print status
  Serial.print("connected\n");
  display.setSegments(SEG_good, 4, 0);
  delay(PRINT_DELAY);
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());
  Serial.print("gateway IP address: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("  local IP address: ");
  Serial.println(WiFi.localIP());
  int rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);
}

void wifi_discon() {
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());
  Serial.print("Disconnecting from ");
  Serial.print(WiFi.SSID());
  Serial.print("...");
  WiFi.disconnect();
  while (WiFi.status() == WL_CONNECTED) {
    delay(PRINT_DELAY);
    Serial.print(".");
  }
  Serial.print("disconnected\n");
  Serial.print("Wi-Fi status : ");
  Serial.println(WiFi.status());

  Serial.println("Enter modem sleep mode...");
  WiFi.forceSleepBegin();  // Wifi off
  delay(1);                //the modem won't go to sleep unless you do a delay
}
