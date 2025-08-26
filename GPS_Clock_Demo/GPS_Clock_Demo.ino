/******
 * .......Clock Test............N.B. : This example is just for testing purpose. Use RTC for awsome Clock.
 *  Example of "CK_MAX" Version : 1.0
 *  Author : Chandan Kumar Mondal
 *  Date : 28/04/2020
 *  
 *  See the result in, CK_MAX/media/results/Result_of_Clock_Check.bmp
 */



// First Include the Library
#include <CK_MAX.h>
#include "Local_time.h"
#include <U8g2lib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_NeoKey_1x4.h"
#include "seesaw_neopixel.h"
#include "Adafruit_HUSB238.h"

Adafruit_HUSB238 husb238;
// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

Adafruit_NeoKey_1x4 neokey;  // Create the NeoKey object

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

SFE_UBLOX_GNSS myGNSS;
IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.
#define IMR_INDEX   5
#define ISR_INDEX   6
// Object... Load or CS pin connected to 10
#define Load 14
#define PPS_PIN 9
#define START_STOP_PIN 5
#define GPS_INT_PIN 4
#define KEY_INT_PIN 0
#define GPS_RESET_PIN 23
#define SD_CS 1
CK_MAX LED(Load);
#define packetLength 36
UBX_TIM_TM2_data_t *startStopTimeStamp=NULL;

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oled1(U8G2_R0, /* cs=*/ 10, /* dc=*/ 16, /* reset=*/ 22);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oled2(U8G2_R0, /s* cs=*/ 8, /* dc=*/ 16, /* reset=*/ 21);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oled3(U8G2_R0, /* cs=*/ 7, /* dc=*/ 16, /* reset=*/ 20);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oled4(U8G2_R0, /* cs=*/ 6, /* dc=*/ 16, /* reset=*/ 15);

// Hold the current time
uint32_t Current_time;
volatile bool refresh=false;
volatile bool timer_run=false;
volatile bool key_press=false;
volatile bool start_stop_debounce=false;
int debounce_count=0;
IntervalTimer it1;
int Hour, Minute, Second;
unsigned long tick=0;

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
  TMRx->CH[2].SCTRL  &= ~(TMR_SCTRL_TCF);  // clear
  TMRx->CH[2].CSCTRL &= ~(TMR_CSCTRL_TCF1);
  tick++;
}

void keyPressInt() {
  key_press=true;
}

void it1cb() {
  refresh = true;
}

byte fix;

void startStopInt() {
  if(!timer_run) {
    timer_run=true;
    TMRx->CH[2].CNTR = 0;
    tick=0;
    TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH;
    digitalWriteFast(GPS_INT_PIN, LOW);
  } else {
    timer_run=false;
    TMRx->CH[2].CTRL = 0;
    digitalWriteFast(GPS_INT_PIN, LOW);
  }
  disableInterrupt(START_STOP_PIN);
  //detachInterrupt(START_STOP_PIN);
  start_stop_debounce=true;
  debounce_count=0;
}

void TIMTM2dataCallback(UBX_TIM_TM2_data_t *ubxDataStruct)
{
  startStopTimeStamp=ubxDataStruct;
  Serial.println("TIMTM2dataCallback");
  Serial.println();

  Serial.print(F("newFallingEdge: ")); // 1 if a new falling edge was detected
  Serial.print(ubxDataStruct->flags.bits.newFallingEdge);

  Serial.print(F(" newRisingEdge: ")); // 1 if a new rising edge was detected
  Serial.print(ubxDataStruct->flags.bits.newRisingEdge);

  Serial.print(F(" Rising Edge Counter: ")); // Rising edge counter
  Serial.print(ubxDataStruct->count);

  Serial.print(F(" towMsR: ")); // Time Of Week of rising edge (ms)
  Serial.print(ubxDataStruct->towMsR);

  Serial.print(F(" towSubMsR: ")); // Millisecond fraction of Time Of Week of rising edge in nanoseconds
  Serial.print(ubxDataStruct->towSubMsR);

  Serial.print(F(" towMsF: ")); // Time Of Week of falling edge (ms)
  Serial.print(ubxDataStruct->towMsF);

  Serial.print(F(" towSubMsF: ")); // Millisecond fraction of Time Of Week of falling edge in nanoseconds
  Serial.println(ubxDataStruct->towSubMsF);

}

void setup(){
  int cnt;
  //startStopTimeStamp = new uint8_t[packetLength]; 
  pinMode(PPS_PIN, INPUT);
  pinMode(START_STOP_PIN, INPUT_PULLUP);
  pinMode(KEY_INT_PIN, INPUT);
  pinMode(KEY_INT_PIN, INPUT_PULLUP);
  pinMode(GPS_INT_PIN, OUTPUT);
  digitalWrite(GPS_INT_PIN, HIGH);
  pinMode(GPS_RESET_PIN, OUTPUT);
  //reset GPS modules
  digitalWrite(GPS_RESET_PIN, LOW);
  delay(50);
  digitalWrite(GPS_RESET_PIN, HIGH);
  // take a shortcut
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);

  // Initialize the HUSB238
  if (husb238.begin(HUSB238_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("HUSB238 initialized successfully.");
  } else {
    Serial.println("Couldn't find HUSB238, check your wiring?");
    while (1);
  }

  if (! neokey.begin(0x30)) {     // begin with I2C address, default is 0x30
    Serial.println("Could not start NeoKey, check wiring?");
    while(1) delay(10);
  }
  
  //LED.DefaultSettings();
  LED.begin();
  LED.Set_Brightness(3);
  // Assigning the time
  Current_time = millis();

  // Turn Off the Auto Refresh
  LED.AutoRefresh(false);

  oled1.begin();
  oled2.begin();
  oled3.begin();
  oled4.begin();

    Serial.print("\nInitializing SD card...");


  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  //if (!card.init(SPI_HALF_SPEED, SD_CS)) {
if (!card.init(1000000, SD_CS)) {    
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    //return;
  } else {
   Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    //return;
  } else {
    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();
    
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    if (volumesize < 8388608ul) {
      Serial.print("Volume size (bytes): ");
      Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
    }
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 2;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
  }
  //myGNSS.setFileBufferSize(109); // setFileBufferSize must be called _before_ .begin
  if (myGNSS.begin(SPI, 17, 1000000) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected on SPI bus. Please check wiring. Freezing."));
    while (1);
  }
  Serial.print(F("NEO-8MU protocol version:"));
  myGNSS.getProtocolVersion();
  Serial.print(myGNSS.getProtocolVersionHigh());
  Serial.print(F("."));
  Serial.println(myGNSS.getProtocolVersionLow());

  myGNSS.getPowerSaveMode();
  
  myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
  myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_SBAS);
  myGNSS.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);
  myGNSS.setPortOutput(COM_PORT_SPI, COM_TYPE_UBX); //Set the SPI port to output UBX only (turn off NMEA noise)
  myGNSS.setNavigationFrequency(20); //Set output to 10 times a second
  myGNSS.setAutoTIMTM2callbackPtr(&TIMTM2dataCallback);
  myGNSS.logTIMTM2(); // Enable TIM TM2 data logging
  // Create storage for the time pulse parameters
  UBX_CFG_TP5_data_t timePulseParameters;

  // Get the time pulse parameters
  if (myGNSS.getTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("getTimePulseParameters failed! Freezing..."));
    while (1) ; // Do nothing more
  }

  // Print the CFG TP5 version
  Serial.print(F("UBX_CFG_TP5 version: "));
  Serial.println(timePulseParameters.version);

  //myGNSS.factoryDefault(); delay(5000);
  //Serial.println(F("reset complete")); while(1);

  //timePulseParameters.tpIdx = 0; // Select the TIMEPULSE pin
  timePulseParameters.tpIdx = 1; // Or we could select the TIMEPULSE2 pin instead, if the module has one

  // We can configure the time pulse pin to produce a defined frequency or period
  // Here is how to set the frequency:

  // While the module is _locking_ to GNSS time, turn off output
  timePulseParameters.freqPeriod = 0; // Set the frequency/period to 0Hz
  timePulseParameters.pulseLenRatio = 0x55555555; // Set the pulse ratio to 1/3 * 2^32 to produce 33:67 mark:space

  // When the module is _locked_ to GNSS time, make it generate 10Hz
  timePulseParameters.freqPeriodLock = 1000; // Set the frequency/period to 10Hz
  timePulseParameters.pulseLenRatioLock = 0x80000000; // Set the pulse ratio to 1/2 * 2^32 to produce 50:50 mark:space

  timePulseParameters.flags.bits.active = 1; // Make sure the active flag is set to enable the time pulse. (Set to 0 to disable.)
  timePulseParameters.flags.bits.lockedOtherSet = 1; // Tell the module to use freqPeriod while locking and freqPeriodLock when locked to GNSS time
  timePulseParameters.flags.bits.isFreq = 1; // Tell the module that we want to set the frequency (not the period)
  timePulseParameters.flags.bits.isLength = 0; // Tell the module that pulseLenRatio is a ratio / duty cycle (* 2^-32) - not a length (in us)
  timePulseParameters.flags.bits.polarity = 1; // Tell the module that we want the rising edge at the top of second. (Set to 0 for falling edge.)
  timePulseParameters.flags.bits.lockGnssFreq =1;
  // Now set the time pulse parameters
  if (myGNSS.setTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("setTimePulseParameters failed!"));
  }
  else
  {
    Serial.println(F("Success!"));
  }

  attachInterrupt(digitalPinToInterrupt(START_STOP_PIN),startStopInt, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEY_INT_PIN),keyPressInt, FALLING); 
  myGNSS.setAutoPVT(true); //Tell the GNSS to "send" each solution
  //myGNSS.saveConfiguration(); //Optional: Save _all_ the current settings to flash and BBR

  //IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 = 1;    // QT3 Timer2 on pin 14
  *(portConfigRegister(PPS_PIN)) = 1;  // ALT 1
  CCM_CCGR6 |= CCM_CCGR6_QTIMER3(CCM_CCGR_ON);
  cnt = 1000 ; 
  TMRx->CH[2].CTRL = 0x000; 
  attachInterruptVector(IRQ_QTIMER4, ppsInterrupt);
  TMRx->CH[2].CNTR = 0;
  TMRx->CH[2].LOAD = 0;  // start val after compare
  TMRx->CH[2].COMP1 = cnt - 1;  // count up to this val and start again
  TMRx->CH[2].CMPLD1 = cnt - 1;
  TMRx->CH[2].SCTRL = TMR_SCTRL_TCFIE;  // enable interrupts;
  //TMRx->CH[2].CSCTRL = TMR_CSCTRL_TCF1EN;
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  //TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH;
  it1.begin(it1cb, 10000);  // microseconds
}

void loop(){
  char buffer[256];
  static double lat, lon;
  static int siv;
  unsigned long tick_hold;
  int cnt_hold;
  static int loops=0;
  static unsigned int speed_samples=0;
  static unsigned long speed_sum=0;
  static long speed=0;
  static UBX_TIM_TM2_data_t timeStamp;
  double avg_speed;
  uint8_t buttons;

  while(!refresh);
  refresh=false;
  cnt_hold=TMRx->CH[2].CNTR;
  tick_hold=tick;
  //sprintf(buffer, "%06ld.%02d", tick_hold, cnt_hold/10);
  LED.RefreshMe();
  sprintf(buffer, "%02d.%02d", tick_hold/3600, (tick_hold/60)%60);
  LED.Set_Position(0);
  LED.ShowMe(buffer);
  sprintf(buffer, "%02d.%02d", (tick_hold/60)%60, tick_hold%60);
  LED.Set_Position(2);
  LED.ShowMe(buffer); 
  sprintf(buffer, "%02d.%02d", tick_hold%60, cnt_hold/10);
  LED.Set_Position(4);
  LED.ShowMe(buffer); 

  if(start_stop_debounce) {
    debounce_count++;
    if(debounce_count==25) {
      digitalWriteFast(GPS_INT_PIN, HIGH);
      start_stop_debounce=false;
      enableInterrupt(START_STOP_PIN);
      //attachInterrupt(digitalPinToInterrupt(START_STOP_PIN),startStopInt, FALLING);
    }
  }

  if(loops%2==0) {
    if(key_press) {
      key_press=false;
      buttons = neokey.read();

      if (buttons & (1<<0)) {
        Serial.println("Button A");
        neokey.pixels.setPixelColor(0, 0xFF0000); // red
      } else {
        neokey.pixels.setPixelColor(0, 0);
      }

      if (buttons & (1<<1)) {
        Serial.println("Button B");
        neokey.pixels.setPixelColor(1, 0xFFFF00); // yellow
      } else {
        neokey.pixels.setPixelColor(1, 0);
      }
      
      if (buttons & (1<<2)) {
        Serial.println("Button C");
        neokey.pixels.setPixelColor(2, 0x00FF00); // green
      } else {
        neokey.pixels.setPixelColor(2, 0);
      }

      if (buttons & (1<<3)) {
        Serial.println("Button D");
        neokey.pixels.setPixelColor(3, 0x00FFFF); // blue
      } else {
        neokey.pixels.setPixelColor(3, 0);
      }  

      neokey.pixels.show();
    }    
  }

  if(loops==10) { 
    loops=0;

    myGNSS.checkUblox();
    myGNSS.checkCallbacks();
    if (myGNSS.getPVT() && (myGNSS.getInvalidLlh() == false))
    {
      myGNSS.getTIMTM2();
      lat=(myGNSS.getLatitude())/10000000.0;
      lon = (myGNSS.getLongitude())/10000000.0;
      siv = myGNSS.getSIV();
      fix = myGNSS.getFixType();
      Hour = myGNSS.getHour();
      Minute = myGNSS.getHour();
      Second = myGNSS.getSecond();
      speed = myGNSS.getGroundSpeed();

      speed_sum += speed;
      speed_samples++;
      memcpy(&timeStamp,&myGNSS.packetUBXTIMTM2->data,sizeof(UBX_TIM_TM2_data_t));
    }
    avg_speed=(double)speed_sum/(double)speed_samples;
    oled1.clearBuffer();					// clear the internal memory
    oled1.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font
    sprintf(buffer, "%02d:%02d:%02d", Hour, Minute, Second);
    oled1.drawStr(64,20,buffer);	// write something to the internal memory
    oled1.setFont(u8g2_font_spleen6x12_mf);
    oled1.drawStr(194,20,"GMT");
    sprintf(buffer, "lattitude: %f\xB0", lat);
    oled1.drawStr(0,32,buffer);	// write something to the internal memory
    sprintf(buffer, "longitude: %f\xB0", lon);
    oled1.drawStr(0,44,buffer);	// write something to the internal memory
    sprintf(buffer, "satellites: %2d", siv);
    oled1.drawStr(0,56,buffer);	// write something to the internal memory
    if(fix == 0) sprintf(buffer, "fix: none");
    else if(fix == 1) sprintf(buffer, "fix: DR");
    else if(fix == 2) sprintf(buffer, "fix: 2D");
    else if(fix == 3) sprintf(buffer, "fix: 3D");
    else if(fix == 4) sprintf(buffer, "fix: GNSS+DR");
    else if(fix == 5) sprintf(buffer, "fix: Time");
    oled1.drawStr(128,56,buffer);	// write something to the internal memory
    oled1.sendBuffer();					// transfer internal memory to the display

    oled2.clearBuffer();
    oled2.setFont(u8g2_font_spleen6x12_mf);	// choose a suitable font
    sprintf(buffer, "speed: %09.4f", speed*0.00223693629);
    oled2.drawStr(0,12,buffer);	// write something to the internal memory  

    sprintf(buffer, "avg speed: %09.4f", avg_speed*0.00223693629);
    oled2.drawStr(102,12,buffer);	// write something to the internal memory  
    sprintf(buffer, "%08ld.%1d", tick_hold, cnt_hold/100);
    oled2.drawStr(0,24,buffer);	// write something to the internal memory
 
    sprintf(buffer, "rcount=%d", timeStamp.count);  
    oled2.drawStr(0,36,buffer);
    sprintf(buffer, "m:%d r:%d fe:%d tb:%d u:%d t:%d re:%d",
      timeStamp.flags.bits.mode,
      timeStamp.flags.bits.run,
      timeStamp.flags.bits.newFallingEdge,
      timeStamp.flags.bits.timeBase,
      timeStamp.flags.bits.utc,
      timeStamp.flags.bits.time,
      timeStamp.flags.bits.newRisingEdge
    );
    oled2.drawStr(0,48,buffer);

    oled2.sendBuffer();					// transfer internal memory to the display

    oled3.clearBuffer();
    oled3.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
    sprintf(buffer, " %7.3f", speed*0.00223693629);
    oled3.drawStr(0,53,buffer);	// write something to the internal memory  
    oled3.sendBuffer();

    oled4.clearBuffer();
    oled4.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
    sprintf(buffer, " %7.3f", avg_speed*0.00223693629);
    oled4.drawStr(0,53,buffer);	// write something to the internal memory  
    oled4.sendBuffer();
  }
  loops++;
}

