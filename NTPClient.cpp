/**
 * \file NTPClient.cpp
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2019-01-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "NTPClient.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <sys/time.h>
#include <time.h>

WiFiUDP udp;
IPAddress ip;
uint8_t interval_;
byte packetBuffer[NTP_PACKET_SIZE];
uint8_t sendflag;
struct timeval TransTimeval, RcvTimeval, CurrentTimeval;
//const char* ntpServerName = "ntp1.aliyun.com";
char ntpServerName[60];
IPAddress timeServerIP;

// NTPClient
NTPClient::~NTPClient() {}

NTPClient::NTPClient(const String &serverAddress, uint8_t interval) {
    interval_ = interval;
    strcpy(ntpServerName, serverAddress.c_str());
    //WiFi.hostByName(serverAddress.c_str(), ip);
    udp.begin(_localPort);
}

void NTPClient::begin() {
    TimerInit();
}

void TimerInit() {
    static os_timer_t os_timer;
    os_timer_disarm(&os_timer);
    //os_timer_setfn(&os_timer, (ETSTimerFunc *) (std::bind(&NTPClient::TimerCallback, this)), NULL);
    os_timer_setfn(&os_timer, (ETSTimerFunc *) (TimerCallback), NULL);
    os_timer_arm(&os_timer, 1, true);
    //Serial.println("TimerInit...");
}

// Timer callback function
void TimerCallback() {
    static uint32_t TimeCountUsec = 0;
    static uint32_t TimeCountSec = 0;
    static uint32_t TimeCountHour = 0;

    TimeCountUsec ++;
    if(sendflag == 1) {
        RcvNtpPacket();
        if(TimeCountUsec >= 1000){
          sendflag = 0;
          TimeCountUsec = 0;
        }
    }
    else if(TimeCountUsec >= 1000) {
        if(++TimeCountSec >= 3600) {
            if(TimeCountHour >= interval_) {
                if(WiFi.status() == WL_CONNECTED) {
                    SendNtpPacket(ip);
                    sendflag = 1;
                }
                TimeCountHour = 0;
            }
            TimeCountSec = 0;
            TimeCountHour += 1;
        }
        TimeCountUsec = 0;
    }
}

// send an NTP request to the time server at the given address
void SendNtpPacket(IPAddress& address) {
    struct ntphdr *ntp;
    WiFi.hostByName(ntpServerName, timeServerIP);
    memset(packetBuffer, 0, NTP_PACKET_SIZE); //set all bytes in the buffer to 0
    ntp = (struct ntphdr *) packetBuffer;
    ntp->ntp_li = NTP_LI;
    ntp->ntp_vn = NTP_VN;
    ntp->ntp_mode = NTP_MODE;
    ntp->ntp_stratum = NTP_STRATUM;
    ntp->ntp_poll = NTP_POLL;
    ntp->ntp_precision = NTP_PRECISION;
    gettimeofday(&TransTimeval, NULL);
    ntp->ntp_transts.intpart = htonl(TransTimeval.tv_sec + JAN_1970);
    ntp->ntp_transts.fracpart = htonl(USEC2FRAC(TransTimeval.tv_usec));

    // send a packet requesting a timestamp:
    //udp.beginPacket(address, 123);
    udp.beginPacket(timeServerIP, 123);
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

int RcvNtpPacket() {
    int cb = udp.parsePacket();
    if (!cb) {
        NTP_DEBUG_PRINTLN("no packet yet");
		return -1;
    }
    else {
        sendflag = 0;
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
        NTP_DEBUG_PRINTLN("received packet");
        NtpSynchronize();
    }

	return 0;
}

// NTP Synchronize
void NtpSynchronize() {
    double offset;
    gettimeofday(&RcvTimeval, NULL);
    offset = GetOffset((struct ntphdr *) packetBuffer, &RcvTimeval); // calculate offset

    gettimeofday(&CurrentTimeval, NULL);
    CurrentTimeval.tv_sec += (int) offset;
    CurrentTimeval.tv_usec += offset - (int) offset;

    if (settimeofday(&CurrentTimeval, NULL) != 0) {                  // update time
        perror("settimeofday error");
    }
}

/*
 *  获取客户端与服务器之间的往返时延
 *  参数ntp指向服务器应答所在的缓冲区；
 *  参数rcvtv指向收到服务器应答的本地时间；
 */
 /**
 * \brief Get round-trip time between client and server
 * 
 * \param ntp Point to the buffer where the server responds.
 * \param rcvtv Local time to receive server reply;
 */
double GetRtt(const struct ntphdr *ntp, const struct timeval *rcvtv)
{
    double t1, t2, t3, t4;

    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = rcvtv->tv_sec + rcvtv->tv_usec / 1000000.0;

    return (t4 - t1) - (t3 - t2);
}

/*
 *  获取客户端与服务器的时间偏移量
 *  参数ntp指向服务器应答所在的缓冲区；
 *  参数rcvtv指向收到服务器应答的本地时间；
 */
/**
 * \brief Get the time offset between client and server.
 * 
 * \param ntp Point to the buffer where the server responds.
 * \param rcvtv Local time to receive server reply;
 */
double GetOffset(const struct ntphdr *ntp, const struct timeval *rcvtv)
{
    double t1, t2, t3, t4;

    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = rcvtv->tv_sec + rcvtv->tv_usec / 1000000.0;

    NTP_DEBUG_PRINTLN(t1);
    return ((t2 - t1) + (t3 - t4)) / 2;
}

uint32_t NTPClient::GetTimeStamp() {
    SendNtpPacket(ip);
    delay(1000);
    RcvNtpPacket();
    struct ntphdr *ntp = (struct ntphdr *) packetBuffer;
    TimeStamp = ntohl(((struct l_fixedpt *)(&ntp->ntp_transts))->intpart) - JAN_1970;
    NTP_DEBUG_PRINT("TimeStamp:");
    NTP_DEBUG_PRINTLN(TimeStamp);
    return TimeStamp;
}

uint64_t NTPClient::GetTimeStampUsec() {
	int ret;
    uint64_t TimeStampSec;
    SendNtpPacket(ip);
    delay(1000);
    ret = RcvNtpPacket();
	if(ret == -1){
		return 0;
	}

    unsigned long highWord = word(packetBuffer[44], packetBuffer[45]);
    unsigned long lowWord = word(packetBuffer[46], packetBuffer[47]);
    unsigned long Millisecond = highWord << 16 | lowWord;
    NTP_DEBUG_PRINT("Millisecond:");
    NTP_DEBUG_PRINTLN(Millisecond);

    struct ntphdr *ntp = (struct ntphdr *) packetBuffer;
    TimeStampSec = ntohl(((struct l_fixedpt *)(&ntp->ntp_transts))->intpart) - JAN_1970;
    TimeStampUsec = TimeStampSec*1000 + Millisecond / 4294967;

    /*unsigned long test;
    test = TimeStampSec*1000/1000;
    //test = Millisecond / 4294967;
    NTP_DEBUG_PRINT("test:");
    NTP_DEBUG_PRINTLN(test);*/
	//NTP_DEBUG_PRINT("TimeStampUsec:");
    //NTP_DEBUG_PRINTLN(String((uint32_t)(TimeStampUsec / 1000)) + String((uint32_t)(TimeStampUsec % 1000)));
	
    return TimeStampUsec;
}
