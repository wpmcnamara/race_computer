#ifndef Local_time_h
#define Local_time_h
#include "Arduino.h"


int id_of_first_colon = 0, id_of_second_colon =0,id_of_first_space = 0, id_of_second_space=0, nowSec =0 , nowMin = 0, nowHr = 0;

int HOUR(){
  String nowTime = String(__TIME__);
  id_of_first_colon = nowTime.indexOf(':');
  id_of_second_colon = nowTime.indexOf( ':',id_of_first_colon+1);
  nowHr = (nowTime.substring(0, id_of_first_colon)).toInt();
 return nowHr;
}

int MINUTE(){
  String nowTime = String(__TIME__);
  id_of_first_colon = nowTime.indexOf(':');
  id_of_second_colon = nowTime.indexOf(':',id_of_first_colon+1);
  nowMin = (nowTime.substring(id_of_first_colon+1, id_of_second_colon)).toInt();
  return nowMin;
}


int SECOND(){
  String nowTime = String(__TIME__);
  id_of_first_colon = nowTime.indexOf(':');
  id_of_second_colon = nowTime.indexOf(':',id_of_first_colon+1);
  nowSec  = (nowTime.substring(id_of_second_colon+1, nowTime.length())).toInt();
  return nowSec;
}


String MONTH(){
  String nowDate = String(__DATE__);
  id_of_first_space = nowDate.indexOf(' ');
  id_of_second_space = nowDate.indexOf( ' ',id_of_first_space+1);
  return (nowDate.substring(0, id_of_first_space));
}

int DAY(){
  String nowDate = String(__DATE__);
  id_of_first_space = nowDate.indexOf(' ');
  id_of_second_space = nowDate.indexOf(' ',id_of_first_space+1);
  return (nowDate.substring(id_of_first_space+1, id_of_second_space+1).toInt());
}

int YEAR(){
  String nowDate = String(__DATE__);
  id_of_first_space = nowDate.indexOf(' ');
  id_of_second_space = nowDate.indexOf( ' ',id_of_first_space+1);
  return (nowDate.substring(id_of_second_space+1, nowDate.length()).toInt());
}

#endif

