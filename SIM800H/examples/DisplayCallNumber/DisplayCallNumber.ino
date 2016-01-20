// libraries
#include <sim800cmd.h>

//initialize the library instance
//fundebug is an application callback function,when someon is calling.
Sim800Cmd sim800demo(fundebug1);

//phone number buffer
char str[15] = {'\0'};

//the setup routine runs once when you press reset:
void setup()
{
    //initialize the digital pin as an output.
    pinMode(13,OUTPUT);
    //initialize SIM800H,return 1 when initialize success.
    while((sim800demo.sim800init()) == 0);
    //enable SIM800H Modular Caller ID
    sim800demo.DisplayPhoneNumber(OPEN);
}

//the loop routine runs over and over again forever:
void loop()
{
  
}
//application callback function
//Note that not too much to handle tasks in this function
void fundebug1(void)
{
  //get the phone number
  sim800demo.getCallnumber(str);
  
  if(memcmp(str,"15982373181",11) == 0)
    sim800demo.answerTelephone(); //answer
  else
    sim800demo.cancelCall();//hangs up
}
