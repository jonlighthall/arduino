long randNumber;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  char buff[64];
  sprintf(buff, "\nDice Example");
  Serial.println(buff);
}

void loop() {
  int j = random(6) + 1;
  int k = random(6) + 1;
  char buff[64];
  sprintf(buff, "%d + %d = %2d", j, k, j + k);
  Serial.print (buff);
  if (j == k) {
    Serial.println(" DOUBLE 1");
    j = random(6) + 1;
    k = random(6) + 1;
    sprintf(buff, "%d + %d = %2d", j, k, j + k);
    Serial.print (buff);
    if (j == k) {
      Serial.println(" DOUBLE 2");
      j = random(6) + 1;
      k = random(6) + 1;
      sprintf(buff, "%d + %d = %2d", j, k, j + k);
      Serial.print (buff);
      if (j == k) {
        Serial.println(" DOUBLE 3 - JAIL");
        delay(2000);
      }
      else
        Serial.println();
    }
    else
      Serial.println();
  }
  else
    Serial.println();
  delay(50);
  Serial.println();
}
