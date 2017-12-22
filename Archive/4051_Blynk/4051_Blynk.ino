/******************************************************************************
  Mux_Analog_Input
  SparkFun Multiplexer Analog Input Example
  Jim Lindblom @ SparkFun Electronics
  August 15, 2016
  https://github.com/sparkfun/74HC4051_8-Channel_Mux_Breakout

  This sketch demonstrates how to use the SparkFun Multiplexer
  Breakout - 8 Channel (74HC4051) to read eight, separate
  analog inputs, using just a single ADC channel.

  Hardware Hookup:
  Mux Breakout ----------- Arduino
     S0 ------------------- 2
     S1 ------------------- 3
     S2 ------------------- 4
     Z -------------------- A0
    VCC ------------------- 5V
    GND ------------------- GND
    (VEE should be connected to GND)

  The multiplexers independent I/O (Y0-Y7) can each be wired
  up to a potentiometer or any other analog signal-producing
  component.

  Development environment specifics:
  Arduino 1.6.9
  SparkFun Multiplexer Breakout - 8-Channel(74HC4051) v10
  (https://www.sparkfun.com/products/13906)
******************************************************************************/

//#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <BlynkSimpleEsp8266.h>   //https://github.com/blynkkk/blynk-library


/////////////////////
// Pin Definitions //
/////////////////////
const int selectPins[3] = {D5, D6, D7}; // S0~GPIO14, S1~GPIO12, S2~GPIO13
const int zVcc = D4; //GPIO2, also built-in LED
const int zInput = A0; // Connect common (Z) to A0 (analog input)
const int vPinOffset = 2; //map Blynk virtual pins to 4051 channels

// For exponential smoothing function
const int filterWeight = 4; // higher numbers = more filtering.
// 4 = 1/4^4 = 1/64 = 0.016, ~ eta 0.1
const int numReadings = 10; // number of readings to smooth

// Blynk setup
char auth[] = "6d8d898441374e6a987baf5735ad8cf4";
char ssid[] = "TheOtherNetwork_2G";
char pass[] = "3CatHouse";

//Blynk definitions
BlynkTimer timer;

void getAnalog() {
  // Loop through all eight pins.
  // Start powering system and let [soil probes] warm up
  digitalWrite(zVcc, HIGH);
  delay(1000);
  for (byte pin = 0; pin <= 7; pin++)
  {
    selectMuxPin(pin); // Select one at a time
    int inputValue = analogRead(A0); // Initialize the variable
    // Exponential Smoothing (should you choose to use it)
    //    for (int i = 0; i < numReadings; i++) {
    //      int inputValue = analogRead(zInput); // and read A0
    //      //admittedly the bitshift is a bit obscure but it works.
    //      average = average + (inputValue - average) >> filterWeight;
    //    }
    for (int j = 0; j < 3; j++) {
      Serial.print(String(selectPins[j]));
    }
    Serial.print(" ");
    Serial.print(String(inputValue) + "\t");
    Blynk.virtualWrite(pin + vPinOffset, inputValue);
  }
  Serial.println();
  digitalWrite(zVcc, LOW);
  delay(100);
}

void setup()
{
  Serial.begin(115200); // Initialize the serial port
  // Set up the select pins as outputs:

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, getAnalog);

  for (int i = 0; i < 3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], HIGH);
  }
  pinMode(zVcc, OUTPUT);
  digitalWrite(zVcc, HIGH);
  pinMode(zInput, INPUT); // Set up Z as an input

  // Print the header:
  Serial.println("Y0\tY1\tY2\tY3\tY4\tY5\tY6\tY7");
  Serial.println("---\t---\t---\t---\t---\t---\t---\t---");
}

void loop()
{
  Blynk.run();
  timer.run();
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

