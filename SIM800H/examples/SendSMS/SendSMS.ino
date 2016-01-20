/*
TTS Speaker

This sketch, for the Arduino SIM800H GPRS Shield,send a short message to telephone
After initialization success ,enable prompt SMS.

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
    //initialize SIM800H,return 1 when initialize success.
    while((sim800demo.sim800init()) == 0);
    delay(1000);
    //enable SMS prompt
    sim800demo.setSMSEnablePrompt(OPEN);
}
void loop()
{
    //send message to telephone,use UCS2 code
    sim800demo.sendSMS("00310035003900380032003300370033003100380031","4F60597DFF0C6B228FCE4F7F752800530049004D0038003000300048");
    //while, do not return
    while(1);
}

//application callback function
//Note that not too much to handle tasks in this function
void fundebug(void)
{
}
