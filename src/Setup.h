#ifndef Setup_h
#define Setup_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <pgmspace.h>
#include <TimeLib.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <ArduinoJson.h>
#include <WiFiManager.h>

/* Common private libraries */
#include <ntp.h>
#include <leds.h>
#include <wifi.h>
#include <wifiAPinfo.h>
#include <spiffs.h>

/* globals.h contains defines and global variables */
#include "globals.h"
/* functions.h contains all function declarations */
#include "functions.h"

#endif
