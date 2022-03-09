#include <Wire.h>
#include <Adafruit_GFX.h>
// Must use Adafruit SSD1306 Wemos Mini OLED Library for 64x48 display (Adafruit_SSD1306_Wemos_OLED)
#include <Adafruit_SSD1306.h>
#include <BH1750.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

BH1750 light(0x23);
char buff[64];
#define PRINT_DELAY 250 // print delay in milliseconds 

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  sprintf(buff, "BH1750 Example");
  Serial.println(buff);
  display.println(buff);
  delay(PRINT_DELAY);
}

// define variables needed between loops
float lux_last=0;
float lux_av=0;
float lux_av_last=0;

void loop() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  if (light.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {    
    float lux = light.readLightLevel();
    sprintf(buff, " L:%7.1f",lux);    
    Serial.print(buff);
    display.println(buff);    
    
    // calculate change in lux
    float dlux = (lux - lux_last);    
    sprintf(buff, "dL:%7.1f",dlux);    
    Serial.print(buff);
    display.println(buff);
    
    lux_last=lux; 
  }
  else
    {
      display.println("Error!");
      display.println("BH1750 not found");
      display.println("Light meter missing");
    }
  display.display();
  delay(1000);
}
