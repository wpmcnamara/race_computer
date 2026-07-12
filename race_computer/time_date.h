#ifndef __TIMEDATE__
#define __TIMEDATE__
#include <Timezone.h>

extern TimeChangeRule gmtRule;         // Standard Time
extern Timezone GMT;

// UTC
extern TimeChangeRule utcRule;     // UTC
extern Timezone UTC;

// US Atlantic Time Zone (Puerto Rico, U.S. Virgin Islands)
extern TimeChangeRule usATRule;  // Atlantic Time = UTC - 4 hours
extern Timezone usAT;

// US Eastern Time Zone (New York, Detroit)
extern TimeChangeRule usEDT;  // Eastern Daylight Time = UTC - 4 hours
extern TimeChangeRule usEST;   // Eastern Standard Time = UTC - 5 hours
extern Timezone usET;

// US Central Time Zone (Chicago, Houston)
extern TimeChangeRule usCDT;
extern TimeChangeRule usCST;
extern Timezone usCT;

// US Mountain Time Zone (Denver, Salt Lake City)
extern TimeChangeRule usMDT;
extern TimeChangeRule usMST;
extern Timezone usMT;

// Arizona is US Mountain Time Zone but does not use DST
extern Timezone usAZ;

//Navajo Nation follows daylight Savings time with the rest of US Mountain Time.
extern Timezone usNN;

// US Pacific Time Zone (Las Vegas, Los Angeles)
extern TimeChangeRule usPDT;
extern TimeChangeRule usPST;
extern Timezone usPT;

// US Alaska Time Zone (Anchorage)
extern TimeChangeRule usAKDT;
extern TimeChangeRule usAKST;
extern Timezone usAKT;

// Hawaii
extern TimeChangeRule htRule;     // UTC
extern Timezone usHST;


class timeDateSettings {
    public:
        timeDateSettings();
        Timezone * getTz(void);
        int8_t getTzIndex(void);
        void incTz(void);
        void decTz(void);
        void setTzIndex(int8_t);
        String getTzDesc();
    private:
        int8_t tzIndex;
        std::array<Timezone *,11> tzList={
            &GMT,
            &UTC,
            &usAT,
            &usET,
            &usCT,
            &usMT,
            &usAZ,
            &usNN,
            &usPT,
            &usAKT,
            &usHST
        };
        std::array<String, 11> tzDescr={
            "Greenwich Mean Time",
            "Universal Time Coordinated",
            "US Atlantic Time",
            "US Eastern Time",
            "US Central Time",
            "US Mountain Time",
            "US Arizona Time",
            "US Navajo Nation Time",
            "US Pacific Time",
            "US Alaska Time",
            "US Hawaii Time"
        };

};

extern class timeDateSettings timeDateSettings;
#endif