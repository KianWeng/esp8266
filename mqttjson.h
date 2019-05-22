#ifndef _MQTTJSON_H
#define _MQTTJSON_H

#include <ArduinoJson.h>
#include <list>
#include "common.h"

using namespace std;

class MQTTJsonFunc {
public:
    MQTTJsonFunc();
    ~MQTTJsonFunc();

	template <typename T>
	bool Response(char *json, T &data){
		StaticJsonBuffer<JSON_BUFF_1K> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(json);
		if (!root.success()) {
			MBOX_DEBUG_PRINTLN("[RegisterResponse] Json parse error.");
    		return false;
  		}
	
		data.result = root["pld"]["result"];

		return true;
	}
	
	void Response(JsonObject& root, int result);
	void RegisterRequest(JsonObject& root, RegistDetail &registdetail);
	bool  SetDevDataRequest(char *json, SetDevData &setdevdata);	
	bool  AddDevProInfoRequest(char *json, AddDevProInfo &adddevproinfo);
	bool  DeleteDevProInfoRequest(char* json, DelDevProInfo &deldevproinfo);
	bool  AddDataModelInfoRequest(char* json, AddDataModelInfo &adddatmodinfo);
	bool  DeleteDataModelInfoRequest(char* json, DelDataModelInfo &deldatmodinfo);
	bool  GetDevConnectStatusRequest(char* json, DevConnectStatus &devconsta);
	bool  OTAUpgradeRequest(char* json, OTAUpgrade &otaupgrade);
	bool  RebootRequest(char* json);	
	bool  SetRouteInfoRequest(char* json, RouteInfo &routeInfo);
	JsonObject& ReportRealTimeDataRequest(JsonObject &root, ReportDataInfo &reportdatainfo);
	JsonObject& ReportRouteInfoRequest(JsonObject &root, RouteInfo &routeInfo);
};

#endif
