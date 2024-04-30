// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

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

void setup() {
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  no_therm = sensors.getDeviceCount();
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
    temp_sum += atempF[i];
  }
  temp_mean = temp_sum / no_therm;
  Serial.print("Average temperature = ");
  Serial.print(temp_mean);

  for (int i = 0 ; i < no_therm; i++) {
    temp_var_sum += sq(atempF[i] - temp_mean);
  }
  temp_var_mean = temp_var_sum / no_therm;
  temp_std = sqrt(temp_var_mean);
  Serial.print(" +/- ");
  Serial.print(temp_std);
  Serial.println(" °F");
}
