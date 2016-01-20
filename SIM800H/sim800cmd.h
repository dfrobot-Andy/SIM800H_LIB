/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#ifndef sim800cmd_h
#define sim800cmd_h

#include "Arduino.h"
#include <inttypeds.h>
#include <stdio.h>
#include "HardwareSerial.h"
#include "atparameter.h"

#define Sim800CmdBUFMAX 5

extern short sim_Switch_State;
extern uchar CSQ_t;
extern char dtmfreq;
extern char phonenumber[15];
extern char SMSbuf[SMS_BUF_MAX];
extern char SMSID[5];

class TTSinfo
{
public:
  uchar _volume;
  uchar _modle;
  uchar _pch;
  uchar _spd;
  uchar _chl;
};

class Sim800Cmd: public TTSinfo
{
private:
	char atinitcheck();

	reqResult_t atsendcmd(char *pstr, char _cter, int _time);

	//char* num2str(uchar _pint, char *pbuf);
	char* num2str(uchar _pint1, uchar _pint2, uchar _pint3, uchar _pint4, uchar _pint5, char *pbuf);
	void atTTS_SetParameter(uchar volume, uchar modle, uchar pch, uchar spd, uchar chl);
	// char askTTSBroadcastState(void);
public:
	Sim800Cmd();
	Sim800Cmd(void (*pRf)());

	char sim800init(void);
	reqResult_t sim800open(void);
	void sim800close(void);
/*********************************TTS function*****************************************************/  
	void setTTSVolume(char _vme);//set TTS volume
	void setTTSVolume(int _vme);
	void setTTSSpeed(char _spd);
	void setTTSSpeed(int _spd);
	void setTTSVmeSpd(char _vme, char _spd);
	void setTTSVmeSpd(int _vme, int _spd);
	void setTTSParameter(char _vme, char _md, char _pch, char _spd, char _chl);
	void setTTSParameter(int _vme, int _md, int _pch, int _spd, int _chl);
	
	char sendTTSUCS2Compulsory (char *_s, reModle_t _mdl);
	char sendTTSUCS2Compulsory (char *_s, reModle_t _mdl, char _chl);
	char sendTTSUCS2Speak(char *_s, reModle_t _mdl);
	char sendTTSUCS2Speak(char *_s, reModle_t _mdl, char _chl);
	
	void stopTTSSpeak(void);//stop voice broadcast
	char getTTSState(void);
	
/*********************************Dial telephone function*****************************************************/  
	char callReadCSQ(uchar *pda);
	void getCallnumber(char *pnbr);
	char DisplayPhoneNumber(reqmodle_t modle_t);
	char dialTelephoneNumber(char *_s);
	void answerTelephone(void);
	void cancelCall(void);
	
/*********************************DTMF function***************************************************************/ 
	char setDTMFenable(reqmodle_t modle_t);
	void setDTMFHandlefunction(void (*pDf)());
	void getDTMFresult(char *pch);
	
/*********************************SMS  function***************************************************************/ 
	void setSMSEnablePrompt(reqmodle_t modle_t);
	void setSMSHandlefunction(void (*pDf)());
	void getSMSID(char *str);
	reqResult_t sendSMS(char *pnber, char *pdata);
	reqResult_t readSMS(char *padr, char *pdata);
	void deleteSMS(char *padr);
};

#endif