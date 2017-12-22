// 20171220:15:20 compile but errors out. Start small
/******************************************************************************
   WifiManager + OTA + Blynk by ThisIsntFunnyAnymore
   Version 0.0.1, 2017 December 20

   This is a prototype function to combine the following services:
      - tzapu's WifiManager
      - Arduino OTA updating
      - Blynk
      - Filesystem & JSON

   In addition to the combination of the above, it also adds:
      - Basic diagnostic outputs in Blynk via Blynk timer
      - Blynk terminal outputs (serial connection not required for most functions)
      - Reset from Blynk (kinda pointless outside of testing, I know)

   The advantage of this function is that it already combines many of the core
   technologies I want to include in my major project. It also hides the personal
   information (SSID, Password, private server name, Blynk token) from public
   sources like GitHub and Blynk community.

   I will not acknowledge everybody's work, so let's just say I'm standing on the
   shoulders of giants.

   Note: Currently the following pins are in use in the sketch:
      - V0:   Terminal widget
      - V2-V9: 4051 Outputs
      - V124: safety for WiFiManager reset
      - V125: trigger for WiFiManager reset
      - V126: WiFi Signal Strength
      - V127: Uptime in Seconds

   For Wemos (apparently same for nodeMCU), stay away from:
      - GPIO16   D0
      - GPIO0    D3
      - GPIO2    D4
      - GPIO15   D8
 * ****************************************************************************/

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>           //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA
#include <BlynkSimpleEsp8266.h>   //https://github.com/blynkkk/blynk-library

//define your default values here, if there are different values in config.json,
//they are overwritten.
char mqtt_server[40] = "blynk-cloud.com";
char mqtt_port[6] = "8442";
char blynk_token[34] = "YOUR_BLYNK_TOKEN";
char esp_hostname[20] = "esp8266_Device";

//flag for saving data
bool shouldSaveConfig = false;

// Pin Definitions
const int selectPins[3] = {14, 12, 13}; // S0~D5, S1~D6, S2~D7
const int zPower = 4; //D2,
const int zInput = A0; // Connect common (Z) to A0 (analog input)
const int vPinOffset = 2; //map virtual pins to 4051 channels

/***Uncomment if using smoothing function*************/
//const int filterWeight = 4; // higher numbers = more filtering.
//// 4 = 1/4^4 = 1/64 = 0.016, ~ eta 0.1
//const int numReadings = 10; // number of readings to smooth
/************************************************************/

//Blynk definitions
BlynkTimer timer;
WidgetTerminal terminal(V0);

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// A simple function to show the Blynk is running. Sends WiFi signal & uptime
void myDeviceStats () {
  Blynk.virtualWrite(V126, WiFi.RSSI());
  Blynk.virtualWrite(V127, millis() / 1000);
}


int safePin = LOW, triggerPin = LOW;

// A reset function for the WiFiManager & JSON data
// The button tied to V124 must be selected when the reset button is pushed for
// anything to happen
void deviceFallback() {

  //this may be a bit too much. I think I want to start a basic on-demand to change
  //SSID and Password only (why would I change token et. al.?)
  if (safePin == HIGH && triggerPin == HIGH) {
    terminal.println(F("Connect to 'AutoConnectAP' and sign in at 192.168.4.1"));
    terminal.println(F("Have your Blynk server/port/token ready, because we're"));
    terminal.println(F("clearing wifi settings and resetting device in  "));
    //simple countdown to give you just enough time to read the terminal.
    for (int i = 3; i >= 0; i--) {
      terminal.print(i);
      terminal.print("...");
      delay(1000);
      terminal.flush();
    }

    // I don't think Blynk can do this.
    Blynk.virtualWrite(V124, LOW);
    Blynk.virtualWrite(V125, LOW);
    Blynk.syncAll();

    // Kinder, gentler reset
    //local instance of WiFiManager
    WiFiManager wifiManager;
    //    if (!wifiManager.startConfigPortal("AutoConnectAP", "password")) {
    //      Serial.println("failed to connect and hit timeout");
    //      delay(3000);
    //      ESP.reset();
    //      delay(5000);
    //    }
    //    Serial.println("connected...yeey :)");

    // Nuclear reset
    wifiManager.resetSettings();
    SPIFFS.format();
    ESP.reset();  //use instead of ESP.restart to allow resetting without pressing button
    //    ESP.restart();

  } else {
    terminal.println(F("Cannot reset WiFi with Safety on."));
  }
}

// The selectMuxPin function sets the S0, S1, and S2 pins
// accordingly, given a pin from 0-7.
void selectMuxPin(byte pin)
{
  for (int i = 0; i < 3; i++)
  {
    if (pin & (1 << i))
      digitalWrite(selectPins[i], HIGH);
    else
      digitalWrite(selectPins[i], LOW);
  }
}

void getAnalog()
{
  // Power on the soil sensors and give a moment to breathe
  digitalWrite(zPower, HIGH);
  delay(1000);
  // Loop through all eight pins.
  for (byte pin = 0; pin <= 7; pin++)
  {
    selectMuxPin(pin); // Select one at a time
    // get an exponentially smoothed average
    int inputValue = analogRead(zInput); // initialize average
    //***Rolling average with exponential smoothing***/
    //    for (int i = 0; i < numReadings; i++) {
    //      //int inputValue = analogRead(zInput); // Initiand read A0
    //      //admittedly the bitshift is a bit obscure but it works.
    //      inputValue = inputValue + (analogRead(zInput) - inputValue) >> filterWeight;
    //    }
    //************************************************/
    Serial.print(String(inputValue) + "\t");
    Blynk.virtualWrite(pin + vPinOffset, inputValue);
  }
  // Note this sketch writes V1-V8
  Serial.println();
  delay(50);
  //Turn off the power pin
  digitalWrite(zPower, LOW);
}
//Serial.println();

//Set safePin
BLYNK_WRITE(V124) {
  safePin = param.asInt();
}

//Fire trigger
BLYNK_WRITE(V125) {
  triggerPin = param.asInt();
  if (triggerPin == HIGH) {
    deviceFallback();
  }
}

//This function just cleans up the terminal once connected.
BLYNK_CONNECTED() {
  char BlynkConnectString[] = "Connection to Blynk established";
  Serial.println(BlynkConnectString);
  // Clear terminal
  for (int i = 1; i < 10; i++) {
    terminal.print("\n");
  }
  terminal.println(BlynkConnectString);
  terminal.print("\n");
  terminal.flush();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();


  // Set up the select pins as outputs and start at 000:
  for (int i = 0; i < 3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], LOW);
  }
  pinMode(zInput, INPUT); // Set up Z as an input
  pinMode(zPower, OUTPUT);
  digitalWrite(zPower, LOW);
  /*clean FS, for testing
    JK - You need to format the SPIFFS to remove /config.json when
    creating or deleting custom parameters, otherwise the sketch will
    crash. Use with wifiManager.resetSettings() below.
    Trust me, I know this. */
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);
          strcpy(esp_hostname, json["esp_hostname"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  WiFiManagerParameter custom_esp_hostname("device", "device hostname", esp_hostname, 20);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  // wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.addParameter(&custom_esp_hostname);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());
  strcpy(esp_hostname, custom_esp_hostname.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;
    json["esp_hostname"] = esp_hostname;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  // Now that we have a successful WiFi connection and all of our data is
  // saved, we can start our services.

  // Set the ESP Hostname to its saved value and start OTA service
  ArduinoOTA.setHostname(esp_hostname);
  ArduinoOTA.begin();

  // Configure and start Blynk (courtesy Costas July 2016 community.blynk.cc
  unsigned int int_mqtt_port = atoi(mqtt_port);
  Blynk.config(blynk_token, mqtt_server, int_mqtt_port); // in place of Blynk.begin(auth, ssid, pass)
  Blynk.connect(); // connect with default timeout of 10 sec
  while (!Blynk.connected()) {
    //wait until Blynk is connected before proceeding to the loop()
  }
  timer.setInterval(2000L, getAnalog);
  timer.setInterval(2000L, myDeviceStats);  // get stats every 10 seconds. Fine for basic knowledge

  // Print the header: Comment out when working in Blynk
//  Serial.println("Y0\tY1\tY2\tY3\tY4\tY5\tY6\tY7");
//  Serial.println("---\t---\t---\t---\t---\t---\t---\t---");

}


void loop() {
  Blynk.run();
  timer.run();
  ArduinoOTA.handle();


}
