#ifndef declarations_h
#define declarations_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

/** WiFiServer class to create TCP socket server on port tcpComPort */
WiFiServer tcpServer(tcpComPort);

/** Hostname & AP name created from device function & 1. and 4. to 6. part of MAC address */
char hostApName[] = "MHC-Lig-xxxxxxxx";
/** Debug name created from last part of hostname */
String OTA_HOST = "Lig-xxxxxxxx";
/** IP address of this module */
IPAddress ipAddr;

/** Timer to switch off the relay */
Ticker relayOffTimer;

/** Flag if heart beat was triggered */
boolean heartBeatTriggered = false;
/** Timer for heart beat */
Ticker heartBeatTimer;
/** Flag if panic button was pressed */
boolean panicOn = false;

/** Flag for light switched off */
boolean lightOffTriggered = false;
/** Flag for manual switch on activity */
boolean lightOnByUser = false;

/** Relay on delay time in seconds */
int onTime = 300;
/** Bug capture trial year of last good NTP time received */
int lastKnownYear = 0;

/** Flag for OTA update */
bool otaRunning = false;
/** Flag for debugging */
bool debugOn = false;

#endif
