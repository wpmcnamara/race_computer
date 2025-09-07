#include "keypad.h"
#include "gps.h"

Adafruit_NeoKey_1x4 keypad;
volatile bool keyPress=false;
volatile bool startDebounce=false;
int debounceCount=0;


void keypadSetup(void) {
  pinMode(KEYPAD_START, INPUT_PULLUP);
  pinMode(KEYPAD_INT, INPUT_PULLUP);

  if (! keypad.begin(0x30)) {     // begin with I2C address, default is 0x30
    Serial.println("Could not start NeoKey, check wiring?");
    while(1) delay(10);
  }
  Serial.println("NeoKey started!");

  // Pulse all the LEDs on to show we're working
  for (uint16_t i=0; i<keypad.pixels.numPixels(); i++) {
    keypad.pixels.setPixelColor(i, 0x808080); // make each LED white
    keypad.pixels.show();
    delay(50);
  }
  for (uint16_t i=0; i<keypad.pixels.numPixels(); i++) {
    keypad.pixels.setPixelColor(i, 0x000000);
    keypad.pixels.show();
    delay(50);
  }

  attachInterrupt(digitalPinToInterrupt(KEYPAD_START),startPressInt, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEYPAD_INT),keyPressInt, FALLING); 
}

void keyPressInt() {
  keyPress=true;
}

void startPressInt() {
  if(!timer_run) {
    timer_run=true;
    TMRx->CH[2].CNTR = 0;
    tick=0;
    TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH;
    digitalWriteFast(GPS_INT, LOW);
    gps.resetOdometer(); //Uncomment this line to reset the odometer
  } else {
    timer_run=false;
    TMRx->CH[2].CTRL = 0;
    digitalWriteFast(GPS_INT, LOW);
  }
  disableInterrupt(KEYPAD_START);
  startDebounce=true;
  debounceCount=0;
}

int readKeypad(void) {
  int button=0;
  uint8_t buttons;
  if(keyPress) {
    keyPress=false;
    buttons = keypad.read();

    if (buttons & (1<<0)) {
      Serial.println("Button A");
      keypad.pixels.setPixelColor(0, 0xFF0000); // red
      button=1;
    } else {
      keypad.pixels.setPixelColor(0, 0);
    }

    if (buttons & (1<<1)) {
      Serial.println("Button B");
      keypad.pixels.setPixelColor(1, 0xFFFF00); // yellow
      button=2;
    } else {
      keypad.pixels.setPixelColor(1, 0);
    }
    
    if (buttons & (1<<2)) {
      Serial.println("Button C");
      keypad.pixels.setPixelColor(2, 0x00FF00); // green
      button=3;
    } else {
      keypad.pixels.setPixelColor(2, 0);
    }

    if (buttons & (1<<3)) {
      Serial.println("Button D");
      keypad.pixels.setPixelColor(3, 0x00FFFF); // blue
      button=4;
    } else {
      keypad.pixels.setPixelColor(3, 0);
    }  

    keypad.pixels.show();
  }    
  return(button);
}

void keypadUpdate(void) {
  if(startDebounce) {
    debounceCount++;
    if(debounceCount==25) {
      digitalWriteFast(GPS_INT, HIGH);
      startDebounce=false;
      enableInterrupt(KEYPAD_START);
    }
  }
}