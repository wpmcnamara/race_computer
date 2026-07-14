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
  menuActionAutoAdjustLeg,
  menuActionResetSettings,
  menuActionEditRaceLeg,
  menuActionSystemInfo,
  menuActionReboot,
  menuActionAdjustTime,
  menuActionAdjustDistance,
  menuActionAdjustSpeed,
  menuActionAdjustMark,
  menuActionAdjustSave,
  menuActionAdjustReset,
  menuActionEsc,
  menuActionUp,
  menuActionMainMenu,
  menuActionSpeedBandSource
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
    bool moreUp(void);
    bool moreDown(void);
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
extern std::vector<menuEntry_t> settinsResetMenuEntries;
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
extern menu_t autoAdjustLegTimeMenu;
extern menu_t resetAllSettingsMenu;
extern menu_t speedBandSourceMenu;

extern menu_t firmwareUpdateMenu;
extern menu_t systemInformationMenu;
extern menu_t rebootMenu;

extern menu_t adjustTimeMenu;
extern menu_t adjustDistanceMenu;
extern menu_t adjustSpeedMenu;
extern menu_t adjustMarkMenu;
extern menu_t adjustSaveMenu;


extern std::vector<menu_t*> menuStack;

extern int legAdjustRow;
extern int legAdjustColumn;
extern int legAdjustMode;
extern int32_t legAdjustTime;
extern int32_t legAdjustTimeTmp;
extern int32_t legAdjustTimeBackup;
extern double legAdjustDist;
extern double legAdjustDistTmp;
extern double legAdjustDistBackup;
extern double legAdjustSpeed;
extern double legAdjustSpeedTmp;
extern double legAdjustSpeedBackup;
extern int32_t legAdjustMark;
extern int32_t legAdjustMarkTmp;
extern int32_t legAdjustMarkBackup;
extern bool autoAdjustLegTimeSave;
extern int speedBandSourceSave;


uint8_t menuSetLedBrightness(uint8_t key);
uint8_t menuSetOledBrightness(uint8_t key);
uint8_t menuSetScreenTimeout(uint8_t key);
uint8_t menuAdjustLegTime(uint8_t key);
uint8_t menuAdjustLegDistance(uint8_t key);
uint8_t menuAdjustLegSpeed(uint8_t key);
uint8_t menuAdjustLegMark(uint8_t key);
uint8_t menuConfirmSettingsReset(uint8_t key);
uint8_t menuSetHwVer(uint8_t key);
uint8_t menuAutoAjustLegTime(uint8_t key);
uint8_t menuSpeedBandSource(uint8_t key);

#endif