/*WifiManager Cleanup
   v0.1, 20 December 2017 by ThisIsntFunnyAnymore
   Leaves a "pristine" wifi configuration file in access point
   mode, ready for OTA. */

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>           //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing - Nuclear option
  SPIFFS.format();

  //read configuration from FS json
  //  Serial.println("mounting FS...");
  //
  //  if (SPIFFS.begin()) {
  //    Serial.println("mounted file system");
  //    //Remove config file for testing - surgical
  //    //SPIFFS.remove("/config.json");
  //  }

  //WiFiManager local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  if (!wifiManager.startConfigPortal("OnDemandAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  ArduinoOTA.setHostname("esp8266_device");
  ArduinoOTA.begin();

}

void loop() {
  ArduinoOTA.handle();
  // put your main code here, to run repeatedly:
}
