#include "Setup.h"
#include "declarations.h"
/** Timer for heart beat */
Ticker heartBeatTimer;

/**
 * Initialization of GPIO pins, WiFi connection, timers and sensors
 */
void setup() {
	initLeds();
	pinMode(relayPort, OUTPUT); // Relay trigger signal
	digitalWrite(relayPort, LOW); // Turn off Relay

	Serial.begin(115200);
	Serial.setDebugOutput(false);
	Serial.println("");
	Serial.println("====================");

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

	// Create device ID from MAC address
	String macAddress = WiFi.macAddress();
	hostApName[8] = OTA_HOST[4] = macAddress[0];
	hostApName[9] = OTA_HOST[5] = macAddress[1];
	hostApName[10] = OTA_HOST[6] = macAddress[9];
	hostApName[11] = OTA_HOST[7] = macAddress[10];
	hostApName[12] = OTA_HOST[8] = macAddress[12];
	hostApName[13] = OTA_HOST[9] = macAddress[13];
	hostApName[14] = OTA_HOST[10] = macAddress[15];
	hostApName[15] = OTA_HOST[11] = macAddress[16];

	Serial.println(hostApName);

	// resetWiFiCredentials();
	// Add parameter for the wifiManager
	WiFiManagerParameter wmDevLoc("loc","House",(char *)&devLoc[0],40);
	wifiManager.addParameter(&wmDevLoc);
	WiFiManagerParameter wmLightID("light","Light",(char *)&lightID[0],40);
	wifiManager.addParameter(&wmLightID);
	WiFiManagerParameter wmCamID("cam","Camera",(char *)&camID[0],40);
	wifiManager.addParameter(&wmCamID);
	WiFiManagerParameter wmSecID("sec","Security",(char *)&secID[0],40);
	wifiManager.addParameter(&wmSecID);
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	// Try to connect to WiFi with captive portal
	ipAddr = connectWiFi(hostApName);
	// ipAddr = connectWiFi(ipAddr, ipGateWay, ipSubNet, apName);

	if (!wmIsConnected) {
		Serial.println("WiFi connection failed!");
	} else {
		Serial.println("");
		Serial.print("Connected to ");
		Serial.println(ssidMHC);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	Serial.print("Build: ");
	Serial.println(compileDate);

	Serial.print("Device: ");
	Serial.println(DEVICE_ID);
	Serial.println("====================");

	// Check if SPIFFS file system is initialized.
	if (!spiffsOK)
	{
		sendDebug("Failed to mount file system", OTA_HOST);
	}

	// Try to get last status & last reboot reason from status.txt
	Serial.println("====================");
	if (!readStatus()) {
		sendDebug("No status file found", OTA_HOST);
		writeRebootReason("Unknown");
		lastRebootReason = "Unknown";
	} else {
		Serial.println("Last reboot because: " + rebootReason);
		lastRebootReason = rebootReason;
	}
	Serial.println("====================");

	// Check if the configuration data has changed
	if (shouldSaveConfig) {
		if (wmDevLoc.getValueLength() != 0) {
			devLoc = String(wmDevLoc.getValue());
			saveConfigEntry("loc", devLoc);
		}
		if (wmLightID.getValueLength() != 0) {
			lightID = String(wmLightID.getValue());
			saveConfigEntry("light", lightID);
		}
		if (wmCamID.getValueLength() != 0) {
			camID = String(wmCamID.getValue());
			saveConfigEntry("cam", camID);
		}
		if (wmSecID.getValueLength() != 0) {
			secID = String(wmSecID.getValue());
			saveConfigEntry("sec", secID);
		}
	}

	// Set initial time
	if (!tryGetTime(debugOn)) {
		tryGetTime(debugOn); // Failed to get time from NTP server, retry
	}
	if (gotTime) {
		lastKnownYear = year();
	} else {
		lastKnownYear = 0;
	}

	// Start heart beat sending every 1 minute
	heartBeatTimer.attach(60, triggerHeartBeat);

	// Send Lights restart message
	sendDebug("Restarting", OTA_HOST);
	sendAlarm(true);

	// Reset boot status flag
	inSetup = false;

	// Start the tcp socket server to listen on port tcpComPort
	tcpServer.begin();

	ArduinoOTA.onStart([]() {
		sendDebug("OTA start", OTA_HOST);
		// Safe reboot reason
		writeRebootReason("OTA");
		otaRunning = true;
		// Detach all interrupts and timers
		wdt_disable();
		doubleLedFlashStart(0.1);
		heartBeatTimer.detach();

		WiFiUDP::stopAll();
		WiFiClient::stopAll();
	});

	// Start OTA server.
	ArduinoOTA.setHostname(hostApName);
	ArduinoOTA.begin();

	MDNS.addServiceTxt("arduino", "tcp", "board", "ESP8266");
	MDNS.addServiceTxt("arduino", "tcp", "type", lightDevice);
	MDNS.addServiceTxt("arduino", "tcp", "id", String(hostApName));
	MDNS.addServiceTxt("arduino", "tcp", "service", mhcIdTag);

	wdt_enable(WDTO_8S);
}
