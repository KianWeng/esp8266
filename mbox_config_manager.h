 /**
 * \file mbox_config_manager.h
 * 
 * \author your name
 * 
 * \brief Managing M-Box’s configuration, including “Thing Name, AP Configuration, Wi-Fi Configuration,
 *        Serial Port Configuration, Network Configuration, Device Property, Device Model, Web Server Configuration,
 *        MQTT Configuration, NTP Configuration, Introspector Configuration”. It supports query and update.
 * 
 * @version 0.1
 * 
 * \date 2018-12-25
 * 
 * @copyright Copyright (c) 2018
 */

#ifndef MBOX_CONFIG_MANAGER
#define MBOX_CONFIG_MANAGER

#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <map>
#include <list>
#include "common.h"


 using namespace std;

 static std::map<String, enum SerialConfig> SerialConfigMap {
             {"SERIAL_5N1",SERIAL_5N1},
             {"SERIAL_6N1",SERIAL_6N1},
             {"SERIAL_7N1",SERIAL_7N1},
             {"SERIAL_8N1",SERIAL_8N1},
             {"SERIAL_5N2",SERIAL_5N2},
             {"SERIAL_6N2",SERIAL_6N2},
             {"SERIAL_7N2",SERIAL_7N2},
             {"SERIAL_8N2",SERIAL_8N2},
             {"SERIAL_5E1",SERIAL_5E1},
             {"SERIAL_6E1",SERIAL_6E1},
             {"SERIAL_7E1",SERIAL_7E1},
             {"SERIAL_8E1",SERIAL_8E1},
             {"SERIAL_5E2",SERIAL_5E2},
             {"SERIAL_6E2",SERIAL_6E2},
             {"SERIAL_7E2",SERIAL_7E2},
             {"SERIAL_8E2",SERIAL_8E2},
             {"SERIAL_5O1",SERIAL_5O1},
             {"SERIAL_6O1",SERIAL_6O1},
             {"SERIAL_7O1",SERIAL_7O1},
             {"SERIAL_8O1",SERIAL_8O1},
             {"SERIAL_5O2",SERIAL_5O2},
             {"SERIAL_6O2",SERIAL_6O2},
             {"SERIAL_7O2",SERIAL_7O2},
             {"SERIAL_8O2",SERIAL_8O2},
         };

namespace mbox_dev_p3 {

class MBoxConfigManager {
public:
    MBoxConfigManager();
    ~MBoxConfigManager();

    /**
     * \brief Set configuration file path.
     * 
     * \param filePath  Configuration file path.
     */
    void SetConfigFilePath(const String &filePath);

    /**
     * \brief Check whether configuration file exists.
     * 
     * \return File existence status. 
     */
    bool ConfigFileExists();

    /**
     * \brief Create default configurations, and save them into SPIFFS.
     * 
     * \return Default configuration create result.
     */
    bool CreateDefaultConfig();

private:
    String cfgFilePath_;
};


class MBoxBasicConfigManager {
public:
    MBoxBasicConfigManager();
    ~MBoxBasicConfigManager();

    /**
     * \brief Load M-Box basic configurations from configuration file.
     * 
     * \return Load configuration result. 
     */
    bool LoadConfig();

    /**
     * \brief Get M-Box’s name.
     * 
     * \return M-Box’s name. 
     */
    String GetThingName() const;
    
    /**
     * \brief Set M-Box’s name.
     * 
     * \param thingName  Name of M-Box.
     * 
     * \return Set result.
     */
    bool SetThingName(const String &thingName);

private:
    String thingName_;
};


class APConfigManager {
public:
    APConfigManager();
    ~APConfigManager();

    /**
     * \brief Load AP configurations from configuration file.
     * 
     * \return Load configuration result. 
     */
    bool LoadConfig();

    String GetName() const;

    /**
     * \brief Get password of AP.
     * 
     * \return Password of AP.
     */
    String GetPassword() const;

    /**
     * \brief Set the password of AP.
     * 
     * \param password  Password of AP.
     * 
     * \return Set result.
     */
    bool SetAPPassword(const String &password);

    /**
     * \brief Get timeout of AP
     * 
     * \return Timeout of AP. 
     */
    uint8_t GetTimeout() const;

    /**
     * \brief Set the timeout of AP.
     * 
     * \param timeout  Timeout of AP.
     * 
     * \return Set result.
     */
    bool SetAPTimeout(uint8_t timeout);

private:
    String password_;
    uint8_t timeout_;
};

class WifiConfigManager {
public:
    WifiConfigManager();
    ~WifiConfigManager();

    /**
     * \brief Load Wifi configurations from configuration file.
     * 
     * \return Load configuration result. 
     */
    bool LoadConfig();

    /**
     * \brief Get password of Wifi.
     * 
     * \return Password of Wifi.
     */
    String GetWifiPassword() const;

    /**
     * \brief Set the password of Wifi.
     * 
     * \param password  Password of Wifi.
     * 
     * \return Set result.
     */
    bool SetWifiPassword(const String &password);

    /**
     * \brief Get SSID of Wifi
     * 
     * \return SSID of Wifi. 
     */
    String GetWifiSSID() const;

    /**
     * \brief Set the SSID of Wifi
     * 
     * \param ssid SSID of Wifi
     * 
     * \return Set result.
     */
    bool SetWifiSSID(const String &ssid);

private:
    String wifiSSID_;
    String wifiPassword_;
};

 class SerialConfigManager {
 public:
     SerialConfigManager();
     ~SerialConfigManager();

     /**
      * \brief Load Serial configurations from configuration file.
      * 
      * \return Load configuration result. 
      */
     bool LoadConfig();

     /**
      * \brief Get baudrate of Serial.
      * 
      * \return baudrate of Serial.
      */
     int GetBaudrate() const;

     /**
      * \brief Set the baudrate of Serial.
      * 
      * \param baudrate  baudrate of Serial.
      * 
      * \return Set result.
      */
     bool SetBaudrate(int baudrate);

     /**
      * \brief Get FlowControl of Serial
      * 
      * \return FlowControl of Serial
      */
     uint8_t GetFlowControl() const;

     /**
      * \brief Set the flowControl of Serial
      * 
      * \param flowControl flowControl of Serial
      * 
      * \return Set result.
      */
     bool SetFlowControl(uint8_t flowControl);

     /**
      * \brief Get DataBits of Serial
      * 
      * \return DataBits of Serial
      */
     uint8_t GetDataBits() const;    

     /**
      * \brief Set the dataBits of Serial
      * 
      * \param dataBits dataBits of Serial
      * 
      * \return Set result.
      */
     bool SetDataBits(uint8_t dataBits);

     /**
      * \brief Get StopBits of Serial
      * 
      * \return StopBits of Serial
      */
     uint8_t GetStopBits() const;    

     /**
      * \brief Set the stopBits of Serial
      * 
      * \param stopBits stopBits of Serial
      * 
      * \return Set result.
      */
     bool SetStopBits(uint8_t stopBits);  

     /**
      * \brief Get Parity of Serial
      * 
      * \return Parity of Serial
      */
     char GetParity() const;      

     /**
      * \brief Set the parity of Serial
      * 
      * \param parity parity of Serial
      * 
      * \return Set result.
      */
     bool SetParity(char parity);      

 private:
     int baudRate_;
     uint8_t flowControl_;
     uint8_t dataBit_;
     uint8_t stopBit_;
     char parity_;

     enum SerialConfig GetSerialConfigEnum(String SerialConfigStr);
     String MakeSerialConfigStr();

 };

class NetworkConfigManager {
public:
    NetworkConfigManager();
    ~NetworkConfigManager();

    /**
     * \brief Load Network configurations from configuration file.
     * 
     * \return Load configuration result. 
     */
    bool LoadConfig();

    /**
     * \brief Get DHCP enable status
     * 
     * \return DHCP enable status 
     */
    bool DHCPEnabled();

    /**
     * \brief Set DHCP status
     * 
     * \param status DHCP status
     * 
     * \return DHCP enable status 
     */
    bool SetDHCPStatus(bool status);

    /**
     * \brief Get ipaddress.
     * 
     * \return ipaddress
     */
    String GetIP() const;

    /**
     * \brief Set the ipaddress.
     * 
     * \param ip  the ipaddress.
     * 
     * \return Set result.
     */
    bool SetIP(const String &ip);

    /**
     * \brief Get netmask 
     * 
     * \return netmask
     */
    String GetNetmask() const;

    /**
     * \brief Set the netmask
     * 
     * \param netmask network netmask
     * 
     * \return Set result.
     */
    bool SetNetmask(const String &netmask);

    /**
     * \brief Get M-Box’s gateway
     * 
     * \return gateway
     */
    String  GetGateway() const;    

    /**
     * \brief Set M-Box’s gateway
     * 
     * \param gateway gateway of M-Box
     * 
     * \return Set result.
     */
    bool SetGateway(const String &gateway);    

private:
    bool dhcpEnabled_;
    String ip_;
    String netmask_;
    String gateway_;

};

 // device property config Manager
 class DevProConfigManager {
 public:
     DevProConfigManager();
     ~DevProConfigManager();

     /**
      * \brief Load Network configurations from configuration file.
      * 
      * \return Load configuration result. 
      */
     bool LoadConfig();

	 bool GetSwapFlag() const;
     /**
      * \brief Get device id
      * 
      * \return device id
      */
     String GetDeviceID() const;

     /**
      * \brief Get device type
      * 
      * \return device type
      */
     String GetDeviceType() const;
	 
	 /**
      * \brief Get device’ protocol type
      * 
      * \return device’ protocol type
      */
	 String GetProtocolType() const;

	 /**
	   * \brief Add a device from M-Box.
	   * 
	   * \param id			   Device id
	   * 
	   * \return result of Add device
	   */
     bool AddDevice(const String &id, uint32_t pollInterval, uint32_t pollTimeout, 
     			uint32_t reportTimeout, uint8_t reportLimit);

 	/**
      * \brief Delete a device from M-Box.
      * 
      * \param id             Device id
      * 
      * \return report timeout.
      */
     bool DeleteDevice(const String &id);

 	/**
      * \brief Get data points’ property list.
      * 
      * \param deviceType        Device type.
      * \param controllerType    Device’s controller type.
      * 
      * \return Data points’ property list.
      */
 	list<DataPoint> GetDeviceModel(const String &deviceType) const;

 	/**
      * \brief Get device data’s polling interval.
      * 
      * \param deviceType        Device type.
      * \param controllerType    Device’s controller type.
      * 
      * \return Device data’s polling interval.
      */
 	uint32_t GetPollInterval(const String &deviceType) const;

 	/**
      * \brief Get device data’s polling timeout.
      * 
      * \param deviceType        Device type.
      * \param controllerType    Device’s controller type.
      * 
      * \return Device data’s polling timeout.
      */
 	uint32_t GetPollTimeout(const String &deviceType) const;

 	/**
      * \brief Get device data’s report timeout.
      * 
      * \param deviceType        Device type.
      * \param controllerType    Device’s controller type.
      * 
      * \return Device data’s report timeout.
      */
 	uint32_t GetReportTimeout(const String &deviceType) const;

 	/**
      * \brief Get device data’s maximum report times when report failed.
      * 
      * \param deviceType        Device type.
      * \param controllerType    Device’s controller type.
      * 
      * \return Device data’s maximum report times when report failed.
      */
 	uint8_t GetReportLimit(const String &deviceType) const;

     /**
      * \brief Add a device model to M-Box.
      * 
      * \param deviceType        Device type
      * \param controllerType    Device’s controller type
      * \param pollInterval      Device data’s polling interval.
      * \param pollTimeout       Device data’s polling timeout.
      * \param reportTimeout     Device data’s report timeout.
      * \param reportLimit       Device data’s maximum report times when report failed.
      * \param dataPoints        Data point list.
      * 
      * \return result of Add Model
      */
     
     bool AddModel(const String &deviceType, const String &protocolType, bool swapFlag, const list<DataPoint> dataPoints);

     /**
      * \brief Delete the device model in M-Box.
      * 
      * \param deviceType    	Device type
      * \param controllerType    Device’s controller type
      * 
      * \return result of Delete Model
      */
     bool DeleteModel(const String &deviceType);

     /**
      * \brief Update the device model in M-Box.
      * 
      * \param deviceType    	Device type
      * \param controllerType    Device’s controller type
      * \param pollInterval      Device data’s polling interval.
      * \param pollTimeout       Device data’s polling timeout.
      * \param reportTimeout     Device data’s report timeout.
      * \param reportLimit       Device data’s maximum report times when report failed.
      * \param dataPoints        Data point list.
      * 
      * \return result of Update Model
      */
     bool UpdateModel(const String &deviceType, uint8_t pollInterval, uint8_t pollTimeout, 
     					 uint8_t reportTimeout, uint8_t reportLimit, const list<DataPoint> dataPoints);

    
 private:
 	 void ParseConfigs(String& config);
	 void ParseDataPoints(String& dataPoints);
	 bool swapFlag_;
     String deviceId_;
     String deviceType_;
	 String protocolType_;
     uint32_t pollInterval_;
     uint32_t pollTimeout_;
     uint32_t reportTimeout_;
     uint8_t reportLimit_;
     list<DataPoint> dataPoints_;
 };

 class HttpProConfigManager {
 public:
     HttpProConfigManager();
     ~HttpProConfigManager();

     /**
      * \brief Get HTTP port.
      * 
      * \return HTTP port. 
      */
     uint16_t GetHTTPPort() const;

     /**
      * \brief Set HTTP port.
      * 
      * \param HTTP port.
      * 
      * \return Result of set HTTP port. 
      */
     bool SetHTTPPort(uint16_t port);

 private:
     uint16_t port_;

 };


 class BrokerProConfigManager {
 public:
     BrokerProConfigManager();
     ~BrokerProConfigManager();

     /**
      * \brief Get broker address.
      * 
      * \return Broker address.
      */
     String GetBrokerAddress() const;

     /**
      * \brief Get broker port.
      * 
      * \return Broker port. 
      */
     uint16_t GetBrokerPort() const;

     /**
      * \brief Get connect broker interval.
      * 
      * \return The interval of connect broker.
      */
     uint8_t GetConnectBrokerInterval() const;

     /**
      * \brief Get connect broker step.
      * 
      * \return The step of connect broker.
      */
     uint8_t GetConnectBrokerStep() const;

     /**
      * \brief Get connect broker limit.
      * 
      * \return The limit of connect broker.
      */
     uint8_t GetConnectBrokerLimit() const;

     /**
      * \brief Set broker address.
      * 
      * \param address 	Broker address.
      * 
      * \return Result of set broker address.
      */
     bool SetBrokerAddress(const String &address);

     /**
      * \brief Set broker port.
      * 
      * \param port 		Broker port.
      * 
      * \return Result of set broker port.
      */
     bool SetBrokerPort(uint16_t port);

     /**
      * \brief Set connect broker interval.
      * 
      * \param interval 		Connect broker interval.
      * 
      * \return Result of set connect broker interval. 
      */
     bool SetConnectBrokerInterval(uint8_t interval);

     /**
      * \brief Set connect broker step.
      * 
      * \param step 		Connect broker step.
      * 
      * \return Result of set connect broker step. 
      */
     bool SetConnectBrokerStep(uint8_t step);

     /**
      * \brief Set connect broker limit.
      * 
      * \param limit 	Connect broker limit.
      * 
      * \return Result of set connect broker limit.
      */
     bool SetConnectBrokerLimit(uint8_t limit);

 private:
    String address_;
 	uint16_t port_;
 	uint8_t interval_;
 	uint8_t step_;
 	uint8_t limit_;
 };

 class RegisterConfigManager {
 public:
     RegisterConfigManager();
     ~RegisterConfigManager();

     /**
      * \brief Get register interval.
      * 
      * \return Register interval.
      */
     uint8_t GetRegisterInterval() const;

     /**
      * \brief Get register step.
      * 
      * \return Register step.
      */
     uint8_t GetRegisterStep() const;

     /**
      * \brief Get register limit.
      * 
      * \return Register limit.
      */
     uint8_t GetRegisterLimit() const;

     /**
      * \brief Set register interval.
      * 
      * \param interval 	Register interval.
      * 
      * \return Result of set register interval.
      */
     bool SetRegisterInterval(uint8_t interval);

     /**
      * \brief Set register step.
      * 
      * \param step 		Register step.
      * 
      * \return Result of set register step.
      */
     bool SetRegisterStep(uint8_t step);

     /**
      * \brief Set register limit.
      * 
      * \param limit 	Register limit.
      * 
      * \return Result of set register limit.
      */
     bool SetRegisterLimit(uint8_t limit);

 private:
     uint8_t interval_;
 	uint8_t step_;
 	uint8_t limit_;
 };

 class NTPConfigManager {
 public:
     NTPConfigManager();
     ~NTPConfigManager();

     /**
      * \brief Get NTP server address.
      * 
      * \return NTP server address.
      */
     String GetNTPServerAddress() const;

     /**
      * \brief Get NTP synchronization interval.
      * 
      * \return NTP synchronization interval.
      */
     uint8_t GetNTPSyncInterval() const;

     /**
      * \brief Set NTP server address.
      * 
      * \param address 		NTP server address.
      * 
      * \return Result of set NTP server address.
      */
     bool SetNTPServerAddress(const String &address);

     /**
      * \brief Set NTP synchronization interval.
      * 
      * \param interval 		NTP synchronization interval.
      * 
      * \return Result of set NTP synchronization interval.
      */
     bool SetNTPSyncInterval(uint8_t interval);

 private:
     String address_;
 	uint8_t interval_;

 };


 class StdAPConfigManager {
 public:
     StdAPConfigManager();
     ~StdAPConfigManager();

     /**
      * \brief Get AP password standard.
      * 
      * \return AP password standard.
      */
     String GetStdAPPassword() const;

     /**
      * \brief Get AP timeout standard.
      * 
      * \return AP timeout standard.
      */
     uint8_t GetStdAPTimeout() const;

     /**
      * \brief Set AP password standard.
      * 
      * \param password 		AP password standard.
      * 
      * \return Result of set AP password standard.
      */
     bool SetStdAPPassword(const String &password);

     /**
      * \brief Set AP timeout standard.
      * 
      * \param timeout 		AP timeout standard.
      * 
      * \return Result of set AP timeout standard.
      */
     bool SetStdAPTimeout(uint8_t timeout);

 private:
     String password_;
 	uint8_t timeout_;

 };

 class StdHttpProConfigManager {
 public:
     StdHttpProConfigManager();
     ~StdHttpProConfigManager();

     /**
      * \brief Get HTTP port standard.
      * 
      * \return HTTP port standard.
      */
     uint16_t GetStdHTTPPort() const;

     /**
      * \brief Set HTTP port standard.
      * 
      * \param port 		HTTP port standard.
      * 
      * \return Result of set HTTP port standard.
      */
     bool SetStdHTTPPort(uint16_t port);

 private:
     uint16_t port_;

 };

 class StdBrokerProConfigManager {
 public:
     StdBrokerProConfigManager();
     ~StdBrokerProConfigManager();

     /**
      * \brief Get broker address standard.
      * 
      * \return Broker address standard.
      */
     String GetStdBrokerAddress() const;

     /**
      * \brief Get broker port standard.
      * 
      * \return Broker port standard.
      */
     uint16_t GetStdBrokerPort() const;

     /**
      * \brief Get connect broker interval standard.
      * 
      * \return The interval of connect broker standard.
      */
     uint8_t GetStdConnectBrokerInterval() const;

     /**
      * \brief Get connect broker step standard.
      * 
      * \return The step of connect broker standard.
      */
     uint8_t GetStdConnectBrokerStep() const;

     /**
      * \brief Get connect broker limit standard.
      * 
      * \return The limit of connect broker standard.
      */
     uint8_t GetStdConnectBrokerLimit() const;

     /**
      * \brief Set broker address standard.
      * 
      * \param address 	Broker address standard.
      * 
      * \return Result of set broker address standard.
      */
     bool SetStdBrokerAddress(const String &address);

     /**
      * \brief Set broker port standard.
      * 
      * \param port 		Broker port standard.
      * 
      * \return Result of set broker port standard.
      */
     bool SetStdBrokerPort(uint16_t port);

     /**
      * \brief Set connect broker interval standard.
      * 
      * \param interval 		Connect broker interval standard.
      * 
      * \return Result of set connect broker interval standard.
      */
     bool SetStdConnectBrokerInterval(uint8_t interval);

     /**
      * \brief Set connect broker step standard.
      * 
      * \param step 		Connect broker step standard.
      * 
      * \return Result of set connect broker step standard.
      */
     bool SetStdConnectBrokerStep(uint8_t step);

     /**
      * \brief Set connect broker limit standard.
      * 
      * \param limit 	Connect broker limit standard.
      * 
      * \return Result of set connect broker limit standard.
      */
     bool SetStdConnectBrokerLimit(uint8_t limit);

 private:
     String address_;
 	uint16_t port_;
 	uint8_t interval_;
 	uint8_t step_;
 	uint8_t limit_;
 };
}

#endif //MBOX_CONFIG_MANAGER
