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

#pragma once

#include "z21/z21InterfaceObserver.h"
#include <array>

class z21 : public virtual z21InterfaceObserver
{
public:
    z21(uint16_t hash, uint32_t serialNumber, HwType hwType, uint32_t swVersion, void (*printFunc)(const char *, ...) = nullptr, bool debugz21 = false, bool debugZ21 = false, bool debugZCan = false);
    virtual ~z21();
    void begin();

    void cyclic();

    void update(Observable<Udp::Message> &observable, Udp::Message *data) override;

private:
    // const uint32_t z21Uid{0xBADEAFFE};

    uint16_t m_serialNumber;

    bool m_debug;

    uint32_t m_minimumCmdIntervalINms{100};
    uint32_t m_lastSpeedCmdTimeINms{0};

    // Pins
    int m_pwmPin1{5};//D1};     // Nodemcu PWM pin
    int m_pwmPin2{4};//D2};     // Nodemcu PWM pin
    int m_ledFrontPin{14};//D5}; // Gpio-14 of nodemcu esp8266
    int m_ledBackPin{12};//D6};  // Gpio-12 of nodemcu esp8266

    // Locomotive Control
    int m_direction{0};
    int m_speed{0};
    bool m_emergencyBreak{false};
    std::array<uint8_t, 32> m_functions;

    uint16_t getSerialNumber() override;

    void setLightControl();
    void setMotorControl();

    void notifyLocoState(uint8_t client, uint16_t Adr);

    // Z21
    void notifyz21InterfacegetSystemInfo(uint8_t client) override;
    void notifyz21InterfaceRailPower(EnergyState State) override;
    void notifyz21InterfaceLocoState(uint16_t Adr, uint8_t data[]) override;
    void notifyz21InterfaceLocoFkt(uint16_t Adr, uint8_t type, uint8_t fkt) override;
    void notifyz21InterfaceLocoSpeed(uint16_t Adr, uint8_t speed, uint8_t stepConfig) override;
};