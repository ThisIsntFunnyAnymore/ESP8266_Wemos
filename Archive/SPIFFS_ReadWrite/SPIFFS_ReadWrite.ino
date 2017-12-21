#include "FS.h"
// Apparently FS.h has to go to the very front. 
/* Write a string to SPIFFS then read it back

    Combines the BasicOTA sketch from the Wemos ESP8266 libraries
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


BlynkTimer timer;

WidgetTerminal terminal(V1);
/*************************************************************
    12/19/2017 Deprecated. Parameter checking included in transformation of
    writeSPIFFS input to read/write modes

  // Check the parameter to verify it is a recognized command.
  int checkPar(char p[10]) {
  int parSuccess = 0;
  // terminal.println(F("checking parameter..."));  //debugging; comment out final
  char *a_pars[3] = {"add", "rep", "done"};
  for (int i = 0; i < 3; i++) {
    if (strcmp(a_pars[i], p) == 0) {
      parSuccess = 1;
      break;
    }
  }
  return parSuccess;
  }
************************************************************/

void writeSPIFFS(char p[10], char v[200]) {
  char writeMode[2];
  if (strcmp("add", p) == 0) {
    strcpy(writeMode, "a");
  } else if (strcmp("rep", p) == 0) {
    strcpy(writeMode,  "w");
  } else if (strcmp("done", p) == 0) {
    strcpy(writeMode, "r");
  } else {
    terminal.print(p);
    terminal.println(" is not a valid parameter.");
    terminal.print("\n");
    return;
  }
  // open file based on writeMode
  //next two lines are for debugging
  // terminal.print("Write Mode: ");
  // terminal.println(writeMode);

  File f = SPIFFS.open("/file.txt", writeMode);
  if (!f) {
    terminal.println("writeSPIFFS: file open failed");
  }
  if (strcmp(writeMode, "r") == 0) {
    terminal.println("====== Reading from SPIFFS file =======");
    while (f.available()) {
      String s = f.readStringUntil('\n');
      terminal.println(s);
    }
    terminal.println("============== done ====================");
    terminal.flush();
  } else {
    f.println(v);
    f.close();
  }
}

/**************************************************************
    12/19/2017 Deprecated: Incorporated into writeSPIFFS subroutine.
  void readSPIFFS() {
    f = SPIFFS.open("/file.txt", "r");
    if (!f) {
      terminal.println("radSPIFFS: file open failed");
    }
    terminal.println("====== Reading from SPIFFS file =======");
    // write 10 strings to file
    while (f.available()) {
      String s = f.readStringUntil('\n');
      terminal.println(s);
    }
    terminal.println("============== done ====================");
    terminal.flush
  }
******************************************************************** */

void parseStr(String fsentence, String fparArray[]) {

  //If the for() loop fails then the full sentence is returned as the first element
  fparArray[0] = fsentence;
  fparArray[1] = "";
  //This should find the first instance of " " in the string
  for (int i = 0; i < fsentence.length(); i++) {
    if (fsentence.substring(i, i + 1) == " ") {
      fparArray[0] = fsentence.substring(0, i);
      fparArray[1] = fsentence.substring(i + 1);
      break;
    }
  }
}

BLYNK_CONNECTED() {
  ArduinoOTA.setHostname("Topsy_Wemos");
  ArduinoOTA.begin();
}

BLYNK_WRITE(V1) {
  //  String sentence = param.asStr();
  //  char par[10];
  //  char parVal[200];
  String parArray[2];
  parseStr(param.asStr(), parArray);
  //  sscanf(sentence.c_str(), "%s \"%s\"", par, parVal);
  int parLen = parArray[0].length() + 1;
  char par[parLen];
  parArray[0].toCharArray(par, parLen);
  int parValLen = parArray[1].length() + 1;
  char parVal[parValLen];
  parArray[1].toCharArray(parVal, parValLen);

  //Debug lines
  //terminal.print(par);
  //terminal.print("=");
  //terminal.print(parVal);

  writeSPIFFS(par, parVal);
  // writeSPIFFS(par, parVal);

}

// A simple function to show the Blynk is running. Sends WiFi signal & uptime
void myDeviceStats () {
  Blynk.virtualWrite(V12, WiFi.RSSI());
  Blynk.virtualWrite(V13, millis() / 1000);
}

void setup() {

  // Start blynk and set up the "stats" 
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, myDeviceStats);
  
  terminal.println(F("Proof-of-concept Read/Write to SPIFFS filesystem."));
  terminal.println(F("Commands: [cmd] \"[value]\" with a space after [cmd] "));
  terminal.println(F("   add=add a new line of text to the file."));
  terminal.println(F("   rep=replace the file with the text."));
  terminal.println(F("   done=Close the file for writing and print."));
  terminal.println(F(""));
  bool result = SPIFFS.begin();
  terminal.println("SPIFFS opened " + result);
  terminal.flush();
}

void loop() {
  Blynk.run();
  timer.run();
  ArduinoOTA.handle();
}
