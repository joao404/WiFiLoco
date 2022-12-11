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
  m_functions.fill(0);
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
  bool backLightOn{false};
  bool frontLightOn{false};

  // function 0 => light on
  // function 1 => both directions if true
  // function 2 => deactivate front light
  // function 3 => deactivate back light
  if (1 == m_functions.at(0))
  {
    // light on
    // default light mode is only single light in one direction
    if (0 == m_functions.at(1))
    {
      if (-1 == m_direction) // backward
      {
        backLightOn = true;
      }
      else
      {
        frontLightOn = true;
      }
    }
    else
    {
      frontLightOn = true;
      backLightOn = true;
    }
  }

  if ((0 == m_functions.at(2)) || frontLightOn)
  {
    digitalWrite(m_ledFrontPin, HIGH);
  }
  else
  {
    digitalWrite(m_ledFrontPin, LOW);
  }

  if ((0 == m_functions.at(3)) || backLightOn)
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
  int speed = static_cast<int>(m_speed) * 8;
  if (1 == m_direction) // forward
  {
    analogWrite(m_pwmPin1, speed); // maxvalue is 1024, max Speed value is 100
    analogWrite(m_pwmPin2, 0);     // maxvalue is 1024, max Speed value is 100
    // digitalWrite(a3, HIGH);
  }
  else if (-1 == m_direction) // backward
  {
    analogWrite(m_pwmPin1, 0);     // maxvalue is 1024, max Speed value is 100
    analogWrite(m_pwmPin2, speed); // maxvalue is 1024, max Speed value is 100
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
  Serial.print("Power: ");
  Serial.println(static_cast<uint8_t>(State), HEX);

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

  uint8_t data[10];
  data[0] = static_cast<uint8_t>(z21Interface::XHeader::LAN_X_LOCO_INFO); // 0xEF X-HEADER
  data[1] = (Adr >> 8) & 0x3F;
  data[2] = Adr & 0xFF;
  data[3] = 4; // 128 steps
  uint8_t speed{0};
  if (m_emergencyBreak)
  {
    speed = 1;
  }
  else
  {

    speed = static_cast<uint8_t>((0 == m_speed) ? 0 : m_speed + 1);
  }
  data[4] = ((-1 == m_direction) ? 0 : 1 << 7) | speed;
  data[5] = static_cast<uint8_t>((m_functions[4] << 4) || (m_functions[3] << 3) || (m_functions[2] << 2) || (m_functions[1] << 1) || m_functions[0]);
  data[6] = static_cast<uint8_t>((m_functions[12] << 7) || (m_functions[11] << 6) || (m_functions[10] << 5) || (m_functions[9] << 4) || (m_functions[8] << 3) || (m_functions[7] << 2) || (m_functions[6] << 1) || m_functions[5]);
  data[7] = static_cast<uint8_t>((m_functions[20] << 7) || (m_functions[19] << 6) || (m_functions[18] << 5) || (m_functions[17] << 4) || (m_functions[16] << 3) || (m_functions[15] << 2) || (m_functions[14] << 1) || m_functions[13]);
  data[8] = static_cast<uint8_t>((m_functions[28] << 7) || (m_functions[27] << 6) || (m_functions[26] << 5) || (m_functions[25] << 4) || (m_functions[24] << 3) || (m_functions[23] << 2) || (m_functions[22] << 1) || m_functions[21]);
  data[9] = static_cast<uint8_t>((m_functions[31] << 2) || (m_functions[30] << 1) || m_functions[29]);

  EthSend(0, 15, z21Interface::Header::LAN_X_HEADER, data, true, (static_cast<uint16_t>(BcFlagShort::Z21bcAll) | static_cast<uint16_t>(BcFlagShort::Z21bcNetAll)));
}

void z21::notifyz21InterfaceLocoState(uint16_t Adr, uint8_t data[])
{
  notifyLocoState(0, Adr);
}

void z21::notifyz21InterfaceLocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt)
{
  Serial.printf("Fkt: %d %d\n", fkt, type);
  if (m_functions.size() > fkt)
  {
    switch (type)
    {
    case 0:
      m_functions.at(fkt) = 0;
      break;
    case 1:
      m_functions.at(fkt) = 1;
      break;
    case 2:
      m_functions.at(fkt) = (0 == m_functions.at(fkt))? 1 : 0;
      break;
    default:
      break;
    }
  }
  notifyLocoState(0, Adr);
}

//--------------------------------------------------------------------------------------------
void z21::notifyz21InterfaceLocoSpeed(uint16_t Adr, uint8_t speed, uint8_t stepConfig)
{
  Serial.printf("Speed %d %d\n", speed, stepConfig);

  m_emergencyBreak = (1 == speed);

  if (0 == speed)
  {
    m_speed = 0;
    m_direction = 0;
  }
  else
  {
    m_speed = speed - 1;

    m_direction = speed & 0x80 ? 1 : -1;
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