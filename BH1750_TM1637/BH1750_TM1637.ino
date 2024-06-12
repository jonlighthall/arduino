/*
  TimeNTP_ESP8266WiFi.ino
  Example showing time sync to NTP time source
*/

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
  | D5  | GPIO14 | SCLK  | LED DIO (display if LED1 defined)
  +-----+--------+-------+----------------+
  | D6  | GPIO12 | MISO  | LED CLK (display if LED1 defined)
  +-----+--------+-------+----------------+
  | D7  | GPIO13 | MOSI  |
  +-----+--------+-------+----------------+
  | D8  | GPIO15 | CS    |
  +-----+--------+-------+----------------+
  | 3V3 | 3.3V   |       |
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
  | G   | GND    | GND   |
  +-----+--------+-------+----------------+
  | 5V  | N/A    | VCC   |
  +-----+--------+-------+----------------+

*/

// custom library headers
#include <TimeLib.h>

// LED display options
// Possible options are:
//   LED1 - enable clock-like time display on 4-bit 7-segment LED
//   LED2 - enable seconds and fractional seconds on second LED
//   LDR  - use LDR to adjust brightness level
//   WEMOS_V4 - use I2C port (requires LED1, conflicts with LED2)
//   CLOCK_BUTTONS - use the buttons on the clock to set options

#define LED1
//#define LDR
//#define LED2

// project library headers
// project library headers
#include "debug.h"
#include "led_utils.h"
#include "serial_utils.h"

#define PRINT_DELAY 500

// desk light
#include <BH1750.h>
BH1750 lightMeter(0x23);
int tlast = -1;
#define READ_DELAY 1000 * 5
const int lux_low_thresh = 30;
const int lux_high_thresh = 100;

int k;
uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
int i;
void wipe() {
  for (i = 0; i < 4; i++)
    data[i] = 0x00;
}

void setup() {
  // initialize on-board LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize Serial
  serial_init();
  Serial.println("---------------");
  char buff[64];
  sprintf(buff, "TimeNTP Example");
  Serial.println(buff);
  Serial.println("---------------");

  LED_init();

  // initialize I2C bus
  // the BH1750 library doesn't do this automatically
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  Wire.begin();

  // initialize BH1750
  // begin() returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }

  // pause for readability
  delay(PRINT_DELAY);
  Serial.println("done with setup");

  display.clear();
  // Run through all the dots
  for (k = 0; k <= 4; k++) {
    Serial.print("dot = ");
    Serial.println(k);
    display.showNumberDecEx(k, (0x80 >> k), true);
    delay(PRINT_DELAY);
  }

  // Alternate run through all the dots
  for (k = 0; k < 4; k++) {
    wipe();
    data[k] = display.encodeDigit(k) + 0x80;
    display.setSegments(data);
    Serial.print("dot ");
    Serial.print(k);
    Serial.print(" = ");
    Serial.print(data[k], DEC);
    Serial.print(", 0x");
    Serial.print(data[k], HEX);
    Serial.print(", 0b");
    Serial.println(data[k], BIN);
    delay(PRINT_DELAY);

    wipe();
    data[k] = display.encodeDigit(k);
    display.setSegments(data);
    Serial.print("dot ");
    Serial.print(k);
    Serial.print(" = ");
    Serial.print(data[k], DEC);
    Serial.print(", 0x");
    Serial.print(data[k], HEX);
    Serial.print(", 0b");
    Serial.println(data[k], BIN);
    delay(PRINT_DELAY);

    wipe();
    data[k] = display.encodeDigit(k) + 0x80;
    display.setSegments(data);
    delay(PRINT_DELAY);
  }

  display.setSegments(SEG_DONE);
  delay(PRINT_DELAY);

  Serial.println("starting loop...");
}

// define loop variables
char buff[64];
uint16_t lux;
float print_lux;
uint16_t disp_lux;
float scale;
float fbrite;

void loop() {

  // ---------------------------------
  // Light controls
  // ---------------------------------
  lux = lightMeter.readLightLevel();
  sprintf(buff, "Light: %d lux", lux);
  Serial.print(buff);

  //data range: 1 – 65,535 lux
  if (lux < 10000) {
    display.showNumberDec(lux, false);
  } else {
    print_lux = (float)lux / 100.;
    sprintf(buff, " = %.2f clx", print_lux);
    Serial.print(buff);
    disp_lux = lux / 10;
    sprintf(buff, " = %d clx", disp_lux);
    Serial.print(buff);
    display.showNumberDecEx(disp_lux, 0b00100000, false);
  }

  scale = (float)lux / 65535.;
  fbrite = scale * (float)brite_max;
  brite = round(fbrite);
  display.setBrightness(brite);
  sprintf(buff, " (%5.1f%% %d)", scale*100.,brite);
  Serial.print(buff);


  if (lux < lux_low_thresh) {
    sprintf(buff, " It is too dark: %d < %d", lux, lux_low_thresh);
    Serial.print(buff);
    Serial.println(" Turning light ON...");
  } else if (lux > lux_high_thresh) {
    sprintf(buff, " It is too bright: %d > %d", lux, lux_high_thresh);
    Serial.print(buff);
    Serial.println(" Turning light OFF...");
  } else {
    Serial.println();
  }
}  // end loop
