/**
 * \file mbox_config_manager.cpp
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2018-12-25
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "mbox_config_manager.h"
#include <ArduinoJson.h>
#include "config_data_access.h"
#include "default_config.hpp"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "data_poll.h"

extern DataPoll dataPoll;
extern String deviceId;
extern bool dataSwapFlag;
extern bool singleWrite;

namespace mbox_dev_p3 {

// MBoxConfigManager
MBoxConfigManager::MBoxConfigManager() {}
MBoxConfigManager::~MBoxConfigManager() {}

void MBoxConfigManager::SetConfigFilePath(const String &filePath) {
    cfgFilePath_ = filePath;
	ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
	cfgDA.get()->SetConfigFilePath(cfgFilePath_);
}

bool MBoxConfigManager::ConfigFileExists() {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    return cfgDA.get()->ConfigFileExists();
}

bool MBoxConfigManager::CreateDefaultConfig() {
	bool result;
	String config;

    config = "devId:null,pollVal:0,pollTot:0,rptTot:0,rptLmt:0,#,dataPoints:null,devType:null,proType:null,swpFlag:0";
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    result = cfgDA.get()->SetConfig(config.c_str());

	return result;
}


// MBoxBasicConfigManager
MBoxBasicConfigManager::MBoxBasicConfigManager() {}
MBoxBasicConfigManager::~MBoxBasicConfigManager() {}

bool MBoxBasicConfigManager::LoadConfig() {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[Basic LoadConfig] Json parse error\n");
		free(config);
    	return false;
  	}
	
    const char* thingName = root["thingName"];	
    thingName_ = thingName;
	
	free(config);
	return true;
}

String MBoxBasicConfigManager::GetThingName() const {
    return thingName_;
}
    
bool MBoxBasicConfigManager::SetThingName(const String &thingName) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetThingName] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}

    root["thingName"] = thingName;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);
    if(ret)
        thingName_ = thingName;

	free(newConfig);
	free(config);
    return ret;
}

// APConfigManager
APConfigManager::APConfigManager() {}
APConfigManager::~APConfigManager() {}

bool APConfigManager::LoadConfig() {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[AP LoadConfig] Json parse error\n");
		free(config);
    	return false;
  	}
	
    String password = root["APPassword"];
    password_ = password;
    uint8_t timeout = root["APTimeout"];
    timeout_ = timeout;

	free(config);
	return true;
}

String APConfigManager::GetPassword() const {
    return password_;
}

bool APConfigManager::SetAPPassword(const String &password) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetAPPassword] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["APPassword"] = password;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
        password_ = password;   
    }

	free(config);
	free(newConfig);
    return ret;
}

uint8_t APConfigManager::GetTimeout() const {
    return timeout_;
}

bool APConfigManager::SetAPTimeout(uint8_t timeout) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetAPTimeout] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["APTimeout"] = timeout;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret)
        timeout_ = timeout;

	free(config);
	free(newConfig);
    return ret;
}


// WifiConfigManager
WifiConfigManager::WifiConfigManager() {}
WifiConfigManager::~WifiConfigManager() {}

bool WifiConfigManager::LoadConfig() {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[Wifi LoadConfig] Json parse error\n");
		free(config);
    	return false;
  	}
	
	String wifiSSID = root["WifiSSID"];
    wifiSSID_ = wifiSSID;
	String wifiPassword = root["WifiPassword"];
    wifiPassword_ = wifiPassword;

	free(config);
	return true;
}

String WifiConfigManager::GetWifiPassword() const {
    return wifiPassword_;
}

bool WifiConfigManager::SetWifiPassword(const String &password) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetWifiPassword] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["WifiPassword"] = password;
    root.printTo(newConfig,JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
        wifiPassword_ = password;
    }

	free(config);
	free(newConfig);
    return ret;
}

String WifiConfigManager::GetWifiSSID() const {
    return wifiSSID_;
}

bool WifiConfigManager::SetWifiSSID(const String &ssid) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetWifiPassword] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["WifiSSID"] = ssid;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
        wifiSSID_ = ssid;
    }

	free(config);
	free(newConfig);
    return ret;
}

 //SerialConfigManager
 SerialConfigManager::SerialConfigManager() {}
 SerialConfigManager::~SerialConfigManager() {}

 bool SerialConfigManager::LoadConfig() {
 	 ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[Serial LoadConfig] Json parse error\n");
		free(config);
    	return false;
  	 }
	 
     baudRate_ = root["baudRate"];
     flowControl_ = root["flowControl"];
     dataBit_ = root["dataBit"];
     stopBit_ = root["stopBit"];
     parity_ = root["parity"];

	 free(config);
	 return true;
 }

 int SerialConfigManager::GetBaudrate() const {
     return baudRate_;
 }

 bool SerialConfigManager::SetBaudrate(int baudrate) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetBaudrate] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["baudRate"] = baudrate;
     root.printTo(newConfig,JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret) {
        baudRate_ = baudrate;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

 enum SerialConfig SerialConfigManager::GetSerialConfigEnum(String SerialConfigStr)
 {
     if (SerialConfigMap.count(SerialConfigStr) == 0)
     {
         return SERIAL_8N1;
     }
     else
     {
         return SerialConfigMap[SerialConfigStr];
     }
 }

 String SerialConfigManager::MakeSerialConfigStr()
 {
     string str;
     //stringstream stream;
     //stream << parity_;
     //str = stream.str();
     return "SERIAL_";// + to_string(dataBit_) + str + to_string(stopBit_);
 }

 uint8_t SerialConfigManager::GetFlowControl() const {
     return flowControl_;
 }

 bool SerialConfigManager::SetFlowControl(uint8_t flowControl) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetFlowControl] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["flowControl"] = flowControl;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);
	 
     if(ret) {
        flowControl_ = flowControl;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

 uint8_t SerialConfigManager::GetDataBits() const {
     return dataBit_;
 }

 bool SerialConfigManager::SetDataBits(uint8_t dataBits) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetDataBits] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["dataBit"] = dataBits;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret) {
        dataBit_ = dataBits;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

 uint8_t SerialConfigManager::GetStopBits() const {
     return stopBit_;
 }

 bool SerialConfigManager::SetStopBits(uint8_t stopBits) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStopBits] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["stopBit"] = stopBits;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret) {
        stopBit_ = stopBits;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

 char SerialConfigManager::GetParity() const {
     return parity_;
 }

 bool SerialConfigManager::SetParity(char parity) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetParity] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["parity"] = parity;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret) {
        parity_ = parity;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

// NetworkConfigManager
NetworkConfigManager::NetworkConfigManager() {}
NetworkConfigManager::~NetworkConfigManager() {}

bool NetworkConfigManager::LoadConfig() {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[Network LoadConfig] Json parse error\n");
		free(config);
    	return false;
  	}
	
    dhcpEnabled_ = root["dhcpEnabled"];
	String ip = root["ip"];
    ip_ = ip;
	String netmask = root["netmask"];
    netmask_ = netmask;
	String gateway = root["gateway"];
    gateway_ = gateway;

	free(config);
	return true;
}

bool NetworkConfigManager::DHCPEnabled() {
    return dhcpEnabled_;
}

bool NetworkConfigManager::SetDHCPStatus(bool status) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetDHCPStatus] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["dhcpEnabled"] = status;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
       dhcpEnabled_ = status;
    }

	free(config);
	free(newConfig);
    return ret;
}

String NetworkConfigManager::GetIP() const {
    return ip_;
}

bool NetworkConfigManager::SetIP(const String &ip) {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetIP] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["ip"] = ip;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
       ip_ = ip;
    }

	free(config);
	free(newConfig);
    return ret;
}

String NetworkConfigManager::GetNetmask() const {
    return netmask_;
}

bool NetworkConfigManager::SetNetmask(const String &netmask)  {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetNetmask] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["netmask"] = netmask;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
       netmask_ = netmask;
    }

	free(config);
	free(newConfig);
    return ret;
}

String NetworkConfigManager::GetGateway() const {
    return gateway_;
}

bool NetworkConfigManager::SetGateway(const String &gateway)  {
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;
	char* newConfig = (char*)malloc(JSON_BUFF_3K);
	if(!newConfig){
		free(config);
		return false;
	}
	
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.parseObject(config);
	if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetGateway] Json parse error\n");
		free(config);
		free(newConfig);
    	return false;
  	}
	
    root["gateway"] = gateway;
    root.printTo(newConfig, JSON_BUFF_3K);
    bool ret = cfgDA.get()->SetConfig(newConfig);

    if(ret) {
       gateway_ = gateway;
    }

	free(config);
	free(newConfig);
    return ret;
}

// DevProConfigManager
DevProConfigManager::DevProConfigManager() {}
DevProConfigManager::~DevProConfigManager() {}

void DevProConfigManager::ParseDataPoints(String& dataPoints){
	char *str;
	char *p;
	char buf[30] = {0};
	char index = 0;
	int position;
	DataPoint dataPoint;
	MBOX_DEBUG_PRINTLN("[ParseDataPoints]Data is:" + dataPoints);
	dataPoints_.clear();
	do{
		index = 0;
		//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Data String is:" + dataPoints);
	 	position = dataPoints.indexOf('*');
		//MBOX_DEBUG_PRINTLN("[ParseDataPoints]position is:" + String(position));
		if(position != -1)
		{
			memset(buf,0,sizeof(buf));
			String strTmp = dataPoints.substring(0,position);
			strncpy(buf, strTmp.c_str(), strTmp.length());
			for(str = strtok_r(buf, "|", &p); str; str = strtok_r(NULL, "|", &p)){
				if(index == 0){
					dataPoint.dataAddress = str;
					//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Address:" + dataPoint.dataAddress);
				}
				else if(index == 1){
					dataPoint.dataType = str;
					//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Type:" + dataPoint.dataType);
				}
				else if(index == 2){
					dataPoint.pollFrequency = String(str).toInt();
					//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Frequency:" + String(dataPoint.pollFrequency));
				}
				index++;
			}
			dataPoints_.push_back(dataPoint);
			dataPoints = dataPoints.substring(position + 1, dataPoints.length());
		}
		else
		{
			if(dataPoints.indexOf('|') == -1){return;}
			else{
				strncpy(buf, dataPoints.c_str(), dataPoints.length());
				for(str = strtok_r(buf, "|", &p); str; str = strtok_r(NULL, "|", &p)){
					if(index == 0){
						dataPoint.dataAddress = str;
						//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Address:" + dataPoint.dataAddress);
					}
					else if(index == 1){
						dataPoint.dataType = str;
						//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Type:" + dataPoint.dataType);
					}
					else if(index == 2){
						dataPoint.pollFrequency = String(str).toInt();
						//MBOX_DEBUG_PRINTLN("[ParseDataPoints]Frequency:" + String(dataPoint.pollFrequency));
					}
					index++;
				}
				dataPoints_.push_back(dataPoint);
			}
		}
	 }while(position >=0);
}

void DevProConfigManager::ParseConfigs(String& config){
	int position;
	
	MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]config:" + config);
	if(config.startsWith("devId")){
		position = config.indexOf(':');
		deviceId_ = config.substring(position + 1,config.length());
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]devId:" + deviceId_);
	}
	else if(config.startsWith("devType")){
		position = config.indexOf(':');
		deviceType_ = config.substring(position + 1,config.length());
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]devType:" + deviceType_);
	}
	else if(config.startsWith("proType")){
		position = config.indexOf(':');
		protocolType_ = config.substring(position + 1,config.length());
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]proType:" + protocolType_);
	}
	else if(config.startsWith("swpFlag")){
		position = config.indexOf(':');
		int swapFlag = config.substring(position + 1,config.length()).toInt();
		if(swapFlag) swapFlag_ = true;
		else swapFlag_ = false;
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]swpFlag:" + String(swapFlag_));
	}
	else if(config.startsWith("pollVal")){
		position = config.indexOf(':');
		pollInterval_ = config.substring(position + 1,config.length()).toInt();
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]pollVal:" + String(pollInterval_));
	}
	else if(config.startsWith("pollTot")){
		position = config.indexOf(':');
		pollTimeout_ = config.substring(position + 1,config.length()).toInt();
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]pollTot:" + String(pollTimeout_));
	}
	else if(config.startsWith("pollTot")){
		position = config.indexOf(':');
		pollTimeout_ = config.substring(position + 1,config.length()).toInt();
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]pollTot:" + String(pollTimeout_));
	}
	else if(config.startsWith("rptTot")){
		position = config.indexOf(':');
		reportTimeout_ = config.substring(position + 1,config.length()).toInt();
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]rptTot:" + String(reportTimeout_));
	}
	else if(config.startsWith("rptLmt")){
		position = config.indexOf(':');
		reportLimit_ = config.substring(position + 1,config.length()).toInt();
		MBOX_DEBUG_PRINTLN("[DevProConfigManager][ParseConfigs]rptLmt:" + String(reportLimit_));
	}
	else if(config.startsWith("dataPoints")){
		position = config.indexOf(':');
		String dataPoints = config.substring(position + 1, config.length());
		ParseDataPoints(dataPoints);
	}
}

bool DevProConfigManager::LoadConfig() {
	 int position;
	 String configStr;
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;

	 configStr = config;
	 MBOX_DEBUG_PRINTLN("[DevPro LoadConfig]" + configStr);
	 do{
	 	position = configStr.indexOf(',');
		if(position != -1)
		{
			String strTmp = configStr.substring(0,position);
			ParseConfigs(strTmp);
			configStr = configStr.substring(position + 1, configStr.length());
		}
		else
		{
			//MBOX_DEBUG_PRINTLN("Config is not correct.")
			ParseConfigs(configStr);
		}
	 }while(position >=0);
	 
	 free(config);
	 return true;
}

 String DevProConfigManager::GetDeviceID() const {
     return deviceId_;
 }

 String DevProConfigManager::GetDeviceType() const {
     return deviceType_;
 }

 String DevProConfigManager::GetProtocolType() const {
 	return protocolType_;
 }

 bool DevProConfigManager::GetSwapFlag() const {
 	return swapFlag_;
 }

 bool DevProConfigManager::AddDevice(const String &id, uint32_t pollInterval, 
 								uint32_t pollTimeout, uint32_t reportTimeout, uint8_t reportLimit) {
	 int position;
	 String oldConfigs;
	 String newConfigs;
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
	 char* config = cfgDA.get()->GetConfig();
	 if(!config) return false; 

	 oldConfigs = config;
	 MBOX_DEBUG_PRINT("Old config:");
	 MBOX_DEBUG_PRINTLN(oldConfigs);
	 String tmp = "devId:" + id;
	 tmp = tmp + ",pollVal:" + String(pollInterval);
	 tmp = tmp + ",pollTot:" + String(pollTimeout);
	 tmp = tmp + ",rptTot:" + String(reportTimeout);
	 tmp = tmp + ",rptLmt:" + String(reportLimit);
	 tmp = tmp + ",";
	 
	 position = oldConfigs.indexOf('#');
	 newConfigs = tmp + oldConfigs.substring(position, oldConfigs.length());

	 MBOX_DEBUG_PRINT("New config:");
	 MBOX_DEBUG_PRINTLN(newConfigs);
     bool ret = cfgDA.get()->SetConfig(newConfigs.c_str());
     if(ret) {
	 	 deviceId = id;
         deviceId_ = id;		 
		 pollInterval_ = pollInterval;
         pollTimeout_ = pollTimeout;
         reportTimeout_ = reportTimeout;
         reportLimit_ = reportLimit;
     }
	 
	 free(config);
     return ret;
 }

 bool DevProConfigManager::DeleteDevice(const String &id) {
 	 bool ret = false;
	 int position;
	 String oldConfigs;
	 String newConfigs;
	 
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;

	 oldConfigs = config;
	 MBOX_DEBUG_PRINT("[DevPro DeleteDevice]Old config:");
	 MBOX_DEBUG_PRINTLN(oldConfigs);

	 if((deviceId_ == id) || (deviceId_ == "null") || (deviceType_.length() == 0)){
	 	 String tmp = "devId:null,pollVal:0,pollTot:0,rptTot:0,rptLmt:0,";
		 position = oldConfigs.indexOf('#');
	     newConfigs = tmp + oldConfigs.substring(position, oldConfigs.length());
		 
		 MBOX_DEBUG_PRINT("New config:");
	 	 MBOX_DEBUG_PRINTLN(newConfigs);
	     ret = cfgDA.get()->SetConfig(newConfigs.c_str());

	     if(ret) {
	         deviceId_ = "null";
			 deviceId = "";
	         pollInterval_ = 0;
	         pollTimeout_ = 0;
	         reportTimeout_ = 0;
	         reportLimit_ = 0;
	     }
	 }

	 free(config);
     return ret;
 }

 list<DataPoint> DevProConfigManager::GetDeviceModel(const String &deviceType) const {
     if(deviceType_ != deviceType)
	 {
	 	list<DataPoint> mDataPoint;
	 	return mDataPoint;
     }
	 
     return dataPoints_;
 }

 uint32_t DevProConfigManager::GetPollInterval(const String &deviceType) const {
     if(deviceType_ != deviceType) return 0;
     return pollInterval_;
 }

 uint32_t DevProConfigManager::GetPollTimeout(const String &deviceType) const {
     if(deviceType_ != deviceType) return 0;
     return pollTimeout_;
 }

 uint32_t DevProConfigManager::GetReportTimeout(const String &deviceType) const {
     if(deviceType_ != deviceType) return 0;
     return reportTimeout_;
 }

 uint8_t DevProConfigManager::GetReportLimit(const String &deviceType) const {
     if(deviceType_ != deviceType) return 0;
     return reportLimit_;
 }

 bool DevProConfigManager::AddModel(const String &deviceType, const String &protocolType, bool swapFlag, const list<DataPoint> dataPoints) {
     int position;
	 String oldConfigs;
	 String newConfigs;
	 String tmp = "dataPoints:";
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;

	 oldConfigs = config;
	 MBOX_DEBUG_PRINT("Old config:");
	 MBOX_DEBUG_PRINTLN(oldConfigs);

	 position = oldConfigs.indexOf('#');
	 
	 list<DataPoint>::const_iterator iter = dataPoints.begin();
	 for(; iter != dataPoints.end(); iter++)
	 {
	 	tmp = tmp + iter->dataAddress + "|" + iter->dataType + "|" + String(iter->pollFrequency) + "*";
	 }

	 newConfigs = oldConfigs.substring(0, position + 2) + tmp.substring(0,tmp.length() - 1) + ",devType:" + deviceType + ",proType:" + protocolType + ",swpFlag:" + String(swapFlag);
	 MBOX_DEBUG_PRINT("New config:");
	 MBOX_DEBUG_PRINTLN(newConfigs);
     bool ret = cfgDA.get()->SetConfig(newConfigs.c_str());
	 free(config);

     if(ret) {
     	deviceType_ = deviceType;
		protocolType_ = protocolType;
	 	if(protocolType.equals("MODBUS-RTU")){
			singleWrite = true;
	 	}
		else if(protocolType.equals("MODBUS-RTU-SMGL3")){
			singleWrite = false;
		}
	 	swapFlag_ = swapFlag;
		dataSwapFlag = swapFlag;
		list<DataPoint> tmpDataPoints = dataPoints;
	 	dataPoll.SetDataModelInfo(tmpDataPoints);
     }else{
     	deviceType_ = "";
		protocolType_ = "";
		swapFlag_ = false;
		dataPoints_.clear();
     }
     return ret;
 }

bool DevProConfigManager::DeleteModel(const String &deviceType) {
	bool ret = false;
	int position;
	String oldConfigs;
	String newConfigs;
    ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
    char* config = cfgDA.get()->GetConfig();
	if(!config) return false;

	oldConfigs = config;
	MBOX_DEBUG_PRINT("Old config:");
	MBOX_DEBUG_PRINTLN(oldConfigs);
	position = oldConfigs.indexOf('#');
	
	if((deviceType_ == deviceType) || (deviceType_ == "null") || (deviceType_.length() == 0)){
		newConfigs = oldConfigs.substring(0, position) + "#,dataPoints:null,devType:null,proType:null,swpFlag:0";		
		MBOX_DEBUG_PRINT("New config:");
	 	MBOX_DEBUG_PRINTLN(newConfigs);
	    ret = cfgDA.get()->SetConfig(newConfigs.c_str());

	    if(ret) {
	         deviceType_ = "";
			 protocolType_ = "";
			 swapFlag_ = false;
			 dataSwapFlag = swapFlag_;
			 list<DataPoint> dataModelInfoEmpty;
			 dataModelInfoEmpty.clear();
	 		 dataPoll.SetDataModelInfo(dataModelInfoEmpty);
	    }
	}

	free(config);
    return ret;
 }

 bool DevProConfigManager::UpdateModel(const String &deviceType, uint8_t pollInterval, uint8_t pollTimeout, 
 										   uint8_t reportTimeout, uint8_t reportLimit, const list<DataPoint> dataPoints) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[UpdateModel] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["deviceType"] = deviceType;
     root["pollInterval"] = pollInterval;
     root["pollTimeout"] = pollTimeout;
     root["reportTimeout"] = reportTimeout;
     root["reportLimit"] = reportLimit;
     //root["dataPoints"] = dataPoints;
     JsonArray& mDataPoints = root.createNestedArray("dataPoints");
	 list<DataPoint>::const_iterator iter = dataPoints.begin();
	 for(; iter != dataPoints.end(); iter++)
	 {
	 	JsonObject& mDataPoint = mDataPoints.createNestedObject();
		mDataPoint["dataType"] = iter->dataType;
		mDataPoint["dataAddress"] = iter->dataAddress;
	 }
	 
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret) {
         deviceType_ = deviceType;
         pollInterval_ = pollInterval;
         pollTimeout_ = pollTimeout;
         reportTimeout_ = reportTimeout;
         reportLimit_ = reportLimit;
         dataPoints_ = dataPoints;
     }

	 free(config);
	 free(newConfig);
     return ret;
 }

 // HttpProConfigManager
 HttpProConfigManager::HttpProConfigManager() {}
 HttpProConfigManager::~HttpProConfigManager() {}

 uint16_t HttpProConfigManager::GetHTTPPort() const {
     return port_;
 }

 bool HttpProConfigManager::SetHTTPPort(uint16_t port) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetHTTPPort] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["port"] = port;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         port_ = port;

	 free(config);
	 free(newConfig);
     return ret;
 }

 // BrokerProConfigManager
 BrokerProConfigManager::BrokerProConfigManager() {}
 BrokerProConfigManager::~BrokerProConfigManager() {}

 String BrokerProConfigManager::GetBrokerAddress() const {
     return address_;
 }

 uint16_t BrokerProConfigManager::GetBrokerPort() const {
     return port_;
 }

 uint8_t BrokerProConfigManager::GetConnectBrokerInterval() const {
     return interval_;
 }

 uint8_t BrokerProConfigManager::GetConnectBrokerStep() const {
     return step_;
 }

 uint8_t BrokerProConfigManager::GetConnectBrokerLimit() const {
     return limit_;
 }

 bool BrokerProConfigManager::SetBrokerAddress(const String &address) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetBrokerAddress] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["address"] = address;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         address_ = address;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool BrokerProConfigManager::SetBrokerPort(uint16_t port) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetBrokerPort] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["port"] = port;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         port_ = port;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool BrokerProConfigManager::SetConnectBrokerInterval(uint8_t interval) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetConnectBrokerInterval] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["interval"] = interval;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         interval_ = interval;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool BrokerProConfigManager::SetConnectBrokerStep(uint8_t step) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetConnectBrokerStep] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["step"] = step;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        step_ = step;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool BrokerProConfigManager::SetConnectBrokerLimit(uint8_t limit) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetConnectBrokerLimit] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
     root["limit"] = limit;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        limit_ = limit;

	 free(config);
	 free(newConfig);
     return ret;
 }

 // RegisterConfigManager
 RegisterConfigManager::RegisterConfigManager() {}
 RegisterConfigManager::~RegisterConfigManager() {}

 uint8_t RegisterConfigManager::GetRegisterInterval() const {
     return interval_;
 }

 uint8_t RegisterConfigManager::GetRegisterStep() const {
     return step_;
 }

 uint8_t RegisterConfigManager::GetRegisterLimit() const {
     return limit_;
 }

 bool RegisterConfigManager::SetRegisterInterval(uint8_t interval) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetRegisterInterval] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["interval"] = interval;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        interval_ = interval;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool RegisterConfigManager::SetRegisterStep(uint8_t step) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetRegisterStep] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["step"] = step;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        step_ = step;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool RegisterConfigManager::SetRegisterLimit(uint8_t limit) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetRegisterLimit] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["limit"] = limit;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        limit_ = limit;

	 free(config);
	 free(newConfig);
     return ret;
 }

 // NTPConfigManager
 NTPConfigManager::NTPConfigManager() {}
 NTPConfigManager::~NTPConfigManager() {}

 String NTPConfigManager::GetNTPServerAddress() const {
     return address_;
 }

 uint8_t NTPConfigManager::GetNTPSyncInterval() const {
     return interval_;
 }

 bool NTPConfigManager::SetNTPServerAddress(const String &address) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetNTPServerAddress] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["address"] = address;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
        address_ = address;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool NTPConfigManager::SetNTPSyncInterval(uint8_t interval) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetNTPSyncInterval] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["interval"] = interval;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         interval_ = interval;

	 free(config);
	 free(newConfig);
     return ret;
 }

 // StdAPConfigManager
 StdAPConfigManager::StdAPConfigManager() {}
 StdAPConfigManager::~StdAPConfigManager() {}

 String StdAPConfigManager::GetStdAPPassword() const {
     return password_;
 }

 uint8_t StdAPConfigManager::GetStdAPTimeout() const {
     return timeout_;
 }

 bool StdAPConfigManager::SetStdAPPassword(const String &password) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdAPPassword] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["password"] = password;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         password_ = password;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool StdAPConfigManager::SetStdAPTimeout(uint8_t timeout) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdAPTimeout] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["timeout"] = timeout;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         timeout_ = timeout;

	 free(config);
	 free(newConfig);
     return ret;
 }

 // StdHttpProConfigManager
 StdHttpProConfigManager::StdHttpProConfigManager() {}
 StdHttpProConfigManager::~StdHttpProConfigManager() {}

 uint16_t StdHttpProConfigManager::GetStdHTTPPort() const {
     return port_;
 }

 bool StdHttpProConfigManager::SetStdHTTPPort(uint16_t port){
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdHTTPPort] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["port"] = port;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         port_ = port;

	 free(config);
	 free(newConfig);
     return ret;
 }


 // StdBrokerProConfigManager
 StdBrokerProConfigManager::StdBrokerProConfigManager() {}
 StdBrokerProConfigManager::~StdBrokerProConfigManager() {}

 String StdBrokerProConfigManager::GetStdBrokerAddress() const {
 	return address_;
 }

 uint16_t StdBrokerProConfigManager::GetStdBrokerPort() const {
 	return port_;
 }

 uint8_t StdBrokerProConfigManager::GetStdConnectBrokerInterval() const {
 	return interval_;
 }

 uint8_t StdBrokerProConfigManager::GetStdConnectBrokerStep() const {
 	return step_;
 }

 uint8_t StdBrokerProConfigManager::GetStdConnectBrokerLimit() const {
     return limit_;
 }

 bool StdBrokerProConfigManager::SetStdBrokerAddress(const String &address) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdBrokerAddress] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["address"] = address;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         address_ = address;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool StdBrokerProConfigManager::SetStdBrokerPort(uint16_t port) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdBrokerPort] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["port"] = port;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         port_ = port;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool StdBrokerProConfigManager::SetStdConnectBrokerInterval(uint8_t interval) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdConnectBrokerInterval] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["interval"] = interval;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         interval_ = interval;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool StdBrokerProConfigManager::SetStdConnectBrokerStep(uint8_t step) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdConnectBrokerStep] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["step"] = step;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         step_ = step;

	 free(config);
	 free(newConfig);
     return ret;
 }

 bool StdBrokerProConfigManager::StdBrokerProConfigManager::SetStdConnectBrokerLimit(uint8_t limit) {
     ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) return false;
	 char* newConfig = (char*)malloc(JSON_BUFF_3K);
	 if(!newConfig){
		 free(config);
		 return false;
	 }
	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
	 if (!root.success()){
		MBOX_DEBUG_PRINTLN("[SetStdConnectBrokerLimit] Json parse error\n");
		free(config);
	 	free(newConfig);
    	return false;
  	 }
	 
     root["limit"] = limit;
     root.printTo(newConfig, JSON_BUFF_3K);
     bool ret = cfgDA.get()->SetConfig(newConfig);

     if(ret)
         limit_ = limit;

	 free(config);
	 free(newConfig);
     return ret;
 }
}









