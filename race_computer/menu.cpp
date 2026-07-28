#include "menu.h"
#include "keypad.h"
#include "display.h"
#include <vector>
#include <iterator>
#include <span>
#include <math.h>

std::vector<menuEntry_t> mainMenuEntries={
    {"Start", menuActionStart, NULL},
    {"Race Menu", menuActionMainMenu, &raceMenu},
    {"Configuration Menu", menuActionMainMenu, &configMenu},
    {"System Menu", menuActionMainMenu, &systemMenu}
};

std::vector<menuEntry_t> raceMenuEntries={
    {"Select Race", menuActionSelectRace, &selectRaceMenu},
    {"Select Leg", menuActionSelectLeg, &selectLegMenu},
    {"Adjust Leg", menuActionAdjustLeg, &adjustLegMenu},
    {"Cancel Race", menuActionCancelRace, &cancelRaceMenu}
};

std::vector<menuEntry_t> configMenuEntries={
    {"Configure Display", menuActionConfigDisplay, &configDisplayMenu},
    {"LED Brightness", menuActionLEDBrightness, &ledBrightMenu},
    {"OLED Brightness", menuActionOLEDBrightness, &oledBrightMenu},
    {"Screen Timeout", menuActionScreenTimeout, &screenTimeoutMenu},
    {"Auto Adjust Leg Timing", menuActionAutoAdjustLeg, &autoAdjustLegTimeMenu},
    {"Speed Band Compare Source", menuActionSpeedBandSource, &speedBandSourceMenu},
    {"Reset All Configuration Settings", menuActionMainMenu, &resetAllSettingsMenu}
};

std::vector<menuEntry_t> systemMenuEntries={
    {"Firmware Update", menuActionFirmwareUpdate, &firmwareUpdateMenu},
    {"System Information", menuActionSystemInfo, &systemInformationMenu},
    {"Reboot", menuActionReboot, &rebootMenu},
};

std::vector<menuEntry_t> adjustLegMenuEntries={
    {"Adjust Time", menuActionAdjustTime,&adjustTimeMenu},
    {"Adjust Distance", menuActionAdjustDistance,&adjustDistanceMenu},
    {"Adjust Target Speed", menuActionAdjustSpeed,&adjustSpeedMenu},
    {"Adjust Starting Mark", menuActionAdjustMark, &adjustMarkMenu},
    {"Save Adjustments", menuActionAdjustSave, nullptr},
    {"Reset All Adjustments", menuActionAdjustReset, nullptr},
};

std::vector<menuEntry_t> settingsResetMenuEntries={
    {"No", menuActionUp, nullptr},
    {"Yes", menuActionResetSettings, nullptr}
};

std::vector<menuEntry_t> dummyMenuEntries;

menu_t mainMenu("Main Menu", mainMenuEntries, 4);
menu_t raceMenu("Race Menu", raceMenuEntries, 4);
menu_t configMenu("Configuration Menu", configMenuEntries, 4);
menu_t systemMenu("System Menu", systemMenuEntries, 3);

menu_t selectRaceMenu("Select Race", dummyMenuEntries, 0);
menu_t selectLegMenu("Select Leg", dummyMenuEntries, 0);
menu_t adjustLegMenu("Adjust Leg", adjustLegMenuEntries, 4);
menu_t cancelRaceMenu("Cancel Race", dummyMenuEntries, 0);

menu_t configDisplayMenu("Config Display", dummyMenuEntries, 0);
menu_t ledBrightMenu("LED Brightness", dummyMenuEntries, 0);
menu_t oledBrightMenu("OLED Brightness", dummyMenuEntries, 0);
menu_t screenTimeoutMenu("Screen Timeout", dummyMenuEntries, 0);
menu_t autoAdjustLegTimeMenu("Leg Timing Adjust", dummyMenuEntries, 0);
menu_t speedBandSourceMenu("Speed Band Source", dummyMenuEntries, 0);
menu_t resetAllSettingsMenu("Reset All Settings", settingsResetMenuEntries, 2);


menu_t firmwareUpdateMenu("Firmware Update", dummyMenuEntries, 0);
menu_t systemInformationMenu("System Information", dummyMenuEntries, 0);
menu_t rebootMenu("Reboot", dummyMenuEntries, 0);

menu_t adjustTimeMenu("Adjust Leg Time", dummyMenuEntries, 0);
menu_t adjustDistanceMenu("Adjust Leg Distance", dummyMenuEntries, 0);
menu_t adjustSpeedMenu("Adjust Leg Speed", dummyMenuEntries, 0);
menu_t adjustMarkMenu("Adjust Start Mark", dummyMenuEntries, 0);
menu_t adjustSaveMenu("Save Adjustments", dummyMenuEntries, 0);


//Main menu is always on the stack.  It's our starting point.
std::vector<menu_t*> menuStack = { &mainMenu };

int legAdjustRow=0;
int legAdjustColumn=-1;
int legAdjustMode=0;
int32_t legAdjustTime;
int32_t legAdjustTimeTmp;
int32_t legAdjustTimeBackup;
double legAdjustDist;
double legAdjustDistTmp;
double legAdjustDistBackup;
double legAdjustSpeed;
double legAdjustSpeedTmp;
double legAdjustSpeedBackup;
int32_t legAdjustMark;
int32_t legAdjustMarkTmp;
int32_t legAdjustMarkBackup;
bool autoAdjustLegTimeSave;
int speedBandSourceSave;

menu::menu(std::string menuTitle, std::vector<menuEntry_t> menuEntries, uint8_t displayLines) {
    title=menuTitle;
    entries=menuEntries;
    entryCount=std::size(menuEntries);
    lines=displayLines;
    displayTop=0;
    if(entryCount) {
        activeEntry=0;
    } else {
        activeEntry=-1;
    }
};


menu::~menu() {
    return;
}

uint8_t menu::keypress(uint8_t key) {
    //If we currently point at a dummy menu entry when called, we need to pop up the stack one level
    //to get back to an active menu.  This case happens when the user hits enter on a menu that leads
    //to an action outside navigating the menu.  
    if(lines==0) {
        if(menuStack.size()>1) {
            //This removes the dummy menu from the menu navigation stack.
            menuStack.pop_back();
            //Treat this as though we are re-entering the active menu.
            //Need to look at the structures and see if we still need this in all cases or if we can 
            //get rid of the dummyEntry in the menu structure.
            return((*menuStack.back()).entries[activeEntry].action);
        }        
        //This case shouldn't happen, but it it does, we reload the main menu and go there.
        menuStack.push_back(&mainMenu);
        return (*menuStack.back()).keypress(key);
    }
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return menuActionNone;
            break;
        case KEYPAD_KEY_ENTER:
            if(entries[activeEntry].subMenu!=nullptr) {
                menuStack.push_back(entries[activeEntry].subMenu);
            }
            return(entries[activeEntry].action);
            break;
        case KEYPAD_KEY_DOWN:
            if(activeEntry==entryCount-1) {
                return menuActionNone;
            }
            activeEntry++;
            if(activeEntry-displayTop==lines) {
                displayTop++;
            }
            return menuActionNone;
            break;
        case KEYPAD_KEY_UP:
            if(activeEntry==0) {
                return menuActionNone;   
            }
            activeEntry--;
            if(activeEntry<displayTop) {
                displayTop=activeEntry;
            }   
            return menuActionNone;
            break;
        case KEYPAD_KEY_ESC:
            if(menuStack.size()>1) {
                menuStack.pop_back();
            }
            return menuActionEsc;
            break;
        
        default:
            return menuActionNone;
            break;
    }
}

const char * menu::getLine(uint8_t line) {
    if((displayTop+line)==entries.size()) {
      return("");
    }
    return entries[displayTop+line].text.c_str();
}

int8_t menu::getActiveLine(void) {
    return activeEntry-displayTop;
}

const char * menu::operator[](uint8_t i) {
    if((displayTop+i)>=entries.size()) {
      return("");
    }
    return entries[displayTop+i].text.c_str();
}

const char * menu::getMenuTitle(void) {
    return title.c_str();
}

bool menu::moreUp(void) {
    if(displayTop!=0) {
        return true;
    } 
    return false;
}

bool menu::moreDown(void) {
    if(displayTop+4<entryCount) {
        return true;
    }
    return false;
}

uint8_t menuSetLedBrightness(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return 0;
            break;
        case KEYPAD_KEY_ENTER:
            ledBrightness=ledBrightnessTmp;
            return 1;
            break;
        case KEYPAD_KEY_DOWN:
            if(ledBrightnessTmp>0) {
                ledBrightnessTmp--;
                while(SPILock);
                doSPILock();
                ledDisp.Set_Brightness(ledBrightnessTmp);
                doSPIUnlock();
            }
            break;
        case KEYPAD_KEY_UP:
            if(ledBrightnessTmp<14) {
                ledBrightnessTmp++;
                while(SPILock);
                doSPILock();
                ledDisp.Set_Brightness(ledBrightnessTmp);
                doSPIUnlock();
            }
            break;
        case KEYPAD_KEY_ESC:
            while(SPILock);
            doSPILock();
            ledDisp.Set_Brightness(ledBrightness);      
            doSPIUnlock();      
            return 2;
            //make analysis tools happy
            break;
        default:
            break;
    }
    return 0;
}

uint8_t menuSetOledBrightness(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return 0;
            break;
        case KEYPAD_KEY_ENTER:
            oledBrightness=oledBrightnessTmp;
            return 1;
            break;
        case KEYPAD_KEY_DOWN:
            if(oledBrightnessTmp>0) {
                oledBrightnessTmp-=25;
                while(SPILock);
                doSPILock();
                oledDisp1.setContrast(oledBrightnessTmp);
                oledDisp2.setContrast(oledBrightnessTmp);
                oledDisp3.setContrast(oledBrightnessTmp);
                oledDisp4.setContrast(oledBrightnessTmp);
                doSPIUnlock();
            }
            break;
        case KEYPAD_KEY_UP:
            if(oledBrightnessTmp<250) {
                oledBrightnessTmp+=25;
                while(SPILock);
                doSPILock();
                oledDisp1.setContrast(oledBrightnessTmp);
                oledDisp2.setContrast(oledBrightnessTmp);
                oledDisp3.setContrast(oledBrightnessTmp);
                oledDisp4.setContrast(oledBrightnessTmp);
                doSPIUnlock();
            }
            break;
        case KEYPAD_KEY_ESC:
            while(SPILock);
            doSPILock();
            oledDisp1.setContrast(oledBrightness);
            oledDisp2.setContrast(oledBrightness);
            oledDisp3.setContrast(oledBrightness);
            oledDisp4.setContrast(oledBrightness);     
            doSPIUnlock();      
            return 2;
            //make analysis tools happy
            break;            
        default:
            break;
    }
    return 0;
}

uint8_t menuSetScreenTimeout(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return 0;
            break;
        case KEYPAD_KEY_ENTER:
            displayTimeout=displayTimeoutTmp;
            return 1;
            break;
        case KEYPAD_KEY_DOWN:
            if(displayTimeoutTmp>0) {
                displayTimeoutTmp-=5;
            }
            break;
        case KEYPAD_KEY_UP:
            if(displayTimeoutTmp<60) {
                displayTimeoutTmp+=5;
            }
            break;
        case KEYPAD_KEY_ESC:
            return 2;
            //make analysis tools happy
            break;            
        default:
            break;
    }
    return 0;
}

uint8_t menuAdjustLegTime(uint8_t key) {
    int ret=0;
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            ret=0;
            break;
        case KEYPAD_KEY_ENTER:
            if(legAdjustMode==1) {
                legAdjustMode=2;
                legAdjustTimeTmp=legAdjustTime;
                legAdjustSpeedTmp=legAdjustSpeed;
            } else {
                legAdjustMode=1;
            }
            ret=0;
            break;
        case KEYPAD_KEY_DOWN:
            if(legAdjustMode==1) {
                if(legAdjustColumn>-3) {
                    if(legAdjustColumn==7 || legAdjustColumn==4 || legAdjustColumn==1) {
                        legAdjustColumn-=2;
                    } else {   
                        legAdjustColumn--;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustTime>1) {
                            legAdjustTime-=1;
                        }
                        break;
                    case -2:
                        if(legAdjustTime>=10) {
                            legAdjustTime-=10;
                        }
                        break;                        
                    case -1:
                        if(legAdjustTime>=100) {
                            legAdjustTime-=100;
                        }
                        break;
                    case 1:
                        if(legAdjustTime>=1000) {
                            legAdjustTime-=1000;
                        }
                        break;    
                    case 2:
                        if(legAdjustTime>=10000) {
                            legAdjustTime-=10000;
                        }
                        break;    
                    case 4:
                        if(legAdjustTime>=60000) {
                            legAdjustTime-=60000;
                        }
                        break;         
                    case 5:
                        if(legAdjustTime>=600000) {
                            legAdjustTime-=600000;
                        }
                        break;         
                    case 7:
                        if(legAdjustTime>=3600000) {
                            legAdjustTime-=3600000;
                        }
                        break;
                    case 8:
                        if(legAdjustTime>=36000000) {
                            legAdjustTime-=36000000;
                        }                        
                        break;         
                }
                //Time changed, so recalculate target speed.  Time is in milliseconds.
                //Scale time to hours to calculate miles per hour.
                legAdjustSpeed=legAdjustDist/(legAdjustTime/3600000.0);
            }
            ret=0;
            break;
        case KEYPAD_KEY_UP:
            if(legAdjustMode==1) {
                if(legAdjustColumn<8) {
                    if(legAdjustColumn==-1 || legAdjustColumn==2 || legAdjustColumn==5) {
                        legAdjustColumn+=2;
                    } else {   
                        legAdjustColumn++;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustTime<359999999) {
                            legAdjustTime+=1;
                        }
                        break;
                    case -2:
                        if(legAdjustTime<359999990) {
                            legAdjustTime+=10;
                        }
                        break;                        
                    case -1:
                        if(legAdjustTime<359999900) {
                            legAdjustTime+=100;
                        }
                        break;
                    case 1:
                        if(legAdjustTime<359999000) {
                            legAdjustTime+=1000;
                        }
                        break;    
                    case 2:
                        if(legAdjustTime<359990000) {
                            legAdjustTime+=10000;
                        }
                        break;    
                    case 4:
                        if(legAdjustTime<359939999) {
                            legAdjustTime+=60000;
                        }
                        break;         
                    case 5:
                        if(legAdjustTime<359399999) {
                            legAdjustTime+=600000;
                        }
                        break;         
                    case 7:
                        if(legAdjustTime<356399999) {
                            legAdjustTime+=3600000;
                        }
                        break;
                    case 8:
                        if(legAdjustTime<323999999) {
                            legAdjustTime+=36000000;
                        }                        
                        break;         
                }
                //Time changed, so recalculate target speed.  Time is in milliseconds.
                //Scale time to hours to calculate miles per hour.
                legAdjustSpeed=legAdjustDist/(legAdjustTime/3600000.0);
            }   
            ret=0;
            break;
        case KEYPAD_KEY_ESC:
            if(legAdjustMode==2) {
                legAdjustMode=1;
                legAdjustTime=legAdjustTimeTmp;
                legAdjustSpeed=legAdjustSpeedTmp;
                ret=0;
            } else {
                ret=2;
            }
            //make analysis tools happy
            break;            
        default:
            break;
    }
    return ret;
}

uint8_t menuAdjustLegDistance(uint8_t key) {
    int ret=0;
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            ret=0;
            break;
        case KEYPAD_KEY_ENTER:
            if(legAdjustMode==1) {
                legAdjustMode=2;
                legAdjustDistTmp=legAdjustDist;
                legAdjustSpeedTmp=legAdjustSpeed;
            } else {
                legAdjustMode=1;
            }
            ret=0;
            break;
        case KEYPAD_KEY_DOWN:
            if(legAdjustMode==1) {
                if(legAdjustColumn>-3) {
                    if(legAdjustColumn==1) {
                        legAdjustColumn-=2;
                    } else {   
                        legAdjustColumn--;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustDist>0.001) {
                            legAdjustDist-=0.001;
                        }
                        break;
                    case -2:
                        if(legAdjustDist>=0.01) {
                            legAdjustDist-=0.01;
                        }
                        break;                        
                    case -1:
                        if(legAdjustDist>=0.1) {
                            legAdjustDist-=0.1;
                        }
                        break;
                    case 1:
                        if(legAdjustDist>=1) {
                            legAdjustDist-=1;
                        }
                        break;    
                    case 2:
                        if(legAdjustDist>=10) {
                            legAdjustDist-=10;
                        }
                        break;    
                    case 3:
                        if(legAdjustDist>=100) {
                            legAdjustDist-=100;
                        }
                        break;                                            
                }
                //Distance changed, so recalculate target speed.  Time is in milliseconds.
                //Scale time to hours to calculate miles per hour.
                legAdjustSpeed=legAdjustDist/(legAdjustTime/3600000.0);                 
            }

            ret=0;
            break;
        case KEYPAD_KEY_UP:
            if(legAdjustMode==1) {
                if(legAdjustColumn<3) {
                    if(legAdjustColumn==-1) {
                        legAdjustColumn+=2;
                    } else {   
                        legAdjustColumn++;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustDist<999.999) {
                            legAdjustDist+=0.001;
                        }
                        break;
                    case -2:
                        if(legAdjustDist<999.99) {
                            legAdjustDist+=0.01;
                        }
                        break;                        
                    case -1:
                        if(legAdjustDist<999.9) {
                            legAdjustDist+=0.1;
                        }
                        break;
                    case 1:
                        if(legAdjustDist<999) {
                            legAdjustDist+=1;
                        }
                        break;    
                    case 2:
                        if(legAdjustDist<990) {
                            legAdjustDist+=10;
                        }
                        break;    
                    case 3:
                        if(legAdjustDist<900) {
                            legAdjustDist+=100;
                        }
                        break;          
                }
                //Distance changed, so recalculate target speed.  Time is in milliseconds.
                //Scale time to hours to calculate miles per hour.
                legAdjustSpeed=legAdjustDist/(legAdjustTime/3600000.0);
            }   
            ret=0;
            break;
        case KEYPAD_KEY_ESC:
            if(legAdjustMode==2) {
                legAdjustMode=1;
                legAdjustDist=legAdjustDistTmp;
                legAdjustSpeed=legAdjustSpeedTmp;
                ret=0;
            } else {
                ret=2;
            }
            //make analysis tools happy
            break;            
        default:
            break;
    }
    return ret;}

uint8_t menuAdjustLegSpeed(uint8_t key) {
    int ret=0;
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            ret=0;
            break;
        case KEYPAD_KEY_ENTER:
            if(legAdjustMode==1) {
                legAdjustMode=2;
                legAdjustSpeedTmp=legAdjustSpeed;
                legAdjustTimeTmp=legAdjustTime;
            } else {
                legAdjustMode=1;
            }
            ret=0;
            break;
        case KEYPAD_KEY_DOWN:
            if(legAdjustMode==1) {
                if(legAdjustColumn>-3) {
                    if(legAdjustColumn==1) {
                        legAdjustColumn-=2;
                    } else {   
                        legAdjustColumn--;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustSpeed>0.001) {
                            legAdjustSpeed-=0.001;
                        }
                        break;
                    case -2:
                        if(legAdjustSpeed>=0.01) {
                            legAdjustSpeed-=0.01;
                        }
                        break;                        
                    case -1:
                        if(legAdjustSpeed>=0.1) {
                            legAdjustSpeed-=0.1;
                        }
                        break;
                    case 1:
                        if(legAdjustSpeed>=1) {
                            legAdjustSpeed-=1;
                        }
                        break;    
                    case 2:
                        if(legAdjustSpeed>=10) {
                            legAdjustSpeed-=10;
                        }
                        break;    
                    case 3:
                        if(legAdjustSpeed>=100) {
                            legAdjustSpeed-=100;
                        }
                        break;                                            
                }
                //Speed changed, so recalculate target time.  Speed over distance will give time in hours
                //Convert to milliseconds.
                legAdjustTime=(legAdjustDist/legAdjustSpeed)*3600000.0; 
            }
            ret=0;
            break;
        case KEYPAD_KEY_UP:
            if(legAdjustMode==1) {
                if(legAdjustColumn<3) {
                    if(legAdjustColumn==-1) {
                        legAdjustColumn+=2;
                    } else {   
                        legAdjustColumn++;
                    }
                }   
            } else {
                switch(legAdjustColumn) {
                    case -3:
                        if(legAdjustSpeed<999.999) {
                            legAdjustSpeed+=0.001;
                        }
                        break;
                    case -2:
                        if(legAdjustSpeed<999.99) {
                            legAdjustSpeed+=0.01;
                        }
                        break;                        
                    case -1:
                        if(legAdjustSpeed<999.9) {
                            legAdjustSpeed+=0.1;
                        }
                        break;
                    case 1:
                        if(legAdjustSpeed<999) {
                            legAdjustSpeed+=1;
                        }
                        break;    
                    case 2:
                        if(legAdjustSpeed<990) {
                            legAdjustSpeed+=10;
                        }
                        break;    
                    case 3:
                        if(legAdjustSpeed<900) {
                            legAdjustSpeed+=100;
                        }
                        break;    
                } 
                //Speed changed, so recalculate target time.  Speed over distance will give time in hours
                //Convert to milliseconds.
                legAdjustTime=(legAdjustDist/legAdjustSpeed)*3600000.0; 
            }   
            ret=0;
            break;
        case KEYPAD_KEY_ESC:
            if(legAdjustMode==2) {
                legAdjustMode=1;
                legAdjustSpeed=legAdjustSpeedTmp;
                legAdjustTime=legAdjustTimeTmp;
                ret=0;
            } else {
                ret=2;
            }
            //make analysis tools happy
            break;            
        default:
            break;
    }
    return ret;
}

uint8_t menuAdjustLegMark(uint8_t key) {
    int ret=0;
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            ret=0;
            break;
        case KEYPAD_KEY_ENTER:
            if(legAdjustMode==1) {
                legAdjustMode=2;
                legAdjustMarkTmp=legAdjustMark;
            } else {
                legAdjustMode=1;
            }
            ret=0;
            break;
        case KEYPAD_KEY_DOWN:
            if(legAdjustMode==1) {
                if(legAdjustColumn>1) {
                    legAdjustColumn--;
                }   
            } else {
                switch(legAdjustColumn) {
                    case 1:
                        if(legAdjustMark>0) {
                            legAdjustMark-=1;
                        }
                        break;
                    case 2:
                        if(legAdjustMark>=10) {
                            legAdjustMark-=10;
                        }
                        break;                        
                }
            }
            ret=0;
            break;
        case KEYPAD_KEY_UP:
            if(legAdjustMode==1) {
                if(legAdjustColumn<2) {
                    legAdjustColumn++;
                }   
            } else {
                switch(legAdjustColumn) {
                    case 1:
                        if(legAdjustMark<59) {
                            legAdjustMark+=1;
                        }
                        break;
                    case 2:
                        if(legAdjustMark<49) {
                            legAdjustMark+=10;
                        }
                        break;                        
                }
            }   
            ret=0;
            break;
        case KEYPAD_KEY_ESC:
            if(legAdjustMode==2) {
                legAdjustMode=1;
                legAdjustMark=legAdjustMarkTmp;
                ret=0;
            } else {
                ret=2;
            }
            break;
        default:
            break;
    }
    return ret;
}

uint8_t menuSetHwVer(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            storeHardwareVersionStruct(&hardwareVersion); 
            return 1;
            break;
        case KEYPAD_KEY_ENTER:
            return 0;
            break;
        case KEYPAD_KEY_DOWN:
            if(hardwareVersion.serialNo>1) {
                hardwareVersion.serialNo--; 
            }
            break;
        case KEYPAD_KEY_UP:
            hardwareVersion.serialNo++;
            break;
        case KEYPAD_KEY_ESC:
            return 0;
        default:
            break;
    }
    return 0;
}

uint8_t menuAutoAdjustLegTime(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return 0;
            break;
        case KEYPAD_KEY_ENTER:
            return 1;
            break;
        case KEYPAD_KEY_DOWN:
            autoAdjustLegTime=!autoAdjustLegTime;
            break;
        case KEYPAD_KEY_UP:
            autoAdjustLegTime=!autoAdjustLegTime;
            break;
        case KEYPAD_KEY_ESC:
            autoAdjustLegTime=autoAdjustLegTimeSave;
            return 2;
        default:
            break;
    }
    return 0;
}

uint8_t menuSpeedBandSource(uint8_t key) {
    switch (key) {
        case KEYPAD_KEY_START_STOP:
            return 0;
            break;
        case KEYPAD_KEY_ENTER:
            return 1;
            break;
        case KEYPAD_KEY_DOWN:
            if(speedBandSource<2) {
                speedBandSource++;
            }
            break;
        case KEYPAD_KEY_UP:
            if(speedBandSource>0) {
                speedBandSource--;
            }
            break;
        case KEYPAD_KEY_ESC:
            speedBandSource=speedBandSourceSave;
            return 2;
        default:
            break;
    }
    return 0;   
}
