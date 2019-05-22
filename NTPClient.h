 /**
 * \file NTPClient.h
 * 
 * \author your name
 * 
 * \brief Inspecting the validity of the configurations of AP, Web Server and MQTT.
 * 
 * @version 0.1
 * 
 * \date 2019-01-18
 * 
 * @copyright Copyright (c) 2019
 */

#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>




//#define NTP_DEBUG
#define NTP_DEBUG_SERIAL Serial1

#ifdef NTP_DEBUG
    #define NTP_DEBUG_PRINT(...) { NTP_DEBUG_SERIAL.print(__VA_ARGS__); }
    #define NTP_DEBUG_PRINTLN(...) { NTP_DEBUG_SERIAL.println(__VA_ARGS__); }
#else
    #define NTP_DEBUG_PRINT(...) {}
    #define NTP_DEBUG_PRINTLN(...) {}
#endif

#define VERSION_3           3
#define VERSION_4           4

#define MODE_CLIENT         3
#define MODE_SERVER         4

#define NTP_LI              3
#define NTP_VN              VERSION_4   
#define NTP_MODE            MODE_CLIENT
#define NTP_STRATUM         0
#define NTP_POLL            6
#define NTP_PRECISION       -6

#define NTP_HLEN            48

#define NTP_PORT            123
#define NTP_SERVER          "182.92.12.11"

#define TIMEOUT             10

#define NTP_PACKET_SIZE     48

#define JAN_1970            0x83AA7E80

#define NTP_CONV_FRAC32(x)  (uint64_t) ((x) * ((uint64_t)1<<32))    
#define NTP_REVE_FRAC32(x)  ((double) ((double) (x) / ((uint64_t)1<<32)))   

#define NTP_CONV_FRAC16(x)  (uint32_t) ((x) * ((uint32_t)1<<16))    
#define NTP_REVE_FRAC16(x)  ((double)((double) (x) / ((uint32_t)1<<16)))    

#define USEC2FRAC(x)        ((uint32_t) NTP_CONV_FRAC32( (x) / 1000000.0 )) 
#define FRAC2USEC(x)        ((uint32_t) NTP_REVE_FRAC32( (x) * 1000000.0 )) 

#define NTP_LFIXED2DOUBLE(x)    ((double) ( ntohl(((struct l_fixedpt *) (x))->intpart) - JAN_1970 + FRAC2USEC(ntohl(((struct l_fixedpt *) (x))->fracpart)) / 1000000.0 ))   

#define _localPort           2390

struct s_fixedpt {
    uint16_t intpart;
    uint16_t fracpart;
};

struct l_fixedpt {
    uint32_t intpart;
    uint32_t fracpart;
};

struct ntphdr {
    uint32_t ntp_mode:3;
    uint32_t ntp_vn:3;
    uint32_t ntp_li:2;
    uint8_t ntp_stratum;
    uint8_t ntp_poll;
    int8_t ntp_precision;
    struct s_fixedpt ntp_rtdelay;
    struct s_fixedpt ntp_rtdispersion;
    uint32_t ntp_refid;
    struct l_fixedpt ntp_refts;
    struct l_fixedpt ntp_orits;
    struct l_fixedpt ntp_recvts;
    struct l_fixedpt ntp_transts;
};

class NTPClient {
public:
    NTPClient(const String &serverAddress, uint8_t interval);
    ~NTPClient();
    void begin();
    uint32_t GetTimeStamp();
    uint64_t GetTimeStampUsec();

private:
    uint32_t TimeStamp;
    uint64_t TimeStampUsec;
};

void TimerInit();
void TimerCallback();
void SendNtpPacket(IPAddress& address);
int RcvNtpPacket();
void NtpSynchronize();
double GetRtt(const struct ntphdr *ntp, const struct timeval *rcvtv);
double GetOffset(const struct ntphdr *ntp, const struct timeval *rcvtv);




#endif //NTPCLIENTFILE
