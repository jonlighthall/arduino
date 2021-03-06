#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WEMOS_DHT12.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

DHT12 dht12;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {

  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  if (dht12.get() == 0) {
    display.println("T: ");
    display.setTextSize(2);
    display.println(dht12.cTemp);

    display.setTextSize(1);
    display.println("H: ");
    display.setTextSize(2);
    display.println(dht12.humidity);
  }
  else
  {
    display.println("Error!");
  }
  display.display();
  delay(1000);
}
