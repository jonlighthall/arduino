#include <Wire.h>
#include <Adafruit_GFX.h>
// Must use Adafruit SSD1306 Wemos Mini OLED Library for 64x48 display (Adafruit_SSD1306_Wemos_OLED)
#include <Adafruit_SSD1306.h>
#include <BH1750.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
int disphei = SSD1306_LCDHEIGHT;
int dispwid = SSD1306_LCDWIDTH;

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

BH1750 light(0x23);
const float lx_max = 54612.5;
const float thresh_inst = 1;
const float thresh_av = 1;

const int PIR = D3;
int PIRState = 0;

char buff[64];
#define PRINT_DELAY 1500 // print delay in milliseconds 
#define LOOP_DELAY 250 // sampling period in milliseconds 
#define AVERGING_INTERVAL 3000 // averaging interval in milliseconds 

// bar size
int min_div = 4;
int max_div = min_div * 3;
int barwid = (int)(dispwid / max_div) * max_div;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(PIR, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);

  // set initial state, LED off
  digitalWrite(BUILTIN_LED, HIGH);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  sprintf(buff, "BH1750 OLED Example");
  Serial.println(buff);
  sprintf(buff, "BH1750\nExample");
  display.println(buff);

  sprintf(buff, "disp is\n %d x %d", dispwid, disphei);
  display.println(buff);

  sprintf(buff, "\nmin div = %d\nmax div = %d\nbarwid = %d\n", min_div, max_div, barwid);
  Serial.print(buff);

  display.display();
  delay(PRINT_DELAY);
}

// define variables needed between loops
float lux_last = 0;
int N = 0;
float lux_av_last = 0;
const int av_int = (int) (AVERGING_INTERVAL / LOOP_DELAY);
float values[av_int] = {0};

void loop() {
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  /*
     BH1750 code
  */

  if (light.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    float lux = light.readLightLevel();
    N++;
    Serial.print(N);
    sprintf(buff, " L:%7.1f", lux);
    Serial.print(buff);
    display.println(buff);

    // draw bar

    // position
    int txt_ht = 7;
    int ypos = txt_ht + 2;
    int xpos = (dispwid - barwid) / 2;

    sprintf(buff, "ypos = %d\nxpos = %d\n", ypos, xpos);
    Serial.print(buff);

    // fill
    float lx_per = lux / lx_max;

    sprintf(buff, "lx percent = %.1f\n", lx_per * 100);
    Serial.print(buff);
    sprintf(buff, "barfill = %.1f\n", lx_per * 60);
    Serial.print(buff);
    sprintf(buff, "barfill = %.1f\n", lx_per * barwid);
    Serial.print(buff);
    float barfillf = (lx_per * (float)barwid);
    sprintf(buff, "barfillf = %.1f\n", barfillf);
    Serial.print(buff);
    int barfill = round(barfillf);
    sprintf(buff, "barfillf = %2d\n", barfill);
    Serial.print(buff);

    //display.drawPixel(barfill,ypos,1);
    display.drawLine(xpos, ypos, xpos + barfill, ypos, 1);
    ypos += 1;

    for (int i = 0; i <= barfill; i += barwid / max_div) {
      display.drawPixel(xpos + i, ypos, 1);
    }
    ypos += 1;
    for (int i = 0; i <= barfill; i += barwid / min_div) {
      display.drawPixel(xpos + i, ypos, 1);
    }

    ypos += 2;
    display.setCursor(0, ypos);
    sprintf(buff, "%7.3f%%\n", lx_per * 100);
    //sprintf(buff, "%7.3f %2d", lx_per * 100,barfill);
    display.print(buff);

    // calculate change in lux
    float dlux = (lux - lux_last);
    sprintf(buff, "dL:%7.1f", dlux);
    Serial.println(buff);
    display.println(buff);

    // calculate index
    int idx = N % av_int;
    Serial.println(idx);

    if (idx > dispwid) {
      display.drawPixel(idx, 0, 1);
    }

    // save value to averaging array
    values[idx] = lux;

    float lux_sum = 0;
    int n = 0;
    for (int i = 0; i < av_int; i ++) {
      //Serial.println(values[i]);
      if (values[i] > 0 ) {
        lux_sum += values[i];
        n++;
      }
    }
    float lux_av  = lux_sum / n;
    sprintf(buff, "Av:%7.1f", lux_av);
    Serial.println(buff);
    display.println(buff);

    float dlux_av =  (lux_av - lux_av_last);
    sprintf(buff, "dA:%7.1f", dlux_av);
    Serial.println(buff);
    display.println(buff);

    lux_last = lux;
    lux_av_last = lux_av;

    /*
       PIR code
    */
    PIRState = digitalRead(PIR);

    if (PIRState == HIGH) {
      digitalWrite(BUILTIN_LED, LOW);  // LED on
      // is the light changing?
      if (abs(dlux) > thresh_inst) {
        sprintf(buff, "dlux = %f OVER THRESHOLD", dlux);
        Serial.println(buff);
        display.setTextSize(2);
        display.setCursor(1, 0);
        display.setTextColor(BLACK, WHITE);
        sprintf(buff, "TRIG1", dlux);
        display.println(buff);
      }
      // is the light changing?
      if (abs(dlux_av) > thresh_av) {
        sprintf(buff, "dlux_AV = %f OVER THRESHOLD", dlux_av);
        Serial.println(buff);
        display.setTextSize(2);
        display.setCursor(1, 0);
        display.setTextColor(BLACK, WHITE);
        sprintf(buff, "TRIG2", dlux);
        display.println(buff);
      }

    } else {
      digitalWrite(BUILTIN_LED, HIGH); // LED off
    }
  }
  else
  {
    display.println("Error!");
    display.println("BH1750 not found");
  }
  display.display();
  delay(LOOP_DELAY);
}
