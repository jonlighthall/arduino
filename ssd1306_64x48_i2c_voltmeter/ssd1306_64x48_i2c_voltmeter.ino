/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 64x48 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <Adafruit_SSD1306.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET); // define display

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// refresh delay
#define DELAY 800
#define WIDE 64

void setup()   {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  delay(DELAY);
}

void loop() {
  display.clearDisplay();
  display.setCursor(0,0); 
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.print("A0");
  display.setCursor(15,0); 
  display.setTextColor(WHITE);
  display.print("ADC ");

  //read sensor
  int sensorValue = analogRead(A0);
  char buf[64];
  sprintf(buf,"%04d\n\n",sensorValue);
  display.print(buf); 
  float scale = sensorValue/1023.;
  sprintf(buf,"%7.3f%%\n",scale*100.);
  //display.print(buf); 
  float vmax=3.3;
  float voltage = (scale)*vmax; 

  //calibration
  const float m=0.945365548;
  const float b=-0.029950454;

  float volt_cal=m*voltage+b;

  display.setTextSize(2);
  display.setCursor(2,14);
  if (volt_cal>=0) 
    sprintf(buf,"%.3f\n",volt_cal);
  else
    sprintf(buf,"%.2f\n",volt_cal);
  display.print(buf); 

  display.setTextSize(1);
  sprintf(buf,"%6.1f mV\n",volt_cal*1000.);
  //display.print(buf); 

  int pscale=scale*WIDE;

  int bar=35;
  // bar graph
  for (uint8_t i=0; i< pscale; i++) {
    display.drawPixel(i,bar,WHITE);
  }

  // volt grid
  for (uint8_t i=0; i< int(volt_cal)+1; i++) {
    int pgrid = i/vmax*WIDE;
    display.drawPixel(pgrid,bar+1,WHITE);
    display.drawPixel(pgrid,bar+2,WHITE);

    //labels
    int xoff=-2;
    if (i>0) 
      display.setCursor(pgrid+xoff,40); 
    else
      display.setCursor(pgrid,40); 
    display.print(i);
  }

  // half-volt grid
  for (float i=0; i< volt_cal; i=i+0.5) {
    int pgrid = i/vmax*WIDE;
    display.drawPixel(pgrid,bar+1,WHITE);
  }

  // fixed labels
  for (uint8_t i=0; i< 4; i=i+3) {
    int pgrid = i/vmax*WIDE;
    int xoff=-2;
    if (i>0) 
      display.setCursor(pgrid+xoff,40); 
    else
      display.setCursor(pgrid,40); 
    //display.print(i);
  }
  display.display();
  delay(DELAY);
}
