#ifndef __MENU__
#define __MENU__
#include <string>
#include <vector>

enum menuAction {
  menuActionNone=0,
  menuActionStart,
  menuActionRaceMenu,
  menuActionConfMenu,
  menuActionSystemMenu,
  menuActionSelectRace,
  menuActionSelectLeg,
  menuActionAdjustLeg,
  menuActionCancelRace,
  menuActionConfigDisplay,
  menuActionLEDBrightness,
  menuActionOLEDBrightness,
  menuActionFirmwareUpdate,
  menuActionScreenTimeout,
  menuActionEditRaceLeg,
  menuActionSystemInfo,
  menuActionReboot
};

typedef enum menuAction menuAction_t;

struct menuEntry {
  std::string text;
  menuAction_t action;
  class menu *subMenu;
};

typedef struct menuEntry menuEntry_t;

class menu {
  public:
    menu(std::string menuTitle, std::vector<menuEntry_t> menuEntries, uint8_t displayLines);
    ~menu();
    uint8_t keypress(uint8_t key);
    const char *getLine(uint8_t line);
    int8_t getActiveLine(void);
    const char * operator[](uint8_t i);
    const char * getMenuTitle(void);
  private:
    std::vector<menuEntry_t> entries;
    std::string title;
    uint8_t displayTop;
    uint8_t entryCount;
    int8_t activeEntry;
    uint8_t lines;
};

typedef class menu menu_t;

extern std::vector<menuEntry_t> mainMenuEntries;
extern std::vector<menuEntry_t> raceMenuEntries;
extern std::vector<menuEntry_t> configMenuEntries;
extern std::vector<menuEntry_t> systemMenuEntries;
extern std::vector<menuEntry_t> dummyMenuEntries;

extern menu_t mainMenu;
extern menu_t raceMenu;
extern menu_t configMenu;
extern menu_t systemMenu;

extern menu_t selectRaceMenu;
extern menu_t selectLegMenu;
extern menu_t adjustLegMenu;
extern menu_t cancelRaceMenu;

extern menu_t configDisplayMenu;
extern menu_t ledBrightMenu;
extern menu_t oledBrightMenu;
extern menu_t screenTimeoutMenu;

extern menu_t firmwareUpdateMenu;
extern menu_t systemInformationMenu;
extern menu_t rebootMenu;

extern std::vector<menu_t*> menuStack;

uint8_t menuSetLedBrightness(uint8_t key);
uint8_t menuSetOledBrightness(uint8_t key);
uint8_t menuSetScreenTimeout(uint8_t key);


#endif