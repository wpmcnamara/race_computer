#include "raceLegDef.h"

std::vector<raceLegDef_t *>::iterator selectedRaceLeg;

raceLegDef::raceLegDef() {
    mSpeed = 0;
    mDriveSpeed = 0;
    mRaceLegEndAvgSpeed = 0;
    mRaceLegEndDriveAvgSpeed = 0;
    mRaceLegEndTargetDistance = 0;
    mRaceLegEndDriveDistance = 0;
    mRaceLegEndTargetTime = 0;
    mSpeedRange = 0;
    mDistance = 0;
    mDriveDistance = 0;
    mTargetTime = 0;
    mId = 0;
    mDescr = "";
    mPointsFile = "";
    mMark = 0;
    mInProgress = false;
    mComplete = false;
}

double raceLegDef::speed(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mSpeed);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mSpeed);
    } else if (units == internal) {
        return mSpeed;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::driveSpeed(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mDriveSpeed);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mDriveSpeed);
    } else if (units == internal) {
        return mDriveSpeed;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::raceLegEndAvgSpeed(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mRaceLegEndAvgSpeed);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mRaceLegEndAvgSpeed);
    } else if (units == internal) {
        return mRaceLegEndAvgSpeed;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::raceLegEndDriveAvgSpeed(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mRaceLegEndDriveAvgSpeed);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mRaceLegEndDriveAvgSpeed);
    } else if (units == internal) {
        return mRaceLegEndDriveAvgSpeed;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::raceLegEndTargetDistance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mRaceLegEndTargetDistance);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceLegEndTargetDistance);
    } else if (units == internal) {
        return mRaceLegEndTargetDistance;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::raceLegEndDriveDistance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mRaceLegEndDriveDistance);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceLegEndDriveDistance);
    } else if (units == internal) {
        return mRaceLegEndDriveDistance;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::raceLegEndTargetTime(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceLegEndTargetTime);
    } else if (units == milliseconds || units == internal) {
        return mRaceLegEndTargetTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::speedRange(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mSpeedRange);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mSpeedRange);
    } else if (units == internal) {
        return mSpeedRange;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::distance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDistance);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDistance);
    } else if (units == internal) {
        return mDistance;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::driveDistance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDriveDistance);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDriveDistance);
    } else if (units == internal) {
        return mDriveDistance;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::targetTime(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mTargetTime);
    } else if (units == milliseconds || units == internal) {
        return mTargetTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceLegDef::mark(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mMark);
    } else if (units == milliseconds || units == internal) {
        return mMark;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

unsigned int raceLegDef::id(void) {
    return mId;
}

String raceLegDef::descr(void) {
    return mDescr;
}

String raceLegDef::pointsFile(void) {
    return mPointsFile;
}

bool raceLegDef::inProgress(void) {
    return mInProgress;
}

bool raceLegDef::complete(void) {
    return mComplete;
}

void raceLegDef::speed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mSpeed = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mSpeed = SPEED_KPH_TO_INTERNAL(value);
    } else if (units == internal) {
        mSpeed = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::driveSpeed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mDriveSpeed = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mDriveSpeed = SPEED_KPH_TO_INTERNAL(value);
    } else if (units == internal) {
        mDriveSpeed = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::raceLegEndAvgSpeed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mRaceLegEndAvgSpeed = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mRaceLegEndAvgSpeed = SPEED_KPH_TO_INTERNAL(value);
    } else if (units == internal) {
        mRaceLegEndAvgSpeed = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::raceLegEndDriveAvgSpeed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mRaceLegEndDriveAvgSpeed = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mRaceLegEndDriveAvgSpeed = SPEED_KPH_TO_INTERNAL(value);
    } else if (units == internal) {
        mRaceLegEndDriveAvgSpeed = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::raceLegEndTargetDistance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mRaceLegEndTargetDistance = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mRaceLegEndTargetDistance = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else if (units == internal) {
        mRaceLegEndTargetDistance = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::raceLegEndDriveDistance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mRaceLegEndDriveDistance = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mRaceLegEndDriveDistance = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else if (units == internal) {
        mRaceLegEndDriveDistance = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::raceLegEndTargetTime(double value, units_t units) {
    if (units == seconds) {
        mRaceLegEndTargetTime = TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds || units == internal) {
        mRaceLegEndTargetTime = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::speedRange(double value, units_t units) {
    if (units == imperial || units == mph) {
        mSpeedRange = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mSpeedRange = SPEED_KPH_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::distance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDistance = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mDistance = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::driveDistance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDriveDistance = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mDriveDistance = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::targetTime(double value, units_t units) {
    if (units == seconds) {
        mTargetTime = TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds) {
        mTargetTime = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::mark(double value, units_t units) {
    if (units == seconds) {
        mMark = TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds) {
        mMark = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceLegDef::id(unsigned int value) {
    mId = value;
}

void raceLegDef::descr(String value) {
    mDescr = value;
}

void raceLegDef::pointsFile(String value) {
    mPointsFile = value;
}

void raceLegDef::inProgress(bool value) {
    mInProgress = value;
}

void raceLegDef::complete(bool value) {
    mComplete = value;
}
