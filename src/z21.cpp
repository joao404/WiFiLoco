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
  m_functions.fill(false);
  m_locoData.fill(0);
}

z21::~z21()
{
}

void z21::begin()
{
  // // PinMode
  pinMode(m_ledFrontPin, OUTPUT);
  pinMode(m_ledBackPin, OUTPUT);
  // // change PWM Frequency
  analogWriteFreq(5000);

  analogWriteRange(128);

  analogWrite(m_pwmPin1, 0); // maxvalue is 1024, max Speed value is 100
  analogWrite(m_pwmPin2, 0); // maxvalue is 1024, max Speed value is 100

  z21InterfaceObserver::begin();
}

void z21::cyclic()
{
}

void z21::update(Observable<Udp::Message> &observable, Udp::Message *data)
{
  z21InterfaceObserver::update(observable, data);
}

uint16_t z21::getSerialNumber()
{
  return m_serialNumber;
}

void z21::setLightControl()
{
  bool backLightOn{false};
  bool frontLightOn{false};
  bool forward{bitRead(m_locoData[1], 7)};
  // function 0 => light on
  // function 1 => both directions if true
  // function 2 => deactivate front light
  // function 3 => deactivate back light
  if (m_functions[0])
  {
    // light on
    // default light mode is only single light in one direction
    if (m_functions[1])
    {
      if (forward)
      {
        if (!m_functions[2])
        {
          frontLightOn = true;
        }
      }
      else
      {
        if (!m_functions[3])
        {
          backLightOn = true;
        }
      }
    }
    else
    {
      frontLightOn = true;
      backLightOn = true;
    }
  }

  if (frontLightOn)
  {
    digitalWrite(m_ledFrontPin, HIGH);
  }
  else
  {
    digitalWrite(m_ledFrontPin, LOW);
  }

  if (backLightOn)
  {
    digitalWrite(m_ledBackPin, HIGH);
  }
  else
  {
    digitalWrite(m_ledBackPin, LOW);
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

void z21::notifyz21InterfaceRailPower(EnergyState State)
{
  if (m_debug)
  {
    Serial.print("Power: ");
    Serial.println(static_cast<uint8_t>(State), HEX);
  }

  if (EnergyState::csNormal == State)
  {
    uint8_t data[16];
    data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
    data[1] = 0x01;
    EthSend(0x00, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
  }
  else if (EnergyState::csEmergencyStop == State)
  {
    uint8_t data[16];
    data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
    data[1] = 0x00; // Power OFF
    EthSend(0, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
  }
  else if (EnergyState::csTrackVoltageOff == State)
  {
    uint8_t data[16];
    data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_BC_TRACK_POWER);
    data[1] = 0x00; // Power OFF
    EthSend(0, 0x07, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
  }
  z21InterfaceObserver::setPower(State);
}

void z21::notifyz21InterfacegetSystemInfo(uint8_t client)
{
  sendSystemInfo(client, 0, 50000, 77); // report System State to z21Interface clients
}

void z21::notifyLocoState(uint8_t client, uint16_t Adr)
{
  if (m_debug)
  {
    Serial.println("notifyLocoState");
  }
  uint8_t data[16];
  data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_LOCO_INFO); // 0xEF X-HEADER
  data[1] = (Adr >> 8) & 0x3F;
  data[2] = Adr & 0xFF;
  data[3] = 4;                   // 128 steps
  data[4] = (char)m_locoData[1]; // DSSS SSSS
  data[5] = (char)m_locoData[2]; // F0, F4, F3, F2, F1
  data[6] = (char)m_locoData[3]; // F5 - F12; Funktion F5 ist bit0 (LSB)
  data[7] = (char)m_locoData[4]; // F13-F20
  data[8] = (char)m_locoData[5]; // F21-F28

  EthSend(0, 14, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
}

void z21::notifyz21InterfaceLocoState(uint16_t Adr, std::array<uint8_t, 8> &data)
{
  if (m_debug)
  {
    Serial.println("notifyz21InterfaceLocoState");
  }
  data[0] = 3;                   // 128 steps
  data[1] = (char)m_locoData[1]; // DSSS SSSS
  data[2] = (char)m_locoData[2]; // F0, F4, F3, F2, F1
  data[3] = (char)m_locoData[3]; // F5 - F12; Funktion F5 ist bit0 (LSB)
  data[4] = (char)m_locoData[4]; // F13-F20
  data[5] = (char)m_locoData[5]; // F21-F28
  data[6] = (char)m_locoData[6]; // F29-F31
}

void z21::notifyz21InterfaceLocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt)
{
  if (m_debug)
  {
    Serial.print("Fkt: ");
    Serial.print(fkt);
    Serial.print(" ");
    Serial.println(type);
  }
  if (0 == type)
  {
    m_functions[fkt] = false;
  }
  else if (1 == type)
  {
    m_functions[fkt] = true;
  }
  else if (2 == type)
  {
    m_functions[fkt] = !m_functions[fkt];
  }

  if (0 == fkt)
  {
    bitWrite(m_locoData[2], 4, m_functions[fkt]);
  }
  else if (fkt < 5)
  {
    bitWrite(m_locoData[2], fkt - 1, m_functions[fkt]);
  }
  else if (fkt < 13)
  {
    bitWrite(m_locoData[3], fkt - 5, m_functions[fkt]);
  }
  else if (fkt < 21)
  {
    bitWrite(m_locoData[4], fkt - 13, m_functions[fkt]);
  }
  else if (fkt < 29)
  {
    bitWrite(m_locoData[5], fkt - 21, m_functions[fkt]);
  }
  else if (fkt < 32)
  {
    bitWrite(m_locoData[6], fkt - 29, m_functions[fkt]);
  }
  else
  {
    Serial.println(F("### ERROR: Function number to big"));
  }
  if (m_debug)
  {
    for (auto data : m_locoData)
    {
      Serial.print(data);
      Serial.print(" ");
    }
    Serial.println();
  }
  setLightControl();
  notifyLocoState(0, Adr);
}

//--------------------------------------------------------------------------------------------
void z21::notifyz21InterfaceLocoSpeed(uint16_t Adr, uint8_t speed, uint8_t stepConfig)
{
  if (m_debug)
  {
    Serial.print("Speed ");
    Serial.print(speed);
    Serial.print(" ");
    Serial.println(stepConfig);
  }
  m_locoData[1] = speed;
  uint8_t speedWithoutDirection{static_cast<uint8_t>(speed & static_cast<uint8_t>(0x7F))};

  m_emergencyBreak = (1 == speedWithoutDirection);

  if (0 == speedWithoutDirection)
  {
    m_speed = 0;
    m_direction = 0;
  }
  else
  {
    m_speed = speedWithoutDirection - 1;

    m_direction = bitRead(m_locoData[1], 7) ? 1 : -1;
  }

  if (m_emergencyBreak || (0 == m_speed))
  {
    setMotorControl();
    setLightControl();
    notifyLocoState(0, Adr);
  }
  else
  {
    uint32_t currentTimeINms = millis();
    if ((m_lastSpeedCmdTimeINms + m_minimumCmdIntervalINms) < currentTimeINms)
    {
      m_lastSpeedCmdTimeINms = currentTimeINms;
      setMotorControl();
      setLightControl();
      notifyLocoState(0, Adr);
    }
  }
}