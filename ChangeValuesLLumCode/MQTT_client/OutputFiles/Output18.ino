#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* assigned_id = "18";

/// IMPORTANT change the last digit of the following three lines to give unique identifier
const char* id = "ESP18";
const char* resetID = assigned_id;

// SYSTEM CONFIG
#define LED_PIN D3
#define ANALOG_READ_PIN A0
#define NUMPIXELS 7

// RESET PINS
#define GPIO0 D3
#define GPIO15 D8



bool DEBUG_MODE = true;
bool DELAY_MODE = true;
bool SENSOR_VALUE_CONSOLE_MONITOR= false;
int delayDuration = 100;

// WIFI CONFIG - Update these with values suitable for your network.
WiFiClient wifiClient;
//const char* ssid = "IAAC-WIFI";
//const char* password = "enteriaac2013";
const char* ssid = "llum_installation_wifi";
const char* password = "99334994";
const char* mqtt_server = "192.168.1.150"; // Raspberry pi has a static ip 192.168.1.150

// MESSAGE BROKER CONFIG
PubSubClient client(wifiClient);
const char* subscribeTopic = "animation" ;
const char* publishTopic = "test";


// SENSOR CONFIG
float persistenceMultiplier = 0.75;
int tresholdRatio = 7;
int minTreshold = 7;
int calibrationDuration = 150;
int analogSensorReading;
int analogValue;
int baselineValue;
float fadeValue;


// LED CONFIG
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledSwitched = false;
int ledMaxBrigthness = 200;
float fadeOutTimeDivider = 1;


// OTA CONFIG
bool ota_flag = true;
int time_elapsed = 0;


void setup() {
  
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);    
   
  // Initialize serial
  // while(!Serial);
  if(DEBUG_MODE){
    Serial.begin(115200);
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("ID : ");
    Serial.println(id);
    Serial.print("Mac Address : ");
    Serial.println(WiFi.macAddress());
  }
  
  setup_wifi();
  setup_OTA();

  // Initialize LEDs
  pixels.begin();
  pixels.show();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if(DEBUG_MODE){
    Serial.println(" ");
    Serial.print("ChipID: ");
    Serial.println(id);
  }
}

void loop() {

  checkOTAflag();

  readSensorValue();
  calculateTreshold();
  detectCollision();
  updateLEDs();
  ensureConnection();
}


void checkOTAflag() {
  if (ota_flag) {
    pixelsOn(150);
    delay(1000);
    pixelsOff();
    delay(100);
    pixelsOn(150);
    delay(1000);
    pixelsOff();
    while (time_elapsed < 15000) {
      ArduinoOTA.handle();
      time_elapsed = millis();
      delay(10);
    }
    pixelsOn(150);
    delay(2000);
    pixelsOff();
    ota_flag = false;
  }
}


void setup_wifi() {

  if(DELAY_MODE){
    delay(10);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
//  WiFi.begin(ssid, password);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    if(DEBUG_MODE){
//      Serial.print(".");
//    }
//  }

}

void setup_OTA(){
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}


void readSensorValue() {
  // Read sensor value
  analogValue = analogRead(ANALOG_READ_PIN);
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
  if(SENSOR_VALUE_CONSOLE_MONITOR){
    Serial.print(" ( ");
    Serial.print(analogValue);
    Serial.print(" / ");
    Serial.print(baselineValue);
    Serial.print(" / ");
    Serial.print(fadeValue);
    Serial.println(" ) ");
  }
}

void detectCollision() {   
  if(abs(analogValue - baselineValue) > (baselineValue / tresholdRatio) && abs(analogValue - baselineValue) > minTreshold && calibrationDuration == 0){
    int startingLed = NUMPIXELS;
    for(int k = startingLed; k < (startingLed + NUMPIXELS); k++){
        fadeValue = ledMaxBrigthness;
    }
    //emitCollisionSignals();
  }  
  if(calibrationDuration > 0){
    calibrationDuration--;
  }  
}

void emitCollisionSignals() {
  client.publish("test", id);
}

void updateLEDs() {
  for(int i=0; i < NUMPIXELS; i++){
        pixels.setPixelColor(i, (int)fadeValue, (int)fadeValue, (int)fadeValue);     
        
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  if (strcmp(topic,"animation")==0){
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    if ((char)payload[0] == '0') {  
      fadeValue = 0;
    } else if((char)payload[0] == '1'){
      fadeValue = ledMaxBrigthness;
    }
  } 

  if (strcmp(topic,"reset")==0){
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    char resetNumber[3];
    resetNumber[0] = payload[0];
    resetNumber[1] = payload[1];
    String one = resetNumber;
    
    Serial.println("");
    Serial.println(one);
    
    if(one == resetID){  /// HARD CODE HERE THE ESP IDENTIFIER
      Serial.println("Resetting");
      digitalWrite(GPIO0, HIGH);
      digitalWrite(GPIO15, LOW);

      WiFi.forceSleepBegin(); 
      wdt_reset(); 
      ESP.restart(); 
      while(1){
        wdt_reset();
      }
    }
  }   
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(id)) { // IMPORTANT connect each ESP with a unique identifier
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("test", "hello world");
      // ... and resubscribe
      client.subscribe("animation");
      client.subscribe("reset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void pixelsOn(int brightness) {
  for (int i = 0; i < NUMPIXELS; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.
    // delay(delayval); // Delay for a period of time (in milliseconds).

  }
}


void pixelsOff() {
  for (int i = 0; i < NUMPIXELS; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.
    // delay(delayval); // Delay for a period of time (in milliseconds).

  }
}


