#include "RocrailServer.h"
#include "ControlVariables.h"

void initRocrailServer(int port)
{
  RocrailServer.begin(port);
  RocrailServer.setNoDelay(true);
}


void handleRocrailServer(void)
{

   int StringLength;
   int StartPosition=0,EndPosition=0;
  //check if there are any new clients
  if (RocrailServer.hasClient()){
    //Serial.println("New Client");
    for(int i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!RocrailServerClients[i] || !RocrailServerClients[i].connected()){//ist einer meiner Verbindungssockets nicht besetzt oder verbunden?
        if(RocrailServerClients[i]) RocrailServerClients[i].stop();//wenn wir hier sind und er ist vorhanden (ungleich null) aber ist nicht verbunden,dann  beenden wir die Verbindung
        RocrailServerClients[i] = RocrailServer.available();//der neue Socket wird mit der speicherstelle verbunden
        //Serial.println("New Client");
        continue;
      }
    }
        
    //no free/disconnected spot so reject
    WiFiClient serverClient = RocrailServer.available();
    //Serial.println("Client versucht eine Verbindung aufzubauen");
    serverClient.stop();
  }
  //check clients for data
  for(int i = 0; i < MAX_SRV_CLIENTS; i++){
    if (RocrailServerClients[i] && RocrailServerClients[i].connected()){
      //an dieser Stelle kann ich eine Nachricht zurueck senden
      //uint8_t sbuf[len];
      //RocrailServerClients[i].write(sbuf,len);
      
      if(RocrailServerClients[i].available()){
        Serial.println("message:");
        String message=RocrailServerClients[i].readStringUntil('>');
        Serial.println(message);
        while(RocrailServerClients[i].available())
        {
          char a = RocrailServerClients[i].read();
        }
        /*
        while(RocrailServerClients[i].available()) 
        {
          //hier muss ich mein Telegramm verarbeiten und die benoetigten Daten senden
          char a = RocrailServerClients[i].read();
          if('\0'==a)
            break;
          message+=String(a);
        }
        Serial.println(message);
        */
        //message+='\0';
        //Serial.write('\0');
        //Serial.println(StringLength);
        //myString.indexOf(val, from)
        //myString.substring(from, to)
        //Verarbeitung der Nachricht
        
        //Geschwindigkeit
        //Position=FindString(DataMessage,StringLength,0,StringLength,SpeedString);
        StartPosition=message.indexOf(SpeedString,0);
         //Serial.print(Position);
        if(StartPosition!=-1)
        {
          StartPosition=StartPosition+SpeedString.length()+1;
          EndPosition=message.indexOf("\"", StartPosition);
          //Serial.println(StartPosition);
          //Serial.println(EndPosition);
          Speed=message.substring(StartPosition,EndPosition).toInt();
          Serial.print("S:");
          Serial.println(Speed);
          //Serial.println(message.substring(StartPosition,EndPosition));
        }

        //Richtung
        //Position=FindString(DataMessage,StringLength,0,StringLength,DirectionString);
        StartPosition=message.indexOf(DirectionString,0);
        if(StartPosition!=-1)
        {
          StartPosition=StartPosition+DirectionString.length()+1;
          EndPosition=message.indexOf("\"", StartPosition);
          //Serial.println(message.substring(StartPosition,EndPosition));
          
          if(0!=message.substring(StartPosition,EndPosition).compareTo(TrueString))
            Direction=1;
          else
            Direction=-1;
          Serial.print("D:");
          Serial.println(Direction);         
        }
        
        //Licht gesamt
        StartPosition=message.indexOf(LightString,0)+1;
        if(StartPosition!=-1)
        {
          StartPosition=StartPosition+LightString.length();
          EndPosition=message.indexOf("\"", StartPosition);
          //Serial.println(message.substring(StartPosition,EndPosition));
          
          if(0!=message.substring(StartPosition,EndPosition).compareTo(TrueString))
            LightMode=1;
          else
            LightMode=0;
          Serial.print("LM:");
          Serial.println(LightMode);
          
        }

        //Licht vorne
        StartPosition=message.indexOf(LightFrontString,0)+1;
        if(StartPosition!=-1)
        {
          StartPosition=StartPosition+LightFrontString.length();
          EndPosition=message.indexOf("\"", StartPosition);
          //Serial.println(message.substring(StartPosition,EndPosition));
          
          if(0!=message.substring(StartPosition,EndPosition).compareTo(TrueString))
            LightFrontOn=1;
          else
            LightFrontOn=0;
          Serial.print("LF:");
          Serial.println(LightFrontOn);
          
        }

        //Licht hinten
        StartPosition=message.indexOf(LightBackString,0)+1;
        if(StartPosition!=-1)
        {
          StartPosition=StartPosition+LightBackString.length();
          EndPosition=message.indexOf("\"", StartPosition);
          //Serial.println(message.substring(StartPosition,EndPosition));
          
          if(0!=message.substring(StartPosition,EndPosition).compareTo(TrueString))
            LightBackOn=1;
          else
            LightBackOn=0;
          Serial.print("LB:");
          Serial.println(LightBackOn);
          
        }

        setLightControl(Direction, LightMode, LightFrontOn, LightBackOn, ledfront, ledback);
        setMotorControl(Direction,Speed,Pwm1,Pwm2);
    
      }
    }



  }

  
}
