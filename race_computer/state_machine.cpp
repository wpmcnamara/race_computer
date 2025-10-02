#include "state_machine.h"
#include "keypad.h"
#include "display.h"
#include "race.h"


class stateMachine stateMachine;

stateMachine::stateMachine(void) {
  state=stateInit;
  lastState=stateInit;
  status.value=0;
  lastStatus.value=0;
}

void stateMachine::run(void) {
  uint8_t keys=getKeyPress();

  //Update the system based on changing state.  We do this first to ensure that we try and pass through
  //every state transition.  There are no state transitions allowed here.  This is actions on entrance to
  //a new state only.
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
      case stateSelectRace:
        Serial.print("from: stateSelectRace");
        break;
      case stateSelectRaceLeg:
        Serial.print("from: stateSelectRaceLeg");
        break;
      case stateSaveSelection:
        Serial.print("from: stateSaveSelection");
        break;
      case stateCancelSelection:
        Serial.print("from: stateCancelSelection");
        break;
      case stateNextLeg:
        Serial.print("from: stateNextLeg");
        break; 
    }
    switch(state) {
      case stateInit:
        Serial.print("  to: stateInit");
        break;
      case stateMainMenu:
        Serial.println("  to: stateMainMenu");
        status.flags.startStopState=stateOff;
        startStopStartsRace=false;        
        dispRace=race.activeRace;
        dispRaceLeg=race.activeLeg;
        menuItem=0;
        raceSelectHighlight=false;
        raceLegSelectHighlight=false;
        ledDispFunc=ledDispDashes;
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayGPSInfo));        
        break;
      case stateRaceStart:
        Serial.println("  to: stateRaceStart");
        status.flags.startStopState=stateBreath;
        prepRace();
        startStopStartsRace=true;
        ledDispFunc=ledDispRaceDeltaSpeed;
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayDeltaSpeedLarge));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayDeltaTimeLarge));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayDistRemainLarge));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayGpsSpeedLarge));
        break;
      case stateDelayedStart:
        Serial.println("  to: stateDelayedStart");
        break;
      case stateLegActive:
        Serial.println("  to: stateLegActive");
        break;
      case stateLegComplete:
        Serial.println("  to: stateLegComplete");
        startStopStartsRace=false;
        updateRace();
        ledDispFunc=ledDispDashes;
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayLegSummaryTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayLegSummary));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceSummaryTitle));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayRaceSummary));        
        break;
      case stateRaceComplete:
        Serial.println("  to: stateRaceComplete");      
        race.inProgress=false;
        race.activeRace->inProgress=false;
        race.activeLeg=NULL;
        race.activeRace=NULL;
        break;
      case stateSelectRace:
        Serial.println("  to: stateSelectRace");
        menuItem=255;
        raceSelectHighlight=true;
        selectedRaceSave=selectedRace;
        selectedRace=races.begin();
        selectedRaceLeg=(*selectedRace)->raceLegs.begin();
        dispRace=(*selectedRace);
        dispRaceLeg=(*selectedRaceLeg);
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayRaceSelectTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayGPSInfo));       
        break;
      case stateSelectRaceLeg:
        Serial.println("  to: stateSelectRaceLeg");
        menuItem=255;
        raceLegSelectHighlight=true;
        selectedRaceLeg=race.activeRace->raceLegs.begin();
        dispRace=(race.activeRace);
        dispRaceLeg=(*selectedRaceLeg);
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayRaceLegSelectTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayGPSInfo));               
        break;
      case stateSaveSelection:
        Serial.println("  to: stateSaveSelection");
        setRace((*selectedRace), (*selectedRaceLeg));
        break;
      case stateCancelSelection:
        Serial.println("  to: stateCancelSelection");
        selectedRace=selectedRaceSave;
        break;
      case stateNextLeg:
        Serial.print("to: stateNextLeg");
        selectedRaceLeg++;
        if(selectedRaceLeg != (*selectedRace)->raceLegs.end()) {
          race.activeLeg=(*selectedRaceLeg);
        } else {
          race.activeLeg=NULL;
        }
        break;
    }      
    lastState=state;
  }

  //evaluated every pass through the state machine.  This handles state transitions caused
  //by async events in the race timing or GPS systems.  No actions or status updates allowed
  //in this block.  State transitions only.
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
      break;
    case stateRaceComplete:
      state=stateMainMenu;
      break;
    case stateSaveSelection:      
      state=stateMainMenu;
      break;
    case stateCancelSelection:
      state=stateMainMenu;
      break;
    case stateSelectRace:
      break;
    case stateSelectRaceLeg:
      break;
    case stateNextLeg:
      if(selectedRaceLeg == (*selectedRace)->raceLegs.end()) {
        state=stateRaceComplete;
      } else {
        state=stateMainMenu;
      }    
      break;
  }

  //Everything below here should only be executed if there is a keypress, or if the status flags 
  //have changed.
  if(keys==0 && (status.value==lastStatus.value)) {
    return;
  }

  //state machine evaluation based on keypad input.  The only state transitions here should be
  //driven by a key press.  No actions or status updates allowed in this block.  State transitions only.
  switch(state) {
    case stateInit:
      break;
    case stateMainMenu:
      if(keys & KEYPAD_KEY_ENTER) {
        switch(menuItem) {
          case 0:
            if(race.activeRace!=NULL && status.flags.gpsReady) {
              state=stateRaceStart;
            }
            break;
          case 1:
            if(!race.inProgress) {
              state=stateSelectRace;
            }
            break;
          case 2:
            if(race.activeRace!=NULL && !race.inProgress) {
              state=stateSelectRaceLeg;
            }
            break;
        }
      }
      if(keys & KEYPAD_KEY_UP) {
        if(menuItem==0) {
          menuItem=2;
        } else {
          menuItem--;
        }
      }
      if(keys & KEYPAD_KEY_DOWN) {
        if(menuItem==2) {
          menuItem=0;
        } else {
          menuItem++;
        }
      }
      break;
    case stateRaceStart:
      if(keys & KEYPAD_KEY_ESC) {
        state=stateMainMenu;
      }
      break;
    case stateDelayedStart:
      break;
    case stateLegActive:
      break;
    case stateLegComplete:
      if(keys & KEYPAD_KEY_ESC) {
          state=stateNextLeg;
      }
      break;
    case stateNextLeg:
      break;
    case stateSelectRace:
      if(keys & KEYPAD_KEY_ENTER) {
        state=stateSaveSelection;
      }
      if(keys & KEYPAD_KEY_UP) {
        if(selectedRace==races.begin()) {
          selectedRace=races.end();
        }
        --selectedRace;
        selectedRaceLeg=(*selectedRace)->raceLegs.begin();
        dispRace=(*selectedRace);
        dispRaceLeg=(*selectedRaceLeg);
      }
      if(keys & KEYPAD_KEY_DOWN) {
        ++selectedRace;
        if(selectedRace==races.end()) {
          selectedRace=races.begin();
        } 
        selectedRaceLeg=(*selectedRace)->raceLegs.begin();
        dispRace=(*selectedRace);
        dispRaceLeg=(*selectedRaceLeg);
      }    
      if(keys & KEYPAD_KEY_ESC) {
        state=stateCancelSelection;
      }    
      break;
    case stateSelectRaceLeg:
      if(keys & KEYPAD_KEY_ENTER) {
        state=stateSaveSelection;
      }
      if(keys & KEYPAD_KEY_UP) {
        if(selectedRaceLeg==race.activeRace->raceLegs.begin()) {
          selectedRaceLeg=race.activeRace->raceLegs.end();
        }
        --selectedRaceLeg;
        dispRaceLeg=(*selectedRaceLeg);
      }
      if(keys & KEYPAD_KEY_DOWN) {
        ++selectedRaceLeg;
        if(selectedRaceLeg==race.activeRace->raceLegs.end()) {
          selectedRaceLeg=race.activeRace->raceLegs.begin();
        } 
        dispRaceLeg=(*selectedRaceLeg);

      }    
      if(keys & KEYPAD_KEY_ESC) {
        state=stateCancelSelection;
      }        
      break;
    case stateRaceComplete:
      break;
    case stateSaveSelection:
      break;
    case stateCancelSelection:
      break;  
  }

  //Handle the button lighting based on the current state of the system.
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
    Serial.printf("flags.startStopState=%d last flags.startStopState=%d\n", status.flags.startStopState, lastStatus.flags.startStopState);
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