#ifndef _COMMON_H_
#define _COMMON_H_

#include <Arduino.h>
#include <list>
#include "trace.h"

using namespace std;

#define SW_VERSION "1.3.7"
#define HW_VERSION "1.0.0"
#define CO_VERSION "1.0.0"
//#define VERSION ("1.0.0")
//#define JSON_BUFF 1024

#define JSON_BUFF_4K (4 * 1024)
#define JSON_BUFF_3K (3 * 1024)   // 6 * 1024 有问题, 太大
#define JSON_BUFF_2K (2 * 1024)
#define JSON_BUFF_1K (1 * 1024)

#define MQTT_REV_BUFF (14 * 1024)  // 14 KB

#define WIFI_SSID_A "BSP"
#define WIFI_SSID_B "YYTD"
#define WIFI_SSID_C "route_A"
#define WIFI_SSID_D "route_B"
#define WIFI_PASSWORD_A "0000000000"
#define WIFI_PASSWORD_B "yytd#1234"
#define WIFI_PASSWORD_C "123456789"
#define WIFI_PASSWORD_D "123456789"

#define MBOX_DEBUG
#define MBOX_DEBUG_SERIAL Serial1 //Serial
//#define MBOX_DEBUG_SERIAL trace

#ifdef MBOX_DEBUG
  #define MBOX_DEBUG_PRINT(...) { MBOX_DEBUG_SERIAL.print(__VA_ARGS__); }
  #define MBOX_DEBUG_PRINTLN(...) { MBOX_DEBUG_SERIAL.println(__VA_ARGS__); }
#else
  #define MBOX_DEBUG_PRINT(...) {}
  #define MBOX_DEBUG_PRINTLN(...) {}
#endif

struct DataPoint{
    uint32_t curTime;
    String dataType;
    String dataAddress;
	uint32_t pollFrequency;
};

struct DataInfo {
	String dataType;
    String dataAddress;
    // 目前只支持 2 字节，4 字节
    uint32_t value;
};

struct ModelData {
    String timestamp;  // use ntp client
    list<DataInfo> dataList; 
};

struct RegistDetail{
    String swVersion;
    String hwVersion;
	int result;
};

struct DevData{
    String timestamp;
    list<DataInfo> datainfo;
};

struct ReportDataInfo{
    String deviceId;
	list<DevData> devdata;
	int result;
};

struct SetDevData{
    String deviceId;
	list<DataInfo> datainfo;
};

struct AddDevProInfo{
    String deviceId;
	uint32_t pollInterval;
    uint32_t pollTimeout;
	uint32_t reportTimeout;
	uint8_t reportLimit;
};


struct DelDevProInfo{
    String deviceId;
};

struct AddDataModelInfo{
	String deviceType;
	String protocolType;
	bool swapFlag;
    list<DataPoint> dataPoints;
};

struct DelDataModelInfo{
	String deviceType;
};

struct DevConnectStatus{
	String deviceId;
	int status;
};

struct GetDevConnectStatus{
    list<DevConnectStatus> devconnectstatus;
};

struct OTAUpgrade{
    String swVersion;
    String uri;
};

struct RouteInfo{
	String bssid;
	String ssid;
	String password;
	int rssi;
	int channel;
	int result;
	bool status;
};
#endif
