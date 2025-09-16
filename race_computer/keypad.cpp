#include "keypad.h"
#include "gps.h"
#include "timer.h"
#include <list>
#include "event.h"

void startStopBreath(void);

Adafruit_NeoKey_1x4 keypad;
volatile bool keyPress=false;
volatile bool startStopKeyPress=false;
volatile bool startDebounce=false;
int debounceCount=0;
int startStopBreathColor=0;
int startStopBreathCount=0;
int startStopBreathDir=1;
bool startStopBreathActive=false;
std::list<uint8_t> keyPresses;
//event_t breathEvent(startStopBreath, eventRepeat, false, 0, 1);

Adafruit_NeoPixel startStop = Adafruit_NeoPixel(1, KEYPAD_LED, NEO_GRB + NEO_KHZ800);

void startStopBreath(void) {
  if(startStopBreathDir==1) {
    if(startStopBreathColor<250) {
      startStopBreathColor+=10;
      startStop.setPixelColor(0, 0, startStopBreathColor, 0);
      startStop.show();
    } else {
      if(startStopBreathCount<10) {
        startStopBreathCount++;
      } else {
        startStopBreathDir=0;
        startStopBreathCount=0;
      }
    }
  } else {
    if(startStopBreathColor>10) {
      startStopBreathColor-=10;
      startStop.setPixelColor(0, 0, startStopBreathColor, 0);
      startStop.show();
    } else {
      if(startStopBreathCount<10) {
        startStopBreathCount++;
      } else {
        startStopBreathDir=1;
        startStopBreathCount=0;
      }      
    }
  }
}

void startStopStartBreath(void) {
  //breathEvent.active=true;
}

void startStopStopBreath(void) {
  //breathEvent.active=false;
  startStop.setPixelColor(0, 0);
  startStop.show();
}

bool startStopIsBreathing(void) {
  return startStopBreathActive;
}
void keypadSetup(void) {
  pinMode(KEYPAD_START, INPUT_PULLUP);
  pinMode(KEYPAD_INT, INPUT_PULLUP);

  if (! keypad.begin(0x30)) {     // begin with I2C address, default is 0x30
    Serial.println("Could not start NeoKey, check wiring?");
    while(1) delay(10);
  }
  Serial.println("NeoKey started!");

  startStop.begin();
  startStop.setBrightness(25);
  startStop.show(); // Initialize all pixels to 'off'

  startStop.setPixelColor(0, 0x808080);
  startStop.show();
  delay(50);
  startStop.setPixelColor(0, 0);
  startStop.show();

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
  new event_t(keypadUpdate, eventRepeat, true, 0, 1, &Serial, "keypadUpdate");
  new event_t(readKeypad, eventRepeat, true, 0, 2, &Serial, "readKeypad");
  new event_t(startStopBreath, eventRepeat, false, 0, 1, &Serial, "startStopBreath");
  startStopStartBreath();
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
    gpsZeroDistance();
  } else {
    timer_run=false;
    TMRx->CH[2].CTRL = 0;
    digitalWriteFast(GPS_INT, LOW);
  }
  disableInterrupt(KEYPAD_START);
  startDebounce=true;
  debounceCount=0;
  startStopKeyPress=true;
}

void readKeypad(void) {
  uint8_t buttons=0;
  if(keyPress) {
    keyPress=false;
    buttons = keypad.read();
    if (buttons & KEY_ENTER) {
      keypad.pixels.setPixelColor(0, 0xFFFFFF); // red
    } else {
      keypad.pixels.setPixelColor(0, 0);
    }
    if (buttons & KEY_DOWN) {
      keypad.pixels.setPixelColor(1, 0xFFFFFF); // yellow
    } else {
      keypad.pixels.setPixelColor(1, 0);
    }
    if (buttons & KEY_UP) {
      keypad.pixels.setPixelColor(2, 0xFFFFFF); // green
    } else {
      keypad.pixels.setPixelColor(2, 0);
    }
    if (buttons & KEY_ESC) {
      keypad.pixels.setPixelColor(3, 0xFFFFFF); // blue
    } else {
      keypad.pixels.setPixelColor(3, 0);
    }  
    keypad.pixels.show();
  } 
  if(startStopKeyPress) {
    buttons|=(KEY_START_STOP);
    startStopKeyPress=false;
    startStop.setPixelColor(0, 0x808080);
    startStop.show();
  }
  if(buttons) {
    keyPresses.push_back(buttons);
  }   
}

void keypadUpdate(void) {
  if(startDebounce) {
    debounceCount++;
    if(debounceCount==25) {
      digitalWriteFast(GPS_INT, HIGH);
      startDebounce=false;
      enableInterrupt(KEYPAD_START);
    }
    if(debounceCount == 5) {
      startStop.setPixelColor(0, 0);
      startStop.show();
    }
  }
}
uint8_t getKeyEvent(void) {
  uint8_t keys;
  if (keyPresses.empty()) {
    return 0;
  } else {
    keys=keyPresses.front();
    keyPresses.pop_front();
    return keys;
  }


}