const int relayPin = D1;
const int PIR = D3;
int PIRState = 0;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(PIR, INPUT);
  digitalWrite(relayPin, LOW);
}

void loop() {
  PIRState = digitalRead(PIR);
  if (PIRState == HIGH) {
    digitalWrite(relayPin, HIGH); // turn on relay
    delay(1000*30);
  } else {
    digitalWrite(relayPin, LOW); // turn off relay
  }
}
