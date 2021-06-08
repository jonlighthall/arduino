int isDST(int ddebug = 0); // set default function value
/*
 * Set debug level
 * 0 - No debugging: isDST() returns 1 for DST and 0 for NOT DST
 * 1 - Print DST string only: isDST() prints "DST" or "NOT DST"
 * 2 - Print all debug
 */
int debug=1;

int isDST(int debug) {
  char buff[50];
  sprintf(buff,"\nisDST() debug = %d\n",debug);
  if (debug>1)
    Serial.print(buff); 
  // first, test if we're in DST
  if (month()>3 && month()<11) {
    if (debug>1)
      Serial.print("easy: ");
    if (debug>0)
      Serial.print("DST");
    return 1;
  } else if (month()==3) {
    // next, test in March
    if (debug>1) {
      sprintf(buff,"It is %s %d, %d, tricky: ",monthStr(month()),day(),year());
      Serial.print(buff); 
    }
    if (day()<8) {
      if (debug>1)
        Serial.print("case 1: too early, ");
      if (debug>0)
	Serial.print("NOT DST"); 
      return 0;
    } else if (day()>14) {
      if (debug>1)
        Serial.print("case 2: too late, ");
      if (debug>0)
	Serial.print("DST");
      return 1;   
    } else {
      int daytest=day()-(weekday()+6);
      if (debug>1) {
        Serial.print("case 3: needs calculating: ");
        sprintf(buff,"day test = %d ",daytest);
        Serial.print(buff);     
      }
      if (daytest>0 && weekday()>1) {
	if (debug>1) {
	  Serial.print("case 3A : after second Sunday: ");
	  Serial.print(dayStr(weekday()));
	}
	if (debug>0)
	  Serial.print("DST");  
	return 1;       
      } else if (weekday()==1) {
        if (debug>1)
          Serial.print("case 3B: date change day! ");
        if (hour()<2) {
	  if (debug>1) 
	    Serial.print("wee morning: ");     	  
	  if (debug>0)
	    Serial.print("NOT DST");
          return 0;
        } else if (hour()==2) {
	  if (debug>1) {
	    Serial.println();
	    Serial.println("--------------------------------------");
	    Serial.println("Hello DST!");
	    Serial.println("--------------------------------------");
	  }
	  if (debug>0) 
	    Serial.print("*** START DST ***"); 
          return 1;
        } else {
	  if (debug>0)
	    Serial.print("DST");
          return 1;
        }
      } else {
	if (debug>1) {
	  Serial.print("case 3C: still too early: ");
	  Serial.print(dayStr(weekday()));
	  Serial.print(" ");
	}
	if (debug>0)
	  Serial.print("NOT DST");
        return 0;
      }
    }
  } else if (month()==11) {
    char buff[50];
    if (debug>1) {
      sprintf(buff,"It is %s %d, %d, tricky: ",monthStr(month()),day(),year());
      Serial.print(buff);
    }
    if (day()>7) {
      if (debug>1)
	Serial.print ("case 1: too late:");
      if (debug>0)
	Serial.print("NOT DST");
      return 0;
    } else {
      if (debug>1)
	Serial.print("case 2: needs calulating: ");
      int daytest=day()-(weekday()-1);
      if (debug>1) {
	sprintf(buff,"day test = %d ",daytest);
	Serial.print(buff);  
      }   
      if (daytest>0 && weekday()>1) {
	if (debug>1) {
	  Serial.print("case 3A : after first Sunday: ");
	  Serial.print(dayStr(weekday()));
	}
	if (debug>0)
	  Serial.print("NOT DST");
	return 0;       
      } else if (weekday()==1) {
	if (debug>1)
	  Serial.print("case 3B: date change day! ");
	if (hour()<2) {
	  if (debug>1) {
	    Serial.print("wee morning: ");
	  }
	  if (debug>0)
	    Serial.print("DST");
	  return 1;
	} else if (hour()==2) {
	  if (debug>1) {
	    Serial.println();
	    Serial.println("--------------------------------------");
	    Serial.println("Goodbye DST!");
	    Serial.println("--------------------------------------");
	  }
	  if (debug>0)
	    Serial.print("*** END DST ***");
	  return 0;
	} else {
	  if (debug>0)
	    Serial.print("NOT DST");
	  return 0;
	}
      }
      else {
	if (debug>1) {
	  Serial.print("case 3C: still too early: ");
	  Serial.print(dayStr(weekday()));
	  Serial.print(" ");
	}
        if (debug>0)
	  Serial.print("DST");
        return 1;
      }
    }   
  } else {
    if (debug>1)
      Serial.print("easy: ");
    if (debug>0)
      Serial.print("NOT DST");
    return 0;
  }
}

bool testTime(int hr, int dy, int mo, int yr) {
  setTime(hr,0,0,dy,mo,yr);
  char buff[50];
  sprintf(buff,"date is %02d.%02d.%4d %02d:%02d ",month(),day(),year(),hour(),minute());
  Serial.print(buff);
  sprintf(buff,": isDST() = %d\n",isDST(debug));
  if (debug>1) {
    Serial.print(buff);
    return isDST();
  }
  else
    Serial.println();
}

void testDST() {
  char buff[50];
  sprintf(buff,"\nTesting DST function...\n");
  Serial.print(buff);
  for (int yr = 2016; yr < 2025; yr++) {
    // Test March
    for (int dy = 7; dy < 16; dy++) {
      sprintf(buff,"dy = %d\n",dy);
      if (debug>1)
        Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
	testTime(hr,dy,3,yr);
      }
    }    
    // Test November
    for (int dy = 1; dy < 9; dy++) {
      sprintf(buff,"dy = %d\n",dy);
      if (debug>1)
	Serial.print(buff);
      for (int hr = 1; hr < 4; hr++) {
	testTime(hr,dy,11,yr);
      }
    }
  }   
}  
