#include "raceDef.h"

std::list<raceDef_t *> races; 
std::list<raceDef_t *>::iterator selectedRace;
std::list<raceDef_t *>::iterator selectedRaceSave;

raceDef::raceDef() {
    mSpeed = 0;
    mDriveSpeed = 0;
    mSpeedRange = 0;
    mDistance = 0;
    mDriveDistance = 0;
    mTargetTime = 0;
    mMark = 0;
    mInProgress = false;

}

double raceDef::speed(units_t units)  {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mSpeed);
    } else if (units == metric || units == kph)  {
        return SPEED_INTERNAL_TO_KPH(mSpeed);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceDef::driveSpeed(units_t units)  {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mDriveSpeed);
    } else if (units == metric || units == kph)  {
        return SPEED_INTERNAL_TO_KPH(mDriveSpeed);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceDef::speedRange(units_t units)  {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mSpeedRange);
    } else if (units == metric || units == kph)  {
        return SPEED_INTERNAL_TO_KPH(mSpeedRange);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}  

double raceDef::distance(units_t units)  {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDistance);
    } else if (units == metric || units == km)  {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDistance);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceDef::driveDistance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDriveDistance);
    } else if (units == metric || units == km)  {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDriveDistance);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceDef::targetTime(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mTargetTime);
    } else if (units == milliseconds)  {
        return mTargetTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceDef::mark(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mMark);
    } else if (units == milliseconds)  {
        return mMark;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }   
}

bool raceDef::inProgress(void) {
    return mInProgress;
}

void raceDef::speed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mSpeed=SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph)  {
        mSpeed=SPEED_KPH_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::driveSpeed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mDriveSpeed=SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph)  {
        mDriveSpeed=SPEED_KPH_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::speedRange(double value, units_t units) {
    if (units == imperial || units == mph) {
        mSpeedRange=SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph)  {
        mSpeedRange=SPEED_KPH_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::distance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDistance=DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km)  {
        mDistance=DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::driveDistance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDriveDistance=DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km)  {
        mDriveDistance=DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::targetTime(double value, units_t units) {
    if (units == seconds) {
        mTargetTime=TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds)  {
        mTargetTime=value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::mark(double value, units_t units) {
    if (units == seconds) {
        mMark=TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds)  {
        mMark=value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void raceDef::inProgress(bool value) {
    mInProgress=value;
}