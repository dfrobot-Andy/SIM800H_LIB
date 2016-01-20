/*
Make Voice Call

This sketch, for the Arduino SIM800H GPRS Shield,puts a voice call to
a remote phone number that you have entered.After initialization success ,
you can dial the telephone.

created Dec 2014
by zcl

This example code is in the public domain.
*/

// libraries
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

//the loop routine runs over and over again forever:
void loop()
{
  //Signal strength
  unsigned char csq = 0;
  //To obtain the signal strength, return 1 when obtain success.
 if( sim800demo.callReadCSQ(&csq) )
 {
     //Make Voice Call
     sim800demo.dialTelephoneNumber("15982373181;");
     while(1);
 }
 digitalWrite(13,HIGH);//turn the LED on by making the voltage HIGH
 delay(500);
 digitalWrite(13,LOW);//turn the LED off by making the voltage LOW
 delay(500);
}
 
//application callback function
void fundebug(void)
{
}

