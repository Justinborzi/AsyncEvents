// COMP-10184 - Mohawk College
// Async Events
//
// This program is a simple web server that serves files from a file system and tracks the current temperature in realtime of an attached sensor device.
//
// @author Justin Borzi
// @id 000798465
// @date 2022-11-07
//
// I created this work and I have not shared it with anyone else.

// Define Imports
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h"

// pin assignments for external LEDs
#define PIN_LED_GREEN D1
#define PIN_LED_YELLOW D2
#define PIN_LED_RED D3

// Define Constants
#define SECOND_DELAY 1000

// Light Pins
#define SUCCESS_LIGHT 1
#define WARNING_LIGHT 2
#define DANGER_LIGHT 3

// pin assignment for PIR input
#define PIN_PIR D5

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
WiFiClient client;

// Set the API Channel and Key
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

// Global Event State
volatile bool bEventOccured;

/**
 * Handle External Events
 */
void IRAM_ATTR isr()
{
  bEventOccured = true;
}

void setup()
{
  Serial.begin(115200); // Initialize serial

  // setup LED outputs
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // setup PIR input
  pinMode(PIN_PIR, INPUT);

  // built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client); // Initialize ThingSpeak

  Serial.println("\nAsync Events");
  Serial.println("\nJustin Borzi, 000798465");
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), isr, RISING);
}

/**
 * Set the LED based on the passed state.
 * @param status - 0 = OFF, 1 = SUCCESS, 2 = WARNING, 3 = DANGER 
 */
void setLEDStatus(int status = 0)
{
  switch (status)
  {
  case 1:
    // GREEN ON
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    delay(200);
    break;
  case 2:
    // YELLOW ON
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
    delay(200);
    break;
  case 3:
    // RED ON
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, HIGH);
    delay(200);
    break;

  default:
    // ALL OFF
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    delay(200);
    break;
  }
}

void loop()
{
  // echo PIR input to built-in LED OUTPUT (note: invert the sense of the PIR sensor!)
  digitalWrite(LED_BUILTIN, HIGH);

  // Handle the EXTERNAL EVENT
  if (bEventOccured)
  {
    Serial.println("Input Detected: " + String(digitalRead(PIN_PIR)));

    // Set up timer.
    int timer = 2;

    // Decrement the seconds.
    while (timer > 0)
    {
      timer--;
      setLEDStatus(DANGER_LIGHT);
      delay(SECOND_DELAY);
    }

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey);

    // Print success if repsonse code is 200 (OK) otherwise print the error with the returned code.
    if (x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    // Set the LED to off state
    setLEDStatus();

    // Set flag as off.
    bEventOccured = false;
  }

  // Wait for WIFI connection
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);

    // Wifi isnt connected, continue until changed.
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      
      // Keep warning light enabled until successful connection.
      setLEDStatus(WARNING_LIGHT);
      delay(5000);
    }

    // Wifi has successfully connected.
    Serial.println("\nConnected.");
    Serial.printf("Server started, %s\n", WiFi.localIP().toString().c_str());

    // Set state to ready.
    setLEDStatus(SUCCESS_LIGHT);
  }
}