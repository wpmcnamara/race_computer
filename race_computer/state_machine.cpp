#include "state_machine.h"
#include "keypad.h"


class stateMachine stateMachine;

stateMachine::stateMachine(void) {
  state=stateInit;
  status.value=0;
  lastStatus.value=0;
}

void stateMachine::run(void) {
  uint8_t keys=getKeyPress();

  if(state != lastState ) {
   switch(lastState) {
      case stateInit:
        Serial.print("from: stateInit");
        break;
      case stateMainMenu:
        Serial.print("from: stateMainMenu");
        break;
      case stateRaceStart:
        Serial.print("from: stateRaceStart");
        break;
      case stateDelayedStart:
        Serial.print("from: stateDelayedStart");
        break;
      case stateLegActive:
        Serial.print("from: stateLegActive");
        break;
      case stateLegComplete:
        Serial.print("from: stateLegComplete");
        break;
      case stateRaceComplete:
        Serial.print("from: stateRaceComplete");
        break;
    }
    switch(state) {
      case stateInit:
        Serial.print("  to: stateInit");
        break;
      case stateMainMenu:
        Serial.println("  to: stateMainMenu");
        break;
      case stateRaceStart:
        Serial.println("  to: stateRaceStart");
        break;
      case stateDelayedStart:
        Serial.println("  to: stateDelayedStart");
        break;
      case stateLegActive:
        Serial.println("  to: stateLegActive");
        break;
      case stateLegComplete:
        Serial.println("  to: stateLegComplete");
        break;
      case stateRaceComplete:
        Serial.println("  to: stateRaceComplete");
        break;
    }      
    lastState=state;
  }
  switch(state) {
    case stateInit:
      state=stateMainMenu;
      break;
    case stateMainMenu:
      break;
    case stateRaceStart:
      if(status.flags.delayedStart==true) {
        state=stateDelayedStart;
      }
      if(status.flags.legActive==true) {
        state=stateLegActive;
      }
      break;
    case stateDelayedStart:
      if(status.flags.legActive==true) {
        state=stateLegActive;
      }
      break;
    case stateLegActive:
      if(status.flags.legActive==false) {
        state=stateLegComplete;
      }
      break;
    case stateLegComplete:
      startStopStartsRace=false;
      state=stateRaceComplete;
      break;
    case stateRaceComplete:
      state=stateMainMenu;
      break;
  }

  if(keys==0 && (status.value==lastStatus.value)) {
    return;
  }

  switch(state) {
    case stateInit:
      break;
    case stateMainMenu:
      if(keys & KEYPAD_KEY_ENTER) {
        if(status.flags.gpsReady) {
          state=stateRaceStart;
          status.flags.startStopState=stateBreath;
          startStopStartsRace=true;
        }
      }
      break;
    case stateRaceStart:
      if(keys & KEYPAD_KEY_ESC) {
        state=stateMainMenu;
        status.flags.startStopState=stateOff;
        startStopStartsRace=false;
      }
      break;
    case stateDelayedStart:
      break;
    case stateLegActive:
      break;
    case stateLegComplete:
      break;
    case stateRaceComplete:
      break;
  }

  Serial.printf("keys:%d status:%d  lastStatus:%d\n", keys, status.value, lastStatus.value);

  if(status.flags.delayedStart!=lastStatus.flags.delayedStart) {
    if(status.flags.delayedStart) {
      status.flags.startStopState=stateBlink;
    } else {
      status.flags.startStopState=stateOff;
    }
  }

  if(status.flags.legActive != lastStatus.flags.legActive) {
    if(status.flags.legActive) {
      startStopColor=COLOR_GREEN;
      status.flags.startStopState=stateOn; 
      Serial.println("start leg");
    } else {
      Serial.println("stop leg");
      status.flags.startStopState=stateOff;
      status.flags.buttonColor=COLOR_BLACK;
    }
  }

  if(status.flags.gpsReady!=lastStatus.flags.gpsReady) {
    if(status.flags.gpsReady) {
      Serial.println("GPS Ready...");
      if(status.flags.delayedStart) {
        startStopColor=COLOR_GREEN;
        Serial.println("state Blink");
        status.flags.startStopState=stateBlink;
      } else if(state == stateRaceStart) {
        Serial.println("state Breath");
        startStopColor=COLOR_GREEN;
        status.flags.startStopState=stateBreath;
      } 
    } else {
      status.flags.startStopState=stateOff;
    }
  }

  if(status.flags.startStopState!=lastStatus.flags.startStopState) {
    Serial.printf("startStopState=%d lastStartStopState=%d\n", status.flags.startStopState, lastStatus.flags.startStopState);
    switch(lastStatus.flags.startStopState) {
      case stateBlink:
        startStopStopBlink();
        break;
      case stateBreath:
        startStopStopBreath();
        break;
      default:
        break;
    }
    switch(status.flags.startStopState) {
      case stateBlink:
        Serial.println("start blink");
        startStopStartBlink();
        break;
      case stateBreath:
        Serial.println("start breath");
        startStopStartBreath();
        break;
      case stateOn:
        Serial.println("start on");
        startStopOn(startStopColor);
        break;
      case stateOff:
        Serial.println("start off");
        if(status.flags.legActive) {
          status.flags.startStopState=stateOn;
          startStopColor=COLOR_GREEN;
          startStopOn(startStopColor);
        } else {
          if(status.flags.delayedStart) {
            startStopColor=COLOR_GREEN;
            status.flags.startStopState=stateBlink;
            startStopStartBlink();
          } else if(status.flags.gpsReady && state==stateRaceStart) {
            startStopColor=COLOR_GREEN;
            status.flags.startStopState=stateBreath;
            startStopStartBreath();
          } else if(status.flags.legActive) {
            status.flags.startStopState=stateOn;
            startStopOn(startStopColor);           
          } else {
            status.flags.startStopState=stateOff;
            startStopOff();
          }
        }
        break;
      default:
        break;
    }
  }

  if(status.flags.buttonColor != lastStatus.flags.buttonColor) {
    switch(status.flags.buttonColor) {
      case 0:
        setAllButtonColor(COLOR_BLACK);
        break;
      case 1:
        setAllButtonColor(COLOR_RED);
        break;
      case 2:
        setAllButtonColor(COLOR_BLUE);
        break;
      case 3:
        setAllButtonColor(COLOR_GREEN);
    }
  }
  lastStatus.value=status.value;
}