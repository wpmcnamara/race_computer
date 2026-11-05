#include "bsp.h"
#include "storage.h"
#include "gps.h"
#include "display.h"
#include "FXUtil.h"
#include "keypad.h"
#include <EEPROM.h>

extern "C" {
  #include "FlashTxx.h"         // TLC/T3x/T4x/TMM flash primitives
}


bool SPILock=false;
uint32_t firmwareSize=0;
uint32_t firmwareProgress=0;
uint8_t firmwareUpdateState=0;
hardwareVersionStruct_t hardwareVersion;
bool initHwVer=false;



void bsp_setup(void) {
  uint8_t ret;

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);

  pinMode(GPS_CS, OUTPUT);
  digitalWrite(GPS_CS, HIGH);

  pinMode(LED_DISP_LOAD, OUTPUT);
  digitalWrite(LED_DISP_LOAD, HIGH);

  pinMode(OLED_DISP1_CS, OUTPUT);
  digitalWrite(OLED_DISP1_CS, HIGH);
  pinMode(OLED_DISP2_CS, OUTPUT);
  digitalWrite(OLED_DISP2_CS, HIGH);
  pinMode(OLED_DISP3_CS, OUTPUT);
  digitalWrite(OLED_DISP3_CS, HIGH);
  pinMode(OLED_DISP4_CS, OUTPUT);
  digitalWrite(OLED_DISP4_CS, HIGH);

  ret=loadHardwareVersionStruct();
  switch(ret) {
    case 0:
      Serial.println("Hardware version structure uninitialized.");
      hardwareVersion.pcbMajor=2;
      hardwareVersion.pcbMinor=2;
      strncpy(hardwareVersion.pcbOther, "beta",16);
      hardwareVersion.pcbOther[15]=0;
      hardwareVersion.serialNo=2;
      hardwareVersion.serialOther[0]=0; 
      initHwVer=true;
      break;
    case 1:
      Serial.println("Hardware version structure valid.");
      Serial.printf("HW Version: %d.%d", hardwareVersion.pcbMajor, hardwareVersion.pcbMinor);
      if(strlen(hardwareVersion.pcbOther)!=0) {
        Serial.printf("-%s\n", hardwareVersion.pcbOther);
      } else {
        Serial.println("");
      }
      Serial.printf("Serial #: %06d", hardwareVersion.serialNo);
      if(strlen(hardwareVersion.serialOther)!=0) {
        Serial.printf("-%s\n", hardwareVersion.serialOther);
      } else {
        Serial.println("");
      }   
      break;
    case 2:
      Serial.println("Hardware version structure checksum invalid.");   
      break;
    case 3:
      Serial.printf("Hardware version structure -- unexpected version: 0x%0X\n", EEPROM.read(HWVERSTRUCTADDR));
      break;
  }
}

bool checkForUpdate(void) {
  //disable display and GPS use of the SPI bus to prevent collisions
  if(!sdCardPresent) {
    Serial.println("No SD card.  Skipping firmware update");
    return false;
  }
  
  //disable display and GPS use of the SPI bus to prevent collisions
  //doSPILock();
  if(!sdCard.exists("orrc/system/race_computer.ino.hex")) {
    //doSPIUnlock();
    Serial.println("No firmware file found.");
    return false;
  }  
  //doSPIUnlock();
  Serial.println("Found orrc/system/race_computer.ino.hex");
  return true;
}

bool doFirmwareUpdate(void) {
  uint32_t buffer_addr, buffer_size;
  Serial.println("Loading firmware file");
  
  //stop all async events.  This becomes the only thread of execution at this point.
  eventTimerStop();
  //turn of key interrupts.  Nothing should interfere with flash programming.
  detachInterrupt(digitalPinToInterrupt(KEYPAD_START));
  detachInterrupt(digitalPinToInterrupt(KEYPAD_INT)); 
  //make sure the keypad LEDs are turned off.
  setAllButtonColor(COLOR_BLACK);
  startStopOff();

  File32 firmwareFile=sdCard.open("orrc/system/race_computer.ino.hex");
  if(!firmwareFile) {
    Serial.println("  file open error");
    //doSPIUnlock();
    return false;
  }
  Serial.printf( "target = %s (%dK flash in %dK sectors)\n", FLASH_ID, FLASH_SIZE/1024, FLASH_SECTOR_SIZE/1024);
  // create flash buffer to hold new firmware
  if (firmware_buffer_init( &buffer_addr, &buffer_size ) == 0) {
    Serial.printf( "unable to create buffer\n" );
    Serial.flush();
    for (;;) {}
  }
  
  Serial.printf( "created buffer = %1luK %s (%08lX - %08lX)\n",
		buffer_size/1024, IN_FLASH(buffer_addr) ? "FLASH" : "RAM",
		buffer_addr, buffer_addr + buffer_size );
  firmwareSize=firmwareFile.size();
  firmwareProgress=0;
  update_firmware( &firmwareFile, &Serial, buffer_addr, buffer_size );
  // return from update_firmware() means error or user abort, so clean up and
  // reboot to ensure that static vars get boot-up initialized before retry
  Serial.println("Update failed");
  Serial.println( "erase FLASH buffer / free RAM buffer..." );
  firmware_buffer_free( buffer_addr, buffer_size );
  Serial.flush();
  REBOOT;
  return false;
}

void doReboot(void) {
  REBOOT;
}

uint8_t loadHardwareVersionStruct(void) {
  uint16_t addr=HWVERSTRUCTADDR;
  uint16_t offset;
  uint32_t checkSum=0;
  uint8_t *hwVerPtr=(uint8_t*)&hardwareVersion;
  hwVerPtr[0]=EEPROM.read(addr);
  if(hardwareVersion.structVer==0 || hardwareVersion.structVer==0xFF) {
    //hardware version is uninitialized in EEPROM
    return 0;
  }
  if (hardwareVersion.structVer != HWVERSTRUCTVER) {
    //We got back an version of the data structure that we don't expect
    //Don't try and read further as we won't know what to do with it or
    //how much data is there.
    return 3;
  }

  //Read the whole structure now.
  for(offset=0; offset<sizeof(hardwareVersionStruct_t); offset++) {
    hwVerPtr[offset]=EEPROM.read(addr+offset);
  }

  //calculate simple check sum 
  for(offset=0; offset<sizeof(hardwareVersionStruct_t)-4; offset++) {
    checkSum+=hwVerPtr[offset];
  }
  //checksum is invalid.  Assume EEPROM is corrupted
  if(checkSum!=hardwareVersion.chkSum) {
    return 2;
  }
  //got a valid hardwareVesion
  return 1;

}

bool storeHardwareVersionStruct(hardwareVersionStruct_t* hwVer) {
  uint16_t addr=HWVERSTRUCTADDR;
  uint16_t offset;
  uint32_t checkSum=0;
  uint8_t *hwVerPtr=(uint8_t*)hwVer;
  hwVer->pad=0;
  //don't assume the caller has set the structure version value
  hwVer->structVer=HWVERSTRUCTVER;
  //calculate the simply checksum
  for(offset=0; offset<sizeof(hardwareVersionStruct_t)-4; offset++) {
    checkSum+=hwVerPtr[offset];
  }
  hwVer->chkSum=checkSum;
  //now write to EEPROM
  for(offset=0; offset<sizeof(hardwareVersionStruct_t); offset++) {
    EEPROM.write(addr+offset, hwVerPtr[offset]);
  }

  //read back to confirm save was successfull
  for(offset=0; offset<sizeof(hardwareVersionStruct_t); offset++) {
    if(EEPROM.read(addr+offset)!=hwVerPtr[offset]) {
      return false;
    }
  }
  return true; 
}