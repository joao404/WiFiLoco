/*
  Autor: Marcel Maage
  Stand: 27.04.2019
  */

/*
 * TODO:
 * EEPROM auslesen und einlesen (done)
 * Ansteuerung Fahrtrichtung und Licht (done)
 * PWM (done)
 * Rocrail Schnittstelle(noch nicht fertig)
 * Vollbildmodus(done)
 */


//includes
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "Webpage.h"
#include "ControlVariables.h"
#include "PinControl.h"
#include "RocrailServer.h"


//defines

#define MaxConnectTries 5//5 secondes

#define LOKDEBUG


//WIFI
#ifndef APSSID
#define APSSID "DummyLok"
#define APPSK  "12345678"
#endif

#ifndef STASSID
#define STASSID "DummyLok"
#define STAPSK  "12345678"
#endif

#define HOSTSIZE  30
#define SSIDSIZE  30
#define PWSIZE    30

//type def
typedef struct{
  byte writen;//if value is 1, the values have been writen
  char mdns[HOSTSIZE];
  char APssid[SSIDSIZE];
  char APpassword[PWSIZE];
  char STAssid[SSIDSIZE];
  char STApassword[PWSIZE];
  IPAddress IP;
  IPAddress NM;
  IPAddress Rtr;
  int RocrailPort;
  
} EEConfigData;




//variables
const char* www_username = "lokadmin";
const char* www_password = "erlangen2019";

//Webserver for Configuration and Smartphonecontrol
ESP8266WebServer webserver(80);

EEConfigData ConfigData;


//functions
void initWebserver(void);



//write to memory
void write_EEPROM(void* x,int pos, int len){
  for(int n=pos;n<(len+pos);n++){
     EEPROM.write(n,((byte*)x)[n-pos]);//später update
  }
  EEPROM.commit();
}

//write to memory
void read_EEPROM(void* x,int pos, int len){
  for(int n=pos;n<(len+pos);n++){
     ((byte*)x)[n-pos]=EEPROM.read(n);
  }
}




//////////////////////////////////////////////////////////////////////////////////////////////
//SETUP
//////////////////////////////////////////////////////////////////////////////////////////////
void setup(void) {

  int i=1;
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  EEPROM.begin(512);//Starting and setting size of the EEPROM

  //PinMode
  pinMode(ledfront, OUTPUT);     
  pinMode(ledback, OUTPUT);  


  //change PWM Frequency
  analogWriteFreq(5000);

  analogWrite(Pwm1, 0);  //maxvalue is 1024, max Speed value is 100
  analogWrite(Pwm2, 0);  //maxvalue is 1024, max Speed value is 100
  
  //Read Data from EEPROM
  read_EEPROM((void*)&ConfigData,0,sizeof(EEConfigData));

  //check if Data has never been writen or if there are values
  if(1!=ConfigData.writen){
    strncpy(ConfigData.mdns,APSSID,HOSTSIZE);
    strncpy(ConfigData.STAssid,STASSID,SSIDSIZE);
    strncpy(ConfigData.STApassword,STAPSK,PWSIZE);
    strncpy(ConfigData.APssid,APSSID,SSIDSIZE);
    strncpy(ConfigData.APpassword,APPSK,PWSIZE);
    ConfigData.IP=IPAddress(192,168,2,95);
    ConfigData.NM=IPAddress(255,255,255,0);
    ConfigData.Rtr=IPAddress(192,168,2,1);
    ConfigData.RocrailPort=4444;
    
  }

  //check if station is available
  Serial.print("Connect to ");
  Serial.println(ConfigData.STAssid);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ConfigData.STAssid, ConfigData.STApassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Versuch ");
    Serial.println(i);
    if(i++>MaxConnectTries)
      break;
  }
  
  if(WiFi.status() != WL_CONNECTED) {//no connection to Station
    WiFi.disconnect();
    //set AP Data

    Serial.print("Configuring access point ");
    Serial.println(ConfigData.APssid);
    WiFi.softAP(ConfigData.APssid, ConfigData.APpassword);  
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  else
  {
    Serial.print("STA IP address: ");
    Serial.println(WiFi.localIP());
  }

  //a network is established

  //MDNS
  MDNS.begin(ConfigData.mdns);

  //Webserver
  initWebserver();
  MDNS.addService("http", "tcp", 80);

  //RocrailServer
  initRocrailServer(ConfigData.RocrailPort);
  //MDNS.addService("http", "tcp", ConfigData.RocrailPort);

  Serial.println("ESP ready");
}

void loop(void) {
  webserver.handleClient();
  MDNS.update();

  handleRocrailServer();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//functions
void initWebserver(void)
{
  //Startpage
  
    webserver.on("/", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.println("HTTP:/");
      #endif
      webserver.sendHeader("Connection", "close");
      webserver.send_P(200, "text/html", WEBPAGE_TABLE[0].page_pointer);
    });

    webserver.on("/index.html", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.print("HTTP:/");
      Serial.println(WEBPAGE_TABLE[0].filename);
      #endif
      webserver.sendHeader("Connection", "close");
      webserver.send_P(200, "text/html", WEBPAGE_TABLE[0].page_pointer);
    });

    webserver.on("/config.html", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.print("HTTP:/");
      Serial.println(WEBPAGE_TABLE[1].filename);
      #endif
      webserver.sendHeader("Connection", "close");
      webserver.send_P(200, "text/html", WEBPAGE_TABLE[1].page_pointer);
    });

    webserver.on("/status.html", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.print("HTTP:/");
      Serial.println(WEBPAGE_TABLE[2].filename);
      #endif
      if (!webserver.authenticate(www_username, www_password)) {
        webserver.requestAuthentication();
      }
      else
      {
        webserver.sendHeader("Connection", "close");
        webserver.send_P(200, "text/html", WEBPAGE_TABLE[2].page_pointer);
      }
    });

    webserver.on("/styles.css", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.print("HTTP:/");
      Serial.println(WEBPAGE_TABLE[3].filename);
      #endif
      webserver.sendHeader("Connection", "close");
      webserver.send_P(200, "text/css", WEBPAGE_TABLE[3].page_pointer);
    });


    //AjaxControlDataExchange
    webserver.on("/AjaxControlDataExchange", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.println("HTTP:/AjaxControlDataExchange");
      #endif
            
      //Motor
      if (webserver.hasArg("SpeedValue"))
      {
        Speed = (webserver.arg("SpeedValue")).toInt();
        #ifdef LOKDEBUG
        Serial.print("S:");
        Serial.println(Speed);
        #endif
      }
      if (webserver.hasArg("DirectionValue"))
      {
        Direction = (webserver.arg("DirectionValue")).toInt();
        #ifdef LOKDEBUG
        Serial.print("D:");
        Serial.println(Direction);
        #endif
      }
      //Light
      if (webserver.hasArg("LightMode"))//Mode
      {
        LightMode = (webserver.arg("LightMode")).toInt();
        #ifdef LOKDEBUG
        Serial.print("LightMode:");
        Serial.println(LightMode);
        #endif
      }
      if (webserver.hasArg("LightFront"))//FrontOn
      {
        LightFrontOn = (webserver.arg("LightFront")).toInt();
        #ifdef LOKDEBUG
        Serial.print("LightFront:");
        Serial.println(LightFrontOn);
        #endif
      }
      if (webserver.hasArg("LightBack"))//FrontOn
      {
        LightBackOn = (webserver.arg("LightBack")).toInt();
        #ifdef LOKDEBUG
        Serial.print("LightBack:");
        Serial.println(LightBackOn);
        #endif
      }


      setLightControl(Direction, LightMode, LightFrontOn, LightBackOn, ledfront, ledback);
      setMotorControl(Direction,Speed,Pwm1,Pwm2);

      String message = "<?xml version = \"1.0\" ?><inputs>";
      message += "<SPEED>";
      message += String(Speed);
      message += "</SPEED>";
      message += "<DIRECTION>";
      message += String(Direction);
      message += "</DIRECTION>"; 
      message += "</inputs>"; 

      webserver.sendHeader("Connection", "close");
      webserver.send(200, "text/xml", message);
    });

    webserver.on("/AjaxConfigDataExchange", HTTP_GET, []() {
      #ifdef LOKDEBUG
      Serial.println("HTTP:/AjaxConfigDataExchange");
      for ( uint8_t i = 0; i < webserver.args(); i++ ) {
        Serial.print(webserver.argName ( i ));
        Serial.print("=");
        Serial.println(webserver.arg ( i ));
      }
      #endif

      if (webserver.hasArg("mDNSName"))
      {
        (webserver.arg("mDNSName")).toCharArray(ConfigData.mdns,HOSTSIZE);
        #ifdef LOKDEBUG
        Serial.print("host:");
        Serial.println(ConfigData.mdns);
        #endif
        ConfigData.writen=1;
      }
      if (webserver.hasArg("APName"))
      {
        (webserver.arg("APName")).toCharArray(ConfigData.APssid,HOSTSIZE);
        #ifdef LOKDEBUG
        Serial.print("APssid:");
        Serial.println(ConfigData.APssid);
        #endif
        ConfigData.writen=1;
      }
      if (webserver.hasArg("APPwd"))
      {
        (webserver.arg("APPwd")).toCharArray(ConfigData.APpassword,HOSTSIZE);
        #ifdef LOKDEBUG
        Serial.print("APpwd:");
        Serial.println(ConfigData.APpassword);
        #endif
        ConfigData.writen=1;
      }
      if (webserver.hasArg("ROCPort"))
      {
        ConfigData.RocrailPort=webserver.arg("ROCPort").toInt();
        #ifdef LOKDEBUG
        Serial.print("RocrailPort:");
        Serial.println(ConfigData.RocrailPort);
        #endif
        ConfigData.writen=1;
      }

      if(1==ConfigData.writen)
      {
        write_EEPROM((void*)&ConfigData,0,sizeof(EEConfigData));//update EEPROM, if something changed
      }
     

      String message = "<?xml version = \"1.0\" ?><inputs>";
      message += "<DATE>";
      message += __DATE__;
      message += "</DATE>";
      message += "<TIME>";
      message += __TIME__;
      message += "</TIME>"; 
      message += "<VERSION>";
      message += __VERSION__;
      message += "</VERSION>"; 
      message += "<VERSION>";
      message += __VERSION__;
      message += "</VERSION>"; 
      message += "<mDNSName>";
      message += ConfigData.mdns;
      message += "</mDNSName>";
      message += "<APName>";
      message += ConfigData.APssid;
      message += "</APName>";
      message += "<APPwd>";
      message += ConfigData.APpassword;
      message += "</APPwd>";
      
      message += "<STName>";
      message += ConfigData.STAssid;
      message += "</STName>";
      message += "<STPwd>";
      message += ConfigData.STApassword;
      message += "</STPwd>";
/*
      message += "<IPAdr>";
      message += ConfigData.IP.toString();
      message += "</IPAdr>";
      message += "<NETMSK>";
      message += ConfigData.NM.toString();
      message += "</NETMSK>";
      message += "<IPRtr>";
      message += ConfigData.Rtr.toString();
      message += "</IPRtr>";
      */
      message += "<ROCPort>";
      message += String(ConfigData.RocrailPort);
      message += "</ROCPort>";
      
      message += "</inputs>";   


      /*
      "<?xml version = \"1.0\" ?><inputs>"
        "<DATE>%s</DATE>"
        "<TIME>%s</TIME>"
        "<VERSION>%s</VERSION>"
        "<mDNSName>%s</mDNSName>"
        "<RouterSsid>%s</RouterSsid>"
        "<RouterPwd>%s</RouterPwd>"
        "<APName>%s</APName>"
        "<APPwd>%s</APPwd>"
        "<DataServerURL>%s</DataServerURL>"
        "<DataServerPort>%i</DataServerPort>"
        "<UpgradeServerURL>%s</UpgradeServerURL>"
        "<Netmask>%s</Netmask>"
        "<MeshID>%s</MeshID>"
        "<MeshPwd>%s</MeshPwd>"
        "</inputs>",
      */
      webserver.sendHeader("Connection", "close");
      webserver.send(200, "text/xml", message);
    });

 
  //Updatefunction
    webserver.on("/update", HTTP_POST, []() {
      webserver.sendHeader("Connection", "close");
      webserver.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = webserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });


    webserver.onNotFound([]() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webserver.uri();
    Serial.println(webserver.uri());
    message += "\nMethod: ";
    message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webserver.args();
    message += "\n";
    for ( uint8_t i = 0; i < webserver.args(); i++ ) {
        message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
        Serial.print(webserver.argName ( i ));
        Serial.print("=");
        Serial.println(webserver.arg ( i ));
    }
    webserver.send ( 404, "text/plain", message );
    });

     webserver.begin();
     Serial.println("Webserver started");
  
}
