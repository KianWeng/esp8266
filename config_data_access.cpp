/**
 * \file config_io.cpp
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2018-12-28
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "config_data_access.h"
#include <FS.h>

namespace mbox_dev_p3 {

ConfigDataAccessPtr ConfigDataAccess::_instance;

ConfigDataAccess::ConfigDataAccess() {}

ConfigDataAccess::~ConfigDataAccess() {}

ConfigDataAccess::ConfigDataAccess(const ConfigDataAccess &) {}

ConfigDataAccess& ConfigDataAccess::operator=(const ConfigDataAccess&) {}

ConfigDataAccessPtr ConfigDataAccess::Instance() {
    if(nullptr == _instance)
        _instance = ConfigDataAccessPtr(new ConfigDataAccess);
    return _instance;
}

void ConfigDataAccess::SetConfigFilePath(const String &filePath) {
    _cfgFilePath = filePath;
}

bool ConfigDataAccess::ConfigFileExists() {
    if(0 == _cfgFilePath.length()) {
        MBOX_DEBUG_PRINTLN("Config file path is empty.");
        return false;
    }

	//MBOX_DEBUG_PRINTLN("SPIFFS remove _cfgFilePath.c_str()");
	//SPIFFS.remove(_cfgFilePath.c_str());
	
    return SPIFFS.exists(_cfgFilePath.c_str());
}

/*
String ConfigDataAccess::GetConfig() const {
    if(0 == _cfgFilePath.length()) {
        MBOX_DEBUG_PRINTLN("Config file path is empty.");
        return "";
    }

    File configFile = SPIFFS.open(_cfgFilePath.c_str(), "r");
    if(!configFile) {
        MBOX_DEBUG_PRINTLN("Failed to open config file.");
        return "";
    }

    size_t fileSize = configFile.size();
    std::unique_ptr<char[]> buf(new char[fileSize]);
    configFile.readBytes(buf.get(), fileSize);

	configFile.close();
    
    return String(buf.get());
}*/

char* ConfigDataAccess::GetConfig() const {
    if(0 == _cfgFilePath.length()) {
        MBOX_DEBUG_PRINTLN("Config file path is empty.");
        return NULL;
    }

    File configFile = SPIFFS.open(_cfgFilePath.c_str(), "r");
    if(!configFile) {
        MBOX_DEBUG_PRINTLN("Failed to open config file.");
        return NULL;
    }

    size_t fileSize = configFile.size();
	char* buff = (char*)calloc(fileSize + 1, sizeof(char));
    //std::unique_ptr<char[]> buf(new char[fileSize]);
    configFile.readBytes(buff, fileSize);

	configFile.close();
    
    return buff;
}

/*    
bool ConfigDataAccess::SetConfig(const String &config) {
    if(0 == config.length())
        return false;
    
    if(0 == _cfgFilePath.length()) {
        MBOX_DEBUG_PRINTLN("Config file path is empty");
        return false;
    }

    File configFile = SPIFFS.open(_cfgFilePath.c_str(), "w+");
    if(!configFile) {
        MBOX_DEBUG_PRINTLN("Failed to open config file");
        return false;
    }

    configFile.write((const uint8_t *)config.c_str(), config.length());
	configFile.close();
	
    return true;
}*/

bool ConfigDataAccess::SetConfig(const char* config) {
    if(!config)
        return false;
    
    if(0 == _cfgFilePath.length()) {
        MBOX_DEBUG_PRINTLN("Config file path is empty");
        return false;
    }

    File configFile = SPIFFS.open(_cfgFilePath.c_str(), "w+");
    if(!configFile) {
        MBOX_DEBUG_PRINTLN("Failed to open config file");
        return false;
    }

    configFile.write((const uint8_t *)config, strlen(config));
	configFile.close();
	
    return true;
}

void ConfigDataAccess::ClearConfig(){
	if(ConfigFileExists()) SPIFFS.remove(_cfgFilePath.c_str());
}

}
