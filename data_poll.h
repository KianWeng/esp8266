#ifndef DATA_POLL
#define DATA_POLL

#include <list>
#include "Arduino.h"
#include "ModbusRtu.h"
#include "common.h"
#include "FS.h"
#include <Ticker.h>

using namespace std;

#define DEV_CONN_ERROR (5)
#define SLAVE_ADDR (1)
#define DATA_RESUME_DIR_PAHT ("/data")

#define RECORD_NUM_MAX (3)  // 一个文件存最大的记录数
#define RESUME_FILE_NUM (900)  // 最多保存文件数

#define RESUME_CONTENT_NUM (1 * 1024)  //申请消息缓存空间用于更新到文件中

struct ResumeMark {
    File fileMark;  // 记录当前
    int markIndex; // 从哪个索引文件开始 最原始的状态为 -1, 
};

class DataPoll {
public:
    DataPoll();
    ~DataPoll();
    // timeOut : ms
    bool SetModbusParam(uint32_t baudRate, uint32_t timeOut, SerialConfig config);
	bool SetModbusDataTimeout(uint32_t timeOut);
    void PollTask();

	// 程序刚开始时从文件中读取数据模型
    bool InitDataModelInfo();
	
    // 在添加数据模块信息时要被调用
    bool SetDataModelInfo(list<DataPoint> &dataModelInfo);

    bool GetDataChangeStatus();
    bool SetDataChangeStatus(bool setStaus);
    // 当外层模块调用完 GetVaryData() 后，需要 SetDataChangeStatus(false)
    ModelData& GetVaryData();
    void ClearVaryData();
    void SortVaryData(ModelData &varyData);
    ModelData GetCurData();

    // 只能设置一个寄存器的值
    void SetWriteData(list<DataInfo> &dataInfos);

    // return 0: offline 1: online
    uint8_t GetDevConnStaus();

	bool openResumeMark();
	bool isExistResumeData();
	bool storeResumeData(ModelData &varyData);
	bool getResumeData(list<ModelData> &varyDataList);
	void removeResumeData();
private:
	void SendSetDataResult(char result);
    bool SendReadQuery(uint8_t slaveNo, uint16_t startAddr, uint16_t regNum);
	bool SendSingleWriteQuery(DataInfo & dataInfo);
	bool SendMultipleWriteQuery(DataInfo & dataInfo);
    uint16_t GetQueryRegNum(String &dataType);
    uint32_t GetQueryValue(String &dataType);
	uint32_t GetWriteValue(String &dataType);

    ModelData curModelData;
    ModelData lastModelData;
	ModelData lastReportModelData;
    bool dataChange;
    ModelData varyModelData;  // the data change of curModelData and lastModelData
    list<DataPoint> indexAddr;
	list<DataInfo> setDataAddr;
    uint32_t devPollWait;    
    uint8_t taskControlStatus;
    uint32_t PointPollWait;  
    uint8_t iterNum;  // 用于遍历请求每个点位 
    uint8_t setDataNum;

    modbus_t telegram;
    Modbus *master;
    uint16_t u16Data[16];
    uint16_t u16WriteData[16];

    uint8_t devConnStatus;  //单个点的通信状态
    uint8_t devConnStatus_; // 设备通信状态 0: offline 1: online
    //uint16_t connFailCount; // 设备通信故障计数

	Dir dirResumeData;  // 保存数据恢复的文件夹
	ResumeMark resumeMark;  

	Ticker dataPollTimer;
	bool setDataFlag = false;
	bool readingFlag = false;
};
#endif 
