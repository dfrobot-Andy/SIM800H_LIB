/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#include "Arduino.h"
#include "sim800cmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "atparameter.h"
#include "msTimer.h"

//Use a C file
extern "C"
{
	#include "doEvent.h"
}

uchar DTMF_T;
uchar CSQ_t;					//CSQ data
char dtmfreq;				    //save DTMF data
short sim_Switch_State;		    //status flag Switch 
short eventStates;			    //The task flags
reqResult_t reqResult;		    //Send command returns a result
powermodule_t powermodule;	    //State of the switch module
CallStatus_t CallStatus;	    //State of the call module
char phonenumber[15];	        //Received telephone number buffer
char SMSbuf[SMS_BUF_MAX];
char SMSID[5];
int csqtimeout=0;		        //csq timeout counter

using namespace Timerszcl;		//use namespace Timerszcl	

void donothingfun(void)
{
//do nothing
}

/**********************************************************************************************
* @fn     taskProcess
*
* @brief  Task callback function.This function would be called every 100 milliseconds after after
*		  the system started.In this function, "AT+CSQ" would send when csqtioeout more than 300,
*         to obtain sim800h modular signal strength data, and judge the on-off state.
*
* @param  voie.
*
* @retval none.
*/
void taskProcess(void)
{
	if(eventStates & SYS_EVENT_POWER)
	{
		if(powermodule == POWER_OFF)
		{
			digitalWrite(12,LOW);
			eventStates &= (~SYS_EVENT_POWER);	
		}
	}
	if(sim_Switch_State & 0x8000)
	{
		csqtimeout += 1;
		if(csqtimeout >= 1500)
		{
			csqtimeout = 0;
			if((sim_Switch_State & 0x0010) == 0)
			{
				_Serial.println("AT+CSQ");
			}
		}
	}
	if(sim_Switch_State & 0x2000)
	{
		if(sim_Switch_State & 0x1000)
		{
			sim_Switch_State &= (~0x1000);
			pSMSfun();
		}
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::Sim800Cmd
*
* @brief  Constructor function.This function initializes the sim800h modular power_pin(12),
*		  register the taskProcess function,but cannot use telephone,just for TTS speaker.
*         
* @param  voie.
*
* @retval none.
*/
Sim800Cmd::Sim800Cmd()
{
	pinMode(12, OUTPUT);
	setRingCBS(NULL,taskProcess);
	powermodule = POWER_UNKNOWN;
}

/**********************************************************************************************
* @fn     Sim800Cmd::Sim800Cmd
*
* @brief  Constructor function.This function initializes the sim800h modular power_pin(12),
*		  register the taskProcess function.Set status flag Switch to turn on.
*         
* @param  *pRf.  When a phone is called, callback the registration function(*pRf)
*
* @retval none.
*/
Sim800Cmd::Sim800Cmd(void (*pRf)())
{
	pinMode(12, OUTPUT);
	setRingCBS(pRf,taskProcess);
	sim_Switch_State = 0x8000;
	powermodule = POWER_UNKNOWN;
}

/**********************************************************************************************
* @fn     timerSYSCBs
*
* @brief  Periodic to judge the serial data_buffer. if have data, call eventMatchFun function.
*         
* @param  void
*
* @retval none.
*/
void timerSYSCBs(void)
{
	int sqrNum;
	char i = 0;
	if((sqrNum=_Serial.available()) > 0)
	{
		char *buffer;
		buffer = (char*)malloc(sqrNum+1);
		if(buffer != NULL)
		{
			while(sqrNum--)
			{
				buffer[i] = _Serial.read();
				i++;
			}
			buffer[i] = '\0';
			eventMatchFun(i,buffer);
			free(buffer);
		}
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::sim800init
*
* @brief  sim800h modular Initializate. Set the periodic function call time and register a
*		  callback function
*         
* @param  void
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char  Sim800Cmd::sim800init(void)
{
	this->_volume = 100;
	this->_modle  = 0;
	this->_pch	  = 50;
	this->_spd    = 50;
	this->_chl    = 0;
	_Serial.begin(19200);
	set(200,timerSYSCBs);
	start();
	if(this->atinitcheck() == 0)
	{
		digitalWrite(12,HIGH);
		delay(2000);
		delay(2000);
		digitalWrite(12,LOW);
	}
	if(this->atinitcheck() == 0) //sim800 is not running
	{
		if(this->sim800open() != RESULT_OK)
		{
			return _FAULSE;
		}
	}
	else
	{
		sim_Switch_State |= 0x0400;
	}
	delay(1000);
	
	powermodule = POWER_ON;
	if(this->atsendcmd("ATE0", 1, 1000) != RESULT_OK)
	{		
		return _FAULSE;
	}
	delay(1000);
//	if((sim_Switch_State&0x0400) == 0)
	{
		if(sim_Switch_State & 0x8000)
		{//set SMS parameters
			char strcmd[15]={0x41,0x54,0x2b,0x43,0x53,0x43,0x53,0x3d,0x22,0x55,0x43,0x53,0x32,0x22};
			char i=0;
			while(1)
			{
				delay(500);
				i++;
				if((sim_Switch_State&0x0200)||(i>10))
				{
					break;
				}
			}
			this->atsendcmd("AT+CNMI=2,1,0,1,0", 2, 1000);
			this->atsendcmd("AT+CMGF=1", 2, 1000);		
			this->atsendcmd(strcmd, 2, 1000);
			this->atsendcmd("AT+CSMP=17,167,2,25", 2, 1000);
		}
	}
	pDTMFfun = donothingfun;  //donothingfun
	pSMSfun = donothingfun;  //donothingfun
	delay(500);
	_Serial.println("AT+CSQ");
	delay(500);
    //Initialization is complete
	return _SUCCESS;
}

/**********************************************************************************************
* @fn     Sim800Cmd::atsendcmd
*
* @brief  Send command or data to sim800h modular. 
*         
* @param  *pstr - will send data
*         _cter - max send times
*         _time - response timeout(millisecond)
*
* @retval reqResult - RESULT_OK/RESULT_ERROR/RESULT_UNKNOWN
*/
reqResult_t Sim800Cmd::atsendcmd(char *pstr, char _cter, int _time)
{
	reqResult = RESULT_UNKNOWN;
	while(_cter--)
	{
		_Serial.println(pstr);
		delay(_time);
		if(reqResult != RESULT_UNKNOWN)
			break;
	}
	return reqResult;
}

/**********************************************************************************************
* @fn     Sim800Cmd::atinitcheck
*
* @brief  sim800h modular power check. 
*         
* @param  void
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::atinitcheck(void)
{
	char number = 2;
	reqResult = RESULT_UNKNOWN;
	while(number--)
	{
		_Serial.println("AT");
		delay(600);
		if(reqResult == RESULT_OK)
			return _SUCCESS;
	}
	return _FAULSE;
}

/**********************************************************************************************
* @fn     Sim800Cmd::sim800open
*
* @brief  Open the sim800h modular power. 
*         
* @param  void
*
* @retval reqResult - RESULT_OK/RESULT_ERROR/RESULT_UNKNOWN
*/
reqResult_t Sim800Cmd::sim800open(void)
{
	reqResult_t _pwCB;
	if(powermodule != POWER_ON)
	{
		digitalWrite(12,HIGH);
		_pwCB = this->atsendcmd("AT", 5, 1000);
		digitalWrite(12,LOW);
	}
	return _pwCB;
}

/**********************************************************************************************
* @fn     Sim800Cmd::sim800close
*
* @brief  Close the sim800h modular power. 
*         
* @param  void
*
* @retval none
*/
void Sim800Cmd::sim800close(void)
{
	if(powermodule == POWER_ON)
	{
		digitalWrite(12,HIGH);
		eventStates |= SYS_EVENT_POWER;
	}
}

/************************************TTS function*********************************************/

/**********************************************************************************************
* @fn     Sim800Cmd::atTTS_SetParameter
*
* @brief  Set TTS parameter. 
*         
* @param  volume - 0--100
*		  modle  - 0--3
*		  pch    - 0--100
*         spd    - 0--100
*         chl    - 0-- 1
*
* @retval none
*/
void Sim800Cmd::atTTS_SetParameter(uchar volume, uchar modle, uchar pch, uchar spd, uchar chl)
{
	char *str;
	str = (char *)malloc(30);
	if(str != NULL)
	{
		strcpy(str,"AT+CTTSPARAM=");

		this->_volume = volume;
		this->_modle  = modle;
		this->_pch	   = pch;
		this->_spd    = spd;
		this->_chl    = chl;
		this->num2str(this->_volume,this->_modle,this->_pch,this->_spd,this->_chl,&str[13]);
		this->atsendcmd(str,2,700);
		free(str);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSParameter
*
* @brief  Set TTS parameter. 
*         
* @param  _vme - 0--100
*		  _md  - 0--3
*		  _pch - 0--100
*         _spd - 0--100
*         _chl - 0-- 1
*
* @retval none
*/
void Sim800Cmd::setTTSParameter(char _vme, char _md, char _pch, char _spd, char _chl)
{
	if((_vme>=0)&&(_vme<=100)&&(_md>=0)&&(_md<=3)&&(_pch>=0)&&(_pch<=100)&&(_spd>=0)&&(_spd<=100)&&(_chl>=0)&&(_chl<=1))
	{
		this->atTTS_SetParameter((uchar)_vme, (uchar)_md, (uchar)_pch, (uchar)_spd, (uchar)_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSParameter
*
* @brief  Set TTS parameter. 
*         
* @param  _vme - 0--100
*		  _md  - 0--3
*		  _pch - 0--100
*         _spd - 0--100
*         _chl - 0-- 1
*
* @retval none
*/
void Sim800Cmd::setTTSParameter(int _vme, int _md, int _pch, int _spd, int _chl)
{
	if((_vme>=0)&&(_vme<=100)&&(_md>=0)&&(_md<=3)&&(_pch>=0)&&(_pch<=100)&&(_spd>=0)&&(_spd<=100)&&(_chl>=0)&&(_chl<=1))
	{
		this->atTTS_SetParameter((uchar)_vme, (uchar)_md, (uchar)_pch, (uchar)_spd, (uchar)_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSVolume
*
* @brief  Set TTS Voice parameter. 
*         
* @param  _vme - 0--100
*
* @retval none
*/
void Sim800Cmd::setTTSVolume(char _vme)
{
	if((_vme>=0)&&(_vme<=100))
	{
		this->_volume = (uchar)_vme;
		this->atTTS_SetParameter(this->_volume,this->_modle,this->_pch,this->_spd,this->_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSVolume
*
* @brief  Set TTS Voice parameter. 
*         
* @param  _vme - 0--100
*
* @retval none
*/
void Sim800Cmd::setTTSVolume(int _vme)
{
	if((_vme>=0)&&(_vme<=100))
	{
		this->_volume = (uchar)_vme;
		this->atTTS_SetParameter(this->_volume,this->_modle,this->_pch,this->_spd,this->_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSSpeed
*
* @brief  Set TTS Voice speed parameter. 
*         
* @param  _spd - 0--100
*
* @retval none
*/
void Sim800Cmd::setTTSSpeed(char _spd)
{
	if((_spd>=0)&&(_spd<=100))
	{
		this->_spd    = (uchar)_spd;
		this->atTTS_SetParameter(this->_volume,this->_modle,this->_pch,this->_spd,this->_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setTTSSpeed
*
* @brief  Set TTS Voice speed parameter. 
*         
* @param  _spd - 0--100
*
* @retval none
*/
void Sim800Cmd::setTTSSpeed(int _spd)
{
	if((_spd>=0)&&(_spd<=100))
	{
		this->_spd    = (uchar)_spd;
		this->atTTS_SetParameter(this->_volume,this->_modle,this->_pch,this->_spd,this->_chl);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::stopTTSSpeak
*
* @brief  Stop TTS Voice speak. 
*         
* @param  void
*
* @retval none
*/
void Sim800Cmd::stopTTSSpeak(void)
{
	this->atsendcmd("AT+CTTS=0",1,500);
}

/**********************************************************************************************
* @fn     Sim800Cmd::sendTTSUCS2Compulsory
*
* @brief  Send data to TTS and compulsory speak. 
*         
* @param  *_s  - send data pointer
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::sendTTSUCS2Compulsory(char *_s, reModle_t _mdl)
{
	char *str;
	str = (char *)malloc(100);
	if(str == NULL)
	{
		return _FAULSE;
	}
	if(sim_Switch_State & 0x0020) //busy
	{
		this->stopTTSSpeak();
		sim_Switch_State &= (~0x0020);
	}
	if(sim_Switch_State & 0x0020)
	{
		free(str);
		return _FAULSE;
	}
	if(_mdl == UCS2)
	{
		strcpy(str,"AT+CTTS=1,");
	}
	else
	{
		strcpy(str,"AT+CTTS=2,");
	}
	strcat(str,_s);
	reqResult_t _rqs = this->atsendcmd(str,2,600);
	free(str);
	if(_rqs == RESULT_OK)
	{
		sim_Switch_State |= 0x0020;
		return _SUCCESS;
	}
	return _FAULSE;
}

/**********************************************************************************************
* @fn     Sim800Cmd::sendTTSUCS2Compulsory
*
* @brief  Send data to TTS and compulsory speak. 
*         
* @param  *_s  - send data pointer
*         _chl - speak channel
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::sendTTSUCS2Compulsory(char *_s, reModle_t _mdl, char _chl)
{
	char *str;
	str = (char*)malloc(100);
	if(str == NULL)
	{
		return _FAULSE;
	}
	this->setTTSParameter(this->_volume,this->_modle,this->_pch,this->_spd,(uchar)_chl);
	if(sim_Switch_State & 0x0020) 
	{
		this->stopTTSSpeak();
		sim_Switch_State &= (~0x0020);
	}
	if(sim_Switch_State & 0x0020)
	{
		free(str);
		return _FAULSE;
	}
	if(_mdl == UCS2)
	{
		strcpy(str,"AT+CTTS=1,");
	}
	else
	{
		strcpy(str,"AT+CTTS=2,");
	}
	strcat(str,_s);
	reqResult_t _rqs = this->atsendcmd(str,2,800);
	free(str);
	if(_rqs == RESULT_OK)
	{
		sim_Switch_State |= 0x0020;
		return _SUCCESS;
	}
	
	return _FAULSE;
}

/**********************************************************************************************
* @fn     Sim800Cmd::sendTTSUCS2Speak
*
* @brief  Send data to TTS and speak. 
*         
* @param  *_s  - send data pointer
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::sendTTSUCS2Speak(char *_s, reModle_t _mdl)
{
	char str[100]="AT+CTTS=1,";
	if(sim_Switch_State & 0x0020)
		return _FAULSE;
	else
	{
		if(_mdl == UCS2)
		{
			strcpy(str,"AT+CTTS=1,");
		}
		else
		{
			strcpy(str,"AT+CTTS=2,");
		}
		strcat(str,_s);
		reqResult_t _rqs = this->atsendcmd(str,2,600);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State |= 0x0020;
			return _SUCCESS;
		}
		return _FAULSE;
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::sendTTSUCS2Speak
*
* @brief  Send data to TTS and speak. 
*         
* @param  *_s  - send data pointer
*         _chl - speak channel
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::sendTTSUCS2Speak(char *_s, reModle_t _mdl, char _chl)
{
	char str[100]={'\0'};
	if(_mdl == UCS2)
	{
		strcpy(str,"AT+CTTS=1,");
	}
	else
	{
		strcpy(str,"AT+CTTS=2,");
	}
	if(sim_Switch_State & 0x0020)
	{
		return _FAULSE;
	}
	else
	{
		strcat(str,_s);
		reqResult_t _rqs = this->atsendcmd(str,2,600);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State |= 0x0020;
			return _SUCCESS;
		}
		return _FAULSE;
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::getTTSState
*
* @brief  Get TTS state. 
*         
* @param  void
*
* @retval 1 - busy  or   0 - available
*/
char Sim800Cmd::getTTSState(void)
{
	if(sim_Switch_State & 0x0020)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/************************************Tel function*********************************************/

/**********************************************************************************************
* @fn     Sim800Cmd::callReadCSQ
*
* @brief  Read sim800h modular signal strength(CSQ). 
*         
* @param  *pda - CSQ return data pointer
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::callReadCSQ(uchar *pda)
{
	if(sim_Switch_State & 0x0800)
	{
		*pda = CSQ_t;
		return _SUCCESS;
	}
	else
	{
		sim_Switch_State &= (~0x0800);
		_Serial.println("AT+CSQ");
		delay(500);
		if(sim_Switch_State & 0x0800)
		{
			*pda = CSQ_t;
			return _SUCCESS;
		}
		else
		{	
			return _FAULSE;
		}
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::getCallnumber
*
* @brief  Get sim800h modular call number. 
*         
* @param  *pda - return data pointer
*
* @retval none
*/
void Sim800Cmd::getCallnumber(char *pnbr)
{
	if((sim_Switch_State&0x0001) && (CallStatus == RING_CALL))
	{
		strcpy(pnbr,phonenumber);
		for(char i=0;i<12;i++)	phonenumber[i]='\0';
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::DisplayPhoneNumber
*
* @brief  Enable sim800h modular display phone number. 
*         
* @param  modle_t - "OPEN"" to enable or "CLOSE" to diable
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::DisplayPhoneNumber(reqmodle_t modle_t)
{
	if(modle_t == OPEN)
	{
		reqResult_t _rqs = this->atsendcmd("AT+CLIP=1",2,1000);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State |= 0x0001;
			return _SUCCESS;
		}
		else	return _FAULSE;
	}
	else if(modle_t == CLOSE)
	{
		reqResult_t _rqs = this->atsendcmd("AT+CLIP=0",2,1000);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State &= (~0x0001);
			return _SUCCESS;
		}
		else	return _FAULSE;
	}
	else	return _FAULSE;
}

/**********************************************************************************************
* @fn     Sim800Cmd::dialTelephoneNumber
*
* @brief  Make voice call. Switch CallStatus flag to no-AVAILABLE_CALL status.
*         
* @param  *_s - dial telephone number pointer
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::dialTelephoneNumber(char *_s)
{
	char *_str;
	_str = (char *)malloc(20);
	if(_str == NULL)
	{
		return _FAULSE;
	}
	strcpy(_str,"ATD");
	strcat(_str,_s);
	if(CallStatus != AVAILABLE_CALL)
	{
		free(_str);
		return _FAULSE;
	}
	CallStatus = BUSY_CALL;
	reqResult_t _rqs = this->atsendcmd(_str,1,1000);
	free(_str);
	if(_rqs == RESULT_OK)
	{
		CallStatus = DIAL_CALL;
		return _SUCCESS;
	}
	else
	{
		CallStatus = AVAILABLE_CALL;
		return _FAULSE;
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::cancelCall
*
* @brief  Hangs up the call. Switch CallStatus flag to AVAILABLE_CALL status.
*         
* @param  void
*
* @retval none
*/
void Sim800Cmd::cancelCall(void)
{
	this->atsendcmd("ATH0",1,100);
	sim_Switch_State &= (~0x4000);
	CallStatus = AVAILABLE_CALL;
}

/**********************************************************************************************
* @fn     Sim800Cmd::answerTelephone
*
* @brief  Answer the call. 
*         
* @param  void
*
* @retval none
*/
void Sim800Cmd::answerTelephone(void)
{
	this->atsendcmd("ATA",1,600);
}

/************************************DTMF function**************************************************/

/**********************************************************************************************
* @fn     Sim800Cmd::setDTMFenable
*
* @brief  Enable or disable the DTMF.
*         
* @param  modle_t - OPEN(enable) or CLOSE(disable)
*
* @retval _SUCCESS - success  or   _FAULSE - faulse
*/
char Sim800Cmd::setDTMFenable(reqmodle_t modle_t)
{
	if(modle_t == OPEN)
	{
		reqResult_t _rqs = this->atsendcmd("AT+DDET=1",2,1000);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State |= 0x0002;
			return _SUCCESS;
		}
		else	return _FAULSE;
	}
	else if(modle_t == CLOSE)
	{
		reqResult_t _rqs = this->atsendcmd("AT+DDET=0",2,1000);
		if(_rqs == RESULT_OK)
		{
			sim_Switch_State &= (~0x0002);
			return _SUCCESS;
		}
		else	return _FAULSE;
	}
	else	return _FAULSE;
}

/**********************************************************************************************
* @fn     Sim800Cmd::setDTMFHandlefunction
*
* @brief  Register the DTMF callback function. This function would callbacked when have DTMF data
*         
* @param  *pDf - callback function pointer
*
* @retval none
*/
void Sim800Cmd::setDTMFHandlefunction(void (*pDf)())
{
	pDTMFfun = pDf;
	sim_Switch_State |= 0x0008;
}

/**********************************************************************************************
* @fn     Sim800Cmd::getDTMFresult
*
* @brief  Get DTMF data and clear DTMF data flag.
*         
* @param  *pch - return data pointer
*
* @retval none
*/
void Sim800Cmd::getDTMFresult(char *pch)
{
	if(sim_Switch_State&0x0004)
	{
		*pch = dtmfreq;
		sim_Switch_State &= (~0x0004);
	}
}

/************************************SMS  function**************************************************/

/**********************************************************************************************
* @fn     Sim800Cmd::setSMSEnablePrompt
*
* @brief  Enable or disable the SMS prompt.
*         
* @param  modle_t - OPEN(enable) or CLOSE(disable)
*
* @retval none
*/
void Sim800Cmd::setSMSEnablePrompt(reqmodle_t modle_t)
{
	if(modle_t == OPEN)
	{
		sim_Switch_State |= 0x2000;
	}
	else if(modle_t == CLOSE)
	{
		sim_Switch_State &= (~0x2000);
	}
}

/**********************************************************************************************
* @fn     Sim800Cmd::setSMSHandlefunction
*
* @brief  Set SMS call back function.
*         
* @param  pDf - call back function pointer
*
* @retval none
*/
void Sim800Cmd::setSMSHandlefunction(void (*pDf)())
{
	pSMSfun = pDf;
}


void Sim800Cmd::getSMSID(char *str)
{
	strcpy(str,SMSID);
}
/**********************************************************************************************
* @fn     Sim800Cmd::sendSMS
*
* @brief  Send message. 
*         
* @param  pnber - telephoen number
*         pdata _ UCS2 data
*
* @retval reqResult - RESULT_OK/RESULT_ERROR/RESULT_UNKNOWN
*/
reqResult_t Sim800Cmd::sendSMS(char *pnber, char *pdata)
{
	char qts = 0x22;
	char i=1;
	char *str;
	str = (char *)malloc(60);
	if(str != NULL)
	{
		strcpy(str,"AT+CMGS=");
		str[8] = 0x22;
		str[9] = '\0';
		strcat(str,pnber);
		while(*str != '\0')
		{
			str++;
			i++;
		}
		*str = 0x22;
		str++;
		*str = '\0';
		str = str - i;
		sim_Switch_State |= 0x0010;
		reqResult_t _rqs = this->atsendcmd(str,1,1000);
		free(str);
		if(_rqs == RESULT_OK)
		{			
			_Serial.print(pdata);
			_Serial.write("\x1a");
			sim_Switch_State &= (~0x0010);
			return RESULT_OK;
		}
		else
		{
			sim_Switch_State &= (~0x0010);
			return RESULT_ERROR;
		}
	}
	return RESULT_ERROR;
}

/**********************************************************************************************
* @fn     Sim800Cmd::readSMS
*
* @brief  Read message. 
*         
* @param  padr - sequence  number
*         pdata _ return data pointer
*
* @retval reqResult - RESULT_OK/RESULT_ERROR/RESULT_UNKNOWN
*/
reqResult_t Sim800Cmd::readSMS(char *padr, char *pdata)
{
	char *str;
	str = (char *)malloc(20);
	if(str != NULL)
	{
		strcpy(str,"AT+CMGR=");
		strcat(str,padr);
		this->atsendcmd(str,1,1000);
		free(str);
		if(sim_Switch_State & 0x0010)
		{			
			sim_Switch_State &= (~0x0010);
			if(strlen(SMSbuf)<4)
			{
				return RESULT_ERROR;
			}
			strcpy(pdata,SMSbuf);
			
			return RESULT_OK;
		}
		else
		{
			return RESULT_ERROR;
		}
	}
	return RESULT_ERROR;
}

/**********************************************************************************************
* @fn     Sim800Cmd::deleteSMS
*
* @brief  Read message. 
*         
* @param  padr - sequence  number
*
* @retval none
*/
void Sim800Cmd::deleteSMS(char *padr)
{
	char *str;
	str = (char *)malloc(15);
	if(str != NULL)
	{
		strcpy(str,"AT+CMGD=");
		strcat(str,padr);
		this->atsendcmd(str,1,1000);
		free(str);
	}
}
/**********************************************************************************************
* @fn     *Sim800Cmd::num2str
*
* @brief  Digital into string.
*         
* @param  _pint1 - 0-100
*		  _pint2 - 0-3
*		  _pint3 - 0-100
*		  _pint4 - 0-100
*		  _pint5 - 0-1
*		  *pbuf  - return string pointer
*
* @retval return string pointer
*/

char *Sim800Cmd::num2str(uchar _pint1, uchar _pint2, uchar _pint3, uchar _pint4, uchar _pint5, char *pbuf)
{
	char str[25]={'\0'};
	char i=0;
	if(_pint5 == 0)
	{
		str[i] = '0';
		i++;
	}
	else
	{
		while(_pint5)
		{
			str[i] = _pint5%10 + 0x30;
			_pint5 = _pint5 / 10;
			i++;
		}
	}
	str[i] = 0x2c;	i++;
	if(_pint4 == 0)
	{	
		str[i] = 0x30;
		i++;
	}
	else{
		while(_pint4){
			str[i] = _pint4%10 + 0x30;
			_pint4 = _pint4 / 10;	i++;}
		}
	str[i] = 0x2c;	i++;
	if(_pint3 == 0)
	{	
		str[i] = 0x30;
		i++;
	}
	else{
		while(_pint3){
			str[i] = _pint3%10 + 0x30;
			_pint3 = _pint3 / 10;	i++;}
		}
	str[i] = 0x2c;	i++;
	if(_pint2 == 0)
	{	
		str[i] = 0x30;
		i++;
	}
	else{
		while(_pint2){
			str[i] = _pint2%10 + 0x30;
			_pint2 = _pint2 / 10;	i++;}
		}
	str[i] = 0x2c;	i++;
	if(_pint1 == 0)
	{
		str[i] = 0x30;
		i++;
	}
	else{
		while(_pint1){
			str[i] = _pint1%10 + 0x30;
			_pint1 = _pint1 / 10;	i++;}
		}
	str[i] = '\0';i--;
	while(i>=0){
		*pbuf = str[i];
		pbuf++;i--;
	}
	*pbuf = '\0';
	return pbuf;
}