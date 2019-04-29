#ifndef ROCRAILSERVER_H
#define ROCRAILSERVER_H
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "ControlVariables.h"
#include "PinControl.h"

#define MAX_SRV_CLIENTS 2

//TCP-Server for Rocrail and Smartphoneapps
static WiFiServer RocrailServer(4444);
static WiFiClient RocrailServerClients[MAX_SRV_CLIENTS];

static const String SpeedString="V=";
static const String DirectionString="dir=";
static const String LightFrontString="f0=";
static const String LightString="f1=";
static const String LightBackString="f2=";
static const String TrueString="true";

void initRocrailServer(int port);
void handleRocrailServer(void);

#endif
