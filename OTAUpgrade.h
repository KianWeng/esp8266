 /**
 * \file OTAUpgrade.h
 * 
 * \author your name
 * 
 * \brief Managing M-Box’s configuration, including “Thing Name, AP Configuration, Wi-Fi Configuration,
 *        Serial Port Configuration, Network Configuration, Device Property, Device Model, Web Server Configuration,
 *        MQTT Configuration, NTP Configuration, Introspector Configuration”. It supports query and update.
 * 
 * @version 0.1
 * 
 * \date 2019-01-23
 * 
 * @copyright Copyright (c) 2019
 */

#ifndef OTAUPGRADE_H
#define OTAUPGRADE_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "common.h"


class OTAUpgradeTmp {
public:
    OTAUpgradeTmp(const String &url);
    ~OTAUpgradeTmp();

};





#endif //OTAUPGRADE_H
