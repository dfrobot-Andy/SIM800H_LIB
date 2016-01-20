/*
www.dfrobot.com
msTimer

created Dec 2014
by zcl
*/

#include <msTimer.h>
#include "atparameter.h"

#if defined(__AVR_ATmega32U4__)
	#define TIMER_INTR_NAME      TIMER1_OVF_vect
#else	
	#define TIMER_INTR_NAME      TIMER2_OVF_vect
#endif
unsigned long Timerszcl::msecs;
void (*Timerszcl::func)();
void (*Timerszcl::pRCB)();
void (*Timerszcl::pEvent)();
void (*Timerszcl::pDTMFfun)();
void (*Timerszcl::pSMSfun)();
volatile unsigned long Timerszcl::count;
volatile unsigned long Timerszcl::Rcount;
volatile unsigned long Timerszcl::Tcount;
volatile unsigned long Timerszcl::DTMFcount;
volatile char Timerszcl::overflowing;
volatile unsigned int Timerszcl::tcnt2;

/**********************************************************************************************
* @fn     Timerszcl::set
*
* @brief  Set system timer,registrate uart function. Start the timer 2.
*
* @param  ms - system timer
*         *f - uart callback function
*
* @retval none.
*/
void Timerszcl::set(unsigned long ms, void (*f)()) {
	float prescaler = 0.0;
	
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	TIMSK2 &= ~(1<<TOIE2);
	TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
	TCCR2B &= ~(1<<WGM22);
	ASSR &= ~(1<<AS2);
	TIMSK2 &= ~(1<<OCIE2A);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2B |= (1<<CS22);
		TCCR2B &= ~((1<<CS21) | (1<<CS20));
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2B |= (1<<CS21);
		TCCR2B &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR2B |= ((1<<CS22) | (1<<CS20));
		TCCR2B &= ~(1<<CS21);
		prescaler = 128.0;
	}
#elif defined (__AVR_ATmega8__)
	TIMSK &= ~(1<<TOIE2);
	TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
	TIMSK &= ~(1<<OCIE2);
	ASSR &= ~(1<<AS2);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2 |= (1<<CS22);
		TCCR2 &= ~((1<<CS21) | (1<<CS20));
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR2 |= ((1<<CS22) && (1<<CS20));
		TCCR2 &= ~(1<<CS21);
		prescaler = 128.0;
	}
#elif defined (__AVR_ATmega128__)
	TIMSK &= ~(1<<TOIE2);
	TCCR2 &= ~((1<<WGM21) | (1<<WGM20));
	TIMSK &= ~(1<<OCIE2);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR2 |= ((1<<CS21) | (1<<CS20));
		TCCR2 &= ~(1<<CS22);
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 256
		TCCR2 |= (1<<CS22);
		TCCR2 &= ~((1<<CS21) | (1<<CS20));
		prescaler = 256.0;
	}
#elif defined (__AVR_ATmega32U4__)
	TIMSK1 &= ~(1<<TOIE1);
	TCCR1A &= ~((1<<WGM11) | (1<<WGM10));
	TCCR1B &= ~((1<<WGM12) | (1<<WGM13));
	TCCR1C = 0;
	TIMSK1 &= ~(1<<OCIE1A);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {	// prescaler set to 64
		TCCR1B |=  (1<<CS10)  | (1<<CS11);
		TCCR1B &= ~(1<<CS12);
		prescaler = 64.0;
	} else if (F_CPU < 1000000UL) {	// prescaler set to 8
		TCCR1B |= (1<<CS11);
		TCCR1B &= ~((1<<CS12) | (1<<CS10));
		prescaler = 8.0;
	} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR1B |= (1<<CS12);
		TCCR1B &= ~((1<<CS11) | (1<<CS10));
		prescaler = 128.0;
	}
#endif
	
	tcnt2 = 256 - (int)((float)F_CPU * 0.001 / prescaler);
	
	if (ms == 0)
		msecs = 1;
	else
		msecs = ms;
		
	func = f;
}

/**********************************************************************************************
* @fn     Timerszcl::setRingCB
*
* @brief  Registrate ring callback function.
*
* @param  *pt - ring callback function
*         
* @retval none.
*/
void Timerszcl::setRingCB(void (*pt)())
{
	pEvent= pt;
}

/**********************************************************************************************
* @fn     Timerszcl::setRingCBS
*
* @brief  Registrate ring callback function.
*
* @param  *pf - ring callback function
*		  *pt - task callback function
*         
* @retval none.
*/
void Timerszcl::setRingCBS(void (*pf)(), void (*pt)())
{
	pRCB = pf;
	pEvent= pt;
}

/**********************************************************************************************
* @fn     Timerszcl::start
*
* @brief  Start timer2.
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::start() {
	count = 0;
	overflowing = 0;
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	TCNT2 = tcnt2;
	TIMSK2 |= (1<<TOIE2);
#elif defined (__AVR_ATmega128__)
	TCNT2 = tcnt2;
	TIMSK |= (1<<TOIE2);
#elif defined (__AVR_ATmega8__)
	TCNT2 = tcnt2;
	TIMSK |= (1<<TOIE2);
#elif defined (__AVR_ATmega32U4__)
	TCNT1H = 255;
	TCNT1L = 0;
	TIMSK1 |= (1<<TOIE1);
#endif
}

/**********************************************************************************************
* @fn     Timerszcl::stop
*
* @brief  Stop timer2.
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::stop() {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	TIMSK2 &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega128__)
	TIMSK &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega8__)
	TIMSK &= ~(1<<TOIE2);
#elif defined (__AVR_ATmega32U4__)
	TIMSK1 |= (1<<TOIE1);
#endif
}

/**********************************************************************************************
* @fn     Timerszcl::_uartoverflow
*
* @brief  callback uart function every 100 millisecond
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::_uartoverflow() {
	count += 1;
	
	if (count >= msecs && !overflowing) {
		overflowing = 1;
		count = 0;
		(*func)();
		overflowing = 0;
	}
}

/**********************************************************************************************
* @fn     Timerszcl::_ringoverflow
*
* @brief  callback ring function when some is calling
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::_ringoverflow()
{
	if(sim_Switch_State&0x8000)
	{
		Rcount += 1;	
		if(Rcount >= 100)
		{
			Rcount = 0;
			if( sim_Switch_State&0x4000)
			{
				CallStatus = RING_CALL;
				sim_Switch_State &= (~0x4000);
				pRCB();		
			}
		}
	}
}

/**********************************************************************************************
* @fn     Timerszcl::_taskoverflow
*
* @brief  callback task function every 100 millisecond
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::_taskoverflow(void)
{
	Tcount += 1;
	if(Tcount >= 100)
	{
		Tcount = 0;
		pEvent();
	}
}

/**********************************************************************************************
* @fn     Timerszcl::_reqDTMFoverflow
*
* @brief  callback task function when have DTMF data.
*
* @param  void
*         
* @retval none.
*/
void Timerszcl::_reqDTMFoverflow()
{
	if((sim_Switch_State&0x0008) &&(sim_Switch_State&0x0004))
	{
		DTMFcount += 1;
		if(DTMFcount>100)
		{
			DTMFcount = 0;
			pDTMFfun();
		}
	}
}

/**********************************************************************************************
* @fn     ISR
*
* @brief  interrupt function,call processing function.
*
* @param  TIMER2_OVF_vect - timer2 interrup
*         
* @retval none.
*/
ISR(TIMER_INTR_NAME) {
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
	TCNT2 = Timerszcl::tcnt2;
#elif defined (__AVR_ATmega128__)
	TCNT2 = Timerszcl::tcnt2;
#elif defined (__AVR_ATmega8__)
	TCNT2 = Timerszcl::tcnt2;
#elif defined (__AVR_ATmega32U4__)
	TCNT1H = 255;
	TCNT1L = 0;
#endif
	Timerszcl::_uartoverflow();
	Timerszcl::_ringoverflow();
	Timerszcl::_taskoverflow();
	Timerszcl::_reqDTMFoverflow();
}