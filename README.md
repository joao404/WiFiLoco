# WiFiLoco8266
--------------

ESP8266 based remotecontrol for locomotives with interface for Roco Z21 App. (Default Ip Adress 192.168.0.111)

## Pin Conifiguration:
- D1/Pin 5 -> Motor pin 1
- D2/Pin 4 -> Motor pin 2
- D5/Pin14 -> Front light pin
- D6/Pin12 -> Back light pin

## Usage:
- Connect to Wifi Loco######. Password is hexadecimal number of Wifi name written two times.
For example: Loco123abc => PW: 123abc123abc
- Connect with Z21 App.
- Create a loco (it does not matter, if you create several locos, because address of a command is ignored).

## Hardware
- ESP8266 (Wemos D1 mini)
- Lithium Battery
- DRV8833 or other motor driver
- step down converter to create 3.3V
- (optional) bust converter to supply motor driver with more than battery voltage.