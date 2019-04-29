#ifndef CONTROLVARIABLES_H
#define CONTROLVARIABLES_H

//Pins
static int Pwm1 = D1; //Nodemcu PWM pin 
static int Pwm2 = D2; //Nodemcu PWM pin
static int ledfront = D5;  //Gpio-14 of nodemcu esp8266  
static int ledback = D6;  //Gpio-12 of nodemcu esp8266  

//Locomotive Control
static int Direction;
static int Speed;
static int LightMode;//0 = No automatic change , 1 = automatic change with activated leds
static int LightFrontOn;
static int LightBackOn;

#endif
