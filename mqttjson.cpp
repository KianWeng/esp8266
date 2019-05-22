#include "mqttjson.h"
#include <ArduinoJson.h>
#include "mbox_config_manager.h"

extern String revMessageId;
extern String revMessageId_heartbeat;
extern String deviceId;

MQTTJsonFunc::MQTTJsonFunc() {}
MQTTJsonFunc::~MQTTJsonFunc() {}

void MQTTJsonFunc::Response(JsonObject& root, int result){

    JsonObject &payLoad = root.createNestedObject("pld");
	
	payLoad["result"] = result;
}

void MQTTJsonFunc::RegisterRequest(JsonObject& root, RegistDetail &registdetail){
    JsonObject& payload = root.createNestedObject("pld");

	payload["swVersion"] = registdetail.swVersion;
	payload["hwVersion"] = registdetail.hwVersion;
}

/*
bool MQTTJsonFunc::RegisterResponse(char *json,RegistDetail &registdetail){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[RegisterResponse] Json parse error.");
    	return false;
  	}
	
	registdetail.result = root["pld"]["result"];

	return true;
}*/

JsonObject& MQTTJsonFunc::ReportRealTimeDataRequest(JsonObject &root, ReportDataInfo &reportdatainfo){
	
	//StaticJsonBuffer<JSON_BUFF> jsonBuffer;

    JsonObject& payLoad = root.createNestedObject("pld");
	payLoad["deviceId"] = reportdatainfo.deviceId;
	JsonArray &array = payLoad.createNestedArray("array");

	list<DevData>::iterator iter = reportdatainfo.devdata.begin();
	for(; iter != reportdatainfo.devdata.end(); iter++)
	{
		//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]begin");
		JsonObject& data_array = array.createNestedObject();
		data_array["ts"] = iter->timestamp;

		JsonArray& datas = data_array.createNestedArray("datas");
		list<DataInfo>::iterator iter_1 = iter->datainfo.begin();
		for(; iter_1 != iter->datainfo.end(); iter_1++)
		{
			//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]data address:" + iter_1->dataAddress);
			//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]data type:" + iter_1->dataType);
			if(iter_1->dataType.startsWith("S16")){
				//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]signed value:" + String((int16_t)iter_1->value));
				String tmp = String(iter_1->dataAddress) + "," + String((int16_t)iter_1->value);
				datas.add(tmp);
			}else if(iter_1->dataType.startsWith("S32")){
				//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]signed value:" + String((int32_t)iter_1->value));
				String tmp = String(iter_1->dataAddress) + "," + String((int32_t)iter_1->value);
				datas.add(tmp);
			}else{
				//MBOX_DEBUG_PRINTLN("[ReportRealTimeDataRequest]unsigned value:" + String(iter_1->value));
				String tmp = String(iter_1->dataAddress) + "," + String(iter_1->value);
				datas.add(tmp);
			}
		}
	}

	return payLoad;
}

/*
bool MQTTJsonFunc::ReportRealTimeDataResponse(char *json,ReportDataInfo &reportdatainfo){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[ReportRealTimeDataResponse] Json parse error.");
    	return false;
  	}

	reportdatainfo.result = root["pld"]["result"];
	return true;
}

JsonObject& MQTTJsonFunc::ReportHistoricalDataRequest(ReportDataInfo &reportdatainfo){
	
	StaticJsonBuffer<JSON_BUFF> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();
	root["deviceId"] = reportdatainfo.deviceId;
	JsonArray& array = root.createNestedArray("array");

	list<DevData>::iterator iter = reportdatainfo.devdata.begin();
	for(; iter != reportdatainfo.devdata.end(); iter++)
	{
		JsonObject& data_array = array.createNestedObject();
		data_array["ts"] = iter->timestamp;

		JsonArray& datas = data_array.createNestedArray("datas");
		list<DataInfo>::iterator iter_1 = iter->datainfo.begin();
		for(; iter_1 != iter->datainfo.end(); iter_1++)
		{
			//JsonObject& data_info = datas.createNestedObject();
			//data_info["address"] = iter_1->dataAddress;
			//data_info["value"] = String(iter_1->value);
			String tmp = String(iter_1->dataAddress) + "," + String(iter_1->value);
			datas.add(tmp);
		}
	}
	
	return root;

}

bool MQTTJsonFunc::ReportHistoricalDataResponse(const String &json,ReportDataInfo &reportdatainfo){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json.c_str());
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[ReportHistoricalDataResponse] Json parse error.");
    	return false;
  	}

	reportdatainfo.result = root["pld"]["result"];
	return true;
}*/

bool  MQTTJsonFunc::SetDevDataRequest(char *json, SetDevData &setdevdata){
	int num = 0;
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject& root = jsonBuffer.parseObject(json);
	
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[SetDevDataRequest] Json parse error.");
    	return false;
  	}

	const char* deviceId = root["pld"]["deviceId"];
	setdevdata.deviceId = deviceId;

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	
	if(setdevdata.deviceId.length() == 0){
		MBOX_DEBUG_PRINTLN("[SetDevDataRequest]Pasre deviceId error.");
		return false;
	}
	else{
		MBOX_DEBUG_PRINT("[SetDevDataRequest]Pasre deviceId:");
		MBOX_DEBUG_PRINTLN(setdevdata.deviceId);
	}
	
	JsonArray& nestedArray = root["pld"]["data"];

	if(nestedArray.size() != 0){
		for(num = 0;num < nestedArray.size();num++){
			MBOX_DEBUG_PRINTLN("[SetDevDataRequest]Pasre data.");
			DataInfo dataInfo;
			const char* type_ = nestedArray[num]["type"];
			//const char* type_ = "U16";
			const char* address_ = nestedArray[num]["address"];
			const char* value_ = nestedArray[num]["value"];
			if(type_ != NULL && address_ != NULL && value_ != NULL) {
				dataInfo.dataType = type_;
				MBOX_DEBUG_PRINT("[SetDevDataRequest]Pasre data type:");
				MBOX_DEBUG_PRINTLN(dataInfo.dataType);
				dataInfo.dataAddress = address_;
				MBOX_DEBUG_PRINT("[SetDevDataRequest]Pasre data address:");
				MBOX_DEBUG_PRINTLN(dataInfo.dataAddress);
				dataInfo.value = atoi(value_);
				MBOX_DEBUG_PRINT("[SetDevDataRequest]Pasre data value:");
				MBOX_DEBUG_PRINTLN(dataInfo.value);
				setdevdata.datainfo.push_back(dataInfo);
			}
		}
		return true;
	}
	else{
		MBOX_DEBUG_PRINTLN("[SetDevDataRequest]Pasre data error.");
		return false;
	}	
}

/*
JsonObject& MQTTJsonFunc::SetDevDataResponse(int result){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
	
	root["result"] = result;
	
	return root;
}*/

bool  MQTTJsonFunc::AddDevProInfoRequest(char *json, AddDevProInfo &addDevProInfo){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);	
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[AddDevProInfoRequest]Json parse error.");
    	return false;
  	}

	const char* deviceId_ = root["pld"]["deviceId"];
	int pollInterval_ = root["pld"]["pollInterval"];
	int pollTimeout_ = root["pld"]["pollTimeout"];
	int reportTimeout_ = root["pld"]["reportTimeout"];
	int reportLimit_ = root["pld"]["reportLimit"];

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	if(deviceId_ != NULL){

		addDevProInfo.deviceId = deviceId_;

		addDevProInfo.pollInterval = pollInterval_;
		addDevProInfo.pollTimeout = pollTimeout_;
		addDevProInfo.reportTimeout = reportTimeout_;
		addDevProInfo.reportLimit = reportLimit_;
		MBOX_DEBUG_PRINT("[AddDevProInfoRequest]Pasre data deviceId:")
		MBOX_DEBUG_PRINTLN(addDevProInfo.deviceId)
		MBOX_DEBUG_PRINT("[AddDevProInfoRequest]Pasre data pollInterval:")
		MBOX_DEBUG_PRINTLN(addDevProInfo.pollInterval)
		MBOX_DEBUG_PRINT("[AddDevProInfoRequest]Pasre data pollTimeout:")
		MBOX_DEBUG_PRINTLN(addDevProInfo.pollTimeout)
		MBOX_DEBUG_PRINT("[AddDevProInfoRequest]Pasre data reportTimeout:")
		MBOX_DEBUG_PRINTLN(addDevProInfo.reportTimeout)
		MBOX_DEBUG_PRINT("[AddDevProInfoRequest]Pasre data reportLimit:")
		MBOX_DEBUG_PRINTLN(addDevProInfo.reportLimit)
		return true;
	}
	
	return false;
}

/*
JsonObject& MQTTJsonFunc::AddDevProInfoResponse(int result){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
	
	root["result"] = result;
	
	return root;
}*/

bool  MQTTJsonFunc::DeleteDevProInfoRequest(char* json, DelDevProInfo &delDevProInfo){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);	
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[DeleteDevProInfoRequest] Json parse error.");
    	return false;
  	}
	
	const char* deviceId_ = root["pld"]["deviceId"];

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	if(deviceId_ != NULL){
		delDevProInfo.deviceId = deviceId_;

		MBOX_DEBUG_PRINT("[DeleteDevProInfoRequest]Pasre data deviceId:")
		MBOX_DEBUG_PRINTLN(delDevProInfo.deviceId)
		return true;
	}
	
	return false;
}

/*
JsonObject& MQTTJsonFunc::DeleteDevProInfoResponse(int result){

	StaticJsonBuffer<JSON_BUFF> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
	
	root["result"] = result;
	
	return root;
}*/

bool  MQTTJsonFunc::AddDataModelInfoRequest(char* json, AddDataModelInfo &addDatModInfo){
	int num = 0;
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
	JsonObject& root = jsonBuffer.parseObject(json);	
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[AddDataModelInfoRequest] Json parse error.");
    	return false;
  	}
	
	const char* deviceType_ = root["pld"]["deviceType"];
	if(deviceType_ == NULL){
		return false;
	}

	const char* protocolType_ = root["pld"]["protocol"];
	if(protocolType_ == NULL){
		return false;
	}

	bool swapFlag_ = root["pld"]["swap"];

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	addDatModInfo.deviceType = deviceType_;
	addDatModInfo.protocolType = protocolType_;
	addDatModInfo.swapFlag = swapFlag_;
	//MBOX_DEBUG_PRINT("[AddDataModelInfoRequest]Pasre data deviceType:")
	//MBOX_DEBUG_PRINTLN(addDatModInfo.deviceType)
	
	JsonArray& nestedArray = root["pld"]["info"];
	if(nestedArray.size() != 0){
		//MBOX_DEBUG_PRINTLN("[AddDataModelInfoRequest]array is not zero.")
		for(num = 0;num < nestedArray.size();num++){
			
			char *str;
			char *p;
			char buf[30] = {0};
			char index = 0;
			DataPoint dataModelInfo_;
			const char* tmpString = nestedArray[num];
			strncpy(buf, tmpString, strlen(tmpString));
			//MBOX_DEBUG_PRINTLN("data is:" + data)
			for(str = strtok_r(buf, ",", &p); str; str = strtok_r(NULL, ",", &p)){
				if(index == 0){
					dataModelInfo_.dataAddress = str;
				}
				else if(index == 1){
					dataModelInfo_.dataType = str;
				}
				else if(index == 2){
					dataModelInfo_.pollFrequency = String(str).toInt();
				}
				index++;
			}
			/*
			position = data.indexOf(',');
			//MBOX_DEBUG_PRINTLN("position is:" + String(position))
			if(position != -1)
			{
				dataModelInfo_.dataAddress = data.substring(0,position);
				dataModelInfo_.dataType = data.substring(position+1,data.length());
				//MBOX_DEBUG_PRINTLN("address is:" + dataModelInfo_.dataAddress)
				//MBOX_DEBUG_PRINTLN("type is:" + dataModelInfo_.dataType)
			}else
			{
				return false;
			}
			*/
			addDatModInfo.dataPoints.push_back(dataModelInfo_);
		}
		return true;
	}
	else{		
		return false;
	}	
}

bool  MQTTJsonFunc::DeleteDataModelInfoRequest(char* json,DelDataModelInfo &deldatmodinfo){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	//MBOX_DEBUG_PRINTLN(json);
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[DeleteDataModelInfoRequest] Json parse error.");
    	return false;
  	}
	
	const char* deviceType_ = root["pld"]["deviceType"];

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	if(deviceType_ != NULL){
		deldatmodinfo.deviceType = deviceType_;
		MBOX_DEBUG_PRINT("[DeleteDataModelInfoRequest]Pasre data deviceType:")
		MBOX_DEBUG_PRINTLN(deldatmodinfo.deviceType)
		return true;
	}
	
	return false;
}

bool  MQTTJsonFunc::GetDevConnectStatusRequest(char* json, DevConnectStatus &devconsta){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject& root = jsonBuffer.parseObject(json);
	int num = 0;
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[GetDevConnectStatusRequest] Json parse error.");
    	return false;
  	}
	
	const char* cRevMessageId = root["mid"];
    revMessageId_heartbeat = cRevMessageId;

	//mbox_dev_p3::DevProConfigManager devConfig;
	//devConfig.LoadConfig();
	
	if(deviceId.length() != 0 && deviceId != "null"){
		devconsta.deviceId = deviceId;
		MBOX_DEBUG_PRINT("[GetDevConnectStatusRequest]Pasre data deviceId:")
		MBOX_DEBUG_PRINTLN(devconsta.deviceId)
		return true;
	}
	return false;
}

bool  MQTTJsonFunc::OTAUpgradeRequest(char* json, OTAUpgrade &otaupgrade){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[OTAUpgradeRequest] Json parse error.");
    	return false;
  	}
	
	const char* swVersion_ = root["pld"]["swVersion"];
	const char* uri_ = root["pld"]["uri"];

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	if(swVersion_ != NULL && uri_ != NULL){
		otaupgrade.swVersion = swVersion_;
		otaupgrade.uri = uri_;
		MBOX_DEBUG_PRINT("[OTAUpgradeRequest]Pasre data swVersion:")
		MBOX_DEBUG_PRINTLN(otaupgrade.swVersion)
		MBOX_DEBUG_PRINT("[OTAUpgradeRequest]Pasre data uri:")
		MBOX_DEBUG_PRINTLN(otaupgrade.uri)
		return true;
	}
	return false;
}

bool  MQTTJsonFunc::RebootRequest(char* json){
	
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[RebootRequest] Json parse error.");
    	return false;
  	}

	const char* cRevMessageId = root["mid"];
    revMessageId = cRevMessageId;

	if(revMessageId.length() != 0) return true;
	else return false;
}

bool MQTTJsonFunc::SetRouteInfoRequest(char* json, RouteInfo &routeInfo){
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) {
		MBOX_DEBUG_PRINTLN("[SetRouteInfoRequest] Json parse error.");
    	return false;
  	}
	
	const char* cRevMessageId = root["mid"];
	revMessageId = cRevMessageId;
	const char* ssid = root["pld"]["ssid"];
	routeInfo.ssid = ssid;
	const char* bssid = root["pld"]["bssid"];
	routeInfo.bssid = bssid;
	//const char* password = root["pld"]["password"];
	if(routeInfo.ssid == WIFI_SSID_A){
		routeInfo.password = WIFI_PASSWORD_A;
	}else if(routeInfo.ssid == WIFI_SSID_B){
		routeInfo.password = WIFI_PASSWORD_B;
	}else if(routeInfo.ssid == WIFI_SSID_C){
		routeInfo.password = WIFI_PASSWORD_C;
	} else if(routeInfo.ssid == WIFI_SSID_D){
		routeInfo.password = WIFI_PASSWORD_D;
	}
	
	routeInfo.channel = root["pld"]["channel"];
	routeInfo.status = true;
	
	return true;
}

JsonObject& MQTTJsonFunc::ReportRouteInfoRequest(JsonObject &root, RouteInfo &routeInfo){
	char mac[18] = {0};
	uint8_t macAddr[6];
	JsonObject& payLoad = root.createNestedObject("pld");

	WiFi.macAddress(macAddr);
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3],macAddr[4], macAddr[5]);
	payLoad["ssid"] = routeInfo.ssid;
	payLoad["bssid"] = routeInfo.bssid;
	payLoad["channel"] =routeInfo.channel;
	//payLoad["rssi"] = routeInfo.rssi;
	payLoad["rssi"] = WiFi.RSSI();
	payLoad["status"] = routeInfo.status;
	payLoad["mac"] = String(mac);

	return payLoad;
}


