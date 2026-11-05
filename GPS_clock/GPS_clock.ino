// First Include the Library
#include <CK_MAX.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>


#define LED_CS 7
#define PPS_PIN 19
#define GPS_RESET_PIN 20
#define GPS_CS 14

#define IMR_INDEX   5
#define ISR_INDEX   6

CK_MAX ledDisp(LED_CS);
SFE_UBLOX_GNSS GPS;
IntervalTimer it1;

volatile uint32_t tick=0;
bool tickRun=false;

char buffer[32];

struct gpsDataStruct {
  double lat;
  double lon;
  int siv;
  uint8_t fix;
  bool timeValid;
  bool fixValid;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  //GPS speed in um/ms
  double speed;
  //GPS odometer distance in um.
  double distance;
} gpsData;

inline void disableInterrupt(uint8_t pin)
{
	if (pin >= CORE_NUM_DIGITAL) return;
	volatile uint32_t *gpio = portOutputRegister(pin);
	uint32_t mask = digitalPinToBitMask(pin);
	gpio[IMR_INDEX] &= ~mask;
}

inline void enableInterrupt(uint8_t pin)
{
	if (pin >= CORE_NUM_DIGITAL) return;
	volatile uint32_t *gpio = portOutputRegister(pin);
	uint32_t mask = digitalPinToBitMask(pin);
  gpio[ISR_INDEX] = mask;  // clear any prior pending interrupt
	gpio[IMR_INDEX] |= mask; // enable interrupt
}

void ppsInterrupt() {
  tick=0;
  if(gpsData.timeValid) {
    setTime(gpsData.hour, gpsData.minute, gpsData.second+1, gpsData.day, gpsData.month, gpsData.year);
  }    
}

void it1cb() {
  tick++;
}

void autoPVT(UBX_NAV_PVT_data_t ubxDataStruct) {
  gpsData.fix = ubxDataStruct.fixType;
  gpsData.siv = ubxDataStruct.numSV;
  gpsData.lat = ubxDataStruct.lat / 10000000.0;
  gpsData.lon = ubxDataStruct.lon / 10000000.0;
  gpsData.fixValid=ubxDataStruct.flags.bits.gnssFixOK;
  gpsData.year = ubxDataStruct.year;
  gpsData.month = ubxDataStruct.month;
  gpsData.day = ubxDataStruct.day;
  gpsData.hour = ubxDataStruct.hour;
  gpsData.minute = ubxDataStruct.min;
  gpsData.second = ubxDataStruct.sec;
  gpsData.timeValid=ubxDataStruct.valid.bits.validTime;
}

void setup() {
    pinMode(PPS_PIN, INPUT);
    pinMode(GPS_RESET_PIN, OUTPUT);
    digitalWrite(GPS_RESET_PIN, HIGH);
    pinMode(LED_CS, OUTPUT);
    digitalWrite(LED_CS, HIGH);
    pinMode(GPS_CS, OUTPUT);
    digitalWrite(GPS_CS, HIGH);
    //reset GPS modules
    digitalWrite(GPS_RESET_PIN, LOW);
    delay(100);
    digitalWrite(GPS_RESET_PIN, HIGH);
	  Serial.begin(1000000);
    
    ledDisp.begin();
    ledDisp.Set_Brightness(3);
    ledDisp.AutoRefresh(false);

    if (GPS.begin(SPI, GPS_CS, 2000000) == false) //Connect to the u-blox module using Wire port
    {
        Serial.println(F("u-blox GNSS not detected on SPI bus. Please check wiring. Freezing."));
        while (1);
    }
    Serial.print(F("NEO-8MU protocol version:"));
    GPS.getProtocolVersion();
    Serial.print(GPS.getProtocolVersionHigh());
    Serial.print(F("."));
    Serial.println(GPS.getProtocolVersionLow());

    GPS.getPowerSaveMode();
    
    GPS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
    GPS.setPortOutput(COM_PORT_SPI, COM_TYPE_UBX); 
    GPS.setMeasurementRate(1);
    GPS.setNavigationRate(1); 
    GPS.setAutoPVTcallback(&autoPVT);
    
    
    

    attachInterrupt(digitalPinToInterrupt(PPS_PIN),ppsInterrupt, RISING);
    //it1.begin(it1cb, 10000);
}

void loop() {
    if (GPS.checkUblox())  {
        GPS.checkCallbacks();
    }
    if(gpsData.timeValid && !tickRun) {
      tickRun=true;
      it1.begin(it1cb, 10000);
    }
    sprintf(buffer, "%02d.%02d", hour(), minute());
    ledDisp.Set_Position(0);
    ledDisp.ShowMe(buffer);
    sprintf(buffer, "%02d.%02d", minute(), second());
    ledDisp.Set_Position(2);
    ledDisp.ShowMe(buffer); 
    sprintf(buffer, "%02d.%02ld", second(), tick%100);
    ledDisp.Set_Position(4);
    ledDisp.ShowMe(buffer);     
}