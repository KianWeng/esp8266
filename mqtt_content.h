#ifndef MQTT_CONTENT
#define MQTT_CONTENT

#include "common.h"
#include "ArduinoJson.h"
#include "mqttjson.h"
#include "mbox_config_manager.h"

using mbox_dev_p3::DevProConfigManager;

class MQTTContentFunc {
public:
    MQTTContentFunc();
    ~MQTTContentFunc();

	template <typename T> 
	bool ReportData(char *json, T &data){
		mqttJsonFunc_.Response(json, data);
		return data.result == 1;
	}

	bool RegisterResult(char *json);	
	String RegisterContent(RegistDetail &registdetail);
	String ReportRealTimeData(ReportDataInfo &reportdatainfo);
	String ReportHistoryData(ReportDataInfo &reportdatainfo);
	String ReportRouteInfo(RouteInfo &routeInfo);
	bool SetDeviceData(char *json);
	bool AddDeviceProperty(char *json);
	bool DeleteDeviceProperty(char *json);
	bool AddDataModel(char *json);
	bool DeleteDataModel(char *json);
	bool DeviceConnectionStatus(char *json, DevConnectStatus &devConnectStatus);
	bool OTA(char *json);
	bool Reboot(char *json);
	bool SetRouteInfo(char *json);
	
private:
	MQTTJsonFunc mqttJsonFunc_;
	String revId;
	String version;
};

#endif 



