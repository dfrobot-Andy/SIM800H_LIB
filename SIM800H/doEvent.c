/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h" 
#include "string.h"
#include "doEvent.h"

//char atComCmd[RE_BUF_LEN];
extern short sim_Switch_State;
extern uchar CSQ_t;
extern char dtmfreq;
extern powermodule_t powermodule;
extern reqResult_t reqResult;
extern short eventStates;
extern char phonenumber[15];
extern char SMSbuf[SMS_BUF_MAX];
extern char SMSID[5];

typedef char (*doEventRequest_t)(char *pEventCmd);

typedef struct reqExecMap{
  const char *cmdStr;
  doEventRequest_t doEventRequest;
}reExecMap_t;

struct reqExecMap atCmdArray[]=
{
	{"AT",doAT},
	{"OK",doOK},
	{"ERROR",doError},
	{"RING",doRing},
	{"+DTMF: ",doDTMF},
	{"+CTTS: ",doCTTS},
	{"BUSY",doBusy},
	{"NO CARRIER",doNocarrier},
	{"+CSQ: ",doReadCSQ},
	{">",doOK},
	{"+CMTI: ",doSMS},
	{"+CMGR: ",doReadSMS},
	{"SMS Ready",doSMSReady},
	{"POWER",doPower}
};
   
char eventMatchFun(uchar _len, char *pbuf)
{	
	if((pbuf[0]==0x0d)&&(pbuf[1]==0x0a))  //clear \r\n
	{
		pbuf+=2;
	}
	return(execEventCmd(pbuf));
}

char execEventCmd(char *pEventCmd)
{
	uchar i;
	for(i=0;i<(sizeof(atCmdArray)/sizeof(reExecMap_t));i++)
	{
		if(memcmp((char *)atCmdArray[i].cmdStr,pEventCmd,strlen(atCmdArray[i].cmdStr)) == 0)
		{
			return atCmdArray[i].doEventRequest(pEventCmd);
		}
	}
	
	return _FAULSE;
}

char doAT(char *pEventCmd)
{
	reqResult = RESULT_OK;
	
	return _SUCCESS;
}

char doOK(char *pEventCmd)
{
	reqResult = RESULT_OK;
	
	return _SUCCESS;
}

char doError(char *pEventCmd)
{
	reqResult = RESULT_ERROR;
	
	return _SUCCESS;
}

char doRing(char *pEventCmd)
{
	if(sim_Switch_State & 0x8000)
	{
		if(!(sim_Switch_State&0x4000))
		{				
			if(sim_Switch_State & 0x0001)
			{
				memcpy(phonenumber,&pEventCmd[18],11);
			}
			sim_Switch_State |= 0x4000;   
			CallStatus = RING_CALL;	
		}
	}
	
	return _SUCCESS;
}

char doDTMF(char *pEventCmd)
{

	dtmfreq = pEventCmd[7];
	sim_Switch_State |= 0x0004;

	return _SUCCESS;
}

char doCTTS(char *pEventCmd)
{
	if(pEventCmd[7] == '0')
	{
		sim_Switch_State &= (~0x0020);		
	}
	return _SUCCESS;
}

char doBusy(char *pEventCmd)
{
	sim_Switch_State &= (~0x4000);
	CallStatus = AVAILABLE_CALL;

	return _SUCCESS;
}

char doNocarrier(char *pEventCmd)
{
	sim_Switch_State &= (~0x4000);
	CallStatus = AVAILABLE_CALL;

	return _SUCCESS;
}

char doReadCSQ(char *pEventCmd)
{
	if(',' == pEventCmd[7])	
	{
		CSQ_t = pEventCmd[6] & 0x0f;
	}
	else
	{
		CSQ_t = ((pEventCmd[6] & 0x0f) * 10) + (pEventCmd[7] & 0x0f);
	}
	sim_Switch_State |= 0x0800;	
	
	return _SUCCESS;
}

char doSMS(char *pEventCmd)
{
	char i=0;
	if(sim_Switch_State & 0x2000)
	{
		pEventCmd += 12;
		for(i=0;i<5;i++)
		{
			SMSID[i] = 0x00;
		}
		i = 0;
		while(1) 
		{
			SMSID[i] = pEventCmd[i];
			if(SMSID[i] == 0x0d)
			{
				SMSID[i] = '\0';
				break;
			}
			i++;
		}
		sim_Switch_State |= 0x1000;
	}
	return _SUCCESS;
}

char doReadSMS(char *pEventCmd)
{
	char i = 0;
	while(1)
	{
		pEventCmd++;
		if(*pEventCmd==0x0a)
		{
			pEventCmd++;
			break;
		}
	}
	while(1)
	{
		SMSbuf[i] = pEventCmd[i];
		if((SMSbuf[i]==0x0d) || (i>SMS_BUF_MAX-1))
		{
			SMSbuf[i] = '\0';
			break;
		}
		i++;
	}
	//pEventCmd += 110;
	//memcpy(SMSbuf,pEventCmd,10);
	sim_Switch_State |= 0x0010;

	return _SUCCESS;
}

char doSMSReady(char *pEventCmd)
{
	sim_Switch_State |= 0x0200;
	return _SUCCESS;
}

char doPower(char *pEventCmd)
{
	if(powermodule == POWER_ON)
	{
		powermodule = POWER_OFF;
	}
	return _SUCCESS;
}
