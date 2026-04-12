#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__
#include "helpers.h"
#include "bsp.h"

extern class stateMachine stateMachine;

enum stateMachineStatesEnum {
  stateInit,
  stateLoadRaceCheckPoint,
  stateMainMenu,
  stateRaceStart,
  stateDelayedStart,
  stateLegActive,
  stateLegComplete,
  stateRaceComplete,
  stateSelectRace,
  stateSelectRaceLeg,
  stateSaveSelection,
  stateCancelSelection,
  stateNextLeg,
  stateDispSelect,
  stateCheckFirmwareUpdate,
  stateConfirmFirmwareUpdate,
  stateDoFirmwareUpdate,
  stateSetLedBrightness,
  stateSetOledBrightness,
  stateSetScreenBlankTime,
  stateSetLegTimeAdjust,
  stateScreenBlank,
  stateScreenWake,
  stateSaveSettings,
  stateLoadSettings,
  stateEditRaceLeg,
  stateAdjustLegTime,
  stateAdjustLegDistance,
  stateAdjustLegSpeed,
  stateAdjustLegSave,
  stateAdjustLegRestore,
  stateAdjustLegCaptureValues,
  stateAdjustLegResetValues,
  stateShowSystemInfo,
  stateSetHwVer,
  stateResetAllSettings,
  stateReboot,
  stateSpeedBandSource,
  stateUnknown
};

enum startStopStateEnum {
  stateOff,
  stateOn,
  stateBlink,
  stateBreath
};

enum startStopColorEnum {
  colorBlack,
  colorRed,
  colorGreen,
  colorWhite
};

typedef enum stateMachineStatesEnum stateMachineStatesEnum_t;
typedef enum startStopStateEnum startStopStateEnum_t;
typedef enum startStopColorEnum startStopColorEnum_t;

union statusUnion {
  uint32_t value;
  struct {
    startStopStateEnum_t startStopState:2;
    uint8_t buttonColor:2;
    startStopColorEnum_t startStopColor:2;
    bool gpsReady:1;
    bool keyPress:1;
    bool delayedStart:1;
    bool legActive:1;
    uint32_t pack:22;
  } flags;
};

class stateMachine {
  public:
    stateMachine(void);
    void run(void);
    union statusUnion status;
    uint32_t startStopColor;
  private:
    stateMachineStatesEnum_t state;
    stateMachineStatesEnum_t lastState;
    union statusUnion lastStatus;
};

#endif