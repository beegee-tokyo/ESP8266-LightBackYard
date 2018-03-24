#include "Setup.h"
#include "declarations.h"

/**
 * Initialization of GPIO pins, WiFi connection, timers and sensors
 */
void setup() {
	initLeds();
	pinMode(relayPort, OUTPUT); // Relay trigger signal
	digitalWrite(relayPort, LOW); // Turn off Relay

	Serial.begin(115200);
	Serial.setDebugOutput(false);

	// Connect to one of the two alternative AP's
	connectInit();
	// Give it 5 seconds to connect
	long waitStart = millis();
	while (0) {
		if ((connStatus == CON_GOTIP) || ((millis()-waitStart)>5000)) {
			break;
		}
	}

	// Now other intializations can be done, but WiFi might not be working yet

	// Start heart beat sending every 1 minute
	heartBeatTimer.attach(60, triggerHeartBeat);

	// Initialize file system.
	bool spiffsOK = false;
	if (!SPIFFS.begin())
	{
		if (SPIFFS.format()){
			spiffsOK = true;
		}
	} else { // SPIFFS ready to use
		spiffsOK = true;
	}
	if (spiffsOK) {
		char tmpLoc[40];
		if (getConfigEntry("loc", tmpLoc)) {
			devLoc = String(tmpLoc);
		}
		if (getConfigEntry("light", tmpLoc)) {
			lightID = String(tmpLoc);
		}
		if (getConfigEntry("cam", tmpLoc)) {
			camID = String(tmpLoc);
		}
		if (getConfigEntry("sec", tmpLoc)) {
			secID = String(tmpLoc);
		}
	}

	wdt_enable(WDTO_8S);
	wdtEnabled = true;
}
