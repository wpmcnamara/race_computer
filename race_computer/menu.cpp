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
    {"Select Race", menuActionSelectRace, NULL},
    {"Select Leg", menuActionSelectLeg, NULL},
    {"Adjust Leg", menuActionAdjustLeg, NULL},
    {"Cancel Race", menuActionCancelRace, NULL}
};

std::vector<menuEntry_t> configMenuEntries={
    {"Configure Display", menuActionConfigDisplay, NULL},
    {"LED Brightness", menuActionLEDBrightness, NULL},
    {"OLED Brightness", menuActionOLEDBrightness, NULL}
};

std::vector<menuEntry_t> systemMenuEntries={
    {"Firmware Update", menuActionFirmwareUpdate, NULL}
};

menu_t mainMenu("Main Menu", mainMenuEntries, 4);
menu_t raceMenu("Race Menu", raceMenuEntries, 4);
menu_t configMenu("Configuration Menu", configMenuEntries, 4);
menu_t systemMenu("System Menu", systemMenuEntries, 4);

//Main menu is always on the stack.  It's our starting point.
std::vector<menu_t*> menuStack = { &mainMenu };

menu::menu(std::string menuTitle, std::vector<menuEntry_t> menuEntries, uint8_t displayLines) {
    title=menuTitle;
    entries=menuEntries;
    entryCount=std::size(menuEntries);
    lines=displayLines;
    displayTop=0;
    activeEntry=0;
};


menu::~menu() {
    return;
}

uint8_t menu::keypress(uint8_t key) {
    Serial.printf("key=%d\n", key);
    Serial.printf("activeEntry=%d\n", activeEntry);
    Serial.printf("displayTop=%d\n", displayTop);
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