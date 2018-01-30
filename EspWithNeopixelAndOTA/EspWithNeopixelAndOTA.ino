#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            D3
#define NUMPIXELS      7
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second
int ledMaxBrightness = 150;

// Update these with values suitable for your network.
//
const char* ssid = "llum_installation_wifi";
const char* password = "99334994";
const char* mqtt_server = "192.168.1.150"; // Raspberry pi has a static ip 192.168.0.103

//const char* ssid = "IAAC-WIFI";
//const char* password = "enteriaac2013";
////const char* ssid = "la_derivada";
////const char* password = "4l4d3r1v4";


/// IMPORTANT change the last digit of the following two lines to give unique identifier
const char* id = "ESP8266Client-2" ;
const char* publishID = "ESP - 2";
char resetID= '2';




WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int ledPin = 2;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  if (strcmp(topic,"animation")==0){
  
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
      // Switch on the LED if an 1 was received as first character
      if ((char)payload[0] == '0') {
        //digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
        pixelsOff();
        // but actually the LED is on; this is because
        // it is acive low on the ESP-01)
      } else if((char)payload[0] == '1'){
        // digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
          pixelsOn(ledMaxBrightness);
      }
   } 

   if (strcmp(topic,"reset")==0){
      for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
      }
      Serial.println();
         if(char(payload[0]) == resetID){  /// HARD CODE HERE THE ESP IDENTIFIER
        // digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
        ESP.restart();
      }
   }   
}

/*
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      //digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
      pixelsOff();
      // but actually the LED is on; this is because
      // it is acive low on the ESP-01)
    } else if((char)payload[0] == '1'){
      // digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
        pixelsOn(ledMaxBrightness);
    } else if((char)payload[0] == '3'){
      // digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
      ESP.restart();
    }
}
*/


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


int sampleNumber = 10;
int total = 0;
int threshold = 0;
int addtoBaseReading = 10;

void setMicThreshold(int i) {
  if (i < 10) {
    Serial.println("Setting Up Mic Threshold");
    Serial.print("Itteration number: ");
    Serial.print(i);
    Serial.print("   Got reading :");
    total += analogRead(A0);
    delay(10);
    Serial.println(analogRead(A0));
  }
  if (i == 10) {
    total = total / sampleNumber;
    Serial.print("Threshold set to: ");
    threshold = total + addtoBaseReading;  // add a small value to the baseline
    Serial.println(threshold);
  }
}



void setup() {
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  // char chipId = ESP.getChipId();
  Serial.println(" ");
  Serial.println(" ");
  Serial.print("ID : ");
  Serial.println(id);
  Serial.print("Mac Address : ");
  Serial.println(WiFi.macAddress());
  
  
 
  

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

  pixels.begin();
  digitalWrite(2,HIGH);
}


bool ota_flag = true;
int time_elapsed = 0;
int ledOn = 0;
int micVal;
int ledOnTime = 1500; // How long the led stays HIGH
int timerLed = 0;
int micThsCounter = 0;
int brightness = 0;


void loop() {
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
    ota_flag = false;
  }


  if (micThsCounter <= 11) {
    setMicThreshold(micThsCounter);
    micThsCounter++;
  }


  long now = millis();

  if (ledOn == 0) {
    micVal = analogRead(A0);
    delay(5);
    //Serial.println("Ledoff");

    if (micVal > threshold) {
      ledOn = 1;
      timerLed = now;
      Serial.println("Got Hit!!");
      Serial.println("OTA WORKS No Auth with Reset!");
      client.publish("test", publishID);
      brightness = 150;
    }
  }


  if (ledOn == 1) {
    if (millis() < timerLed + ledOnTime) {
      int b = timerLed + ledOnTime - millis();
      //Serial.println(b);
      if (brightness > 0) {
        brightness = map(b, 0, ledOnTime, 0, ledMaxBrightness);
        // Serial.println(brightness);
      }
      pixelsOn(brightness);
      // Serial.println("Lit up led");
    }
    else {
      ledOn = 0;
      pixelsOff();
    }
  }

//  if (!client.connected()) {
//    reconnect();
//  }
  client.loop();
  
}



