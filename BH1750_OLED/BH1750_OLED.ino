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
float lux_last = 0;
int N = 0;
float lux_av_last = 0;
const int av_int = 5;
float values[av_int] = {0};

void loop() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  if (light.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    float lux = light.readLightLevel();
    N++;
    Serial.print(N);
    sprintf(buff, " L:%7.1f", lux);
    Serial.print(buff);
    display.println(buff);

    // calculate change in lux
    float dlux = (lux - lux_last);
    sprintf(buff, "dL:%7.1f", dlux);
    Serial.println(buff);
    display.println(buff);

    int idx = N % av_int;
    Serial.println(idx);

    values[idx] = lux;

    float lux_sum = 0;
    int n = 0;
    for (int i = 0; i < av_int; i ++) {
      Serial.println(values[i]);
      if (values[i] > 0 ) {
        lux_sum += values[i];
        n++;
      }
    }
    float lux_av  = lux_sum / n;
    sprintf(buff, "SL:%7.1f", lux_av);
    Serial.println(buff);
    display.println(buff);


    float dlux_av =  (lux_av - lux_av_last);
    sprintf(buff, "dS:%7.1f", dlux_av);
    Serial.println(buff);
    display.println(buff);

    lux_last = lux;
    lux_av_last = lux_av;
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
