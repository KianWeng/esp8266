/**
 * \file OTAUpgrade.cpp
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2019-01-23
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "OTAUpgrade.h"

// OTAUpgrade
OTAUpgradeTmp::~OTAUpgradeTmp() {}

OTAUpgradeTmp::OTAUpgradeTmp(const String &url) {

    //if ((WiFi.status() == WL_CONNECTED)) {
    //if ((WiFiMulti.run() == WL_CONNECTED)) {
    MBOX_DEBUG_PRINTLN("OTA upgrade uri: " + url)
    t_httpUpdate_return ret = ESPhttpUpdate.update(url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            MBOX_DEBUG_PRINTLN("HTTP_UPDATE_FAILD Error (" + String(ESPhttpUpdate.getLastError()) + "): " + ESPhttpUpdate.getLastErrorString());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            MBOX_DEBUG_PRINTLN("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            MBOX_DEBUG_PRINTLN("HTTP_UPDATE_OK");
            break;
    }
    /*}
    else {
        Serial.printf("NOT CONNECTED\n");
    }*/
}
