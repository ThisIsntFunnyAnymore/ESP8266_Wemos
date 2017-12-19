/* Write a string to SPIFFS then read it back 
 *  
 *  Combines the BasicOTA sketch from the Wemos ESP8266 libraries
    with the BlynkBlink sketch to incorporate minimal OTA + minimal
    Blynk functions.
*/
/* Comment this out to disable prints and save space */
// #define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>


//Blynk auth token for project
char auth[] = "6d8d898441374e6a987baf5735ad8cf4";
//WiFi SSID & Password
char ssid[] = "TheOtherNetwork_2G";
char pass[] = "3CatHouse";

BLYNK_CONNECTED() {
  ArduinoOTA.setHostname("Topsy_Wemos");
  ArduinoOTA.begin();
}

void setup() {

  Blynk.begin(auth, ssid, pass);

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
}
