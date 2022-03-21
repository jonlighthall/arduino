int isDST(int default_debugDST = 0); // set default function value
/*
   Debug level
   0 - No debugging: isDST() returns 1 for DST and 0 for NOT DST
   1 - Print DST string only: isDST() prints "DST" or "NOT DST"
   2 - Print all debug
*/

int isDST(int debugDST) {
  char buff[50];
  sprintf(buff, "\nisDST() debugDST = %d\n", debugDST);
  if (debugDST > 1)
    Serial.print(buff);
  // first, test if we're in DST
  if (month() > 3 && month() < 11) {
    if (debugDST > 1)
      Serial.print("easy: ");
    if (debugDST > 0)
      Serial.print("DST");
    return 1;
  } else if (month() == 3) {
    // next, test in March
    if (debugDST > 1) {
      sprintf(buff, "It is %s %d, %d, tricky: ", monthStr(month()), day(), year());
      Serial.print(buff);
    }
    if (day() < 8) {
      if (debugDST > 1)
        Serial.print("case 1: too early, ");
      if (debugDST > 0)
        Serial.print("NOT DST");
      return 0;
    } else if (day() > 14) {
      if (debugDST > 1)
        Serial.print("case 2: too late, ");
      if (debugDST > 0)
        Serial.print("DST");
      return 1;
    } else {
      int daytest = day() - (weekday() + 6);
      if (debugDST > 1) {
        Serial.print("case 3: needs calculating: ");
        sprintf(buff, "day test = %d ", daytest);
        Serial.print(buff);
      }
      if (daytest > 0 && weekday() > 1) {
        if (debugDST > 1) {
          Serial.print("case 3A : after second Sunday: ");
          Serial.print(dayStr(weekday()));
        }
        if (debugDST > 0)
          Serial.print("DST");
        return 1;
      } else if (weekday() == 1) {
        if (debugDST > 1)
          Serial.print("case 3B: date change day! ");
        if (hour() < 2) {
          if (debugDST > 1)
            Serial.print("wee morning: ");
          if (debugDST > 0)
            Serial.print("NOT DST");
          return 0;
        } else if (hour() == 2) {
          if (debugDST > 1) {
            Serial.println();
            Serial.println("--------------------------------------");
            Serial.println("Hello DST!");
            Serial.println("--------------------------------------");
          }
          if (debugDST > 0)
            Serial.print("*** START DST ***");
          return 1;
        } else {
          if (debugDST > 0)
            Serial.print("DST");
          return 1;
        }
      } else {
        if (debugDST > 1) {
          Serial.print("case 3C: still too early: ");
          Serial.print(dayStr(weekday()));
          Serial.print(" ");
        }
        if (debugDST > 0)
          Serial.print("NOT DST");
        return 0;
      }
    }
  } else if (month() == 11) {
    char buff[50];
    if (debugDST > 1) {
      sprintf(buff, "It is %s %d, %d, tricky: ", monthStr(month()), day(), year());
      Serial.print(buff);
    }
    if (day() > 7) {
      if (debugDST > 1)
        Serial.print ("case 1: too late:");
      if (debugDST > 0)
        Serial.print("NOT DST");
      return 0;
    } else {
      if (debugDST > 1)
        Serial.print("case 2: needs calulating: ");
      int daytest = day() - (weekday() - 1);
      if (debugDST > 1) {
        sprintf(buff, "day test = %d ", daytest);
        Serial.print(buff);
      }
      if (daytest > 0 && weekday() > 1) {
        if (debugDST > 1) {
          Serial.print("case 3A : after first Sunday: ");
          Serial.print(dayStr(weekday()));
        }
        if (debugDST > 0)
          Serial.print("NOT DST");
        return 0;
      } else if (weekday() == 1) {
        if (debugDST > 1)
          Serial.print("case 3B: date change day! ");
        if (hour() < 2) {
          if (debugDST > 1) {
            Serial.print("wee morning: ");
          }
          if (debugDST > 0)
            Serial.print("DST");
          return 1;
        } else if (hour() == 2) {
          if (debugDST > 1) {
            Serial.println();
            Serial.println("--------------------------------------");
            Serial.println("Goodbye DST!");
            Serial.println("--------------------------------------");
          }
          if (debugDST > 0)
            Serial.print("*** END DST ***");
          return 0;
        } else {
          if (debugDST > 0)
            Serial.print("NOT DST");
          return 0;
        }
      }
      else {
        if (debugDST > 1) {
          Serial.print("case 3C: still too early: ");
          Serial.print(dayStr(weekday()));
          Serial.print(" ");
        }
        if (debugDST > 0)
          Serial.print("DST");
        return 1;
      }
    }
  } else {
    if (debugDST > 1)
      Serial.print("easy: ");
    if (debugDST > 0)
      Serial.print("NOT DST");
    return 0;
  }
}

bool testTime(int hr, int dy, int mo, int yr, int dbg) {
  setTime(hr, 0, 0, dy, mo, yr);
  char buff[50];
  sprintf(buff, "date is %02d.%02d.%4d %02d:%02d ", month(), day(), year(), hour(), minute());
  Serial.print(buff);
  sprintf(buff, "(test time debug = %d) ", dbg);
  if (dbg > 1)
    Serial.print(buff);
  sprintf(buff, ": isDST() = %d\n", isDST(dbg));
  if (dbg > 1) {
    Serial.print(buff);
  } else {
    Serial.println();
  }
  return isDST();
}

void testDST(int default_test_debugDST = 1); // set default function value

void testDST(int test_debugDST) {
  char buff[50];
  sprintf(buff, "\nTesting DST function...\n");
  Serial.print(buff);
  sprintf(buff, "test DST debug = %d\n", test_debugDST);
  Serial.print(buff);
  for (int yr = 2020; yr < 2021; yr++) {
    // Test March
    for (int dy = 7; dy < 16; dy++) {
      sprintf(buff, "dy = %d\n", dy);
      if (test_debugDST > 1)
        Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
        testTime(hr, dy, 3, yr, test_debugDST);
      }
    }
    // Test November
    for (int dy = 1; dy < 9; dy++) {
      sprintf(buff, "dy = %d\n", dy);
      if (test_debugDST > 1)
        Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
        testTime(hr, dy, 11, yr, test_debugDST);
      }
    }
  }
}
