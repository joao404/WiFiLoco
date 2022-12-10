/*********************************************************************
 * UdpInterfaceEsp8266
 *
 * Copyright (C) 2022 Marcel Maage
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * LICENSE file for more details.
 */

#include "Arduino.h"
#include "z21/UdpInterfaceEsp8266.h"

UdpInterfaceEsp8266::UdpInterfaceEsp8266(uint16_t maxNumberOfClients, int16_t port, boolean debug)
    : m_port(port),
      m_mem(new listofIP[maxNumberOfClients]),
      m_maxNumberOfClients(maxNumberOfClients),
      m_countIP(0),
      m_IPpreviousMillis(0)
{
}

void UdpInterfaceEsp8266::begin()
{
  if (!m_Udp.begin(m_port))
  {
    Serial.printf("UDP Init failed\n");
  }
}

void UdpInterfaceEsp8266::handlePacket(uint8_t client, uint8_t *packet, size_t packetLength)
{
  // completely transmitted to

  if (4 <= packetLength)
  {
    uint16_t index = 0;
    uint16_t length = 0;
    for (size_t left_size = packetLength; left_size > 3;)
    {
      length = (packet[index + 1] << 8) + packet[index];
      if (left_size < length)
      {
        break;
      }
      UdpMessage udpMessage{client, &(packet[index])};
      notify(&udpMessage);
      left_size -= length;
    }
  }
}

bool UdpInterfaceEsp8266::transmit(UdpMessage &message)
{
  // send data now via new interface using transmit function
 bool returnValue{false};
  uint16_t len = message.data[0] + (message.data[1] << 8);
  if (message.client == 0x00)
  { // Broadcast
    // Serial.print("B");
    //   for(uint16_t i=0;i<len;i++)
    //   {
    //     Serial.print(" ");
    //     Serial.print(data[i]);
    //   }
    // Serial.print("\n");
    // if(0 == countIP)
    // {
    //   Udp.broadcastTo(data, len, port);//, TCPIP_ADAPTER_IF_AP);
    // }

    // for (byte s = 0; s < countIP; s++) {
    //   if (mem[s].time > 0) {
    //     Udp.writeTo(data, len, mem[s].IP, port);
    //   }
    // }
    // Serial.println("B");
    // m_Udp.broadcastTo(message.data, message.data[0], m_port)
    m_Udp.beginPacket(IPAddress(192, 168, 0, 255), m_port);
    m_Udp.write(message.data, len);
    if(m_Udp.endPacket())
    {
      returnValue = true;
    }
    m_Udp.endPacket();
  }
  else
  {
    // Serial.print("C ");
    // Serial.print(mem[client-1].IP);
    // for(uint16_t i=0;i<len;i++)
    // {
    //   Serial.print(" ");
    //   Serial.print(data[i]);
    // }
    // Serial.print("\n");
    m_Udp.beginPacket(m_mem[message.client - 1].IP, m_port);
    m_Udp.write(message.data, len);
    if(m_Udp.endPacket())
    {
      returnValue = true;
    }
  }
  return returnValue;
}

bool UdpInterfaceEsp8266::receive(UdpMessage &message)
{
  return false;
}

/**********************************************************************************/
byte UdpInterfaceEsp8266::addIP(IPAddress ip)
{
  // suche ob IP schon vorhanden?
  for (byte i = 0; i < m_countIP; i++)
  {
    if (m_mem[i].IP == ip)
    {
      m_mem[i].time = ActTimeIP; // setzte Zeit
      return i + 1;              // Rückgabe der Speicherzelle
    }
  }
  // nicht vorhanden!
  if (m_countIP >= m_maxNumberOfClients)
  {
    for (byte i = 0; i < m_countIP; i++)
    {
      if (m_mem[i].time == 0)
      { // Abgelaufende IP, dort eintragen!
        m_mem[i].IP = ip;
        m_mem[i].time = ActTimeIP; // setzte Zeit
        return i + 1;
      }
    }
    Serial.print("EE"); // Fail
    return 0;           // Fehler, keine freien Speicherzellen!
  }
  m_mem[m_countIP].IP = ip;          // eintragen
  m_mem[m_countIP].time = ActTimeIP; // setzte Zeit
  m_countIP++;                       // Zähler erhöhen
  return m_countIP;                  // Rückgabe
}

void UdpInterfaceEsp8266::cyclic()
{
  int packetSize = m_Udp.parsePacket();
  if (packetSize)
  {
    uint8_t packetBuffer[packetSize + 1];
    int len = m_Udp.read(packetBuffer, packetSize);
    if (len > 0)
    {
      packetBuffer[len] = 0;
      handlePacket(addIP(m_Udp.remoteIP()), packetBuffer, len);
    }
  }

  // Nutzungszeit IP's bestimmen
  unsigned long currentMillis = millis();
  if (currentMillis - m_IPpreviousMillis > interval)
  {
    m_IPpreviousMillis = currentMillis;
    for (byte i = 0; i < m_countIP; i++)
    {
      if (m_mem[i].time > 0)
        m_mem[i].time--; // Zeit herrunterrechnen
    }
    // notifyz21InterfacegetSystemInfo(0); //SysInfo an alle BC Clients senden!
  }
  // notifyz21InterfacegetSystemInfo(0);
}
