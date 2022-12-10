#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WiFiWebServer = ESP8266WebServer;
#include <AutoConnect.h>

#include <AutoConnectFS.h>
AutoConnectFS::FS &FlashFS = AUTOCONNECT_APPLIED_FILESYSTEM;

#include "z21/UdpInterfaceEsp8266.h"
#include "z21.h"
#include "xprintf.h"

AutoConnect portal;

void uart_putc(uint8_t d)
{
  Serial.print((char)d);
}

const uint16_t hash{0};
const uint32_t serialNumber{0xFFFFFFF0};
const uint16_t swVersion{0x0142};
const int16_t z21Port{21105};

std::shared_ptr<UdpInterfaceEsp8266> udpInterface = std::make_shared<UdpInterfaceEsp8266>(30, z21Port, false);

z21 Z21Connection(hash, serialNumber, z21Interface::HwType::Z21Interface_START, swVersion, xprintf, true, false, false);

// Redirects from root to the hello page.
void onRoot()
{
  WiFiWebServer &webServer = portal.host();
  webServer.sendHeader("Location", String("http://") + webServer.client().localIP().toString());
  webServer.send(302, "text/plain", "");
  webServer.client().flush();
  webServer.client().stop();
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  xdev_out(uart_putc);
  Serial.println();
  FlashFS.begin(AUTOCONNECT_FS_INITIALIZATION);

  AutoConnectConfig configAutoConnect;

  String idOfEsp = String(ESP.getFlashChipId(), HEX);
  while (idOfEsp.length() < 4)
  {
    idOfEsp += "0";
  }
  Serial.printf("ID of chip: ");
  Serial.println(idOfEsp);

  configAutoConnect.ota = AC_OTA_BUILTIN;
  configAutoConnect.apid = "Loco-" + idOfEsp;
  configAutoConnect.psk = idOfEsp + idOfEsp;
  configAutoConnect.apip = IPAddress(192, 168, 0, 111); // Sets SoftAP IP address
  configAutoConnect.netmask = IPAddress(255, 255, 255, 0);
  configAutoConnect.channel = random(1, 12);
  Serial.printf("Wifi Channel:%d\n", configAutoConnect.channel);
  configAutoConnect.title = "z21";
  configAutoConnect.beginTimeout = 15000;
  configAutoConnect.autoReset = false;

  configAutoConnect.homeUri = "/";

  // reconnect with last ssid in handleClient
  configAutoConnect.autoReconnect = true;
  configAutoConnect.reconnectInterval = 15;

  configAutoConnect.portalTimeout = 1;

  configAutoConnect.immediateStart = true;
  configAutoConnect.autoRise = true;
  configAutoConnect.retainPortal = true;
  portal.config(configAutoConnect);
  portal.begin();

  WiFiWebServer &webServer = portal.host();
  webServer.on("/", onRoot); // Register the root page redirector.

  if (nullptr != udpInterface.get())
  {
    udpInterface->begin();
  }

  if (!Z21Connection.setUdpObserver(udpInterface))
  {
    Serial.println("ERROR: No udp interface defined");
  }

  Z21Connection.begin();
}

void loop()
{
  portal.handleClient();
  udpInterface->cyclic();
  Z21Connection.cyclic();
  delayMicroseconds(1);
}
