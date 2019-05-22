#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "AsyncMqttClient.h"
#include "mqtt_process.h"
#include "mqttjson.h"
#include "default_config.hpp"
#include "mbox_config_manager.h"
#include "trace.h"
#include "data_poll.h"
#include "ModbusRtu.h"
#include "WebConf.h"
#include "NTPClient.h"

#include "OTAUpgrade.h"

using mbox_dev_p3::MBoxConfigManager;
using mbox_dev_p3::DefaultConfig;
using mbox_dev_p3::DevProConfigManager;

#define WIFI_SSID "YYTD"
#define WIFI_PASSWORD "yytd#1234"
#define SC_CONNECT_TIMEOUT (20 * 1000)

MqttProc mqttProc;
DataPoll dataPoll;
String revMessageId;
String revMessageId_heartbeat;

bool wifiConnFlag = false;
bool wifiFirstConnFlag = false;
bool mqttFirstConFlag = true;
bool mqttConFlag = false;
bool rebootFlag = false;
bool syncTimeFlag = false;
bool singleWrite = true;
bool platformRegister = false;
bool dataSwapFlag = false;
bool rptRouteInfoFlag = true;
bool needReconnectWifi = false;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
Ticker rptDataTimer;
NTPClient NTPClientInstance("ntp1.aliyun.com", 6);

int pinInterrupt = 0;//flash button is gpio0
int recoveryCount = 0;
int upgradeCount = 0;
String upgradeUri = "";

int scConnectCount = -1;
unsigned long scConnectUpdateTime;

int rptRouteInfoCount = 0;
int rptDataCount = 0;
int rptHisDataCount = 0;
unsigned long rptDataTime = 0;
unsigned long rptHisDataTime = 0;
unsigned long rptRouteInfoTime = 0;
int rptDataMaxTimeout = 10 * 1000;  // 用于数据上报的超时时间, 默认 10 秒
int rptDataMaxCount = 3;  // 数据上报的最大超时次数, 默认 3 次

int pollDataInterval = 10 * 1000;  // 向下轮询数据的间隔, 默认是 10 秒
int pollDataTimeout = 2 * 1000;  // 向下采集数据超时时间

String mBoxId;
String deviceId;

void connectToWifi() {
  MBOX_DEBUG_PRINTLN("Connecting to Wi-Fi...");
  MBOX_DEBUG_PRINTLN("wifi station mode:"+ String(webConf.isSTAMode()) + " wifi connection flag:" + String(wifiConnFlag));
  if(webConf.isSTAMode() == true && wifiConnFlag == false)
  {
    //MBOX_DEBUG_PRINTLN("connectToWifi webConf.isSTAMode() == true");
  	WiFi.mode(WIFI_STA);
    if(webConf.getDHCPMode() == false)
    {
      WiFi.config(webConf.getStaticIP(), webConf.getGateWay(), webConf.getNetMask()); 
    }
    if(webConf.getAuotoScanMode() == false){
      WiFi.begin(webConf.getWifiSSID().c_str(), webConf.getWifiPassword().c_str());
    }else{
      RouteInfo routeInfo;
      uint32 bssid[6] = {0};
      uint8 mac[6];
      routeInfo = webConf.getRouteInfo();
      //strcpy(mac, routeInfo.bssid.c_str());
      //Serial1.printf("mac is:%s",mac);
      sscanf(routeInfo.bssid.c_str(),"%02x:%02x:%02x:%02x:%02x:%02x",&bssid[0],&bssid[1],&bssid[2],&bssid[3],&bssid[4],&bssid[5]);
      for(int i = 0; i < 6; i++)
        mac[i] = (uint8)bssid[i];
      WiFi.begin(routeInfo.ssid.c_str(), routeInfo.password.c_str(), routeInfo.channel, mac, true);
    }
  }
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  MBOX_DEBUG_PRINTLN("Connected to wifi:" + WiFi.SSID());
  MBOX_DEBUG_PRINT("IP address: ");
  MBOX_DEBUG_PRINTLN(WiFi.localIP());
  wifiConnFlag = true;
  needReconnectWifi = false;
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  MBOX_DEBUG_PRINTLN("Disconnected from Wi-Fi.");
  wifiConnFlag = false;
  //mqttProc.GetTicker().detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.attach(30, connectToWifi);
}

void connectToMqtt() {
  MBOX_DEBUG_PRINTLN("Connecting to MQTT...");
  mqttProc.GetMqttClient().connect();
}

void dataPollFunc(){
  bool changeStatus = dataPoll.GetDataChangeStatus();
  //debug start
  if(changeStatus)
  {
    MBOX_DEBUG_PRINTLN("rptDataTime:" + String(rptDataTime));
    MBOX_DEBUG_PRINTLN("scConnectCount:" + String(scConnectCount));
    MBOX_DEBUG_PRINTLN("rptDataCount:" + String(rptDataCount));
    MBOX_DEBUG_PRINTLN("RegisterStatus:" + String(mqttProc.GetRegisterStatus()));
    MBOX_DEBUG_PRINTLN("time:" + String(millis() - rptDataTime));
  }
  //end debug
  if(changeStatus && ((unsigned long)(millis() - rptDataTime) > rptDataMaxTimeout) && (mqttProc.GetRegisterStatus() == true) && 
     (rptDataCount < rptDataMaxCount)){

    MBOX_DEBUG_PRINTLN(" rptDataCount = " + String(rptDataCount));
    //trace.println("changeStatus = " + String(changeStatus));
    ModelData& varyData = dataPoll.GetVaryData();
    dataPoll.SortVaryData(varyData);
    varyData.timestamp = String(now()) + "000";
    mqttProc.ReportRealTimeData(varyData);
    rptDataCount++;
    rptDataTime = millis();
  }

  //检测到与平台断开口，在重新连接上之前要进行数据备份
  if((mqttProc.GetRegisterStatus() == false) && changeStatus)
  {
    MBOX_DEBUG_PRINTLN("backup data when device disconnect to platform");
    ModelData& varyData = dataPoll.GetVaryData();
    dataPoll.SortVaryData(varyData);
    varyData.timestamp = String(now()) + "000";
    dataPoll.storeResumeData(varyData);
    dataPoll.ClearVaryData();
    dataPoll.SetDataChangeStatus(false);
  }
}

void(* resetFunc) (void) = 0; //制造重启命令
time_t getNtpTime()
{
  uint64_t tmMs;
  tmMs  = NTPClientInstance.GetTimeStampUsec();
  return (tmMs / 1000);
}

void doRecovery(){
  int i;
  SPIFFS.format();

  EEPROM.begin(4096);
  for(i = 5; i < 4096; i++){
    EEPROM.write(i,0xFF);
  }
  EEPROM.commit();

  delay(200);
  ESP.restart();
}

void setup() {
  Serial1.begin(38400);
  
  MBOX_DEBUG_PRINTLN("Mbox start......");
  MBOX_DEBUG_PRINTLN();
  MBOX_DEBUG_PRINTLN();

  MBOX_DEBUG_PRINTLN("Start Left heap size: " + String(ESP.getFreeHeap()))
  DefaultConfig defaultConfig;
  MBoxConfigManager mboxConfigManager;
  
  if(!SPIFFS.begin()){
      MBOX_DEBUG_PRINTLN("An Error has occurred while mounting SPIFFS");
      return;
  }
  uint8_t boot_count;
  EEPROM.begin(4);//SPIFFS后面4byte
  boot_count = EEPROM.read(1);
  MBOX_DEBUG_PRINTLN(" boot_count = " + String(boot_count));
  EEPROM.write(1,0);
  EEPROM.commit();
  mBoxId = "mbox_" + String(ESP.getChipId(), HEX);
  MBOX_DEBUG_PRINTLN(" mBoxId =  " + mBoxId);
  
  mboxConfigManager.SetConfigFilePath(defaultConfig.GetConfigFilePath());
  if(!mboxConfigManager.ConfigFileExists()) {
      MBOX_DEBUG_PRINTLN("config file is not exitsts");
      mboxConfigManager.CreateDefaultConfig();
  }
  else{
      MBOX_DEBUG_PRINTLN("config file is exitsts");
  }
    
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  mqttProc.Init();

  webConf.init();

  //Serial1.println("");
  //Serial1.println("WiFi connected");
  //Serial1.println("IP address: ");
  //Serial1.println(WiFi.localIP());

  delay(1000);
  dataPoll.SetModbusParam(webConf.getSerialSpeed(), 2000, webConf.getSerialConfig());
  delay(10);
  
  NTPClientInstance.begin();
  
  Serial1.begin(38400);

  DevProConfigManager initDevConfig;
  initDevConfig.LoadConfig();
  deviceId = initDevConfig.GetDeviceID();
  MBOX_DEBUG_PRINTLN(" deviceId =  " + deviceId);
  
  String protocolType = initDevConfig.GetProtocolType();
  if(protocolType.equals("MODBUS-RTU"))
    singleWrite = true;
  else if(protocolType.equals("MODBUS-RTU-SMGL3"))
    singleWrite = false;
  //singleWrite = false; //for test
  MBOX_DEBUG_PRINTLN(" singleWrite =  " + String(singleWrite));

  dataSwapFlag = initDevConfig.GetSwapFlag();
  MBOX_DEBUG_PRINTLN(" dataSwapFlag =  " + String(dataSwapFlag));
  
  uint32_t initRptTimeout = initDevConfig.GetReportTimeout(initDevConfig.GetDeviceType());
  MBOX_DEBUG_PRINTLN(" initRptTimeout =  " + String(initRptTimeout));
  if(initRptTimeout > 0)
  	rptDataMaxTimeout = initRptTimeout;

  uint8_t initRptDataCount = initDevConfig.GetReportLimit(initDevConfig.GetDeviceType());
  MBOX_DEBUG_PRINTLN(" initRptDataCount =  " + String(initRptDataCount));
  if(initRptDataCount > 0)
  	rptDataMaxCount = initRptDataCount;

  uint32_t initpollDataInterval = initDevConfig.GetPollInterval(initDevConfig.GetDeviceType());
  MBOX_DEBUG_PRINTLN(" initpollDataInterval =  " + String(initpollDataInterval));
  if(initpollDataInterval > 0)
  	pollDataInterval = initpollDataInterval;

  uint32_t initpollDataTimeout = initDevConfig.GetPollTimeout(initDevConfig.GetDeviceType());
  MBOX_DEBUG_PRINTLN(" initpollDataTimeout =  " + String(initpollDataTimeout));
  if(initpollDataTimeout > 0)
  	pollDataTimeout = initpollDataTimeout;

  rptDataTimer.attach_ms(pollDataInterval, dataPollFunc);

  setSyncProvider(getNtpTime);
  setSyncInterval(3600);
  MBOX_DEBUG_PRINT("Current time test:")
  MBOX_DEBUG_PRINTLN(getNtpTime())

  dataPoll.openResumeMark();
  //MBOX_DEBUG_PRINTLN(" Dir =  " + String(dataPoll.openResumeMark()));
  list<DataPoint> mDataPoint = initDevConfig.GetDeviceModel(initDevConfig.GetDeviceType());
  dataPoll.SetDataModelInfo(mDataPoint);
  dataPoll.SetModbusDataTimeout(pollDataTimeout);
  MBOX_DEBUG_PRINTLN("Left heap size after setup: " + String(ESP.getFreeHeap()))
}

void loop() {
  //trace.task();
  //yield();  

  webConf.doLoop();
  
  if(rebootFlag){
    MBOX_DEBUG_PRINTLN("M-box will reboot after 3 seconds...");
    delay(3000);
    ESP.restart();
  }

  if((wifiConnFlag == true) && (syncTimeFlag == false) && (getNtpTime() != 0))
  {
    setTime(getNtpTime());
    syncTimeFlag = true;
  }

  if((wifiConnFlag == true) && (mqttFirstConFlag == true) && (mqttConFlag == false)) {
    mqttFirstConFlag = false;
    MBOX_DEBUG_PRINTLN("start to connect mqtt. ");
    mqttProc.SetBrokerInfo(webConf.getBrokerIp(), webConf.getBrokerPort());
    connectToMqtt();
  }
  
  if(webConf.isSTAMode() == true && wifiFirstConnFlag == false) 
  {
    wifiFirstConnFlag = true;
    MBOX_DEBUG_PRINTLN(" webConf.isSTAMode() == true  connectToWifi()");
    webConf.findBestRoute();
    connectToWifi();
  }
  else if(webConf.isSTAMode() == true && wifiConnFlag == true) 
  {
    //如果是重连wifi，当连接成功后取消wifi连接ticker
    if(wifiReconnectTimer.active())
    {
      MBOX_DEBUG_PRINTLN("wifi reconnected, detach wifi reconnect timer");
      wifiReconnectTimer.detach();
    }

    //上报route info
    if(((unsigned long)(millis() - rptRouteInfoTime) > 10 * 1000) && rptRouteInfoFlag && (mqttConFlag == true) && (rptRouteInfoCount < 10)){
      mqttProc.ReportRouteInfo(webConf.getRouteInfo());
      rptRouteInfoCount++;
      rptRouteInfoTime = millis();
    }

    //上报route info超过10次 清除上报route info标记 不再上报
    if(rptRouteInfoCount >= 10){
      rptRouteInfoFlag = false;
    }
      
    //wifi连接成功 mqtt连接成功后开始向平台注册
    if((mqttProc.GetRegisterStatus() == false) && ((unsigned long)(millis() - mqttProc.GetRegisterTime()) > 10 * 1000) && (mqttConFlag == true) && (rptRouteInfoFlag == false)) 
    {
      MBOX_DEBUG_PRINTLN("register device to platform...");
      mqttProc.SetRegisterTime(millis());
      mqttProc.SetRegisterCount(mqttProc.GetRegisterCount() + 1);
      RegistDetail registdetail;
      registdetail.swVersion = SW_VERSION;
      registdetail.hwVersion = HW_VERSION;
      mqttProc.RegisterReport(registdetail);
    }
    
    //平台注册成功
    if(mqttProc.GetRegisterStatus()){
      //检测平台心跳是否超时
      if((unsigned long)(millis() - scConnectUpdateTime) > SC_CONNECT_TIMEOUT) {
        scConnectCount++;
        scConnectUpdateTime = millis();
        if(scConnectCount > 5)// 平台断联超时计数超过 5 次进行重新注册 
        {
          MBOX_DEBUG_PRINTLN(" scConnectCount > 5 ");
          mqttProc.SetRegisterStatus(false); 
        }
      }
      
      //当重发 3 次数据上报，进行重新注册业务
      if((rptDataCount >= rptDataMaxCount) || (rptHisDataCount >= rptDataMaxCount)) 
      {
        mqttProc.SetRegisterStatus(false); 
      }

      //当程序已经成功注册上平台，则开始上报恢复数据
      if((mqttProc.GetRegisterStatus() == true) && dataPoll.isExistResumeData() && (rptHisDataCount < rptDataMaxCount) && ((unsigned long)(millis() - rptHisDataTime) > rptDataMaxTimeout))
      {
        list<ModelData> varyDataList;
        bool result = dataPoll.getResumeData(varyDataList);
        if(result)
        {
          mqttProc.ReportResumeData(varyDataList);
        }
        rptHisDataCount++;
        rptHisDataTime = millis();
      }

      //升级检测
      if((upgradeUri.length()) > 0 && (upgradeCount < 5))
      {
        delay(1000);
        OTAUpgradeTmp OTAUpgrade_tmp(upgradeUri);
        upgradeCount++;
      }
      else
      {
        upgradeUri = "";
        upgradeCount = 0;
      } 

      //平台注册上后开始采集数据
      if(syncTimeFlag == true){
        dataPoll.PollTask();
      }
      
    }else{
      /*
      if(((unsigned long)(millis() - mqttProc.GetRegisterTime()) > 10 * 1000) && (mqttConFlag == true) && (rptRouteInfoFlag == false)){
        MBOX_DEBUG_PRINTLN("register device to platform...");
        mqttProc.SetRegisterTime(millis());
        mqttProc.SetRegisterCount(mqttProc.GetRegisterCount() + 1);
        RegistDetail registdetail;
        registdetail.swVersion = SW_VERSION;
        registdetail.hwVersion = HW_VERSION;
        mqttProc.RegisterReport(registdetail);
      }*/
      //注册平台超过10次,重启设备
      if(mqttProc.GetRegisterCount() > 10)
      {
        MBOX_DEBUG_PRINTLN("Disconnect to platform and register over 10 times,M-box restart.");
        ESP.restart();
      }
      //平台注册成功后再断开需要保存历史数据
      if((syncTimeFlag == true) && (platformRegister == true)){
        dataPoll.PollTask();
      }
    }
  }

  while(digitalRead(pinInterrupt) == LOW){
    delay(200);
    recoveryCount++;
    if(recoveryCount >=15){
      MBOX_DEBUG_PRINTLN("Recovery key pressed!")
      doRecovery();
      //break;
    }
  }
  recoveryCount = 0;
  
}
