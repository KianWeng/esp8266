#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "mqttjson.h"
#include "ArduinoJson.h"
#include "mqtt_content.h"
#include "data_poll.h"
#include "WebConf.h"

extern Ticker rptDataTimer;
extern DataPoll dataPoll;
extern String revMessageId;

extern String upgradeUri; 
extern int rptDataMaxTimeout;  
extern int rptDataMaxCount;  
extern int pollDataInterval;  
extern int pollDataTimeout;
extern bool singleWrite;

extern void dataPollFunc();

MQTTContentFunc::MQTTContentFunc() {}
MQTTContentFunc::~MQTTContentFunc() {
}

String MQTTContentFunc::RegisterContent(RegistDetail &registdetail)
{
	//StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
    JsonObject &root = jsonBuffer.createObject();
	
	root["mid"] = String(millis());
	root["version"] = CO_VERSION;
	mqttJsonFunc_.RegisterRequest(root, registdetail);

	String jsonStr;
	root.printTo(jsonStr);

	return jsonStr;
}

String MQTTContentFunc::ReportRealTimeData(ReportDataInfo &reportdatainfo)
{
	String jsonStr;
	//StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject &root = jsonBuffer.createObject();

	root["mid"] = String(millis());
	root["version"] = CO_VERSION;

	mqttJsonFunc_.ReportRealTimeDataRequest(root, reportdatainfo);
	root.printTo(jsonStr);

	return jsonStr;
}

String MQTTContentFunc::ReportHistoryData(ReportDataInfo &reportdatainfo)
{
	String jsonStr;
	//StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject &root = jsonBuffer.createObject();

	root["mid"] = String(millis());
	root["version"] = CO_VERSION;

	mqttJsonFunc_.ReportRealTimeDataRequest(root, reportdatainfo);
	root.printTo(jsonStr);

	return jsonStr;
}


bool MQTTContentFunc::SetDeviceData(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[SetDeviceData] enter.");
	SetDevData setDevData;
	result = mqttJsonFunc_.SetDevDataRequest(json, setDevData);
	if(result){
		MBOX_DEBUG_PRINTLN("[SetDeviceData]Pasrse set device data json success.");
		if(singleWrite)
			dataPoll.SetWriteData(setDevData.datainfo);
		else{
			DataInfo password_1, password_2;
			password_1.dataAddress = "36895";
			password_2.dataAddress = "36895";
			password_1.dataType = "U16";
			password_2.dataType = "U16";
			password_1.value = 20170;//19120
			password_2.value = 0;
			setDevData.datainfo.push_front(password_1);
			//setDevData.datainfo.push_back(password_2);//去掉写寄存器的pwd清零步骤
			dataPoll.SetWriteData(setDevData.datainfo);
		}	
	}else{
		MBOX_DEBUG_PRINTLN("[SetDeviceData]Pasrse set device data json fail.");
	}
	return result;
}

bool MQTTContentFunc::AddDeviceProperty(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[AddDeviceProperty] enter.");
	AddDevProInfo addDevProInfo;
	result = mqttJsonFunc_.AddDevProInfoRequest(json, addDevProInfo);
	if(result){
		MBOX_DEBUG_PRINTLN("[AddDeviceProperty]Pasrse add device property json success.");
		//add device property interface add here
		DevProConfigManager devProConfigManager_;
		devProConfigManager_.LoadConfig();
		result = devProConfigManager_.AddDevice(addDevProInfo.deviceId, addDevProInfo.pollInterval, 
				 addDevProInfo.pollTimeout, addDevProInfo.reportTimeout, addDevProInfo.reportLimit);
		if(result) {
			if((addDevProInfo.pollInterval > 0) && (pollDataInterval != addDevProInfo.pollInterval)) {
				pollDataInterval = addDevProInfo.pollInterval;
				if(rptDataTimer.active()){
					MBOX_DEBUG_PRINTLN("[AddDeviceProperty]Report data time changed, reattach report data timer");
					rptDataTimer.detach();
					delay(100);
					rptDataTimer.attach_ms(pollDataInterval, dataPollFunc);
				}
			}
			if(addDevProInfo.pollTimeout > 0) {
				pollDataTimeout = addDevProInfo.pollTimeout;
				dataPoll.SetModbusDataTimeout(pollDataTimeout);
			}
			if(addDevProInfo.reportTimeout > 0) rptDataMaxTimeout = addDevProInfo.reportTimeout;
			if(addDevProInfo.reportLimit > 0) rptDataMaxCount = addDevProInfo.reportLimit;
			MBOX_DEBUG_PRINTLN("[AddDeviceProperty]store device property to config file success.")
		}
		else MBOX_DEBUG_PRINTLN("[AddDeviceProperty]store device property to config file fail.")
	}
 
	return result;
}

bool MQTTContentFunc::DeleteDeviceProperty(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[DeleteDeviceProperty] enter.");
	DelDevProInfo delDevProInfo;
	result = mqttJsonFunc_.DeleteDevProInfoRequest(json, delDevProInfo);
	if(result){
		MBOX_DEBUG_PRINTLN("[DeleteDeviceProperty]Pasrse delete device property json success.");
		DevProConfigManager devProConfigManager_;
		devProConfigManager_.LoadConfig();
		result = devProConfigManager_.DeleteDevice(delDevProInfo.deviceId);
		if(result) MBOX_DEBUG_PRINTLN("[DeleteDeviceProperty]delete device property from config file success.")
		else MBOX_DEBUG_PRINTLN("[DeleteDeviceProperty]delete device property from config file fail.")
	}

	return result;
}

bool MQTTContentFunc::AddDataModel(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[AddDataModel] enter.");
	AddDataModelInfo addDataModelInfo;
	result = mqttJsonFunc_.AddDataModelInfoRequest(json, addDataModelInfo);
	if(result){
		MBOX_DEBUG_PRINTLN("[AddDataModel]Pasrse add data model json success.");
		DevProConfigManager devProConfigManager_;
		devProConfigManager_.LoadConfig();
		result = devProConfigManager_.AddModel(addDataModelInfo.deviceType, addDataModelInfo.protocolType, addDataModelInfo.swapFlag, addDataModelInfo.dataPoints);
		if(result) MBOX_DEBUG_PRINTLN("[AddDataModel]store device data to config file success.")
		else MBOX_DEBUG_PRINTLN("[AddDataModel]store device data to config file fail.")
	}

	return result;
}

bool MQTTContentFunc::DeleteDataModel(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[DeleteDataModel] enter.");
	DelDataModelInfo delDataModelInfo;
	result = mqttJsonFunc_.DeleteDataModelInfoRequest(json, delDataModelInfo);
	if(result){
		MBOX_DEBUG_PRINTLN("[DeleteDataModel]Pasrse del data model json success.");
		DevProConfigManager devProConfigManager_;
		devProConfigManager_.LoadConfig();
		result = devProConfigManager_.DeleteModel(delDataModelInfo.deviceType);
		if(result) MBOX_DEBUG_PRINTLN("[DeleteDataModel]delete device data from config file success.")
		else MBOX_DEBUG_PRINTLN("[DeleteDataModel]delete device data from config file fail.")
	}

	return result;
}

bool MQTTContentFunc::DeviceConnectionStatus(char *json, DevConnectStatus &devConnectStatus)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[DeviceConnectionStatus] enter.");
	//GetDevConnectStatus getDevConnectStatus;
	result = mqttJsonFunc_.GetDevConnectStatusRequest(json, devConnectStatus);
	if(result){
		MBOX_DEBUG_PRINTLN("[DeviceConnectionStatus]Pasrse device connection status json success.");
		devConnectStatus.status = dataPoll.GetDevConnStaus();
	}

    //
	return result;
}

bool MQTTContentFunc::OTA(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[OTA] enter.");
	OTAUpgrade mOTAUpgrade;
	result = mqttJsonFunc_.OTAUpgradeRequest(json, mOTAUpgrade);
	if(result){
		MBOX_DEBUG_PRINTLN("[OTA]Pasrse OTA json success.");
		upgradeUri = mOTAUpgrade.uri;
	}

	return result;
}

bool MQTTContentFunc::Reboot(char *json)
{
	bool result = false;

	MBOX_DEBUG_PRINTLN("[Reboot] enter.");
	result = mqttJsonFunc_.RebootRequest(json);
	if(result){
		MBOX_DEBUG_PRINTLN("[Reboot]Pasrse reboot json success.");
	}

	return result;
}

bool MQTTContentFunc::SetRouteInfo(char *json){
	bool result = false;
	RouteInfo routeInfo;

	MBOX_DEBUG_PRINTLN("[SetRouteInfo] enter.");
	result = mqttJsonFunc_.SetRouteInfoRequest(json, routeInfo);
	if(result){
		MBOX_DEBUG_PRINTLN("[Reboot]Pasrse set route info json success.");
		webConf.setRouteInfo(routeInfo);
	}

	return result;
}

String MQTTContentFunc::ReportRouteInfo(RouteInfo &routeInfo){
	String jsonStr;

	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject &root = jsonBuffer.createObject();

	root["mid"] = String(millis());
	root["version"] = CO_VERSION;

	mqttJsonFunc_.ReportRouteInfoRequest(root, routeInfo);
	root.printTo(jsonStr);

	return jsonStr;
}


