/*
  ESP8266 Button Example

   1 Button Shield - Simple Push
   Press the pushbutton to switch on the LED

   1 Button Shield pushbutton connects pin D3 to GND
*/

#include <U8g2lib.h>

// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

const int buttonPin = D3;
const int ledPin = BUILTIN_LED;

int buttonState = 0;
int previousState = 0;

void setup() {
  Serial.begin(9600);

  // display settings
  u8g2.begin();
  u8g2.clearBuffer();      // clear the internal memory

  randomSeed(analogRead(0));
  char buff[64];
  sprintf(buff, "Button");
  Serial.println(buff);

  u8g2.setFont(u8g2_font_timB08_tr);  // choose a suitable font
  int texthei = u8g2.getAscent();
  int ypos = texthei;
  u8g2.drawStr(0, ypos, buff);        // write something to the internal memory
  u8g2.sendBuffer();      // transfer internal memory to the display
  delay(2000);

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);  // Initialize the LED_BUILTIN pin as an output

  // set initial state, LED off
  digitalWrite(ledPin, HIGH); // the LED is is active low on the ESP-01
}

void loop() {
  // read button state, HIGH when not pressed, LOW when pressed
  buttonState = digitalRead(buttonPin);
  // only update on state chagne
  if (buttonState != previousState) {
    u8g2.clearBuffer();
    char buff[64];
    u8g2.setFont(u8g2_font_timB08_tr);
    int texthei = u8g2.getAscent();
    int ypos = texthei;
    // if the push button pressed, switch on the LED
    if (buttonState == LOW) {
      digitalWrite(ledPin, LOW);  // LED on
      sprintf(buff, "on");
      Serial.println(buff);
      u8g2.drawStr(0, ypos, buff);
      u8g2.sendBuffer();
    } else {
      digitalWrite(ledPin, HIGH); // LED off
      sprintf(buff, "off");
      Serial.println(buff);
      u8g2.drawStr(0, ypos, buff);
      u8g2.sendBuffer();
    }
  }
  previousState = buttonState;
}
