/**************************************************************
 * Bart Wybouw 31/12/2023
 * 
 * This file contains the GPS related functions and definitions
 *  - 
 *  - 
 *  - 
 *
 **************************************************************/
// GPS_functions.h
#ifndef GPS_FUNCTIONS_H
#define GPS_FUNCTIONS_H
#include <TinyGsmClient.h>

void enableGPS();
void disableGPS();

extern TinyGsm modem;

// GPS functions
enum GpsState {
    GPS_DISABLED,
    GPS_WAITING_FOR_LOCK,
    GPS_LOCK_ACQUIRED,
    GPS_LOCK_FAILED
};

//GPS Functions
GpsState gpsState = GPS_DISABLED;
unsigned long lastGpsCheckTime = 0;
const unsigned long gpsAttemptTime = 90000; // 90 seconds for GPS lock attempt
const unsigned long batteryReadInterval = 60000; // 60 seconds
static unsigned long lastBatteryReadTime = 0;
float lat, lon; // GPS coordinates

void enableGPS() {
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void disableGPS() {
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}

#endif // GPS_FUNCTIONS_H