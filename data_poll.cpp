#include <ArduinoJson.h>
#include <TimeLib.h>
#include "Arduino.h"
#include "FS.h"
#include "data_poll.h"
#include "NTPClient.h"
#include "config_data_access.h"
#include "default_config.hpp"
#include "mqtt_process.h"

using mbox_dev_p3::ConfigDataAccess;

extern int pollDataInterval;
extern MqttProc mqttProc;
extern String mBoxId;
extern bool singleWrite;
extern bool dataSwapFlag;

void DataPoll::SendSetDataResult(char result){
	char* response = (char *)malloc(JSON_BUFF_1K);
	mqttProc.CreateResponseJson(response, JSON_BUFF_1K, result);
	mqttProc.GetMqttClient().publish((mBoxId + "/MCloud_product_001/SetDeviceDataAck").c_str(), 2, false, response);
	free(response);
}

DataPoll::DataPoll():dataChange(false), devPollWait(0),taskControlStatus(0), iterNum(0), setDataNum(0), devConnStatus(0)
{ 
  master = new Modbus(0, 0, 2);
}

 DataPoll::~DataPoll()
{
  delete master;
}

uint16_t DataPoll::GetQueryRegNum(String &dataType)
{
    uint16_t regNum = 0;
    if(dataType.startsWith("U16") || dataType.startsWith("S16"))
      regNum = 1;
    else if(dataType.startsWith("U32") || dataType.startsWith("S32"))
      regNum = 2;

    return regNum;
}

uint32_t DataPoll::GetQueryValue(String &dataType)
{
  uint32_t value = 0;
  if(dataType.startsWith("U16") || dataType.startsWith("S16"))
      value = u16Data[0];
  else if(dataType.startsWith("U32") || dataType.startsWith("S32")) {
  	  if(dataSwapFlag){
	      uint8_t buf0 = u16Data[0] & 0xFF;
	      uint8_t buf1 = (u16Data[0] >> 8) & 0xFF;
	      uint8_t buf2 = u16Data[1] & 0xFF;
	      uint8_t buf3 = (u16Data[1] >> 8) & 0xFF;
	      value = buf2 & 0xff;
	      value |= (buf3 << 8) & 0xff00;
	      value |= (buf0 << 16) & 0xff0000;
	      value |= (buf1 << 24) & 0xff000000;
  	  }
	  else{
	      uint8_t buf0 = u16Data[0] & 0xFF;
	      uint8_t buf1 = (u16Data[0] >> 8) & 0xFF;
	      uint8_t buf2 = u16Data[1] & 0xFF;
	      uint8_t buf3 = (u16Data[1] >> 8) & 0xFF;
	      value = buf0 & 0xff;
	      value |= (buf1 << 8) & 0xff00;
	      value |= (buf2 << 16) & 0xff0000;
	      value |= (buf3 << 24) & 0xff000000;
	  }
    }

  memset(u16Data, 0, sizeof(uint16_t)*16);
  return value;
}

uint32_t DataPoll::GetWriteValue(String &dataType)
{
  uint32_t value = 0;
  if(dataType.startsWith("U16"))
      value = u16WriteData[0];
  else if(dataType.startsWith("U32")) {
      uint8_t buf0 = u16WriteData[0] & 0xFF;
      uint8_t buf1 = (u16WriteData[0] >> 8) & 0xFF;
      uint8_t buf2 = u16WriteData[1] & 0xFF;
      uint8_t buf3 = (u16WriteData[2] >> 8) & 0xFF;
      value = buf0 & 0xff;
      value |= (buf1 << 8) & 0xff00;
      value |= (buf2 << 16) & 0xff0000;
      value |= (buf3 << 24) & 0xff000000;
    }
      
  return value;
}


bool DataPoll::SendReadQuery(uint8_t slaveNo, uint16_t startAddr, uint16_t regNum)
{
  uint8_t result = -1;
  telegram.u8id = slaveNo; // slave address
  telegram.u8fct = 3; // function code (this one is registers read)
  telegram.u16RegAdd = startAddr; // start address in slave
  telegram.u16CoilsNo = regNum; // number of elements (coils or registers) to read
  telegram.au16reg = u16Data; // pointer to a memory array in the Arduino

  result = master->query( telegram ); // send query (only once)
  if(result < 0)
  	return false;
  else
  	return true;
}

bool DataPoll::SendSingleWriteQuery(DataInfo &dataInfo){
	uint8_t result = -1;
	uint8_t type = 0;

	if(dataInfo.dataType.startsWith("U16") || dataInfo.dataType.startsWith("S16"))
		type = 0;
	else if(dataInfo.dataType.startsWith("U32") || dataInfo.dataType.startsWith("S32"))
		type = 1;
	
    telegram.u8id = SLAVE_ADDR; // slave address
    telegram.u16RegAdd = dataInfo.dataAddress.toInt(); // start address in slave
    if(type == 0){
    	telegram.u8fct = 6; // function code (this one is registers read)
    	telegram.u16CoilsNo = 1; // number of elements (coils or registers) to read
    	u16WriteData[0] = dataInfo.value;
    	telegram.au16reg = u16WriteData; // pointer to a memory array in the Arduino
    }
    else if(type == 1){
		telegram.u8fct = 16;
		telegram.u16CoilsNo = 2; // number of elements (coils or registers) to read
		if(dataSwapFlag){
			u16WriteData[0] = dataInfo.value >> 16;
			u16WriteData[1] = dataInfo.value & 0xFFFF;
		}else{
    		u16WriteData[0] = dataInfo.value & 0xFFFF;
			u16WriteData[1] = dataInfo.value >> 16;
		}
    	telegram.au16reg = u16WriteData; // pointer to a memory array in the Arduino
    }

    result = master->query( telegram ); // send query (only once)
    if(result < 0)
		return false;
	else
		return true;
}

bool DataPoll::SendMultipleWriteQuery(DataInfo &dataInfo){
	uint8_t result = 0;

	telegram.u8id = SLAVE_ADDR; // slave address
	telegram.u8fct = 16;
	telegram.u16RegAdd = 8096;
	telegram.u16CoilsNo = 5;
	u16WriteData[0] = 1;
	if(dataInfo.dataType.startsWith("U16") || dataInfo.dataType.startsWith("S16"))
		u16WriteData[1] = 0;
	else if(dataInfo.dataType.startsWith("U32") || dataInfo.dataType.startsWith("S32"))
		u16WriteData[1] = 1;
	
	u16WriteData[2] = dataInfo.dataAddress.toInt();
	u16WriteData[3] = dataInfo.value & 0xFFFF;
	u16WriteData[4] = dataInfo.value >> 16;
	telegram.au16reg = u16WriteData;
	result = master->query(telegram);
	//MBOX_DEBUG_PRINTLN("[DataPoll][SendMultipleWriteQuery]query result is:" + String(result));
    if(result < 0)
  	    return false;
	else
		return true;
}

bool DataPoll::SetModbusParam(uint32_t baudRate, uint32_t timeOut, SerialConfig config) {
  devPollWait = millis() + 1000;
  
 // master->begin(baudRate); // baud-rate at 19200
  master->begin(baudRate, config);//SERIAL_8E1
  
  master->setTimeOut(timeOut); // if there is no answer in 2000 ms, roll over
}

bool DataPoll::SetModbusDataTimeout(uint32_t timeOut) {
	master->setTimeOut(timeOut); // if there is no answer in 2000 ms, roll over
}


void DataPoll::PollTask() {
  bool result = false;
  int8_t revNum = 0;
  uint16_t startAddr = 0;
  uint16_t regNum = 0;
  if(indexAddr.begin() == indexAddr.end()) return;

  String cStrTM;
  //MBOX_DEBUG_PRINTLN("[PollTask]status is:" + String(taskControlStatus));
  switch(taskControlStatus) {
    case 0:
	  if(setDataFlag){
	  	  MBOX_DEBUG_PRINTLN("[PollTask]setDataFlag is:" + String(setDataFlag));
	  	  taskControlStatus++;
	  }
	  else{
	      //if((unsigned long)(millis() - devPollWait) > pollDataInterval){
		  //	MBOX_DEBUG_PRINTLN("[PollTask]time is:" + String(millis()));
		  //    MBOX_DEBUG_PRINTLN("[PollTask]Wait is:" + String(devPollWait));
		  //	taskControlStatus++; // wait state
	      //}
	      auto dataPoint = std::next(indexAddr.begin(), iterNum);
		  //dataPoint->pollFrequency = 100;
	  	  //MBOX_DEBUG_PRINTLN("[PollTask]now is:" + String(millis()));
		  //MBOX_DEBUG_PRINTLN("[PollTask]curTime is:" + String(dataPoint->curTime));
		  //MBOX_DEBUG_PRINTLN("current time:" + String(millis()) + " data time:" + String(dataPoint->curTime) + " Frequency:" + String(dataPoint->pollFrequency));
		  if((unsigned long)(millis() - dataPoint->curTime) > dataPoint->pollFrequency){
		      taskControlStatus++;
			  dataPoint->curTime = millis();
		  }
		  else{
		  	if(iterNum + 1 == indexAddr.size()) {
				if(varyModelData.dataList.size() > 0){
					dataChange = true;
					varyModelData.timestamp = cStrTM;
            	}
				iterNum = 0;
			}
		  	else iterNum++;
		  }
	  }
      break;
	case 1: 
	  if(setDataFlag){
	      auto it = std::next(setDataAddr.begin(), setDataNum);
		  if(singleWrite){
		  	result = SendSingleWriteQuery(*it);
			if(result)
				taskControlStatus++;
		  }
		  else{
		  	result = SendMultipleWriteQuery(*it);
			if(result)
				taskControlStatus++;
		  }
	  }
	  else{
	      startAddr = std::next(indexAddr.begin(), iterNum)->dataAddress.toInt();
	      regNum = GetQueryRegNum(std::next(indexAddr.begin(), iterNum)->dataType);
	      //MBOX_DEBUG_PRINTLN("startAddr = " + String(startAddr) + " regNum = " + String(regNum));
	      //MBOX_DEBUG_PRINTLN("[PollTask]now is:" + String(millis()));
	      result = SendReadQuery(SLAVE_ADDR, startAddr, regNum);
		  if(result){
		  	//MBOX_DEBUG_PRINTLN("[PollTask]send read query success.")
	      	taskControlStatus++;
			readingFlag = true;
		  }
	  }
      break;
    case 2:
	  if(setDataFlag && !readingFlag){
	  	  revNum = master->poll();
		  if (master->getState() == COM_IDLE && revNum >= 6) {
		  	if(singleWrite){
				SendSetDataResult(1);
	        	taskControlStatus = 0;
				setDataFlag = false;
		  	}
			else{
				if(setDataNum + 1 == setDataAddr.size()){
					SendSetDataResult(1);
					taskControlStatus = 0;
					setDataNum = 0;
					setDataFlag = false;
				}
				else{
					setDataNum++;
					taskControlStatus = 1;
					//PointPollWait = millis();
					delay(800);//延时500ms 等待设备写寄存器成功
				}
			}
	      } 
		  else if(master->getState() == COM_IDLE) {
		      SendSetDataResult(0);
			  setDataFlag = false;
	      }
	  }
	  else{
	      revNum = master->poll(); // check incoming messages
	      //如果revNum大于0.则表明与设备的通信连接是好的
	      if (master->getState() == COM_IDLE && revNum > 0) {
	        taskControlStatus++;
	        //connFailCount = 0;
	        devConnStatus = 1;
	       // MBOX_DEBUG_PRINTLN("master->getState() is COM_IDLE");
	      } else if(master->getState() == COM_IDLE) {
	          //connFailCount = 0;
	          //一轮查询里如果有一个点成功了，则判定与设备的通信是好的
	          if(devConnStatus != 1) devConnStatus = 0;
			  //重新开始查询下一个点,如果一轮结束则从状态0开始
			  if(iterNum + 1 == indexAddr.size()){
				  iterNum = 0;
				  taskControlStatus = 0;
			  	  devConnStatus_ = devConnStatus;
				  devConnStatus = 0;
			  	  devPollWait = millis();
			  }else{
			  	  iterNum++;
				  taskControlStatus = 0;//开始轮询下一个点
			  }
	      }
	  }
      break;
    case 3:
      std::next(curModelData.dataList.begin(), iterNum)->value = GetQueryValue(std::next(indexAddr.begin(), iterNum)->dataType);
      if(std::next(curModelData.dataList.begin(), iterNum)->value !=  std::next(lastModelData.dataList.begin(), iterNum)->value) {
            DataInfo dataInfo;
            dataInfo.dataAddress = std::next(curModelData.dataList.begin(), iterNum)->dataAddress;
            dataInfo.value = std::next(curModelData.dataList.begin(), iterNum)->value;
			dataInfo.dataType = std::next(curModelData.dataList.begin(), iterNum)->dataType;
			//限定变化数据的个数为60个，如果超过则循环覆盖
			if(varyModelData.dataList.size() >= 60){
				varyModelData.dataList.pop_front();
            	varyModelData.dataList.push_back(dataInfo);
			}else{
				varyModelData.dataList.push_back(dataInfo);
			}
			
			//数据变化了 更新last mode data
			std::next(lastModelData.dataList.begin(), iterNum)->value = std::next(curModelData.dataList.begin(), iterNum)->value;
      }

      // 一轮设备的数据查询结束
      if(iterNum + 1 == curModelData.dataList.size()) {
            iterNum = 0;
            devPollWait = millis();  // 下一轮数据查询延迟 10 后进行
            taskControlStatus = 0;
			//一轮轮询后保存设备通信状态
			devConnStatus_ = devConnStatus;
			devConnStatus = 0;
            //lastModelData.dataList = curModelData.dataList;
			cStrTM = String(now()) + "000";
			curModelData.timestamp = cStrTM;
			lastModelData.timestamp = curModelData.timestamp;
            // 记得写上时间戳
            if(varyModelData.dataList.size() > 0){
				dataChange = true;
				varyModelData.timestamp = cStrTM;
            }
      } else {
        iterNum++;
        taskControlStatus = 4;
        PointPollWait = millis();
      }
	  readingFlag = false;
      break;
    case 4:
      if ((unsigned long)(millis() - PointPollWait) > 10) taskControlStatus = 0; // 下一轮数据查询延迟 10ms 后进行
      break;
    default:
    	taskControlStatus = 0;
        break;
  }
}

bool DataPoll::InitDataModelInfo() {
	 mbox_dev_p3::ConfigDataAccessPtr cfgDA = ConfigDataAccess::Instance();
     char* config = cfgDA.get()->GetConfig();
	 if(!config) {
	 	list<DataPoint> dataModelInfoEmpty;
		dataModelInfoEmpty.clear();
	 	SetDataModelInfo(dataModelInfoEmpty);
	 	return false;
	 }
	 	
     DynamicJsonBuffer jsonBuffer(JSON_BUFF_3K);
     JsonObject &root = jsonBuffer.parseObject(config);
     if (!root.success()) {
	 	
		list<DataPoint> dataModelInfoEmpty;
		dataModelInfoEmpty.clear();
		SetDataModelInfo(dataModelInfoEmpty);
		free(config);
    	return false;
  	 }

	 JsonArray& mDataPoints = root["dataPoints"];
	 if(!mDataPoints.success()){
	 	list<DataPoint> dataModelInfoEmpty;
		dataModelInfoEmpty.clear();
		SetDataModelInfo(dataModelInfoEmpty);
		free(config);
    	return false;
	 }else{
		list<DataPoint> tmpDataModelInfo;
		for(int num = 0; num < mDataPoints.size(); num++) {
				char *str;
				char *p;
				char buf[30] = {0};
				char index = 0;
				DataPoint dataPoint;
				const char* tmpString = mDataPoints[num];
				strncpy(buf, tmpString, strlen(tmpString));
				//MBOX_DEBUG_PRINTLN("[InitDataModelInfo]Data is:" + buf)
				for(str = strtok_r(buf, ",", &p); str; str = strtok_r(NULL, ",", &p)){
					if(index == 0){
						dataPoint.dataAddress = str;
						//MBOX_DEBUG_PRINTLN("[InitDataModelInfo]Address:" + dataPoint.dataAddress);
					}
					else if(index == 1){
						dataPoint.dataType = str;
						//MBOX_DEBUG_PRINTLN("[InitDataModelInfo]Type:" + dataPoint.dataType);
					}
					else if(index == 2){
						dataPoint.pollFrequency = String(str).toInt();
						//MBOX_DEBUG_PRINTLN("[InitDataModelInfo]Frequency:" + String(dataPoint.pollFrequency));
					}
					index++;
				}
				/*
			    int position = -1;
				DataPoint dataPoint;
				const char* tmpString = mDataPoints[num];
				String data = tmpString;
				position = data.indexOf(',');
				if(position != -1)
				{
					dataPoint.dataAddress = data.substring(0,position);
					//MBOX_DEBUG_PRINTLN("[InitDataModelInfo]data address is:" + dataPoint.dataAddress);
					dataPoint.dataType = data.substring(position+1,data.length());
				}
				else{
					continue;
				}*/
   		    	tmpDataModelInfo.push_back(dataPoint);
			}
		SetDataModelInfo(tmpDataModelInfo);
	 }

	 free(config);
     return true;
}

bool DataPoll::SetDataModelInfo(list<DataPoint> &dataModelInfo) {
  curModelData.dataList.clear();
  lastModelData.dataList.clear();
  lastReportModelData.dataList.clear();
  varyModelData.dataList.clear();
  indexAddr = dataModelInfo;
  list<DataPoint>::iterator iter = indexAddr.begin();
  for(; iter != indexAddr.end(); iter++) {
      DataInfo dataInfo;
      dataInfo.dataAddress = iter->dataAddress;
  	  dataInfo.dataType = iter->dataType;
      dataInfo.value = 999999999;
      curModelData.dataList.push_back(dataInfo);
      lastModelData.dataList.push_back(dataInfo);
	  lastReportModelData.dataList.push_back(dataInfo);
  }

  taskControlStatus = 0;
  iterNum = 0;

}

bool DataPoll::GetDataChangeStatus() {
   return dataChange;
}

bool DataPoll::SetDataChangeStatus(bool setStaus) {
   dataChange = setStaus;
   return true;
}

ModelData& DataPoll::GetVaryData() {
   return varyModelData;
}

void DataPoll::SortVaryData(ModelData &varyData){
	int i,j,k;

	//去掉相同地址的数据，选择变化差异最大的值
	for(i = 0; i < varyData.dataList.size(); i++){
		//MBOX_DEBUG_PRINTLN("[SortVaryData]i is:" + String(i))
		for(j = varyData.dataList.size() - 1; j > i; j--){
			//MBOX_DEBUG_PRINTLN("[SortVaryData]j is:" + String(j))
			auto it_i = std::next(varyData.dataList.begin(),i);
			auto it_j = std::next(varyData.dataList.begin(),j);
			if(it_i->dataAddress == it_j->dataAddress)
			{
				//if(it_i->value < it_j->value){
				//	it_i->value = it_j->value;
				//}
				//与lastReportModelData比较，选择变化差异大的数据
				for(k = 0; k < lastReportModelData.dataList.size(); k++){
					auto it_k = std::next(lastReportModelData.dataList.begin(),k);
					//MBOX_DEBUG_PRINTLN("[SortVaryData]it_k address:" + String(it_i->dataAddress) + " k:" + String(k))
					if(it_i->dataAddress == it_k->dataAddress){
						//MBOX_DEBUG_PRINTLN("[SortVaryData]it_i value:" + String(it_i->value))
						//MBOX_DEBUG_PRINTLN("[SortVaryData]it_j value:" + String(it_j->value))
						//MBOX_DEBUG_PRINTLN("[SortVaryData]it_k value:" + String(it_k->value))
						//MBOX_DEBUG_PRINTLN("[SortVaryData]abs i value:" + String(abs((int32_t)(it_i->value - it_k->value))))
						//MBOX_DEBUG_PRINTLN("[SortVaryData]abs j value:" + String(abs((int32_t)(it_j->value - it_k->value))))
						if(abs((int32_t)(it_i->value - it_k->value)) < abs((int32_t)(it_j->value - it_k->value))){
							it_i->value = it_j->value;
						}
					}
				}
				varyData.dataList.erase(it_j);
			}
		}
		ESP.wdtFeed();//喂狗
	}
	//更新lastReportModelData的数据
	for(i = 0; i < lastReportModelData.dataList.size(); i++){
		for(j = 0; j < varyData.dataList.size(); j++){
			auto it_i = std::next(lastReportModelData.dataList.begin(),i);
			auto it_j = std::next(varyData.dataList.begin(),j);
			//MBOX_DEBUG_PRINTLN("[SortVaryData]vary data value:" + String(it_j->value))
			//MBOX_DEBUG_PRINTLN("[SortVaryData]vary data address:" + String(it_j->dataAddress))
			//MBOX_DEBUG_PRINTLN("[SortVaryData]last mode data value:" + String(it_i->value))
			//MBOX_DEBUG_PRINTLN("[SortVaryData]last mode data address:" + String(it_i->dataAddress))
			if(it_i->dataAddress == it_j->dataAddress){
				it_i->value = it_j->value;
			}
		}
	}
	ESP.wdtFeed();//喂狗
}

void DataPoll::ClearVaryData() {
  varyModelData.dataList.clear();
}

ModelData DataPoll::GetCurData() {
  return lastModelData;
}

void DataPoll::SetWriteData(list<DataInfo> &dataInfos) {
	if(setDataFlag) return;
	setDataAddr = dataInfos;
	setDataFlag = true;
}

uint8_t DataPoll::GetDevConnStaus() {
    return devConnStatus_;
}

bool DataPoll::openResumeMark() {
	
	if(SPIFFS.exists("/data/resume_mark")) {
		resumeMark.fileMark = SPIFFS.open("/data/resume_mark", "r");
		String markIndexStr = resumeMark.fileMark.readStringUntil('\n');
		resumeMark.markIndex = markIndexStr.toInt();
		MBOX_DEBUG_PRINTLN("exist resumeMark.markIndex = " + markIndexStr);
	} else {
		resumeMark.fileMark = SPIFFS.open("/data/resume_mark", "w");
		resumeMark.markIndex = -1;
		resumeMark.fileMark.println(String(resumeMark.markIndex));
		MBOX_DEBUG_PRINTLN("not exist resumeMark.markIndex = " + String(resumeMark.markIndex));
	}
	
	resumeMark.fileMark.close();
	return true;
}

bool DataPoll::isExistResumeData() {
	if(resumeMark.markIndex <= -1) //小于等于-1
		return false;
	else
		return true;
}

/*
 * 初步打算是一个文件中存 1 次记录，
 * 一共存 10 个文件，10 个文件存满后不再保存
 * 文件中一条记录内容是 : "时间戳,点的个数#点的地址|点的值......\n"
 * 例如: "1552098584000,2#01009|20#01010|25\n"  ---> 其中 1552098584000 是以毫秒为单位的，点的个数为 2, 其中一个点的地址为 01009, 对应的值为 20
 */
bool DataPoll::storeResumeData(ModelData &varyData) {
	if(resumeMark.markIndex >= RESUME_FILE_NUM) return false;
	
	if(isExistResumeData()) {
			resumeMark.markIndex++;
			resumeMark.fileMark = SPIFFS.open(("/data/" + String(resumeMark.markIndex)).c_str(), "w");
	} else {
		resumeMark.markIndex = 0;
		resumeMark.fileMark = SPIFFS.open(("/data/" + String(resumeMark.markIndex)).c_str(), "w");
	}

	char *resumeData = (char *)malloc(RESUME_CONTENT_NUM);
	if(!resumeData){
		MBOX_DEBUG_PRINTLN("[storeResumeData]malloc memory fail.")
		return false;
	}
	memset(resumeData, 0, RESUME_CONTENT_NUM);
	// 拷贝时间戳
	int index = 0;
	strncpy(resumeData + index, varyData.timestamp.c_str(), varyData.timestamp.length());
	index += varyData.timestamp.length();
	// 时间戳分割符
	strncpy(resumeData + index, ",", 1);
	index++; // 考虑到 ','
	strncpy(resumeData + index, String(varyData.dataList.size()).c_str(), String(varyData.dataList.size()).length());
	index += String(varyData.dataList.size()).length();

	list<DataInfo>::iterator it = varyData.dataList.begin(); 
	for(; it != varyData.dataList.end(); ++it) {
		strncpy(resumeData + index, "#", 1);
		index++;
		strncpy(resumeData + index, (it->dataAddress).c_str(), (it->dataAddress).length());  // 拷贝地址
		index += (it->dataAddress).length();
		strncpy(resumeData + index, "|", 1);
		index++;
		strncpy(resumeData + index, (String(it->value)).c_str(), (String(it->value)).length());  // 拷贝数据
		index += (String(it->value)).length();	
	}

	MBOX_DEBUG_PRINTLN("storeResumeData data = " + String(resumeData));
	resumeMark.fileMark.println(String(resumeData));  // 也可以其他方式写入

	free(resumeData);
	resumeMark.fileMark.close();
	
	resumeMark.fileMark = SPIFFS.open("/data/resume_mark", "w");
	resumeMark.fileMark.println(String(resumeMark.markIndex));
	resumeMark.fileMark.close();
	return true;
}

/*
 * 对一个文件中所有记录进行读取解析 
 * 文件中一条记录内容是 : "时间戳,点的个数#点的地址|点的值......\n"
 * 例如: "1552098584000,2#01009|20#01010|25\n"  ---> 其中 1552098584000 是以毫秒为单位的，点的个数为 2, 其中一个点的地址为 01009, 对应的值为 20
 */
bool DataPoll::getResumeData(list<ModelData> &varyDataList) {
	if(isExistResumeData() == false) return false;
	String readIndex = "/data/" + String(resumeMark.markIndex);
	resumeMark.fileMark = SPIFFS.open(readIndex.c_str(), "r");

	// 读取一个文件中的一个记录
	String record = resumeMark.fileMark.readStringUntil('\n');
	MBOX_DEBUG_PRINTLN("getResumeData record = " + record);
	int indexEnd = record.indexOf(',', 0);
	ModelData varyData;
	varyData.timestamp = record.substring(0, indexEnd);
	MBOX_DEBUG_PRINTLN("getResumeData timestamp = " + varyData.timestamp);
	int indexBegin = indexEnd;
	indexEnd = record.indexOf('#', indexBegin);
	int dataPointNum = record.substring(indexBegin + 1, indexEnd).toInt();
	MBOX_DEBUG_PRINTLN("storeResumeData dataPointNum = " + String(dataPointNum));
	for(int i = 0; i < dataPointNum; i++) {
		indexBegin = indexEnd;
		DataInfo dataInfo;
		indexEnd = record.indexOf('|', indexBegin);
		dataInfo.dataAddress = record.substring(indexBegin + 1, indexEnd);
		//MBOX_DEBUG_PRINTLN("storeResumeData dataAddress = " + dataInfo.dataAddress);
		indexBegin = indexEnd;
		indexEnd = record.indexOf('#', indexBegin);
		dataInfo.value = record.substring(indexBegin + 1, indexEnd).toInt();
		//MBOX_DEBUG_PRINTLN("storeResumeData dataInfo.value = " + String(dataInfo.value));
		varyData.dataList.push_back(dataInfo);
	}
	varyDataList.push_back(varyData);

	resumeMark.fileMark.close();
	return true;
}

void DataPoll::removeResumeData(){
	String readIndex = "/data/" + String(resumeMark.markIndex);

	//删除已经上报的文件
	SPIFFS.remove(readIndex.c_str());

	//下一个文件
	resumeMark.markIndex--;

	//将新的写入文件index和记录数写入文件
	resumeMark.fileMark = SPIFFS.open("/data/resume_mark", "w");
	resumeMark.fileMark.println(String(resumeMark.markIndex));
	resumeMark.fileMark.close();
}

