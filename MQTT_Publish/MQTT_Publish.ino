#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "IAAC-WIFI";
const char* password = "enteriaac2013";
const char* mqtt_server = "192.168.5.225";

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("test", "hello world");
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


int sampleNumber = 10;
int total = 0;
int threshold = 0;
int addtoBaseReading = 15;

void setMicThreshold(int i){
      if(i<10){
        Serial.println("Setting Up Mic Threshold");
        Serial.print("Itteration number: ");
        Serial.print(i);
        Serial.print("   Got reading :");
        total += analogRead(A0);
        delay(10);
        Serial.println(analogRead(A0));
      }
      if(i==10){
        total = total / sampleNumber;
        Serial.print("Threshold set to: ");
        threshold = total + addtoBaseReading;  // add a small value to the baseline 
        Serial.println(threshold);
        }
}


void setup() {
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
}


int ledOn = 0;
int micVal;
int ledOnTime = 2000; // How long th
int timerLed = 0;
int micThsCounter = 0;


void loop() {

if(micThsCounter <= 11){
   setMicThreshold(micThsCounter);
   micThsCounter++;
  }  
  

  long now = millis();

  if(ledOn == 0){
    micVal = analogRead(A0);
    delay(10);
    //Serial.println("Ledoff");

      if(micVal > threshold){
        ledOn = 1;
        timerLed = now;
        Serial.println("Got Hit!!");
        client.publish("test", "ESP - 2");
      }
  }
  

  if(ledOn == 1){
    if(millis() < timerLed + ledOnTime){
      digitalWrite(2,LOW);
     // Serial.println("Lit up led");
    }
    else{
      ledOn = 0;
      digitalWrite(2,HIGH);
    }
  }

  

    if (!client.connected()) {
    reconnect();
  }
  client.loop();


}
