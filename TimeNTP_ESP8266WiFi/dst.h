int isDST() {
  //setTime(8,0,0,1,1,2021);
  
  if (month()>3 && month()<11) {
    Serial.println("easy DST");
    return 1;
  } else if (month()==3) {
    Serial.print("it is ");
    Serial.print(monthStr(month()));
    Serial.println("tricky DST");
    if (day()<8) {
      Serial.println("too early, no DST");
      return 0;
    } else if (day()>14) {
      Serial.println("too late, no DST");
      return 1;
    
    } else {
      Serial.println("really tricky DST - code more");
      int daytest=weekday()+6-day();
      char* buff;
      sprintf(buff,"day test = %d\n",daytest);
      Serial.print(buff);
      if (daytest>0 && weekday()>1) {
      Serial.println("hard DST");
      return 1;
        
      } else if (weekday()==1) {
        Serial.println("hardest DST");
        Serial.println("date change day!");
        if (hour()<2) {
          Serial.println("wee morning");
          Serial.println("NO DST");
          return 0;
        } else {
          Serial.println("Happy DST!");
          return 1;
        }
      } else {
        return 0;
      }
    }
  } else if (month()==11) {
    Serial.print("it is ");
    Serial.print(monthStr(month()));
    Serial.println("tricky DST");
    if (day()>7) {
      Serial.println("too late, no DST");
      return 0;
    } else {
      Serial.println("really tricky DST - code more");
    }   
  } else {
    Serial.println("easy not DST");
    return 0;
  }
}
