#include <Wire.h>
#include <Adafruit_GFX.h>
// Must use Adafruit SSD1306 Wemos Mini OLED Library for 64x48 display (Adafruit_SSD1306_Wemos_OLED)
#include <Adafruit_SSD1306.h>
#include <BH1750.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
int disphei=SSD1306_LCDHEIGHT;
int dispwid=SSD1306_LCDWIDTH;

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

BH1750 light(0x23);
char buff[64];
#define PRINT_DELAY 250 // print delay in milliseconds 

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  sprintf(buff, "BH1750 Example");
  Serial.println(buff);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.println(buff);
}

void loop() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  if (light.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {    
    float lux = lightMeter.readLightLevel();
    sprintf(buff, "L: %f lux",flux);    
    Serial.print(buff);
    display.println(buff);
    
    // draw bar
    // size
    int min_div=4
    int max_div=man_div * 3;
    int barwid=(int)(dispwid/max_div) * max_div;       
    
    // position
    // default text height is 13, 19, or 28
    int ypos=13+2;
    int xpos=(dispwid - barwid)/2;
    
    // fill
    float lx_max=54612.5;    
    float lx_per=lux/lx_max;
    int barfill = (int) lx_per * barwid;    
    
    for (int i = 0; i < barfill + 1; i++) {
    u8g2.drawPixel(i + xpos, ypos);
  }
  ypos += 1;
  for (int i = 0; i < barfill + 1; i = i + barwid/max_div) {
    u8g2.drawPixel(i + xpos, ypos);
  }
  ypos += 1;
  for (int i = 0; i < barfill + 1; i = i + barwid/min_div) {
    u8g2.drawPixel(i + xpos, ypos);
  }
    
    
  }
  else
  {
    display.println("Error!");
  }
  display.display();
  delay(1000);
}

