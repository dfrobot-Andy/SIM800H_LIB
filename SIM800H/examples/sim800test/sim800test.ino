//libraries
#include <sim800cmd.h>

//initialize the library instance
//fundebug is an application callback function,when someon is calling.
Sim800Cmd sim800demo(fundebug);

//return DTMF value
char rqt = 0;

//the setup routine runs once when you press reset:
void setup()
{
   pinMode(13,OUTPUT);
    //turn the LED off by making the voltage LOW
    digitalWrite(13,LOW);
    while((sim800demo.sim800init()) == 0);
    delay(1000);
    sim800demo.setSMSEnablePrompt(OPEN);
    //enable DTMF
    sim800demo.setDTMFenable(OPEN);
    //registration fundebug2 function
    sim800demo.setDTMFHandlefunction(fundebug2);
}

//the loop routine runs over and over again forever:
void loop()
{
  //calling
  if(rqt == 1)
  {  
     rqt = 0;
     sim800demo.sendTTSUCS2Compulsory("003253D1900177ED4FE1FF0C00315F00706FFF0C00305173706F",UCS2);
  }
  else if(rqt == '3')
  {
    rqt = 0;
    sim800demo.sendTTSUCS2Compulsory("6B63572853D1900177ED4FE1",UCS2);
    delay(2000);
    sim800demo.cancelCall();
    delay(2000);
    sim800demo.sendSMS("00310035003900380032003300370033003100380031","4F60597DFF0C6B228FCE4F7F752800530049004D0038003000300048");
  }
  else if(rqt == '1')
  {
    rqt = 0;
    digitalWrite(13,HIGH);//turn the LED off by making the voltage LOW
    sim800demo.sendTTSUCS2Compulsory("706F5DF25F00",UCS2);
  }
  else if(rqt == '0')
  {
    rqt = 0;
    digitalWrite(13,LOW);//turn the LED off by making the voltage LOW
    sim800demo.sendTTSUCS2Compulsory("706F5DF25173",UCS2);
  }
}

//application callback function
void fundebug(void)
{
   rqt = 1;
   sim800demo.answerTelephone();
}

//application callback function
void fundebug2(void)
{
  //get DTMF data
  sim800demo.getDTMFresult(&rqt);
}
