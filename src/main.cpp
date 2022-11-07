// ******************************************************************
// COMP-10184
// External LED Wiring, PIR Sensor Test Program
//
// @author Mohawk College

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

#define SUCCESS_LIGHT 1
#define WARNING_LIGHT 2
#define DANGER_LIGHT 3

// pin assignment for PIR input
#define PIN_PIR D5

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
int keyIndex = 0;          // your network key Index number (needed only for WEP)
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

int number = 0;

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
}

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
  bool bPIR;

  // echo PIR input to built-in LED OUTPUT (note: invert the sense of the PIR sensor!)
  digitalWrite(LED_BUILTIN, HIGH);
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      setLEDStatus(WARNING_LIGHT);
      delay(5000);
    }

    Serial.println("\nConnected.");
    Serial.printf("Server started, %s\n", WiFi.localIP().toString().c_str());
    setLEDStatus(SUCCESS_LIGHT);
  }

  bPIR = digitalRead(PIN_PIR);
  if (bPIR)
  {
    Serial.println("Input Detected: " + String(digitalRead(PIN_PIR)));

    int timer = 2;
    while (timer > 0)
    {
      timer--;
      setLEDStatus(DANGER_LIGHT);
      delay(SECOND_DELAY);
    }
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey);
    if (x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    setLEDStatus();
  }
}