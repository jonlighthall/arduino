int isDST(int debug=0) {
  // first, test if we're in DST
  if (month()>3 && month()<11) {
    if (debug)
      Serial.println("easy: DST");
    return 1;
  } else if (month()==3) {
    // next, test in March
    char buff[50];
    if (debug) {
      sprintf(buff,"It is %s %d, %d, tricky: ",monthStr(month()),day(),year());
      Serial.print(buff); 
    }
    if (day()<8) {
      if (debug)
        Serial.println("case 1: too early, NO DST");
      return 0;
    } else if (day()>14) {
      if (debug)
        Serial.println("case 2: too late, DST");
      return 1;   
    } else {
      if (debug)
        Serial.print(" case 3: needs calculating: ");
      int daytest=day()-(weekday()+6);
      if (debug) {
        sprintf(buff,"day test = %d ",daytest);
        Serial.print(buff);     
      }
      if (daytest>0 && weekday()>1) {
	if (debug) {
	  Serial.print("case 3A : after second Sunday: DST ");
	  Serial.println(dayStr(weekday()));
	}
	return 1;       
      } else if (weekday()==1) {
        if (debug)
          Serial.print("case 3B: date change day! ");
        if (hour()<2) {
	  if (debug) {

	    Serial.print("wee morning: ");
	    Serial.println("NO DST");
	  }
          return 0;
        } else if (hour()==2) {
	  if (debug) {

	    Serial.println();
	    Serial.println("--------------------------------------");
	    Serial.println("Hello DST!");
	    Serial.println("--------------------------------------");
	  }
          return 1;
        } else {
	  if (debug)
	    Serial.println("DST");
          return 1;
        }
      } else {
	if (debug) {

	  Serial.print("case 3C: still too early: NO DST ");
	  Serial.println(dayStr(weekday()));
	}
        return 0;
      }
    }
  } else if (month()==11) {
    char buff[50];
    if (debug) {

      sprintf(buff,"It is %s %d, %d, tricky: ",monthStr(month()),day(),year());
      Serial.print(buff);
    }
    if (day()>7) {
      if (debug)
	Serial.println ("case 1: too late: NO DST");
      return 0;
    } else {
      if (debug)
	Serial.println("case 2: needs calulating: ");
      int daytest=day()-(weekday()-1);
      if (debug) {
	sprintf(buff,"day test = %d ",daytest);
	Serial.print(buff);  
      }   
      if (daytest>0 && weekday()>1) {
	if (debug) {

	  Serial.print("case 3A : after first Sunday: NO DST ");
	  Serial.println(dayStr(weekday()));
	}
	return 0;       
      } else if (weekday()==1) {
	if (debug)
	  Serial.print("case 3B: date change day! ");
	if (hour()<2) {
	  if (debug) {

	    Serial.print("wee morning: ");
	    Serial.println("DST");
	  }
	  return 1;
	} else if (hour()==2) {
	  if (debug) {
	    Serial.println();
	    Serial.println("--------------------------------------");
	    Serial.println("Goodbye DST!");
	    Serial.println("--------------------------------------");
	  }
	  return 0;
	} else {
	  if (debug)
	    Serial.println("NO DST");
	  return 0;
	}
      }
      else {
	if (debug) {
	  Serial.print("case 3C: still too early: DST ");
	  Serial.println(dayStr(weekday()));
	}
        return 1;
      }
    }   
  } else {
    if (debug)
      Serial.println("easy: NO DST");
    return 0;
  }
}

void testDST() {
  char buff[50];
  for (int yr = 2016; yr < 2025; yr++) {
    // Test March
    for (int i = 7; i < 16; i++) {
      sprintf(buff,"i = %d\n",i);
      Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
	setTime(hr,0,0,i,3,yr);
	sprintf(buff,"date is %d.%d.%d %02d:%02d\n",month(),day(),year(),hour(),minute());
	Serial.print(buff);
	sprintf(buff,"isDST() = %d\n",isDST());
	Serial.print(buff);
      }
    }
    // Test November
    for (int i = 1; i < 9; i++) {
      sprintf(buff,"i = %d\n",i);
      Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
	setTime(hr,0,0,i,11,yr);
	sprintf(buff,"date is %d.%d.%d %02d:%02d\n",month(),day(),year(),hour(),minute());
	Serial.print(buff);
	sprintf(buff,"isDST() = %d\n",isDST());
	Serial.print(buff);
      }
    }
    
  }  
}
