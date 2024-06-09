const int relayPin = D1;
const int PIR = D2; // don't use default D3; it is pulled up and boot will fail if pulled low
int PIRState = 0;

void setup() {
  // initialize on-board LED
  //pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  //digitalWrite(LED_BUILTIN,HIGH);  // Turn the LED off by making the voltage HIGH

  // initialize PIR
  pinMode(PIR, INPUT);

  // initialize relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // initialize Serial
  Serial.begin(9600);
  while (!Serial)
    ;  // Needed for Leonardo only
  // Serial welcome message
  delay(250);
  Serial.println();
  Serial.println("---------------");
  char buff[64];
  sprintf(buff, "PIR Relay Example");
  Serial.println(buff);
  Serial.println("---------------");
}

//int last_state = !PIRState;

void loop() {
  PIRState = digitalRead(PIR);
  if (PIRState == HIGH) {
    //if (PIRState != last_state) {
      Serial.println("HIGH: turning relay ON");
      digitalWrite(relayPin, HIGH); // turn relay on
  //    digitalWrite(LED_BUILTIN, LOW); // turn LED on

    //}
  } else {
    //if (PIRState != last_state) {

      Serial.println(" LOW: turning relay OFF");
      digitalWrite(relayPin, LOW); // turn relay off
      //digitalWrite(LED_BUILTIN, HIGH); // turn LED off
    //}
  }
       Serial.println("      waiting...");
      delay(1000 * 1.5);
//  last_state = PIRState;
}
