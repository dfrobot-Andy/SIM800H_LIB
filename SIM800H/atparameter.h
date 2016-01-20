/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#ifndef atparameter_h
#define atparameter_h

#include <stdio.h>
#include "Arduino.h"
#include <inttypeds.h>
//_AVR_IOM32U4_H_
#if defined (__AVR_ATmega32U4__)
  #define _Serial Serial1
#else
  #define _Serial Serial
#endif

#define SMS_BUF_MAX 21

#define _SUCCESS 1
#define _FAULSE  0

#define SYS_EVENT_POWER			0x0001
//#define SYS_EVENT_DIALTELE		0x0002
//#define SYS_EVENT_RING			0x0004

typedef unsigned char   uchar;
typedef unsigned int    uint;

typedef enum
{
	AVAILABLE_CALL,
	RING_CALL,
	DIAL_CALL,
	BUSY_CALL
}CallStatus_t;

typedef enum
{
	POWER_ON,
	POWER_OFF,
	POWER_UNKNOWN
}powermodule_t;

typedef enum
{
	RESULT_OK,
	RESULT_ERROR,
	RESULT_UNKNOWN
}reqResult_t;

typedef enum
{
	OPEN,
	CLOSE,
}reqmodle_t;

typedef enum
{
	UCS2,
	TEXT,
}reModle_t;

extern short sim_Switch_State;
extern short eventStates;
extern uchar CSQ_t;
extern char dtmfreq;
extern powermodule_t powermodule;
extern reqResult_t reqResult;
extern CallStatus_t CallStatus;
extern char phonenumber[15];
extern char SMSbuf[SMS_BUF_MAX];
extern char SMSID[5];
#endif

