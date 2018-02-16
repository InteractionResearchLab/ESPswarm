#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/// IMPORTANT change the last digit of the following three lines to give unique identifier
const char* assigned_id = "06";
const char* id = "ESP06";
bool networkCapable = true;

const char* resetID = assigned_id;

// SYSTEM CONFIG
#define LED_PIN D3
#define ANALOG_READ_PIN A0
#define NUMPIXELS 7

// RESET PINS
#define GPIO0 D3
#define GPIO15 D8

bool RGB_VALUE_CONSOLE_MONITOR = false;
bool SENSOR_VALUE_CONSOLE_MONITOR = true;
bool HIT_INTENSITY_MODE = true;
bool DEBUG_MODE = true;
bool DELAY_MODE = true;
int delayDuration = 1;



// WIFI CONFIG - Update these with values suitable for your network.
WiFiClient wifiClient;
//
//const char* ssid = "IAAC-WIFI";
//const char* password = "enteriaac2013";
//const char* mqtt_server = "192.168.5.56"; // localhost

const char* ssid = "llum_installation_wifi";
const char* password = "99334994";
const char* mqtt_server = "192.168.1.150"; // Raspberry pi has a static ip 192.168.1.150

// MESSAGE BROKER CONFIG
const char* publishTopic = "events";
PubSubClient client(wifiClient);

// OTA CONFIG
bool ota_flag = true;
int time_elapsed = 0;



// LED CONFIG
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
bool LED_PULSE_MODE = false;
bool ledSwitched = false;
float fadeOutTimeDivider = 1;
int ledMaxBrightness = 250;
int ledMinBrightness = 60;
int pulseFactor = 1;
bool ANIMATION_MODE = false;

// SENSOR CONFIG
float persistenceMultiplier = 0.7;
int calibrationDuration = 150;
int analogSensorReading;
int analogValue;
int baselineValue;
int tresholdRatio;
int minTreshold;
float fadeValue;
float maxRatio = 255;
float redRatio = maxRatio;
float greenRatio = maxRatio;
float blueRatio = maxRatio;
int calculatedIntensity = 0;
int hitIntensityMin = 60;
int hitIntensityMax = 800;

char* unconfiguredSpheres[] = { "03","02","01","18","21","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60","61","62","63","64" };
char* configuredSpheres[] = { "04","05","06","07","08","09","10","11","12","13","14","15","16","17","20","22","23","24","25" };
char* problematicSpheres[] = { "19" };

void setup() {
  
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);    
   
  if(DEBUG_MODE){
    Serial.begin(115200);
    Serial.println(" ");
    Serial.print("ID : ");
    Serial.println(id);
    Serial.print("Mac Address : ");
    Serial.println(WiFi.macAddress());
  }

  if(!networkCapable){
    HIT_INTENSITY_MODE = false;
  }

  if(networkCapable){
    setup_wifi();
    setup_OTA();
  }
  
  // Initialize LEDs
  pixels.begin();
  pixels.show();

  if(networkCapable){
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
  }
  

  configureSphereType();

  if(DEBUG_MODE){
    Serial.println(" ");
    Serial.print("ChipID: ");
    Serial.println(id);
  }
}

void loop() {

  if(networkCapable){
    checkOTAflag();
  }
  

  readSensorValue();
  calculateTreshold();
  detectCollision();
  updateLEDs();

  if(networkCapable){
    ensureConnection();
  }
}

void configureSphereType(){
  // UNCONFIGURED SPHERES 
  Serial.print("Sphere#");
  Serial.print(assigned_id);
  for(int i = 0; i<sizeof(unconfiguredSpheres)/sizeof(*unconfiguredSpheres); i++){
    if(assigned_id == unconfiguredSpheres[i]){
      tresholdRatio = 40;
      minTreshold = 7;
      Serial.println(": unconfigured");
    }
  }
  
  // CONFIGURED SPHERES
  for(int i = 0; i<sizeof(configuredSpheres)/sizeof(*configuredSpheres); i++){
    if(assigned_id == configuredSpheres[i]){
      tresholdRatio = 40;
      minTreshold = 7;
      Serial.println(": configured");
    }
  }
  
  // PROBLEMATIC SPHERES 
  for(int i = 0; i<sizeof(problematicSpheres)/sizeof(*problematicSpheres); i++){
    if(assigned_id == problematicSpheres[i]){
      tresholdRatio = 40;
      minTreshold = 7;
      Serial.println(": problematic");
    }
  }
}



void setLedColor(int RedBrightness, int GreenBrightness, int BlueBrightness) {
  for (int i = 0; i < NUMPIXELS; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(RedBrightness, GreenBrightness, BlueBrightness)); // Moderately bright green color.

    pixels.show(); // This sends the updated pixel color to the hardware.
    // delay(delayval); // Delay for a period of time (in milliseconds).

  }
}

void checkOTAflag() {
  if (ota_flag) {
    setLedColor(0,ledMaxBrightness,0);
    delay(200);
    turnOffLed();
    delay(50);
    setLedColor(0,ledMaxBrightness,0);
    delay(200);
    turnOffLed();
    while (time_elapsed < 15000) {
      ArduinoOTA.handle();
      time_elapsed = millis();
      delay(2);
    }
    setLedColor(ledMaxBrightness,0,0);
    delay(200);
    turnOffLed();
    ota_flag = false;
  }
}


void setup_wifi() {

  if(DELAY_MODE){
    delay(15);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void setup_OTA(){
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");
  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");
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
}

void detectCollision() { 
  if(SENSOR_VALUE_CONSOLE_MONITOR){
    Serial.print(" signal:");
    Serial.print(analogValue);
    Serial.print(" (");
    Serial.print(baselineValue);
    Serial.print(") / treshold:");
    Serial.print(abs(analogValue - baselineValue));
    Serial.print(" (");
    Serial.print(baselineValue / tresholdRatio);
    Serial.print("|");
    Serial.print(minTreshold);
    Serial.print(") / LED:");
    Serial.print(fadeValue);
    Serial.println(" ) ");
  }
    
  if(abs(analogValue - baselineValue) > (baselineValue / tresholdRatio) && abs(analogValue - baselineValue) > minTreshold && calibrationDuration == 0){
    if(HIT_INTENSITY_MODE){
      calculatedIntensity = abs(analogValue - baselineValue) - (baselineValue / tresholdRatio); 
    }    
    registerHit();
  }  
  if(calibrationDuration > 0){
    calibrationDuration--;
  }  
}

void registerHit(){
  if(HIT_INTENSITY_MODE){
    Serial.print("calculated intensity: ");
    Serial.print(calculatedIntensity);
  }

  if(HIT_INTENSITY_MODE){

    int intensityRatio = (calculatedIntensity * ledMaxBrightness / hitIntensityMax);
    
    Serial.print(fadeValue);
    Serial.print(" vs ");
    Serial.print("fadeintensity//");
    Serial.print(intensityRatio);
    Serial.print(" vs ");
    Serial.println(calculatedIntensity);
    
    if(intensityRatio > fadeValue){        
      fadeValue = intensityRatio;
      if(fadeValue < ledMinBrightness) {
        fadeValue = ledMinBrightness;
      }
    }
  }else {
    fadeValue = ledMaxBrightness;
  }
  
  emitCollisionSignals();
}

void emitCollisionSignals() {
  if(!ANIMATION_MODE){
    client.publish(publishTopic, assigned_id, true);  
  }
  
  if(HIT_INTENSITY_MODE){
    if(HIT_INTENSITY_MODE){
      Serial.print("Intensity: ");
    }
    
    String hitReportDraft = "xx 000";
    hitReportDraft[0] = ((String)assigned_id)[0];
    hitReportDraft[1] = ((String)assigned_id)[1];
    int intensityDigits = ((String)calculatedIntensity).length();
    
    for(int i = 0; i < intensityDigits + 1; i++){
      hitReportDraft[6-i] = ((String)calculatedIntensity)[intensityDigits-i];
    }
  
    if(HIT_INTENSITY_MODE){
      Serial.println(hitReportDraft);
    }
    
    char hitReport[7];
    strncpy(hitReport, hitReportDraft.c_str(), sizeof(hitReport));
    hitReport[sizeof(hitReport) - 1] = '\0';
    client.publish("events-hit", hitReport);  
  }
  
}

void updateLEDs() {
  
  int redValue =  (int)(fadeValue * (int) redRatio / (maxRatio));
  int greenValue = (int)(fadeValue * (int) greenRatio / (maxRatio));
  int blueValue = (int)(fadeValue * (int) blueRatio / (maxRatio));  

  
  if(RGB_VALUE_CONSOLE_MONITOR){
    Serial.print("RGB values: ");
    Serial.print(redValue);
    Serial.print("/");
    Serial.print(greenValue);
    Serial.print("/");
    Serial.println(blueValue);
  }

  if(!ANIMATION_MODE){
    for(int i=0; i < NUMPIXELS; i++){
      if(LED_PULSE_MODE){
        pixels.setPixelColor(i, LEDpulsator(fadeValue * redRatio / maxRatio), LEDpulsator(fadeValue * greenRatio / maxRatio), LEDpulsator(fadeValue * blueRatio / maxRatio));       
      } else {
        pixels.setPixelColor(i, redValue, greenValue, blueValue);        
      }
    }
  }
  
  fadeValue -= fadeOutTimeDivider;
  if(fadeValue < 0) {
    fadeValue = 0;
  }
  
  pixels.show();
}

int LEDpulsator(float fadeNumeral) {
  return (int)(fadeNumeral * ( sin(pulseFactor*2*M_PI*fadeNumeral/255)  + 1) / 2);
}

void ensureConnection(){
  // Reconnect
  if (!client.connected()) {
    reconnect();
  }
  // DO NOT EDIT 
  client.loop();
  // DO NOT TAKE THIS DELAY(10) AWAY, IT IS NECESSARY FOR CLIENT LOOP TO CONNECT TO WIFI
  delay(10);
  // DO NOT EDIT 
}

void callback(char* topic, byte* payload, unsigned int messageLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("- ");

  // ANIMATE ALL
  if (strcmp(topic,"animation")==0){
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println(' ');
    if ((char)payload[0] == '0') {  
      fadeValue = 0;
    } else if((char)payload[0] == '1'){
      fadeValue = ledMaxBrightness;
    }
  } 

  // RESET CONDITIONS
  if (strcmp(topic,"reset")==0){
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println(' ');
    char sphereNumber[3];
    sphereNumber[0] = (char)payload[0];
    sphereNumber[1] = (char)payload[1];
    sphereNumber[2] = (char)'\0';
    String payloadID = sphereNumber;

    Serial.print("Comparison: ");
    Serial.print(payloadID);
    Serial.print(" vs ");
    Serial.println(resetID);
    
    if(payloadID == resetID){  /// HARD CODE HERE THE ESP IDENTIFIER
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

  
   // SET & ANIMATE COLOR 
  if (strcmp(topic,"animate-color")==0){
    char sphereNumber[3];
    char hexNumeral[8];
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    hexNumeral[0] = (char)payload[3];
    hexNumeral[1] = (char)payload[4];
    hexNumeral[2] = (char)payload[5];
    hexNumeral[3] = (char)payload[6];
    hexNumeral[4] = (char)payload[7];
    hexNumeral[5] = (char)payload[8];
    hexNumeral[6] = (char)payload[9];
    hexNumeral[7] = (char)'\0';
    String hexString = hexNumeral;

    sphereNumber[0] = (char)payload[0];
    sphereNumber[1] = (char)payload[1];
    sphereNumber[2] = (char)'\0';
    String payloadID = sphereNumber;

    Serial.print("Comparison: ");
    Serial.print(payloadID);
    Serial.print(" vs ");
    Serial.println(assigned_id);
    
    if(payloadID == assigned_id){  /// HARD CODE HERE THE ESP IDENTIFIER
      Serial.print("Lighting up LED for ");
      Serial.println(assigned_id);

      
      setSphereColor(hexString);
      calculatedIntensity = 500;
      registerHit();
    }
  }  


  // SET ESP COLOR 
  if (strcmp(topic,"set-color")==0){
    char sphereNumber[3];
    char hexNumeral[8];
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    hexNumeral[0] = (char)payload[3];
    hexNumeral[1] = (char)payload[4];
    hexNumeral[2] = (char)payload[5];
    hexNumeral[3] = (char)payload[6];
    hexNumeral[4] = (char)payload[7];
    hexNumeral[5] = (char)payload[8];
    hexNumeral[6] = (char)payload[9];
    hexNumeral[7] = (char)'\0';
    String hexString = hexNumeral;

    sphereNumber[0] = (char)payload[0];
    sphereNumber[1] = (char)payload[1];
    sphereNumber[2] = (char)'\0';
    String payloadID = sphereNumber;

    Serial.print("Comparison: ");
    Serial.print(payloadID);
    Serial.print(" vs ");
    Serial.println(assigned_id);
    
    if(payloadID == assigned_id){  /// HARD CODE HERE THE ESP IDENTIFIER
      Serial.print("Lighting up LED for ");
      Serial.println(assigned_id);
      
      setSphereColor(hexString);
    }
  } 

  // SET ALL ESPS COLOR 
  if (strcmp(topic,"set-all-color")==0){
    char hexNumeral[8];
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    hexNumeral[0] = (char)payload[0];
    hexNumeral[1] = (char)payload[1];
    hexNumeral[2] = (char)payload[2];
    hexNumeral[3] = (char)payload[3];
    hexNumeral[4] = (char)payload[4];
    hexNumeral[5] = (char)payload[5];
    hexNumeral[6] = (char)payload[6];
    hexNumeral[7] = (char)'\0';
    String hexString = hexNumeral;

    Serial.println("changing all esp color");
    
    setSphereColor(hexString);

  } 

  // SET ALL ESPS LED MAX BRIGHTNESS 
  if (strcmp(topic,"set-all-brightness-max")==0){
    char hexNumeral[8];
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    hexNumeral[0] = (char)payload[0];
    hexNumeral[1] = (char)payload[1];
    hexNumeral[2] = (char)payload[2];
    hexNumeral[3] = (char)'\0';
    String hexString = hexNumeral;

    Serial.println("changing all esp brighness max");

    ledMaxBrightness = hexString.toInt();;
  } 

  // SET ALL ESPS LED MIN BRIGHTNESS 
  if (strcmp(topic,"set-all-brightness-min")==0){
    char hexNumeral[8];
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    
    hexNumeral[0] = (char)payload[0];
    hexNumeral[1] = (char)payload[1];
    hexNumeral[2] = (char)'\0';
    String hexString = hexNumeral;

    Serial.println("changing all esp min brightness");
    
    ledMinBrightness = hexString.toInt();

  } 

  // ANIMATION CONDITIONS
  if (strcmp(topic,"animation-mode-on")==0){
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println(' ');
    char sphereNumber[3];
    sphereNumber[0] = (char)payload[0];
    sphereNumber[1] = (char)payload[1];
    sphereNumber[2] = (char)'\0';
    String payloadID = sphereNumber;

    Serial.print("Comparison: ");
    Serial.print(payloadID);
    Serial.print(" vs ");
    Serial.println(assigned_id);
    
    if(payloadID == assigned_id){  /// HARD CODE HERE THE ESP IDENTIFIER
      activateAnimationMode();
    }
  }  

  // ANIMATION CONDITIONS
  if (strcmp(topic,"animation-mode-off")==0){
    for (int i = 0; i < messageLength; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println(' ');
    char sphereNumber[3];
    sphereNumber[0] = (char)payload[0];
    sphereNumber[1] = (char)payload[1];
    sphereNumber[2] = (char)'\0';
    String payloadID = sphereNumber;

    Serial.print("Comparison: ");
    Serial.print(payloadID);
    Serial.print(" vs ");
    Serial.println(assigned_id);
    
    if(payloadID == assigned_id){  /// HARD CODE HERE THE ESP IDENTIFIER
      deactivateAnimationMode();
    }
  }  

  // SET ALL ESPS ANIMATION MODE ON
  if (strcmp(topic,"set-all-animation-mode-on")==0){
    Serial.println("setting on animation for all");
    activateAnimationMode();
  } 

  // SET ALL ESPS ANIMATION MODE ON
  if (strcmp(topic,"set-all-animation-mode-off")==0){
    Serial.println("setting off animation for all");
    deactivateAnimationMode();
  } 
  
  
}

void setSphereColor(String hexstring)
{
 
    long number = (long) strtol( &hexstring[1], NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    Serial.print("red value: ");
    Serial.println(r);
    Serial.print("green value: ");
    Serial.println(g);
    Serial.print("blue value: ");
    Serial.println(b);

    redRatio = r;
    greenRatio = g;
    blueRatio = b;
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
      client.subscribe("animate-color");
      client.subscribe("animation-mode-on");
      client.subscribe("animation-mode-off");
      client.subscribe("set-color");
      client.subscribe("set-all-color");
      client.subscribe("set-all-brightness-min");
      client.subscribe("set-all-brightness-max");
      client.subscribe("set-all-animation-mode-on");
      client.subscribe("set-all-animation-mode-off");
      
      client.subscribe("reset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}

void turnOffLed() {
  calculatedIntensity = 0;
}

void activateAnimationMode() {
  Serial.println("animation mode activated");
  ANIMATION_MODE = true;
}

void deactivateAnimationMode() {
  Serial.println("animation mode deactivated");
  ANIMATION_MODE = false;
}

