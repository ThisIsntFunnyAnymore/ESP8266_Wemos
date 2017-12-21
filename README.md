# ESP8266_Wemos
Build out the core wireless functions using a Wemos D1 mini

## End Goal
_Wemos can connect to Blynk local server and display data from 4051 sketch without any authorization data visible from the sketch._

## Key Technologies
1. Connect to WiFi
2. OTA Updating
   - Test OTA, then combine OTA updating with the Terminal_JSON sketch. 
   - Stretch goal HTTP Server OTA (FOTA) [Blynk example](https://community.blynk.cc/t/self-updating-from-web-server-http-ota-firmware-for-esp8266/18544)
3. Writing config data to SPIFFS
   - Test SPIFFS write/read, then fold into Terminal_JSON sketch
4. OTA Uploading of config file to SPIFFS
5. ~~Error handling OTA & Config file~~ (Replace with on-device data logging later)
   - Make a second project called "Error" on blynk-cloud with notifier widget. If Blynk cannot connect to the local server kick out a notification to the public project (?)

## Start Point
Start with sketch from Terminal_JSON repository. Simple error check board type (?) and write JSON string to SPIFFS.
Add a handler on setup() if configuration exists. 

## Journal
12/18/2017: Finished task #1 with basic web server and Blynk sketch (builtin led tied to button). 

12/19/2017: 
* Finished task #2. I now have a basic Blynk and OTA sketch. I even have a Blynk/OTA/SPIFFS sketch. 
* I have done basic read/write to SPIFFS. 
* I understand enough to do #3 & #4, but  [tzapu WiFi manager](https://github.com/tzapu/WiFiManager/tree/master/examples/AutoConnectWithFSParameters) will take care of these and is a well-supported product.  
* Added stretch goal for FOTA. Apparently Python has a simple HTTP server built in that works on port 8000. This may be enough.

12/21/2017:
* Finished Tasks #3 and #4 using the aforementioned WiFiManager library and the Filesystem example. 
* I don't know if #5 is worth the time. If there is an error it is just as likely be something that would affect a Public Blynk project, and just shows my lack of confidence in my server. 
   - Later in the program I will implement on-device data logging (I still have 3MB SPIFFS but will probably have to switch to 1MB SPIFFS when my sketch gets larger (still need to add Dallas, 1wire & DHT libraries)
