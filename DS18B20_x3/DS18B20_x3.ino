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

// arrays to hold device addresses
//DeviceAddress therm1, therm2, therm3;

// Assign address manually. The addresses below will need to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)
 DeviceAddress therm1 = { 0x28, 0xC0, 0xA9, 0x16, 0xA8, 0x01, 0x3C, 0x7A };
 DeviceAddress therm2   = { 0x28, 0x8E, 0x6E, 0x16, 0xA8, 0x01, 0x3C, 0x6F };
 DeviceAddress therm3   = { 0x28, 0x9B, 0x5C, 0x61, 0x2D, 0x20, 0x01, 0xE7  };

void setup(void)
{
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

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  oneWire.reset_search();
  // assigns the first address found to therm1
  if (!oneWire.search(therm1)) Serial.println("Unable to find address for therm1");
  // assigns the seconds address found to therm2
  if (!oneWire.search(therm2)) Serial.println("Unable to find address for therm2");
  // assigns the seconds address found to therm3
  if (!oneWire.search(therm3)) Serial.println("Unable to find address for therm3");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(therm1);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(therm2);
  Serial.println();

  Serial.print("Device 2 Address: ");
  printAddress(therm3);
  Serial.println();

  // set the resolution to 9 bit per device
  sensors.setResolution(therm1, TEMPERATURE_PRECISION);
  sensors.setResolution(therm2, TEMPERATURE_PRECISION);
  sensors.setResolution(therm3, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(therm1), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(therm2), DEC);
  Serial.println();

  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(therm3), DEC);
  Serial.println();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
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
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

  // print the device information
  printData(therm1);
  printData(therm2);
  printData(therm3);
}
