#include "menu.h"
#include "keypad.h"
#include <vector>
#include <iterator>
#include <span>

std::vector<menuEntry_t> mainMenuEntries={
    {"Start", menuActionStart, NULL},
    {"Race Menu", menuActionNone, &raceMenu},
    {"Configuration Menu", menuActionNone, &configMenu},
    {"System Menu", menuActionNone, &systemMenu}
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
    {"OLED Brightness", menuActionOLEDBrightness, &oledBrightMenu}
};

std::vector<menuEntry_t> systemMenuEntries={
    {"Firmware Update", menuActionFirmwareUpdate, &firmwareUpdateMenu}
};

std::vector<menuEntry_t> dummyMenuEntries;

menu_t mainMenu("Main Menu", mainMenuEntries, 4);
menu_t raceMenu("Race Menu", raceMenuEntries, 4);
menu_t configMenu("Configuration Menu", configMenuEntries, 4);
menu_t systemMenu("System Menu", systemMenuEntries, 4);

menu_t selectRaceMenu("Select Race", dummyMenuEntries, 0);
menu_t selectLegMenu("Select Leg", dummyMenuEntries, 0);
menu_t adjustLegMenu("Adjust Leg", dummyMenuEntries, 0);
menu_t cancelRaceMenu("Adjust Leg", dummyMenuEntries, 0);

menu_t configDisplayMenu("Config Display", dummyMenuEntries, 0);
menu_t ledBrightMenu("LED Brightness", dummyMenuEntries, 0);
menu_t oledBrightMenu("OLED Brightness", dummyMenuEntries, 0);

menu_t firmwareUpdateMenu("Firmware Update", dummyMenuEntries, 0);

//Main menu is always on the stack.  It's our starting point.
std::vector<menu_t*> menuStack = { &mainMenu };

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
            menuStack.pop_back();
            return (*menuStack.back()).keypress(key);
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
            if(entries[activeEntry].subMenu!=NULL) {
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
            return menuActionNone;
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

uint8_t menu::getActiveLine(void) {
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