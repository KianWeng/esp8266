/**
 * \file default_config.hpp
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2018-12-30
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>

namespace mbox_dev_p3 {

class DefaultConfig {
public:
    static String GetConfigFilePath()  {
        return "config.json";
    }

    static String GetThingName()  {
        return "Matsui_M-Box";
    }

    static String GetAPPassword()  {
        return "1234567890";
    }

    static uint8_t GetAPTimeout()  {
        return 60;
    }

    static String GetWifiSSID()  {
        return "";
    }

    static String GetWifiPassword()  {
        return "";
    }

    static int GetBaudrate()  {
        return 115200;
    }

    static uint8_t GetFlowControl()  {
        return 0;
    }

    static uint8_t GetDataBits()  {
        return 0;
    }

    static uint8_t GetStopBits()  {
        return 1;
    }

    static char GetParity()  {
        return 'N';
    }

    static bool GetDHCPStatus()  {
        return false;
    }

    static String GetIP()  {
        return "";
    }    

    static String GetNetmask()  {
        return "";
    } 

    static String GetGateway()  {
        return "";
    } 

    static IPAddress GetAPLocalIP()  {
    	IPAddress localIP(192, 168, 5, 1);
        return localIP;
    }

    static IPAddress GetAPGateway()  {
    	IPAddress gateway(192, 168, 5, 1);
        return gateway;
    }

    static IPAddress GetAPSubnet()  {
    	IPAddress subnet(255, 255, 255, 0);
        return subnet;
    }

};

}

#endif  //DEFAULT_CONFIG