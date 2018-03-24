#include <ESP8266WiFi.h>

/** Output to activate Relay */
#define relayPort 5

/** Hostname & AP name created from device function & 1. and 4. to 6. part of MAC address */
extern char hostApName[];
/** Debug name created from last part of hostname */
extern String OTA_HOST;
/** IP address of this module */
extern IPAddress ipAddr;
/** ID for monitor to be replaced by function in the future */
#define DEVICE_ID "ly1"

/** Build time */
extern const char compileDate [];
/** WiFiServer class to create TCP socket server on port tcpComPort */
extern WiFiServer tcpServer;

/** Timer to switch off the relay */
extern Ticker relayOffTimer;

/** Flag if heart beat was triggered */
extern boolean heartBeatTriggered;
/** Timer for heart beat */
extern Ticker heartBeatTimer;
/** Flag if panic button was pressed */
extern boolean panicOn;
/** Flag for debugging */
extern bool debugOn;
/** Relay on delay time in seconds */
extern int onTime;
/** Bug capture trial year of last good NTP time received */
extern int lastKnownYear;

/** Flag for light switched off */
extern boolean lightOffTriggered;
/** Flag for manual switch on activity */
extern boolean lightOnByUser;

/** Flag for OTA update */
extern bool otaRunning;
