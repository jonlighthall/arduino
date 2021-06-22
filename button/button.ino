/*
  ESP8266 Button Example

   1 Button Shield - Simple Push
   Press the pushbutton to switch on the LED

   1 Button Shield pushbutton connects pin D3 to GND
*/

const int buttonPin = D3;
const int ledPin = BUILTIN_LED;

int buttonState = 0;
int previousState = 0;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  char buff[64];
  sprintf(buff, "\nButton");
  Serial.println(buff);

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
    // if the push button pressed, switch on the LED
    if (buttonState == LOW) {
      digitalWrite(ledPin, LOW);  // LED on
      Serial.println("on");
    } else {
      digitalWrite(ledPin, HIGH); // LED off
      Serial.println("off");
    }
  }
  previousState = buttonState;
}
