/*********************************************************************
 * z21
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

#include "z21.h"

z21::z21(uint16_t hash, uint32_t serialNumber, HwType hwType, uint32_t swVersion, void (*printFunc)(const char *, ...), bool debugz21, bool debugZ21, bool debugZCan)
    : z21InterfaceObserver(hwType, swVersion, debugZ21),
      m_serialNumber(serialNumber),
      m_debug(debugz21)
{
}

z21::~z21()
{
}

void z21::begin()
{
  // PinMode
  pinMode(m_ledFrontPin, OUTPUT);
  pinMode(m_ledBackPin, OUTPUT);
  // change PWM Frequency
  analogWriteFreq(5000);

  analogWrite(m_pwmPin1, 0); // maxvalue is 1024, max Speed value is 100
  analogWrite(m_pwmPin2, 0); // maxvalue is 1024, max Speed value is 100

  z21InterfaceObserver::begin();

  delay(1000);
}

void z21::cyclic()
{
  uint32_t currentTimeINms = millis();
  // TODO send cyclic ping and power voltage information
  if ((m_lastPingSendTimeINms + 1000) < currentTimeINms)
  {
    m_lastPingSendTimeINms = currentTimeINms;
  }
}

void z21::update(Observable &observable, void *data)
{
  z21InterfaceObserver::update(observable, data);
}

uint16_t z21::getSerialNumber()
{
  return m_serialNumber;
}

void z21::setLightControl()
{
  if (0 == m_lightMode) // no automatic change
  {
    if (0 == m_lightFrontOn)
    {
      digitalWrite(m_ledFrontPin, LOW);
    }
    else
    {
      digitalWrite(m_ledFrontPin, HIGH);
    }
    if (0 == m_lightBackOn)
    {
      digitalWrite(m_ledBackPin, LOW);
    }
    else
    {
      digitalWrite(m_ledBackPin, HIGH);
    }
  }
  else // automatic change
  {
    if (-1 == m_direction) // backward
    {
      if (0 == m_lightFrontOn)
      {
        digitalWrite(m_ledFrontPin, LOW);
      }
      else
      {
        digitalWrite(m_ledFrontPin, HIGH);
      }
      digitalWrite(m_ledBackPin, LOW);
    }
    else
    {
      if (0 == m_lightBackOn)
      {
        digitalWrite(m_ledBackPin, LOW);
      }
      else
      {
        digitalWrite(m_ledBackPin, HIGH);
      }
      digitalWrite(m_ledFrontPin, LOW);
    }
  }
}

void z21::setMotorControl()
{
  if (1 == m_direction) // forward
  {
    analogWrite(m_pwmPin1, m_speed); // maxvalue is 1024, max Speed value is 100
    analogWrite(m_pwmPin2, 0);       // maxvalue is 1024, max Speed value is 100
    // digitalWrite(a3, HIGH);
  }
  else if (-1 == m_direction) // backward
  {
    analogWrite(m_pwmPin1, 0);       // maxvalue is 1024, max Speed value is 100
    analogWrite(m_pwmPin2, m_speed); // maxvalue is 1024, max Speed value is 100
    // digitalWrite(a3, HIGH);
  }
  else
  {                            // stop
    analogWrite(m_pwmPin1, 0); // maxvalue is 1024, max Speed value is 100
    analogWrite(m_pwmPin2, 0); // maxvalue is 1024, max Speed value is 100
  }
}

// onCallback

// void z21::notifyz21InterfaceRailPower(EnergyState State)
// {
//   Serial.print("Power: ");
//   Serial.println(static_cast<uint8_t>(State), HEX);

//   if (EnergyState::csNormal == State)
//   {
//     uint8_t data[16];
//     data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
//     data[1] = 0x01;
//     EthSend(0x00, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
//   }
//   else if (EnergyState::csEmergencyStop == State)
//   {
//     uint8_t data[16];
//     data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
//     data[1] = 0x00; // Power OFF
//     EthSend(0, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
//   }
//   else if (EnergyState::csTrackVoltageOff == State)
//   {
//     uint8_t data[16];
//     data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
//     data[1] = 0x00; // Power OFF
//     EthSend(0, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
//   }
//   z21InterfaceObserver::setPower(State);
// }
void z21::notifyz21InterfaceLocoState(uint16_t Adr, uint8_t data[])
{
  // for (auto finding = m_locos.begin(); finding != m_locos.end(); ++finding)
  // {
  //   if (finding->adrZ21 == Adr)
  //   {
  //     uint8_t index = 0;
  //     for (auto i : finding->data)
  //     {
  //       data[index++] = i;
  //     }
  //     // if (finding->data[0] == static_cast<uint8_t>(StepConfig::Step128))
  //     // {
  //     //   data[0] = 4;
  //     // }
  //     return;
  //   }
  // }
  // if (m_debug)
  // {
  //   Serial.print("notifyz21InterfaceLocoState:");
  //   Serial.println(Adr);
  // }
  // if (m_locos.size() >= m_maxNumberOfLoco)
  // {
  //   m_locos.pop_back();
  // }
  // addToLocoList(Adr, static_cast<uint8_t>(AdrMode::Motorola), static_cast<uint8_t>(StepConfig::Step128));
  // saveLocoConfig();
  // data[0] = static_cast<uint8_t>(StepConfig::Step128);
  // memset(&data[1], 0, 5);
  // before state was requested her, which is not possible if mode of loco is not known
}

void z21::notifyz21InterfaceLocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt)
{
  // for (auto finding = m_locos.begin(); finding != m_locos.end(); ++finding)
  // {
  //   if (finding->adrZ21 == Adr)
  //   {
  //     setLocoFunc(finding->adrTrainbox, fkt, type);
  //     return;
  //   }
  // }
  // if (m_locos.size() >= m_maxNumberOfLoco)
  // {
  //   m_locos.pop_back();
  // }
  // if (m_debug)
  // {
  //   Serial.print("Loco not found:");
  //   Serial.println(Adr);
  // }
  // addToLocoList(Adr, static_cast<uint8_t>(AdrMode::Motorola), static_cast<uint8_t>(StepConfig::Step128));
  // saveLocoConfig();
}

//--------------------------------------------------------------------------------------------
void z21::notifyz21InterfaceLocoSpeed(uint16_t Adr, uint8_t speed, uint8_t stepConfig)
{
  // unsigned long currentTimeINms = millis();
  //         // we are sending speed in case that we already received an answer for the last command or the time is up
  //         if (((finding->lastSpeedCmdTimeINms + minimumCmdIntervalINms) < currentTimeINms) || (finding->speedResponseReceived))

  bool emergencyStop = false;
  if ((0 == speed) || (1 == speed))
  {
    m_speed = 0;
    m_direction = 0;
  }
  else
  {
    m_speed = speed - 1;

    m_direction = speed & 0x80 ? 1 : -1;
  }

  setMotorControl();
  setLightControl();
}