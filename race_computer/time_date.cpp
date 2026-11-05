#include "helpers.h"
#include "time_date.h"

TimeChangeRule gmtRule = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone GMT(gmtRule);

// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);

// US Atlantic Time Zone (Puerto Rico, U.S. Virgin Islands)
TimeChangeRule usATRule = {"AST", Second, Sun, Mar, 1, -240};  // Atlantic Time = UTC - 4 hours
Timezone usAT(usATRule);

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
Timezone usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
Timezone usMT(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
Timezone usAZ(usMST);

//Navajo Nation follows daylight Savings time with the rest of US Mountain Time.
Timezone usNN(usMDT, usMST);

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);

// US Alaska Time Zone (Anchorage)
TimeChangeRule usAKDT = {"AKDT", Second, Sun, Mar, 2, -480};
TimeChangeRule usAKST = {"AKST", First, Sun, Nov, 2, -540};
Timezone usAKT(usAKDT, usAKST);

// Hawaii
TimeChangeRule htRule = {"HST", Last, Sun, Mar, 1, -600};     // UTC
Timezone usHST(htRule);

class timeDateSettings timeDateSettings;

timeDateSettings::timeDateSettings() {
    tzIndex=4; //default to Central Time, if there is no saved timezone
}

Timezone * timeDateSettings::getTz(void) {
    return tzList[tzIndex];
}

int8_t timeDateSettings::getTzIndex(void) { return tzIndex; }

void timeDateSettings::incTz(void) {
    tzIndex++;
    if(tzIndex==tzList.size()) {
        tzIndex=0;
    }
}

void timeDateSettings::decTz(void) {
    tzIndex--;
    if(tzIndex<0) {
        tzIndex=tzList.size()-1;
    }
}

void timeDateSettings::setTzIndex(int8_t index) {
    if(tzIndex<tzList.size() && tzIndex>=0) {
        tzIndex=index;
    } else {
        //pass it an invalid timezone and you get set to CST/CDT
        tzIndex=4;
    }
}


String timeDateSettings::getTzDesc() {
    return tzDescr[tzIndex];
}