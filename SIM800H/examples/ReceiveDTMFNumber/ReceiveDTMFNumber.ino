/*
Receive DTMF Number

This sketch, for the Arduino SIM800H GPRS Shield,enable SIM800H Modular Caller ID, receives voice calls, get the 
Answer,enable SIM800H Modular DTMF,get DTMF number.

created Dec 2014
by zcl

This example code is in the public domain.
*/

//libraries
#include <sim800cmd.h>

//initialize the library instance
//fundebug1 is an application callback function,when someon is calling.
Sim800Cmd sim800demo(fundebug1);

//the setup routine runs once when you press reset:
void setup()
{
    //initialize the digital pin as an output.
    pinMode(13,OUTPUT);
    //turn the LED off by making the voltage LOW
    digitalWrite(13,LOW);
    //initialize SIM800H,return 1 when initialize success.
    while((sim800demo.sim800init()) == 0);
    delay(1000);
    //enable DTMF
    sim800demo.setDTMFenable(OPEN);
    //registration fundebug2 function
    sim800demo.setDTMFHandlefunction(fundebug2);
}

//the loop routine runs over and over again forever:
void loop()
{
  
}

//application callback function,when Someone is calling,callback this function
//Note that not too much to handle tasks in this function
void fundebug1(void)
{
   sim800demo.answerTelephone();
}

//application callback function,when a DTMF data input,callback this function
//Note that not too much to handle tasks in this function
void fundebug2(void)
{
  char x = 0;
  //get DTMF data
  sim800demo.getDTMFresult(&x);

  if(x == '1')
    digitalWrite(13,HIGH); //turn the LED on by making the voltage HIGH
  else if(x == '0')
    digitalWrite(13,LOW);//turn the LED off by making the voltage LOW
}
