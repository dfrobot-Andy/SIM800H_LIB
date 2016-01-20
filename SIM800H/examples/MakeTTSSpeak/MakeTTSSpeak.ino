/*
TTS Speaker

This sketch, for the Arduino SIM800H GPRS Shield,make Shield to speak.
Use UCS2 Code

created Dec 2014
by zcl

This example code is in the public domain.
*/

// libraries
#include <sim800cmd.h>

//initialize the library instance
//fundebug is an application callback function,when someon is calling.
Sim800Cmd sim800demo;

//the setup routine runs once when you press reset:
void setup()
{
  //initialize the digital pin as an output.
  pinMode(13,OUTPUT);
  //initialize SIM800H,return 1 when initialize success.
  while((sim800demo.sim800init()) == 0);
  delay(1000);
}

void loop()
{
  //set TTS parameters
  sim800demo.setTTSParameter(50,0,50,50,0);
  digitalWrite(13,HIGH);//turn the LED on by making the voltage HIGH
  //send data to speaker buffer use UCS2 and default speaker channel
  sim800demo.sendTTSUCS2Speak("0031002C5F00706FFF0C0030002C5173706F",UCS2);
  digitalWrite(13,LOW);//turn the LED off by making the voltage LOW  
  delay(3000);
  
  digitalWrite(13,HIGH);//turn the LED on by making the voltage HIGH
  //send data to speaker buffer use UCS2 and speaker channel 1
  sim800demo.sendTTSUCS2Compulsory("0031002C5F00706FFF0C0030002C5173706F",UCS2,1);
  digitalWrite(13,LOW);//turn the LED off by making the voltage LOW
  delay(3000);  
}
