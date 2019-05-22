#include "mqtt_process.h"
#include "AsyncMqttClient.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include "WebConf.h"
#include "data_poll.h"
#include "WebConf.h"

extern String revMessageId;
extern String revMessageId_heartbeat;
extern int scConnectCount;
extern unsigned long scConnectUpdateTime;
extern DataPoll dataPoll;

extern int rptDataCount;
extern unsigned long rptDataTime;
extern int rptHisDataCount;
extern unsigned long rptHisDataTime;
extern String mBoxId;
extern String deviceId;
extern bool rebootFlag;
extern bool rptHisDataFlag;
extern bool mqttConFlag;
extern bool platformRegister;
extern bool rptRouteInfoFlag;
extern int rptRouteInfoCount;
extern bool needReconnectWifi;

static char *recvPayload = NULL;
//char mqttServerName[60];

MqttProc::MqttProc()
{
    MqttRegisterControl_.continueRegister = false;//false need to register to platform, true already register to platform
    MqttRegisterControl_.time = 0;
	MqttRegisterControl_.registerCount = 0;
}

void MqttProc::Init() {
    mqttClient_.onConnect(std::bind(&MqttProc::onMqttConnect, this, std::placeholders::_1));
    mqttClient_.onDisconnect(std::bind(&MqttProc::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient_.onSubscribe(std::bind(&MqttProc::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));
    mqttClient_.onUnsubscribe(std::bind(&MqttProc::onMqttUnsubscribe, this, std::placeholders::_1));
    mqttClient_.onMessage(std::bind(&MqttProc::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, 
                          std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    mqttClient_.onPublish(std::bind(&MqttProc::onMqttPublish, this, std::placeholders::_1));
	
}

AsyncMqttClient& MqttProc::GetMqttClient() {
    return mqttClient_;
}

//Ticker& MqttProc::GetTicker() {
//    return mqttReconnectTimer_;
//}

void MqttProc::onMqttConnect(bool sessionPresent){
    MBOX_DEBUG_PRINTLN("[onMqttConnect]Connected to MQTT.");
    MBOX_DEBUG_PRINT("[onMqttConnect]Session present: ");
    MBOX_DEBUG_PRINTLN(sessionPresent);
    uint16_t packetIdSub = mqttClient_.subscribe("test/lol", 2);
    MBOX_DEBUG_PRINT("[onMqttConnect]Subscribing at QoS 2, packetId: ");
    MBOX_DEBUG_PRINTLN(packetIdSub);

    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/RegisterAck").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/ReportRtDataAck").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/ReportHistDataAck").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/SetDeviceData").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/AddDeviceProperty").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/DeleteDeviceProperty").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/AddDataModel").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/DeleteDataModel").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/DeviceConnectionStatus").c_str(), 2);
    mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/OTA").c_str(), 2);
	mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/Reboot").c_str(), 2);
	mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/SetRouteInfo").c_str(), 2);
	mqttClient_.subscribe(("MCloud_product_001/" + mBoxId + "/ReportRouteInfoAck").c_str(), 2);
	
    // mqttClient_.publish("test/lol", 0, true, "test 1");
    // Serial.println("Publishing at QoS 0");

    // uint16_t packetIdPub1 = mqttClient_.publish("test/lol", 1, true, "test 2");
    // Serial.print("Publishing at QoS 1, packetId: ");
    // Serial.println(packetIdPub1);

    uint16_t packetIdPub2 = mqttClient_.publish("test/lol", 2, true, "test 3");
    MBOX_DEBUG_PRINT("[onMqttConnect]Publishing at QoS 2, packetId: ");
    MBOX_DEBUG_PRINTLN(packetIdPub2);
	mqttConFlag = true;
}

void MqttProc::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  MBOX_DEBUG_PRINTLN("Disconnected from MQTT.");
  mqttConFlag = false;
  if (WiFi.isConnected())
  {
  	  SetBrokerInfo(webConf.getBrokerIp(), webConf.getBrokerPort());
      mqttClient_.connect();
  }
}

void MqttProc::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  MBOX_DEBUG_PRINTLN("Subscribe acknowledged.");
  MBOX_DEBUG_PRINT("  packetId: ");
  MBOX_DEBUG_PRINTLN(packetId);
  MBOX_DEBUG_PRINT("  qos: ");
  MBOX_DEBUG_PRINTLN(qos);
  //SetRegisterStatus(true);
}

void MqttProc::onMqttUnsubscribe(uint16_t packetId) {
  MBOX_DEBUG_PRINTLN("Unsubscribe acknowledged.");
  MBOX_DEBUG_PRINT("  packetId: ");
  MBOX_DEBUG_PRINTLN(packetId);
}

// 最多收 1460 bytes, 如果数据大了是调用多次 onMqttMessage
void MqttProc::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //char recvPayload[1024] = {0};
  
  MBOX_DEBUG_PRINTLN("Publish received.");
  MBOX_DEBUG_PRINT("  topic: ");
  MBOX_DEBUG_PRINTLN(topic);
  MBOX_DEBUG_PRINT("  qos: ");
  MBOX_DEBUG_PRINTLN(properties.qos);
  MBOX_DEBUG_PRINT("  dup: ");
  MBOX_DEBUG_PRINTLN(properties.dup);
  MBOX_DEBUG_PRINT("  retain: ");
  MBOX_DEBUG_PRINTLN(properties.retain);
  MBOX_DEBUG_PRINT("  len: ");
  MBOX_DEBUG_PRINTLN(len);
  MBOX_DEBUG_PRINT("  index: ");
  MBOX_DEBUG_PRINTLN(index);
  MBOX_DEBUG_PRINT("  total: ");
  MBOX_DEBUG_PRINTLN(total);

  //strncpy(recvPayload, payload, len);
  //recvPayload[len] = '\0';
  //MBOX_DEBUG_PRINTLN(recvPayload);
  MBOX_DEBUG_PRINTLN(topic);

  //如何当前设备可用内存低于4K 此次业务不做处理
  if(ESP.getFreeHeap() < 4098)
  {
  	if(recvPayload != NULL){
		free(recvPayload);
		recvPayload = NULL;
  	}
	return;
  }

  
  if(len == total) {  // 一个数据包就可以包含完整的信息, 
  	recvPayload = (char *)malloc(total * sizeof(char));
 	memset(recvPayload, 0, total);
  	strncpy(recvPayload, payload, total);
  }
  else if(index == 0) {  // 一个数据包无法包含完整的信息但是是第一个数据包需要分配内存
  	recvPayload = (char *)malloc(total * sizeof(char));
    memset(recvPayload, 0, total);
  	strncpy(recvPayload, payload, len);
  	return;	
  } 
  else if(index + len != total) { // 中间的数据包，只需要拷贝
  	strncpy(recvPayload + index, payload, len);
  	return;
  }
  else { // 最后一个数据包
  	strncpy(recvPayload + index, payload, len);
  }
  
  if(strstr(topic, "RegisterAck") != NULL) {
  	RegistDetail registDetail;
    bool result = mqttContent_.ReportData(recvPayload, registDetail);
    if(result){
		MBOX_DEBUG_PRINTLN("Register platform success.");
		SetRegisterStatus(true); 
		SetRegisterCount(0);
		rptDataCount = 0;//注册成功后将数据上报次数清零
		rptHisDataCount = 0;//注册成功后将历史数据上报次数清零
		scConnectCount = 0;
		scConnectUpdateTime = millis();
		platformRegister = true;
    }
  }
  else if(strstr(topic, "ReportRtDataAck") != NULL){
  	ReportDataInfo reportDataInfo;
  	bool result = mqttContent_.ReportData(recvPayload, reportDataInfo);
  	MBOX_DEBUG_PRINT("ReportRtDataAck result ");
	MBOX_DEBUG_PRINTLN(result)
	dataPoll.ClearVaryData();
    dataPoll.SetDataChangeStatus(false);
	rptDataCount = 0;
	rptDataTime = 0;  // 下一次数据变化马上上报
  }
  else if(strstr(topic, "ReportHistDataAck") != NULL){
  	ReportDataInfo reportDataInfo;
	bool result = mqttContent_.ReportData(recvPayload, reportDataInfo);
    if(result){
  		MBOX_DEBUG_PRINT("ReportHistDataAck result");
		MBOX_DEBUG_PRINTLN(result);
		dataPoll.removeResumeData();
		rptHisDataCount = 0;
		rptHisDataTime = 0;
    }
  }
  else if(strstr(topic, "SetDeviceData") != NULL){
  	//bool result;
	//char* response = (char *)malloc(JSON_BUFF_CREATE);
	
  	mqttContent_.SetDeviceData(recvPayload);

	//if(result == false){
	//	CreateResponseJson(response, JSON_BUFF_CREATE, result);
	//	mqttClient_.publish((mBoxId + "/MCloud_product_001/SetDeviceDataAck").c_str(), 2, false, response);
	//	free(response);
	//}
  }
  else if(strstr(topic, "AddDeviceProperty") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);
	
  	result = mqttContent_.AddDeviceProperty(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/AddDevicePropertyAck").c_str(), 2, false, response);
	free(response);
  }
  else if(strstr(topic, "DeleteDeviceProperty") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);
	
  	result = mqttContent_.DeleteDeviceProperty(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/DeleteDevicePropertyAck").c_str(), 2, false, response);
	free(response);
  }
  else if(strstr(topic, "AddDataModel") != NULL){
  	bool result;
	
  	result = mqttContent_.AddDataModel(recvPayload);
  	char* response = (char *)malloc(JSON_BUFF_1K);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/AddDataModelAck").c_str(), 2, false, response);
	free(response);

  }
  else if(strstr(topic, "DeleteDataModel") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);
	
  	result = mqttContent_.DeleteDataModel(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/DeleteDataModelAck").c_str(), 2, false, response);
	free(response);
  }
  else if(strstr(topic, "DeviceConnectionStatus") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);
	DevConnectStatus devConnectStatus;
	
  	result = mqttContent_.DeviceConnectionStatus(recvPayload, devConnectStatus);
	CreateConnectionStatusResponse(response, JSON_BUFF_1K, devConnectStatus);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/DeviceConnectionStatusAck").c_str(), 2, false, response);
	free(response);
	scConnectCount = 0;
	scConnectUpdateTime = millis();
  }
  else if(strstr(topic, "OTA") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);

  	result = mqttContent_.OTA(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/OTAAck").c_str(), 2, false, response);
	free(response);
  }
  else if(strstr(topic, "Reboot") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);

	result = mqttContent_.Reboot(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/RebootAck").c_str(), 2, false, response);
	free(response);

	if(result){
		rebootFlag = true;
	}
  }
  else if(strstr(topic, "SetRouteInfo") != NULL){
  	bool result;
	char* response = (char *)malloc(JSON_BUFF_1K);

	result = mqttContent_.SetRouteInfo(recvPayload);
	CreateResponseJson(response, JSON_BUFF_1K, result);
	//MBOX_DEBUG_PRINTLN("start send set route info ack");
	mqttClient_.publish((mBoxId + "/MCloud_product_001/SetRouteInfoAck").c_str(), 2, false, response);
	//MBOX_DEBUG_PRINTLN("end send set route info ack");
	free(response);
	//delay(500);
	//WiFi.disconnect();//断开wifi重新连接
	SetRegisterStatus(false); //需要重新注册平台
	SetRegisterCount(0);
	mqttConFlag = false;//需要重新注册mqtt
	rptRouteInfoFlag = true; //需要重新上报route info
	rptRouteInfoCount = 0;
	needReconnectWifi = true;
  }
  else if(strstr(topic, "ReportRouteInfoAck") != NULL){
  	RouteInfo routeInfo;
  	bool result = mqttContent_.ReportData(recvPayload, routeInfo);
  	MBOX_DEBUG_PRINT("SendRouteInfoAck result ");
	MBOX_DEBUG_PRINTLN(result);
	rptRouteInfoFlag = false;
  }

  free(recvPayload);
  recvPayload = NULL;
  MBOX_DEBUG_PRINTLN("Left heap size: " + String(ESP.getFreeHeap()))
  
}

void MqttProc::onMqttPublish(uint16_t packetId) {
  MBOX_DEBUG_PRINTLN("Publish acknowledged.");
  MBOX_DEBUG_PRINT("  packetId: ");
  MBOX_DEBUG_PRINTLN(packetId);
  if(needReconnectWifi == true)
  	WiFi.disconnect();//断开wifi重新连接
}

bool MqttProc::GetRegisterStatus() {
  return MqttRegisterControl_.continueRegister;
}

bool MqttProc::SetRegisterStatus(bool status) {
  MqttRegisterControl_.continueRegister = status;
}

unsigned long MqttProc::GetRegisterTime() {
  return MqttRegisterControl_.time;
}

bool MqttProc::SetRegisterTime(unsigned long time) {
  MqttRegisterControl_.time = time;
  return true;
}

uint32_t MqttProc::GetRegisterCount() {
	return MqttRegisterControl_.registerCount;
}
bool MqttProc::SetRegisterCount(uint32_t count) {
 	MqttRegisterControl_.registerCount = count;
    return true;
}
	
void MqttProc::RegisterReport(RegistDetail &registdetail) {
    String reportContent = mqttContent_.RegisterContent(registdetail);
	mqttClient_.publish((mBoxId + "/MCloud_product_001/Register").c_str(), 2, false, reportContent.c_str());
}

void MqttProc::ReportRealTimeData(ModelData& modeData){
	ReportDataInfo reportdatainfo;
	DevData devData;

	devData.timestamp = modeData.timestamp;
	devData.datainfo = modeData.dataList;
	
	reportdatainfo.deviceId = deviceId;
	reportdatainfo.devdata.push_back(devData);
	String response = mqttContent_.ReportRealTimeData(reportdatainfo);
	if(response.length() != 0){
		//MBOX_DEBUG_PRINTLN(response);
		mqttClient_.publish((mBoxId + "/MCloud_product_001/ReportRtData").c_str(), 2, false, response.c_str());
	}
}

void MqttProc::SetBrokerInfo(const String &brokerIp, unsigned long brokerPort) {
	int count = 0;
	IPAddress mqttAddr;
	
	MBOX_DEBUG_PRINTLN(" MqttProc::SetBrokerInfo() brokerIp = " + String(brokerIp));
    MBOX_DEBUG_PRINTLN(" MqttProc::SetBrokerInfo() brokerPort = " + String(brokerPort));

	if(mqttAddr.isValid(brokerIp)){
		mqttAddr.fromString(brokerIp);
	}else{
		MBOX_DEBUG_PRINTLN(" MqttProc::SetBrokerInfo() user host to connect to mqtt broker.");
		//strcpy(mqttServerName, brokerIp.c_str());
		//MBOX_DEBUG_PRINTLN("serber name:" + mqttServerName)
		//int ret = WiFi.hostByName(brokerIp.c_str(), mqttAddr);
		while((WiFi.hostByName(brokerIp.c_str(), mqttAddr) != 1) && (count <= 5)){
			count++;
			delay(1000);
		}
	}
	MBOX_DEBUG_PRINTLN(" MqttProc::SetBrokerInfo() serverIp = " + mqttAddr.toString());
	mqttClient_.setServer(mqttAddr, brokerPort);
	//mqttClient_.setServer(MQTT_HOST, MQTT_PORT);
}

void MqttProc::ReportResumeData(list<ModelData> &varyDataList) {
	ReportDataInfo reportdatainfo;
	//DevProConfigManager devConfig;
	//devConfig.LoadConfig();
	
	reportdatainfo.deviceId = deviceId;
	
	list<ModelData>::iterator iter = varyDataList.begin();
  	for(; iter != varyDataList.end(); iter++) {
		DevData devData;
		devData.timestamp = iter->timestamp;
		devData.datainfo = iter->dataList;
		reportdatainfo.devdata.push_back(devData);
	}
	
	
	String response = mqttContent_.ReportHistoryData(reportdatainfo);
	if(response.length() != 0){
		MBOX_DEBUG_PRINTLN(response);
		mqttClient_.publish((mBoxId + "/MCloud_product_001/ReportHistData").c_str(), 2, false, response.c_str());
	}
}

void MqttProc::CreateResponseJson(char* json, int size, char result){
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	
	JsonObject& root = jsonBuffer.createObject();
	root["mid"] = revMessageId;
	root["version"] = CO_VERSION;
	JsonObject& payLoad = root.createNestedObject("pld");
	payLoad["result"] = result;
	root.printTo(json, size);
}

void MqttProc::CreateConnectionStatusResponse(char* json, int size, DevConnectStatus &devConnectStatus){
	DynamicJsonBuffer jsonBuffer(JSON_BUFF_1K);
	
	JsonObject& root = jsonBuffer.createObject();
	root["mid"] = revMessageId_heartbeat;
	root["version"] = CO_VERSION;
	JsonObject& payLoad = root.createNestedObject("pld");
	JsonArray &array = payLoad.createNestedArray("deviceIds");
	if(devConnectStatus.deviceId.length() > 0) {
		JsonObject& data_info = array.createNestedObject();
		data_info["deviceId"] = devConnectStatus.deviceId;
		data_info["result"] = devConnectStatus.status;
	}

	root.printTo(json, size);
}

void MqttProc::ReportRouteInfo(RouteInfo routeInfo){
	String response = mqttContent_.ReportRouteInfo(routeInfo);
	if(response.length() != 0){
		MBOX_DEBUG_PRINTLN(response);
		mqttClient_.publish((mBoxId + "/MCloud_product_001/ReportRouteInfo").c_str(), 2, false, response.c_str());
	}
}