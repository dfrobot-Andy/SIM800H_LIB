/*
Receive Voice Call

This sketch, for the Arduino SIM800H GPRS Shield,enable SIM800H Modular Caller ID, receives voice calls, get the 
Answer.

created Dec 2014
by zcl

This example code is in the public domain.
*/

//libraries
#include <sim800cmd.h>

//initialize the library instance
//fundebug is an application callback function,when someon is calling.
Sim800Cmd sim800demo(fundebug);

//the setup routine runs once when you press reset:
void setup()
{
    //initialize the digital pin as an output.
    pinMode(13,OUTPUT);
    //initialize SIM800H,return 1 when initialize success.
    while((sim800demo.sim800init()) == 0);
}

void loop()
{
 digitalWrite(13,HIGH);//turn the LED on by making the voltage HIGH
 delay(200);
 digitalWrite(13,LOW);//turn the LED off by making the voltage LOW
 delay(200);
}
 
//application callback function
//Note that not too much to handle tasks in this function
void fundebug(void)
{
    //answer the call
    sim800demo.answerTelephone();
}

