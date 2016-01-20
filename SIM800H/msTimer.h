/*
www.dfrobot.com
sim800cmd

created Dec 2014
by zcl
*/

#ifndef msTimer_h
#define msTimer_h

#include <avr/interrupt.h>

namespace Timerszcl {
	extern unsigned long msecs;
	extern void (*func)();
	extern void (*pEvent)();
	extern void (*pRCB)();
	extern void (*pDTMFfun)();
	extern void (*pSMSfun)();
	extern volatile unsigned long count;
	extern volatile unsigned long Rcount;
	extern volatile unsigned long Tcount;
	extern volatile unsigned long DTMFcount;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;

	void set(unsigned long ms, void (*f)());
	void setRingCB(void (*pt)());
	void setRingCBS(void (*pf)(), void (*pt)());
	void start();
	void stop();
	
	void _reqDTMFoverflow();
	void _uartoverflow();
	void _ringoverflow();
	void _taskoverflow();
}

#endif
