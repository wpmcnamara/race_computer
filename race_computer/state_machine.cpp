#include "state_machine.h"
#include "keypad.h"
#include "display.h"
#include "race.h"
#include "bsp.h"
#include "menu.h"
#include "storage.h"
#include "math.h"


class stateMachine stateMachine;
unsigned long lastKeyPress;
bool dispSelectChange=false;

stateMachine::stateMachine(void) {
  state=stateInit;
  lastState=stateUnknown;
  status.value=0;
  lastStatus.value=0;
}

void stateMachine::run(void) {
  uint8_t keys=getKeyPress();
  uint8_t menuAction;
  //Update the system based on changing state.  We do this first to ensure that we try and pass through
  //every state transition.  There are no state transitions allowed here.  This is actions on entrance to
  //a new state only.
  if(state != lastState ) {
    switch(lastState) {
      case stateInit:
        Serial.print("from: stateInit");
        break;
      case stateLoadRaceCheckPoint:
        Serial.print("from: loadRaceCheckPoint");
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
      case stateDispSelect:
        Serial.print("from: stateDispSelect");
        break; 
      case stateCheckFirmwareUpdate:
        Serial.print("from: stateCheckFirmwareUpdate");
        break; 
      case stateConfirmFirmwareUpdate:
        Serial.print("from: stateConfirmFirmwareUpdate");
        break; 
      case stateDoFirmwareUpdate:
        Serial.print("from: stateDoFirmwareUpdate");
        break; 
      case stateSetLedBrightness:
        Serial.print("from: stateSetLedBrightness");
        break; 
      case stateSetLegTimeAdjust:
        Serial.print("from: stateSetLegTimeAdjust");
        break; 
      case stateSetOledBrightness:
        Serial.print("from: stateDoFirmwareUpdate");
        break; 
      case stateSetScreenBlankTime:
        Serial.print("from: stateSetOledBrightness");
        break; 
      case stateScreenBlank:
        Serial.print("from: stateScreenBlank");
        break; 
      case stateScreenWake:
        Serial.print("from: stateScreenWake");
        break;         
      case stateSaveSettings:
        Serial.print("from: stateSaveSettings");
        break; 
      case stateLoadSettings:
        Serial.print("from: stateLoadSettings");
        break; 
      case stateEditRaceLeg:
        Serial.print("from: stateEditRaceLeg");
        break; 
      case stateAdjustLegTime:
        Serial.print("from: stateAdjustLegTime");
        break; 
      case stateAdjustLegDistance:
        Serial.print("from: stateAdjustLegDistance");
        break; 
      case stateAdjustLegSpeed:
        Serial.print("from: stateAdjustLegSpeed");
        break; 
      case stateAdjustLegSave:
        Serial.print("from: stateAdjustLegSave");
        break;       
      case stateAdjustLegRestore:
        Serial.print("from: stateAdjustLegRestore");
        break;    
      case   stateAdjustLegCaptureValues:
        Serial.printf("from: stateAdjustLegCaptureValues"); 
        break;          
      case stateAdjustLegResetValues:
        Serial.printf("from: stateAdjustLegResetValues"); 
        break;             
      case stateShowSystemInfo:
        Serial.printf("from: stateShowSystemInfo"); 
        break;         
      case stateSetHwVer:
        Serial.println("from: stateSetHwVer");
        break;
      case stateReboot:
        Serial.print("from: stateReboot");
        break; 
      case stateResetAllSettings:
        Serial.print("from: resetAllSettings");
        break;                 
      case stateUnknown:
        Serial.print("from: stateUnknown");
        break; 
    }
    switch(state) {
      case stateInit:
        Serial.println("  to: stateInit");
        lastKeyPress=millis()/1000;
        break;
      case stateLoadRaceCheckPoint:
        Serial.println("  to: stateLoadRaceCheckPoint");
        loadRaceCheckPoint();     
        break;
      case stateMainMenu:
        Serial.println("  to: stateMainMenu");
        displayTimeOutEnable=true;
        status.flags.startStopState=stateOff;
        startStopStartsRace=false;        
        dispRace=race.activeRace;
        dispRaceLeg=race.activeLeg;
        menuItem=0;
        raceSelectHighlight=false;
        raceLegSelectHighlight=false;
        //This is to pop any dummy menus of the stack and return us the the active menu tree.
        //If we weren't in a dummy menu, then this is a no op.
        (*menuStack.back()).keypress(0);
        setAllButtonColor(COLOR_BLACK);
        ledDispFunc=ledDispDashes;
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));
      
        break;
      case stateRaceStart:
        Serial.println("  to: stateRaceStart");
        displayTimeOutEnable=false;
        status.flags.startStopState=stateBreath;
        prepRace();
        startStopStartsRace=true;
        ledDispFunc=LEDDisplayFuncs[LEDDisplayActive];
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, OLEDDisplayFuncs[OLEDDisplayActive[0]]));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, OLEDDisplayFuncs[OLEDDisplayActive[1]]));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, OLEDDisplayFuncs[OLEDDisplayActive[2]]));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, OLEDDisplayFuncs[OLEDDisplayActive[3]]));
        break;
      case stateDelayedStart:
        Serial.println("  to: stateDelayedStart");
        ledDispFunc=ledDispStartCountdown;
        break;
      case stateLegActive:
        Serial.println("  to: stateLegActive");
        ledDispFunc=LEDDisplayFuncs[LEDDisplayActive];
        break;
      case stateLegComplete:
        Serial.println("  to: stateLegComplete");
        startStopStartsRace=false;
        updateRace();
        raceCheckPoint();
        logRace(race.legData, 0);
        logRace(&race, 1);
        clearRacePoints(race.activeLeg);
        ledDispFunc=ledDispDashes;
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayLegSummaryActual));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayLegSummaryAdjusted));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceSummary1));
        displayList.push_back(new displayContent(oledDisp4, dispNA, dispNA, displayRaceSummary2));        
        break;
      case stateRaceComplete:
        Serial.println("  to: stateRaceComplete");   
        race.inProgress=false;
        if(race.activeRace!=NULL ) {
          race.activeRace->inProgress=false;
        }
        race.activeLeg=NULL;
        race.activeRace=NULL;
        raceCheckPoint();
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
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));    
        break;
      case stateSelectRaceLeg:
        Serial.println("  to: stateSelectRaceLeg");
        menuItem=255;
        raceLegSelectHighlight=true;
        selectedRaceLeg=race.activeRace->raceLegs.begin();
        dispRace=(race.activeRace);
        dispRaceLeg=(*selectedRaceLeg);
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayRaceInfo));              
        break;
      case stateSaveSelection:
        Serial.println("  to: stateSaveSelection");
        setRace((*selectedRace));
        setLeg((*selectedRaceLeg));
        break;
      case stateCancelSelection:
        Serial.println("  to: stateCancelSelection");
        selectedRace=selectedRaceSave;
        break;
      case stateNextLeg:
        Serial.println("  to: stateNextLeg");
        selectedRaceLeg++;
        if(selectedRaceLeg != (*selectedRace)->raceLegs.end()) {
          race.activeLeg=(*selectedRaceLeg);
          setLeg((*selectedRaceLeg));
        } else {
          race.activeLeg=NULL;
        }

        break;
      case stateDispSelect:
        Serial.println("  to: stateDispSelect");
        dispSelectChange=false;
        LEDDisplaySelect=LEDDisplayActive;
        for (int idx=0; idx<4; idx++) {
          OLEDDisplaySelect[idx]=OLEDDisplayActive[idx];
        }
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayDisplayConfig));
        break;
      case stateCheckFirmwareUpdate:
        Serial.println("  to: stateCheckFirmwareUpdate");
        break;
      case stateConfirmFirmwareUpdate:
        Serial.println("  to: stateConfirmFirmwareUpdate");
        setButtonColor(KEYPAD_KEY_ENTER, COLOR_GREEN);
        setButtonColor(KEYPAD_KEY_ESC, COLOR_RED);
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayFirmwareConfirm));
        break;
      case stateDoFirmwareUpdate:
        setAllButtonColor(COLOR_BLACK);
        displayList.erase(displayList.begin(), displayList.end());
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayFirmwareUpdate));        
        Serial.println("  to: stateDoFirmwareUpdate");
        doFirmwareUpdate();
        break;
      case stateSetLedBrightness:
        Serial.println("  to: stateSetLedBrightness");
        displayList.erase(displayList.begin(), displayList.end());
        ledBrightnessTmp=ledBrightness;
        ledDispFunc=ledDispEights;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySetLedBrightness));       
        break; 
      case stateSetOledBrightness:
        Serial.println("  to: stateSetOledBrightness");
        displayList.erase(displayList.begin(), displayList.end());
        oledBrightnessTmp=oledBrightness;
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySetOledBrightness));               
        break; 
      case stateSetScreenBlankTime:
        Serial.println("  to: stateSetScreenBlankTime");
        displayTimeoutTmp=displayTimeout;
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySetDisplayTimeout));     
        break; 
      case stateSetLegTimeAdjust:
        Serial.println("  to: stateSetLegTimeAdjust");
        autoAdjustLegTimeSave=autoAdjustLegTime;
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySetLegTimeAdjust));     
        break; 
      case stateScreenBlank:
        Serial.println("  to: stateScreenBlank");
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispBlank;    
        keypadStartBreath();    
        break; 
      case stateScreenWake:
        Serial.println("  to: stateScreenWake");
        keypadStopBreath();
        break;
      case stateSaveSettings:
        Serial.println("  to: stateSaveSettings");
        saveSettings();
        break; 
      case stateLoadSettings:
        Serial.println("  to: stateLoadSettings");
        loadSettings();
        break; 
      case stateEditRaceLeg:
        Serial.println("  to: stateEditRaceLeg");
        legAdjustMode=0;
        //This is to pop any dummy menus of the stack and return us the the active menu tree.
        //If we weren't in a dummy menu, then this is a no op.
        (*menuStack.back()).keypress(0);
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displayAdjustLeg));     
        break; 
      case stateAdjustLegTime:
        Serial.println("to: stateAdjustLegTime");
        //only editing one, but we need to backup all three current values because we will restore
        //all three values if we escape out of the edit.  Don't want to restore the wrong contents for
        //either none edited value.
        legAdjustDistBackup=legAdjustDist;
        legAdjustSpeedBackup=legAdjustSpeed;
        legAdjustTimeBackup=legAdjustTime;
        legAdjustRow=0;
        legAdjustColumn=-1;
        legAdjustMode=1;
        break; 
      case stateAdjustLegDistance:
        Serial.println("to: stateAdjustLegDistance");
        //only editing one, but we need to backup all three current values because we will restore
        //all three values if we escape out of the edit.  Don't want to restore the wrong contents for
        //either none edited value.        
        legAdjustDistBackup=legAdjustDist;
        legAdjustSpeedBackup=legAdjustSpeed;
        legAdjustTimeBackup=legAdjustTime;
        legAdjustRow=1;
        legAdjustColumn=-1;
        legAdjustMode=1;
        break; 
      case stateAdjustLegSpeed:
        Serial.println("to: stateAdjustLegSpeed");
        //only editing one, but we need to backup all three current values because we will restore
        //all three values if we escape out of the edit.  Don't want to restore the wrong contents for
        //either none edited value.
        legAdjustDistBackup=legAdjustDist;
        legAdjustSpeedBackup=legAdjustSpeed;
        legAdjustTimeBackup=legAdjustTime;
        legAdjustRow=2;
        legAdjustColumn=-1;
        legAdjustMode=1;
        break; 
      case stateAdjustLegSave:
        Serial.println("to: stateAdjustLegSave");
        //convert distance and speed from floating point miles and mph back to internal integer units.
        //Time carries through directly as milliseconds.
        race.legData->driveDistance=DISTANCE_MILES_TO_INTERNAL(legAdjustDist);
        race.legData->adjustedTargetSpeed=SPEED_MPH_TO_INTERNAL(legAdjustSpeed);
        race.legData->time=legAdjustTime;
        legAdjustMode=0;
        break;       
      case stateAdjustLegRestore:
        Serial.println("to: stateAdjustLegRestore");
        //To keep from having three different states, we simply restore all the values if we escape out
        //of any edit.  This works because we backed up the current value for all three fields when we
        //entered edit more for any of them.
        legAdjustDist=legAdjustDistBackup;
        legAdjustSpeed=legAdjustSpeedBackup;
        legAdjustTime=legAdjustTimeBackup;  
        legAdjustMode=0;         
        break;
      case stateAdjustLegCaptureValues:
        Serial.println("to: stateAdjustLegCaptureValues");
        //grab the leg values we can edit.  
        //time is in milliseconds and can easily be manipulated that way.
        //We convert distance and speed to miles and mph to make the edit logic way, way
        //easier.
        legAdjustDist=DISTANCE_INTERNAL_TO_MILES(race.legData->driveDistance);
        legAdjustSpeed=SPEED_INTERNAL_TO_MPH(race.legData->adjustedTargetSpeed);
        legAdjustTime=(int32_t)round(race.legData->time);
        Serial.printf("legAjustTime: %d, race.legData->time: %f\n", legAdjustTime,race.legData->time);
        break;
      case stateAdjustLegResetValues:
        //probably could just use stateSaveSelection as it is the same action, but if we need to do something
        //different, we won't need to split the states.
        Serial.println("to: stateAdjustLegResetValues");
        setLeg((*selectedRaceLeg));
        break;
      case stateShowSystemInfo:
        Serial.println("to: stateShowSystemInfo");      
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp1, dispNA, dispNA, displayMenuTitle));
        displayList.push_back(new displayContent(oledDisp2, dispNA, dispNA, displayMenu));
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySystemInfo));        
        break;     
      case stateSetHwVer:
        Serial.println("  to: stateSetHwVer");  
        displayList.erase(displayList.begin(), displayList.end());
        ledDispFunc=ledDispDashes;
        displayList.push_back(new displayContent(oledDisp3, dispNA, dispNA, displaySystemInfo));                 
        break;             
      case stateResetAllSettings:
        Serial.println("  to: stateResetAllSettings");
        resetSettings();
        break;
      case stateReboot:
        Serial.println("  to: stateReboot");
        doReboot();
        break;
      case stateUnknown:
        Serial.println("  to: stateUnknown");
        break;
    }      
    lastState=state;
  }

  //evaluated every pass through the state machine.  This handles state transitions caused
  //by async events in the race timing or GPS systems or any other logic based state transitions.  
  //No actions or status updates allowed in this block.  State transitions only.
  switch(state) {
    case stateInit:
      if(initHwVer) {
        state=stateSetHwVer;
      } else {
        state=stateLoadSettings;
      }
      break;
    case stateLoadRaceCheckPoint:
      state=stateMainMenu;  
      break;      
    case stateMainMenu:
      if(displayTimeout>0 && (millis()/1000)-lastKeyPress>(displayTimeout*60)) {
        state=stateScreenBlank;
      }
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
    case stateDispSelect:
      break;
    case stateCheckFirmwareUpdate:
        state=stateMainMenu;
        if(checkForUpdate()) {
          state=stateConfirmFirmwareUpdate;
        }
        break;
    case stateConfirmFirmwareUpdate:
      break;
    case stateDoFirmwareUpdate:
      state=stateMainMenu;
      break;      
    case stateSetLedBrightness:
      break; 
    case stateSetOledBrightness:
      break; 
    case stateSetScreenBlankTime:
      break;
    case stateSetLegTimeAdjust:
      break;
    case stateScreenBlank:  
      break; 
    case stateScreenWake:
      state=stateMainMenu;
      break;
    case stateSaveSettings:
      state=stateMainMenu;  
      break; 
    case stateLoadSettings:
      state=stateLoadRaceCheckPoint; 
      break; 
    case stateEditRaceLeg:
      break; 
    case stateAdjustLegTime:
      break; 
    case stateAdjustLegDistance:
      break; 
    case stateAdjustLegSpeed:
      break; 
    case stateAdjustLegSave:
      state=stateMainMenu;
      break;       
    case stateAdjustLegRestore:
      state=stateMainMenu;
      break;
    case stateAdjustLegCaptureValues:
      state=stateEditRaceLeg;
      break;
    case stateAdjustLegResetValues:
      state=stateMainMenu;
      break;
    case stateShowSystemInfo:
        break;
    case stateSetHwVer:
        break;
    case stateReboot:
        break;
    case stateResetAllSettings:
        state=stateReboot;
        break;
    case stateUnknown:
      break;
  }

  //Everything below here should only be executed if there is a keypress, or if the status flags 
  //have changed.
  if(keys==0 && (status.value==lastStatus.value)) {
    return;
  }
  lastKeyPress=millis()/1000;
  //state machine evaluation based on keypad input.  The only state transitions here should be
  //driven by a key press.  No actions or status updates allowed in this block.  State transitions only.
  //note that if we just transitioned into stateDisplayBlank, on this pass through the loop, and a key
  //was also pressed, that keystroke will get swallowed by the transition into and out of screen blanking
  //even though the screen will never blank.  We could do a special case of that detection by comparing 
  //the current state with the last state, and if they differ AND the current state is stateDisplayBlank
  //then we drop back to the last state, which should be stateMainMenu.  Will see how often it comes up
  //before putting the code in.
  switch(state) {
    case stateInit:
      break;
    case stateLoadRaceCheckPoint:
      break;            
    case stateMainMenu:
    case stateEditRaceLeg:
      menuAction=(*menuStack.back()).keypress(keys);
      switch(menuAction) {
        case menuActionStart:
          if(race.activeRace!=NULL && status.flags.gpsReady) {
              state=stateRaceStart;
          }
          break;
        case menuActionSelectRace:
          if(!race.inProgress) {
            state=stateSelectRace;
          } else {
            //we can't prevent entering the menu, but we don't want to go in there since there isn't a race 
            //selected, so we force a return back up one level of the menu stack.  It will appear as though
            //nothing happened when we selected the menu.
            menuStack.pop_back();
          }
          break;
        case menuActionSelectLeg:
          if(race.activeRace!=NULL && !race.inProgress) {
            state=stateSelectRaceLeg;
          } else {
            //we can't prevent entering the menu, but we don't want to go in there since there isn't a race 
            //selected, so we force a return back up one level of the menu stack.  It will appear as though
            //nothing happened when we selected the menu.
            menuStack.pop_back();
          }
          break;
        case menuActionConfigDisplay:
          state=stateDispSelect;
          break;
        case menuActionCancelRace:
          state=stateRaceComplete;
          break;
        case menuActionFirmwareUpdate:
          state=stateCheckFirmwareUpdate;
          break;
        case menuActionLEDBrightness:
          state=stateSetLedBrightness;
          break;
        case menuActionOLEDBrightness:
          state=stateSetOledBrightness;
          break;
        case menuActionScreenTimeout:
          state=stateSetScreenBlankTime;
          break;
        case menuActionAutoAdjustLeg:
          state=stateSetLegTimeAdjust;
          break;
        case menuActionAdjustLeg:
          state=stateAdjustLegCaptureValues;
          break;
        case menuActionSystemInfo:
          state=stateShowSystemInfo;
          break;
        case menuActionReboot:
          state=stateReboot;
          break;
        case menuActionResetSettings:
          state=stateResetAllSettings;
          break;
        case menuActionAdjustTime:
          state=stateAdjustLegTime;
          break;
        case menuActionAdjustDistance:
          state=stateAdjustLegDistance;
          break;
        case menuActionAdjustSpeed:
          state=stateAdjustLegSpeed;
          break;
        case menuActionAdjustSave:
          //We are currently in the leg adjustment menu and "Save" was selected.  We need to exit
          //this menu to signify that settings were saved.  We simulate an ESC being pressed and
          //pop the adjustment menu off the menu stack and return to the main menu state.
          menuStack.pop_back();
          state=stateAdjustLegSave;
          break;          
        case menuActionEsc:
          if(state==stateEditRaceLeg) {
            state=stateAdjustLegRestore;
          }
          break;
        case menuActionMainMenu:
          state=stateMainMenu;
          break;
        case menuActionAdjustReset:
          //We are currently in the leg adjustment menu and "Reset" was selected.  We need to exit
          //this menu to signify that settings were saved.  We simulate an ESC being pressed and
          //pop the adjustment menu off the menu stack and return to the main menu state.
          menuStack.pop_back();        
          state=stateAdjustLegResetValues;
          break;
        case menuActionUp:
          menuStack.pop_back();
          break;
        default:
          break;
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
    case stateDispSelect:
      if(displaySelectLineMode==false) {
        if(keys & KEYPAD_KEY_ESC) {
          if(dispSelectChange) {
            state=stateSaveSettings;
          } else {
            state=stateMainMenu;
          }
        }
        if(keys & KEYPAD_KEY_UP) {
          if(displaySelectLine==0) {
            displaySelectLine=4;
          } else {
            displaySelectLine--;
          }
        }
        if(keys & KEYPAD_KEY_DOWN) {
          if(displaySelectLine==4) {
            displaySelectLine=0;
          } else {
            displaySelectLine++;
          }
        } 
        if(keys & KEYPAD_KEY_ENTER) {
          displaySelectLineMode=true;
        }
      } else {
        if(keys & KEYPAD_KEY_ESC) {
          displaySelectLineMode=false;
          if(displaySelectLine==0) {
            LEDDisplaySelect=LEDDisplayActive;
          } else {
            OLEDDisplaySelect[displaySelectLine-1]=OLEDDisplayActive[displaySelectLine-1];
          }   
        }       

        if(keys & KEYPAD_KEY_UP) {
          if(displaySelectLine==0) {
            if(LEDDisplaySelect==LEDDispFuncMinValue) {
              LEDDisplaySelect=LEDDispFuncMaxValue-1;
            } else {
              LEDDisplaySelect--;
            } 
          } else {
            if(OLEDDisplaySelect[displaySelectLine-1]==OLEDDispFuncMinValue) {
              OLEDDisplaySelect[displaySelectLine-1]=OLEDDispFuncMaxValue-1;
            } else {
              OLEDDisplaySelect[displaySelectLine-1]--;
            }
          }
        }        

        if(keys & KEYPAD_KEY_DOWN) {
          if(displaySelectLine==0) {
            if(LEDDisplaySelect==LEDDispFuncMaxValue-1) {
              LEDDisplaySelect=LEDDispFuncMinValue;
            } else {
              LEDDisplaySelect++;
            } 
          } else {
            if(OLEDDisplaySelect[displaySelectLine-1]==OLEDDispFuncMaxValue-1) {
              OLEDDisplaySelect[displaySelectLine-1]=OLEDDispFuncMinValue;
            } else {
              OLEDDisplaySelect[displaySelectLine-1]++;
            }
          }
        }  
        
        if(keys & KEYPAD_KEY_ENTER) {
          displaySelectLineMode=false;
          dispSelectChange=true;
          if(displaySelectLine==0) {
            LEDDisplayActive=LEDDisplaySelect;
          } else {
            OLEDDisplayActive[displaySelectLine-1]=OLEDDisplaySelect[displaySelectLine-1];
          }
        }
      }
           
      break;
    case stateCheckFirmwareUpdate:
      break;
    case stateConfirmFirmwareUpdate:
      if(keys & KEYPAD_KEY_ESC) {
        state=stateMainMenu;
      }
      if(keys & KEYPAD_KEY_ENTER) {
        state=stateDoFirmwareUpdate;
      }      
      break;
    case stateDoFirmwareUpdate:
      break;    
    case stateSetLedBrightness:
      menuAction=menuSetLedBrightness(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateSaveSettings;
          break;
        case 2:
          state=stateMainMenu;
          break;
        default:
          break;
      }
      break; 
    case stateSetOledBrightness:
      menuAction=menuSetOledBrightness(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateSaveSettings;
          break;
        case 2:
          state=stateMainMenu;
          break;
        default:
          break;
      }
      break; 
    case stateSetScreenBlankTime:
      menuAction=menuSetScreenTimeout(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateSaveSettings;
          break;
        case 2:
          state=stateMainMenu;
          break;
        default:
          break;
      }
      break; 
    case stateSetLegTimeAdjust:
      menuAction=menuAutoAjustLegTime(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateSaveSettings;
          break;
        case 2:
          state=stateMainMenu;
          break;
        default:
          break;
      }
      break; 

    case stateScreenBlank:
      state=stateScreenWake;
      break; 
    case stateScreenWake:
      break;
    case stateSaveSettings:
      break; 
    case stateLoadSettings:
      break; 
    case stateAdjustLegTime:
      menuAction=menuAdjustLegTime(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateEditRaceLeg;
          break;
        case 2:
          state=stateEditRaceLeg;
          break;
        default:
          break;
      }
      break;
    case stateAdjustLegDistance:
      menuAction=menuAdjustLegDistance(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateEditRaceLeg;
          break;
        case 2:
          state=stateEditRaceLeg;
          break;
        default:
          break;
      }  
      break;
    case stateAdjustLegSpeed:
      menuAction=menuAdjustLegSpeed(keys);
      switch(menuAction) {
        case 0:
          break;
        case 1:
          state=stateEditRaceLeg;
          break;
        case 2:
          state=stateEditRaceLeg;
          break;
        default:
          break;
      }      
      break; 
    case stateAdjustLegSave:
      break;       
    case stateAdjustLegRestore:
      break;
    case stateAdjustLegCaptureValues:
      break;
    case stateAdjustLegResetValues:
      break;
    case stateShowSystemInfo:
      state=stateMainMenu;
      break;
    case stateSetHwVer:
      menuAction=menuSetHwVer(keys);
      if(menuAction==1) {
        state=stateLoadSettings;
      }
      break;
    case stateReboot:
      break;
    case stateResetAllSettings:
      break;
    case stateUnknown:
      break;
  }

  //Handle the button lighting based on the current state of the system.
  //Serial.printf("keys:%d status:%d  lastStatus:%d\n", keys, status.value, lastStatus.value);

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

  if(status.flags.buttonColor != lastStatus.flags.buttonColor && race.legData->inProgress) {
    Serial.printf("set all buttons: %d\n",status.flags.buttonColor);
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