int isDST() {
  //setTime(8,0,0,1,1,2021);
  
  if (month()>3 && month()<11) {
    Serial.println("easy DST");
    return 1;
  } else if (month()==3) {
    char buff[50];
    sprintf(buff,"It is %s %d, %d, tricky DST: ",monthStr(month()),day(),year());
    Serial.print(buff);
    
    if (day()<8) {
      Serial.println("case 1: too early, no DST");
      return 0;
    } else if (day()>14) {
      Serial.println("case 2: too late, no DST");
      return 1;
    
    } else {
      Serial.print(" case 3: needs calculating: ");
      int daytest=day()-(weekday()+6);
      sprintf(buff,"day test = %d ",daytest);
      Serial.print(buff);
      
      if (daytest>0 && weekday()>1) {
      Serial.print("case 3A : after second sunday: DST ");
           Serial.println(dayStr(weekday()));
      return 1;
        
      } else if (weekday()==1) {
        Serial.print("case 3B: date change day! ");
        if (hour()<2) {
          Serial.print("wee morning: ");
          Serial.println("NO DST");
          return 0;
        } else if (hour()==2) {
          Serial.println();
          Serial.println("--------------------------------------");
          Serial.println("Happy DST!");
          Serial.println("--------------------------------------");
          return 1;
        } else {
          Serial.println("DST!");
          return 1;
        }
      } else {
        Serial.print("case 3C: still too early, no DST ");
        Serial.println(dayStr(weekday()));
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

void testDST() {
  char buff[50];
  for (int yr = 2016; yr < 2025; yr++) {
    for (int i = 7; i < 16; i++) {
      sprintf(buff,"i = %d\n",i);
      Serial.print(buff);
        for (int hr = 1; hr < 4; hr++) {
          setTime(hr,0,0,i,3,yr);
          sprintf(buff,"date is %d.%d.%d %02d:%02d\n",month(),day(),year(),hour(),minute());
          Serial.print(buff);
          isDST();
        }
    }

    for (int i = 1; i < 9; i++) {
      sprintf(buff,"i = %d\n",i);
      Serial.print(buff);
        for (int hr = 1; hr < 4; hr++) {
          setTime(hr,0,0,i,11,yr);
          sprintf(buff,"date is %d.%d.%d %02d:%02d\n",month(),day(),year(),hour(),minute());
          Serial.print(buff);
          isDST();
        }
    }
    
  }  
}
