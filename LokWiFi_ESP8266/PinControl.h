#ifndef PINCONTROL_H
#define PINCONTROL_H

#include <ESP8266WiFi.h>


void setLightControl(int Direction, int Mode, int LightFront, int LightBack, int FrontPin, int BackPin);
void setMotorControl(int Direction, int Speed, int PwmPin1,  int PwmPin2);

#endif
