#include "bsp.h"
#include "storage.h"
#include "gps.h"
#include "display.h"
#include "FXUtil.h"
#include "keypad.h"
extern "C" {
  #include "FlashTxx.h"         // TLC/T3x/T4x/TMM flash primitives
}


bool SPILock=false;
uint32_t firmwareSize=0;
uint32_t firmwareProgress=0;
uint8_t firmwareUpdateState=0;

void bsp_setup(void) {

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
  pinMode(OLED_DISP3_CS, OUTPUT);
  digitalWrite(OLED_DISP1_CS, HIGH);
}

bool checkForUpdate(void) {
  //disable display and GPS use of the SPI bus to prevent collisions
  if(!sdCardPresent) {
    Serial.println("No SD card.  Skipping firmware update");
    return false;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc/system/race_computer.ino.hex")) {
    doSPIUnlock();
    Serial.println("No firmware file found.");
    return false;
  }  
    doSPIUnlock();
    Serial.println("Found orrc/system/racec_omputer.ino.hex");
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
    doSPIUnlock();
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