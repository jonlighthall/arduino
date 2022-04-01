const int relayPin = D1;
const int PIR = D3;
int PIRState = 0;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(relayPin, LOW);
}

void loop() {
  PIRState = digitalRead(PIR);
  if (PIRState == HIGH) {
    digitalWrite(BUILTIN_LED, LOW);  // LED on
    digitalWrite(relayPin, HIGH); // turn on relay with voltage HIGH
  } else {
    digitalWrite(BUILTIN_LED, HIGH); // LED off
    digitalWrite(relayPin, LOW);  // turn off relay with voltage LOW
  }
}
