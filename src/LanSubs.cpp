#include "Setup.h"

/** Local name of server */
static const char* host = "ly1";

/**
 * Send broadcast message over UDP into local network
 *
 * @param makeShort
 *		If true send short status, else send long status
 */
void sendAlarm(boolean makeShort) {
	comLedFlashStart(0.2);
	if (debugOn) {
		sendDebug("sendAlarm", OTA_HOST);
	}
	/** Buffer for Json object */
	DynamicJsonBuffer jsonBuffer;

	// Prepare json object for the response
	/* Json object with the alarm message */
	JsonObject& root = jsonBuffer.createObject();

	if (debugOn) {
		sendDebug("Create status", OTA_HOST);
	}

	// Create status
	createStatus(root, makeShort);

	/** WiFiUDP class for creating UDP communication */
	WiFiUDP udpClientServer;

	// Start UDP client for sending broadcasts
	udpClientServer.begin(udpBcPort);

	int connectionOK = udpClientServer.beginPacketMulticast(multiIP, udpBcPort, ipAddr);
	if (connectionOK == 0) { // Problem occured!
		comLedFlashStop();
		udpClientServer.stop();
		if (debugOn) {
			sendDebug("UDP write multicast failed", OTA_HOST);
		}
		return;
	}
	String broadCast;
	root.printTo(broadCast);
	udpClientServer.print(broadCast);
	udpClientServer.endPacket();
	udpClientServer.stop();

	udpClientServer.beginPacket(ipMonitor,udpBcPort);
	udpClientServer.print(broadCast);
	udpClientServer.endPacket();
	udpClientServer.stop();

	if (debugOn) {
		sendDebug(broadCast, OTA_HOST);
	}
	comLedFlashStop();
}

/**
 * Answer request on tcp socket server
 * Commands:
 *		s	to get short status message
 *		i	to get detailed status information
 *		l=0	to switch off the lights
 *		l=1	to switch on the lights
 *		b to switch on the lights for 5 minutes
 *		d	to enable TCP debug
 *		x to reset the device
 *		y=YYYY,MM,DD,HH,mm,ss to set time and date
 *		zloc=[40] location of the device
 *		zsec=[40] connected security device
 *		zcam=[40] connected camera device
 *		zlight=[40] connected light device
 *
 * @param httpClient
 *		Connected WiFi client
 */
void socketServer(WiFiClient tcpClient) {

	// Get data from client until he stops the connection or timeout occurs
	long timeoutStart = now();
	String req = "123456789012345678901";
	char inByte;
	byte index = 0;
	while (tcpClient.connected()) {
		if (tcpClient.available()) {
			inByte = tcpClient.read();
			req[index] = inByte;
			index++;
			if (index >= 21) break; // prevent buffer overflow
		}
		if (now() > timeoutStart + 3000) { // Wait a maximum of 3 seconds
			break; // End the while loop because of timeout
		}
	}

	req[index] = 0;

	tcpClient.flush();
	tcpClient.stop();
	if (req.length() < 1) { // No data received
		if (debugOn) {
			sendDebug("Socket server - no data received", OTA_HOST);
		}
		return;
	}

	if (debugOn) {
		String debugMsg = "TCP cmd = " + req;
		sendDebug(debugMsg, OTA_HOST);
	}

	// Request short status
	if (req.substring(0, 1) == "s") {
		// Send back short status over UDP
		sendAlarm(true);
		return;
	// Request long status
	} else if (req.substring(0, 1) == "i") {
		// Send back long status over UDP
		sendAlarm(false);
		return;
	// Switch lights on
	} else if (req.substring(0, 1) == "l") {
		if (req.substring(2, 3) == "0") { // Switch lights off
			digitalWrite(relayPort, LOW);
			lightOnByUser = false;
		} else { // Switch lights on
			digitalWrite(relayPort, HIGH);
			lightOnByUser = true;
		}
		// Send back status over UDP
		sendAlarm(true);
		return;
		// PANIC!!!! set the alarm off
	} else if (req.substring(0, 1) == "p") {
		if (panicOn) {
			digitalWrite(relayPort, LOW); // Switch off lights
			panicOn = false;
		} else {
			digitalWrite(relayPort, HIGH); // Switch on lights
			panicOn = true;
		}
		// Send back status over UDP
		sendAlarm(true);
		return;
		// Switch lights on for 5 minutes
	} else if (req.substring(0, 1) == "b") {
		if (!lightOnByUser) { // User didn't switch on lights manually!
			// Switch on lights for 5 minutes
			relayOffTimer.detach();
			relayOffTimer.once(onTime, relayOff);
			digitalWrite(relayPort, HIGH);
		}
		// Send back status over UDP
		sendAlarm(true);
		return;
	// Enable debugging
	} else if (req.substring(0, 1) == "d") {
		// toggle debug flag
		debugOn = !debugOn;
		if (debugOn) {
			sendDebug("Debug over TCP is on", OTA_HOST);
		} else {
			sendDebug("Debug over TCP is off", OTA_HOST);
		}
		writeStatus();
		return;
		// Date/time received
	} else if (req.substring(0, 2) == "y=") {
		int nowYear = 0;
		int nowMonth = 0;
		int nowDay = 0;
		int nowHour = 0;
		int nowMinute = 0;
		int nowSecond = 0;

		if (isDigit(req.charAt(2))
		&& isDigit(req.charAt(3))
		&& isDigit(req.charAt(4))
		&& isDigit(req.charAt(5))
		&& isDigit(req.charAt(7))
		&& isDigit(req.charAt(8))
		&& isDigit(req.charAt(10))
		&& isDigit(req.charAt(11))
		&& isDigit(req.charAt(13))
		&& isDigit(req.charAt(14))
		&& isDigit(req.charAt(16))
		&& isDigit(req.charAt(17))
		&& isDigit(req.charAt(19))
		&& isDigit(req.charAt(20))) {
			String cmd = req.substring(2, 6);
			int nowYear = cmd.toInt();
			cmd = req.substring(7, 9);
			int nowMonth = cmd.toInt();
			cmd = req.substring(10, 12);
			int nowDay = cmd.toInt();
			cmd = req.substring(13, 15);
			int nowHour = cmd.toInt();
			cmd = req.substring(16, 18);
			int nowMinute = cmd.toInt();
			cmd = req.substring(19, 21);
			int nowSecond = cmd.toInt();

			if (debugOn) {
				String debugMsg = "Changed time to " + String(nowYear) + "-" + String(nowMonth) + "-" + String(nowDay) + " " + String(nowHour) + ":" + String(nowMinute) + ":" + String(nowSecond);
				sendDebug(debugMsg, OTA_HOST);
			}
			setTime(nowHour,nowMinute,nowSecond,nowDay,nowMonth,nowYear);
			gotTime = true;
		} else {
			String debugMsg = "Received wrong time format: " + req;
			sendDebug(debugMsg, OTA_HOST);
		}
		// Location received
	} else if (req.substring(0,5) == "zloc=") {
		// copy location
		devLoc = req.substring(5);
		// save new location
		if (!saveConfigEntry("loc", (char *)&devLoc[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		MDNS.addServiceTxt("arduino", "tcp", "loc", devLoc);
		MDNS.update();
		return;
		// Light device ID received
	} else if (req.substring(0,7) == "zlight=") {
		// copy light device ID
		lightID = req.substring(7);
		// save new light device ID
		if (!saveConfigEntry("light", (char *)&lightID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Security device ID received
	} else if (req.substring(0,5) == "zsec=") {
		// copy security device ID
		secID = req.substring(5);
		// save new security device ID
		if (!saveConfigEntry("sec", (char *)&secID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Camera device ID received
	} else if (req.substring(0,5) == "zcam=") {
		// copy camera device ID
		camID = req.substring(5);
		// save new camera device ID
		if (!saveConfigEntry("cam", (char *)&camID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Reset device
	} else if (req.substring(0, 1) == "x") {
		sendDebug("Reset device", OTA_HOST);
		writeStatus();
		// Reset the ESP
		delay(3000);
		ESP.reset();
		delay(5000);
	}
}
