#include "PinControl.h"

void setLightControl(int Direction, int Mode, int LightFront, int LightBack, int FrontPin, int BackPin)
{
 if(0==Mode)//no automatic change
 {
  if(0==LightFront)
  {
    digitalWrite(FrontPin,LOW); 
  }
  else
  {
    digitalWrite(FrontPin,HIGH);
  }
  if(0==LightBack)
  {
    digitalWrite(BackPin,LOW); 
  }
  else
  {
    digitalWrite(BackPin,HIGH);
  }
 }
 else//automatic change
 {
  if(-1==Direction)//backward
  {
    if(0==LightFront)
    {
      digitalWrite(FrontPin,LOW); 
    }
    else
    {
      digitalWrite(FrontPin,HIGH);
    }
    digitalWrite(BackPin,LOW); 
  }
  else
  {
     if(0==LightBack)
    {
      digitalWrite(BackPin,LOW); 
    }
    else
    {
      digitalWrite(BackPin,HIGH);
    }
    digitalWrite(FrontPin,LOW); 
  }
 }
}

void setMotorControl(int Direction, int Speed, int PwmPin1,  int PwmPin2)
{
  if(1==Direction)//forward
  {
    analogWrite(PwmPin1, Speed*10);  //maxvalue is 1024, max Speed value is 100
    analogWrite(PwmPin2, 0);  //maxvalue is 1024, max Speed value is 100
    //digitalWrite(a3, HIGH);
  }
  else if(-1==Direction)//backward
  {
    analogWrite(PwmPin1, 0);  //maxvalue is 1024, max Speed value is 100
    analogWrite(PwmPin2, Speed*10);  //maxvalue is 1024, max Speed value is 100
    //digitalWrite(a3, HIGH);
  }
  else{//stop
    analogWrite(PwmPin1, 0);  //maxvalue is 1024, max Speed value is 100
    analogWrite(PwmPin2, 0);  //maxvalue is 1024, max Speed value is 100
  }
}
