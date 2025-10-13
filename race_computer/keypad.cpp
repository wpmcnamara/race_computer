#include "keypad.h"
#include "gps.h"
#include "timer.h"
#include <list>
#include "event.h"
#include "race.h"
#include "gps.h"
#include "state_machine.h"
#include "display.h"

void startStopBreath(void);
void startStopOff();
void keyDebounce();

Adafruit_NeoKey_1x4 keypad;
volatile bool keyPress=false;
volatile bool startPress=false;
volatile bool startStopKeyPress=false;
//volatile bool startDebounce=false;
//int debounceCount=0;
int startStopBreathColor=0;
int startStopBreathCount=0;
int startStopBreathDir=1;
bool startStopBreathActive=false;
int startStopBlinkColor=0x00FF00;
bool startStopBlinkState=false;
bool startStopBlinkActive=false;
uint8_t buttonState=0;
bool startStopState=true;
bool lastStartStopState=true;
bool keysLocked=false;
bool startStopStartsRace=false;
std::list<uint8_t> keyPresses;
//event_t breathEvent(startStopBreath, eventRepeat, false, false, 0, 1);
event_t *breathEvent;
event_t *blinkEvent;
event_t *startStopOffEvent;
event_t *keyDebounceEvent;
//event_t startStopOffEvent(startStopOff , eventSingle, false, false, 0, 10);
Adafruit_NeoPixel startStop = Adafruit_NeoPixel(1, KEYPAD_LED, NEO_GRB + NEO_KHZ800);

void startStopOff(void) {
  startStop.setPixelColor(0, 0);
  startStop.show(); 
}
void startStopBreath(void) {
  if(startStopBreathDir==1) {
    if(startStopBreathColor<250) {
      startStopBreathColor+=5;
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
      startStopBreathColor-=5;
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

void startStopFastBlink(void) {
  startStopBlinkState=!startStopBlinkState;
  if(startStopBlinkState) {
    startStop.setPixelColor(0, startStopBlinkColor);    
  } else {
    startStop.setPixelColor(0, 0); 
  }
  startStop.show();
  return;
}

void startStopStartBreath(void) {
  breathEvent->active=true;
}

void startStopStopBreath(void) {
  breathEvent->active=false;
  //startStopOffEvent->active=true;
}

bool startStopIsBreathing(void) {
  return breathEvent->active;
}

void startStopStartBlink(void) {
  blinkEvent->active=true;
}

void startStopStopBlink(void) {
  blinkEvent->active=false;
  //startStopOffEvent->active=true;
}

bool startStopIsBlinking(void) {
  return blinkEvent->active;
}

void keypadSetup(void) {
  pinMode(KEYPAD_START, INPUT_PULLUP);
  pinMode(KEYPAD_INT, INPUT_PULLUP);

  if (! keypad.begin(0x30)) {     // begin with I2C address, default is 0x30
    Serial.println("Could not start NeoKey, check wiring?");
    displayError("NeoKey start error");
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
  new event_t(readKeypad, eventRepeat, true, false, 0, 2, &Serial, "readKeypad");
  breathEvent=new event_t(startStopBreath, eventRepeat, false, false, 0, 1, &Serial, "breathEvent");
  blinkEvent=new event_t(startStopFastBlink, eventRepeat, false, false, 0, 20, &Serial, "blinkEvent");
  startStopOffEvent=new event_t(startStopOff , eventSingle, false, false, 0, 10, &Serial, "startStopOffEvent");
  keyDebounceEvent=new event_t(keyDebounce, eventSingle, false, false, 0, 15, &Serial, "keyDebounceEvent");
}

void keyPressInt() {
  keyPress=true;
}

void startPressInt() {
  //Serial.println("start/stop interrupt");
  startPress=true;
  //Serial.print("startStopState=");
  //Serial.println(startStopState);  
  if(startStopStartsRace) {
    if(startStopState==1) {
      if(!race.legData->inProgress) {
        TMRx->CH[2].CNTR = 0;
        TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH;
        digitalWriteFast(GPS_INT, LOW);
        timerVal.seconds=0;
        timer_run=true;
      } else {
        TMRx->CH[2].CTRL = 0;
        digitalWriteFast(GPS_INT, LOW);
        timer_run=false;
      }
    }
  }
  disableInterrupt(KEYPAD_START);
  keyDebounceEvent->active=true;
}

void readKeypad(void) {
  uint8_t buttons=0;
  if(keyPress) {
    //Serial.println("keypress");
    keyPress=false;
    buttons = keypad.read();
    if(!keysLocked) {
      if (buttons & KEYPAD_KEY_ENTER) {
        keypad.pixels.setPixelColor(0, 0xFFFFFF); 
      } else {
        keypad.pixels.setPixelColor(0, 0);
      }
      if (buttons & KEYPAD_KEY_DOWN) {
        keypad.pixels.setPixelColor(1, 0xFFFFFF); 
      } else {
        keypad.pixels.setPixelColor(1, 0);
      }
      if (buttons & KEYPAD_KEY_UP) {
        keypad.pixels.setPixelColor(2, 0xFFFFFF); 
      } else {
        keypad.pixels.setPixelColor(2, 0);
      }
      if (buttons & KEYPAD_KEY_ESC) {
        keypad.pixels.setPixelColor(3, 0xFFFFFF); 
      } else {
        keypad.pixels.setPixelColor(3, 0);
      }  
      keypad.pixels.show();
    } else {
      buttons=0;
    }
  } 
  if(startPress) {
    //Serial.println("startPress");
    startPress=false;
    lastStartStopState=startStopState;
    startStopState=digitalReadFast(KEYPAD_START);
    if(lastStartStopState!=startStopState) {
      if(startStopState==0 ) {
          buttons|=KEYPAD_KEY_START_STOP;
          stateMachine.startStopColor=COLOR_WHITE;
          stateMachine.status.flags.startStopState=stateOn;
      } else {
        stateMachine.status.flags.startStopState=stateOff; 
      }
    }
  }
  if(buttons) {
    //Serial.printf("buttons: %d\n", buttons);
    keyPresses.push_back(buttons);
  }   
}

void keyDebounce(void) {
  digitalWriteFast(GPS_INT, HIGH);
  lastStartStopState=startStopState;
  startStopState=digitalReadFast(KEYPAD_START);
  //because we are in the debounce routine, startStopState is 0, coming into 
  //this routine, so the only transition we could get would be to 1, which would
  //represent the button being released
  if(lastStartStopState!=startStopState) {
    stateMachine.status.flags.startStopState=stateOff; 
  }      
  enableInterrupt(KEYPAD_START);
}

uint8_t getKeyPress(void) {
  uint8_t keys;
  if (keyPresses.empty()) {
    return 0;
  } else {
    keys=keyPresses.front();
    keyPresses.pop_front();
    return keys;
  }
}

void setAllButtonColor(uint32_t color) {
  keypad.pixels.setPixelColor(0,color);
  keypad.pixels.setPixelColor(1,color);
  keypad.pixels.setPixelColor(2,color);
  keypad.pixels.setPixelColor(3,color);
  keypad.pixels.show();  
}

void startStopOn(uint32_t color) {
  startStop.setPixelColor(0, color);
  startStop.show(); 
}
