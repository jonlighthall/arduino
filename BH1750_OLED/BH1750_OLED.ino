#include <Wire.h>
#include <Adafruit_GFX.h>
// Must use Adafruit SSD1306 Wemos Mini OLED Library for 64x48 display (Adafruit_SSD1306_Wemos_OLED)
#include <Adafruit_SSD1306.h>
#include <BH1750.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

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

float lux_last;
int N=0;
const int N_max=5;
//float luxes[N_max] = {0};
float luxes[5];
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
    sprintf(buff, "L: %f lux",lux);    
    Serial.print(buff);
    display.println(buff);
    
    // draw bar
    // size
    int min_div=4;
    int max_div=min_div * 3;
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
    
   // calculate change in lux
    float dlux = (lux - lux_last);    
    sprintf(buff, "dL: %f lux",dlux);    
    Serial.print(buff);
    
    ypos +=2;
    display.setCursor(0, ypos);
    display.println(buff);
    
    // calculate average lux
    N=(N+1)%N_max;
    sprintf(buff, "N = %d",N);    
    Serial.print(buff);
    
//    luxes(N)=lux;    
    lux_av_last=lux_av;
    lux_av=0;
    int n=0;
    for (int i=0; i<N_max;i++) {
//      if (luxes(i) > 0 ) {
  //      n+=1;
        //lux_av+=luxes(i);
    //  }
    
    }
    lux_av=lux_av/n;
    sprintf(buff, "La: %f lux",lux_av);    
    Serial.print(buff);
    
    ypos +=1;
    display.setCursor(0, ypos);
    display.println(buff);
       
    // calculate change in average flux
    float dlux_av=(lux_av - lux_av_last);
      
      sprintf(buff, "dLa: %f lux",dlux_av);    
    Serial.print(buff);
    
    ypos +=1;
    display.setCursor(0, ypos);
    display.println(buff);    
    
lux_last=lux; 
  }
  else
  {
    display.println("Error!");
  }
  display.display();
  delay(1000);
}
