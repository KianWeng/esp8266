#ifndef MQTT_PROCESS
#define MQTT_PROCESS

#include <memory>
#include "AsyncMqttClient.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include "mqtt_content.h"
#include "mbox_config_manager.h"

//#define MQTT_HOST IPAddress(172, 16, 5, 205) //192.168.0.3
#define MQTT_HOST IPAddress(192, 168, 0, 3) //192.168.0.3

#define MQTT_PORT 1883

using namespace std;
using mbox_dev_p3::BrokerProConfigManager;

struct MqttRegisterControl {
  bool continueRegister;
  unsigned long time;
  uint32_t registerCount;
};
class MqttProc {
public:
    MqttProc();
    void Init();
    AsyncMqttClient& GetMqttClient();
    //Ticker& GetTicker();
    void connectToMqtt();
	bool SetRegisterStatus(bool status);
	bool GetRegisterStatus();
	unsigned long GetRegisterTime();
	bool SetRegisterTime(unsigned long time);
	uint32_t GetRegisterCount();
	bool SetRegisterCount(uint32_t count);
	void RegisterReport(RegistDetail &registdetail);
	void ReportRealTimeData(ModelData& modeData);
	void ReportResumeData(list<ModelData> &varyDataList);
	void SetBrokerInfo(const String &brokerIp, unsigned long brokerPort);
	void CreateResponseJson(char* json, int size, char result);
	void ReportRouteInfo(RouteInfo routeInfo);

private:
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, 
                       size_t len, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);
	void CreateConnectionStatusResponse(char* json, int size, DevConnectStatus &devConnectStatus);

    AsyncMqttClient mqttClient_;
    //Ticker mqttReconnectTimer_;
    BrokerProConfigManager brokerConfig_;
    MQTTContentFunc mqttContent_;
    struct MqttRegisterControl MqttRegisterControl_;
};
#endif 
