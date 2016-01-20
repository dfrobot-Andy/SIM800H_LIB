/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#ifndef doEvent_h
#define doEvent_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "atparameter.h"

char doAT(char *pEventCmd);
char doOK(char *pEventCmd);
char doError(char *pEventCmd);
char doRing(char *pEventCmd);
char doDTMF(char *pEventCmd);
char doCTTS(char *pEventCmd);
char doBusy(char *pEventCmd);
char doNocarrier(char *pEventCmd);
char doSMS(char *pEventCmd);
char doReadSMS(char *pEventCmd);
char doPower(char *pEventCmd);
char doReadCSQ(char *pEventCmd);
char doSMSReady(char *pEventCmd);

char eventMatchFun(uchar _len, char *pbuf);
char execEventCmd(char *pEventCmd);

#endif