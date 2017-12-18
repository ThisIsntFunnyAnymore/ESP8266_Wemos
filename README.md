# ESP8266_Wemos
Build out the core wireless functions using a Wemos D1 mini

## End Goal
_Wemos can connect to Blynk local server and display data from 4051 sketch without any authorization data visible from the sketch._

## Key Technologies
1. Connect to WiFi
2. OTA Updating
   - Test OTA, then combine OTA updating with the Terminal_JSON sketch. 
3. Writing config data to SPIFFS
   - Test SPIFFS write/read, then fold into Terminal_JSON sketch
4. OTA Uploading of config file to SPIFFS
5. Error handling OTA & Config file
   - Make a second project called "Error" on blynk-cloud with notifier widget. If Blynk cannot connect to the local server kick out a notification to the public project (?)

## Start Point
Start with sketch from Terminal_JSON repository. Simple error check board type (?) and write JSON string to SPIFFS.
Add a handler on setup() if configuration exists. 

## Journal
12/18/2017: Finished task #1 with basic web server and Blynk sketch (builtin led tied to button). 
