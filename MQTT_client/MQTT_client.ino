#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

// SYSTEM CONFIG
#define LED_PIN 2
#define ANALOG_READ_PIN A0
#define NUMPIXELS 7
bool DEBUG_MODE = true;
bool DELAY_MODE = false;
int delayDuration = 100;

// WIFI CONFIG - Update these with values suitable for your network.
WiFiClient wifiClient;
const char* ssid = "IAAC-WIFI";
const char* password = "enteriaac2013";
const char* mqtt_server = "192.168.5.225";

// MESSAGE BROKER CONFIG
PubSubClient client(wifiClient);
const char* subscribeTopic = "animation" ;
const char* publishTopic = "test";

// ESP CONFIG - IMPORTANT change the last digit of the following two lines to give unique identifier
const char* chipID = "ESP8266-Client-1" ;
const char* publishID = "ESP-1";

// SENSOR CONFIG
float persistenceMultiplier = 0.75;
int tresholdRatio = 3;
int minTreshold = 20;
int calibrationDuration = 150;
int analogSensorReading;
int analogValue;
int baselineValue;
int fadeValue;

// LED CONFIG
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
bool ledSwitched = false;
int ledMaxBrigthness = 200;
int fadeOutTimeDivider = 6;


void setup() {
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);     
  // Initialize serial
  Serial.begin(115200);
  while(!Serial);
  // Initialize LEDs
  pixels.begin;
  pixels.show();
  // Initialize WIFI and MQTT connection
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if(DEBUG_MODE){
    Serial.println(" ");
    Serial.print("ChipID: ");
    Serial.println(chipID);
  }
}

void readSensorValue() {
  // Read sensor value
  analogValues = analogRead(ANALOG_READ_PIN);
  if(!baselineValue) {
    baselineValue = analogValue;
  } 
  if(!fadeValue) {
    fadeValue = 0;
  } 
  // Delay could be introduced if required by the analog read 
  if(DELAY_MODE){
    delay(delayDuration);  
  }
}

void calculateTreshold() {
  baselineValue = baselineValue * persistenceMultiplier + (1-persistenceMultiplier) * analogValue;  
}

void detectColision() {   
  if(abs(analogValue - baselineValue) > (baselineValue / tresholdRatio) && abs(analogValue - baselineValue) > minTreshold && calibrationDuration == 0){
    int startingLed = i * NUMPIXELS;
    for(int k = startingLed; k < (startingLed + NUMPIXELS); k++){
        fadeValue = ledMaxBrigthness;
    }
  }  
  if(calibrationDuration > 0){
    calibrationDuration--;
  }  
}

void updateLEDs() {
  for(int i=0; i < NUMPIXELS; i++){
        pixels.setPixelColor(i, fadeValue, fadeValue, fadeValue);     
        
        fadeValue -= fadeOutTimeDivider;
        if(fadeValue < 0) {
          fadeValue = 0;
        }
  }
  pixels.show();
}

void ensureConnection(){
  // Reconnect
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void loop() {

  readSensorValue();
  calculateTreshold();
  detectColision();
  updateLEDs();
  ensureConnection();

  // // Calculate treshold
  // if(TresholdAverageSampleSize <= 11){
  //  setMicThreshold(TresholdAverageSampleSize);
  //  TresholdAverageSampleSize++;
  // }  

  // // Take snapshot
  // long now = millis();

  // // Read sensor
  // if(!ledSwitched){
  //   micVal = analogRead(A0);
  //   delay(1);

    
  //   if(micVal > threshold){
  //     ledSwitched = true;
  //     timeSnapshot = now;
  //     Serial.println("Hit Registered");
  //     client.publish("test", publishID);
  //   }
  // }
  
  // // Dim and turn off
  // if(ledSwitched){
  //   if(millis() < timeSnapshot + dimmingDuration){
  //     digitalWrite(2,LOW);
  //   }
  //   else{
  //     ledSwitched = false;
  //     digitalWrite(2,HIGH);
  //   }
  // }

  // // Check connection status and reconnect if necessary
  // ensureConnection()

}

void setup_wifi() {
  if(DELAY_MODE){
    delay(1);
  }

  // We start by connecting to a WiFi network
  if(DEBUG_MODE){
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(DEBUG_MODE){
      Serial.print(".");
    }
  }

  if(DEBUG_MODE){
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }  
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  if(DEBUG_MODE){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }

  // Switch on the LED if 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(chipID)) { // IMPORTANT connect each ESP with a unique identifier
      Serial.println("connected");
      // Once connected, publish an announcement...
     // client.publish("test", "hello world");
      // ... and resubscribe
      client.subscribe("animation");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// void setMicThreshold(int i){
//   if(i<10){
//     Serial.println("Setting Up Mic Threshold");
//     Serial.print("Itteration number: ");
//     Serial.print(i);
//     Serial.print("   Got reading :");
//     total += analogRead(A0);
//     delay(1);
//     Serial.println(analogRead(A0));
//   }
//   if(i==10){
//     total = total / sampleNumber;
//     Serial.print("Threshold set to: ");
//     threshold = total + addtoBaseReading;  // add a small value to the baseline 
//     Serial.println(threshold);
//     }
// }

// 22
// int sampleNumber = 10;
// int total = 0;
// int threshold = 0;
// int addtoBaseReading = 15;
// int dimmingDuration = 2000; // How long the led stays HIGH
// int timeSnapshot = 0;
// int TresholdAverageSampleSize = 0;