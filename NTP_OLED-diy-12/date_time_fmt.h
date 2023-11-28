/*
 * The time structure is called tm and has teh following values:

  Definition of struct tm:
  Member  Type  Meaning Range
  tm_sec  int seconds after the minute  0-61*
  tm_min  int minutes after the hour  0-59
  tm_hour int hours since midnight  0-23
  tm_mday int day of the month  1-31
  tm_mon  int months since January  0-11
  tm_year int years since 1900
  tm_wday int days since Sunday 0-6
  tm_yday int days since January 1  0-365
  tm_isdst  int Daylight Saving Time flag

  because the values are somhow akwardly defined, I introduce a function
 makeHumanreadable() where all values are adjusted according normal numbering.
  e.g. January is month 1 and not 0 And Sunday or monday is weekday 1 not 0
 (according definition of MONDAYFIRST)

 */

char* getDOW(uint8_t tm_wday) {
  switch (tm_wday) {
    case 1:
      return (char*)"Monday";
      break;
    case 2:
      return (char*)"Tuesday";
      break;
    case 3:
      return (char*)"Wednesday";
      break;
    case 4:
      return (char*)"Thursday";
      break;
    case 5:
      return (char*)"Friday";
      break;
    case 6:
      return (char*)"Saturday";
      break;
    case 7:
      return (char*)"Sunday";
      break;
    default:
      return (char*)"Error";
      break;
  }
}

char* getMo(uint8_t tm_mon) {
  switch (tm_mon) {
    case 1:
      return (char*)"Jan";
      break;
    case 2:
      return (char*)"Feb";
      break;
    case 3:
      return (char*)"Mar";
      break;
    case 4:
      return (char*)"Apr";
      break;
    case 5:
      return (char*)"May";
      break;
    case 6:
      return (char*)"Jun";
      break;
    case 7:
      return (char*)"Jul";
      break;
    case 8:
      return (char*)"Aug";
      break;
    case 9:
      return (char*)"Sep";
      break;
    case 10:
      return (char*)"Oct";
      break;
    case 11:
      return (char*)"Nov";
      break;
    case 12:
      return (char*)"Dec";
      break;
    default:
      return (char*)"Error";
      break;
  }
}
