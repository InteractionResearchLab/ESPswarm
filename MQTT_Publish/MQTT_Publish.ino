/*
 * ESP8266 (Adafruit HUZZAH) Mosquitto MQTT Publish Example
 * Thomas Varnish (https://github.com/tvarnish), (https://www.instructables.com/member/Tango172)
 * Made as part of my MQTT Instructable - "How to use MQTT with the Raspberry Pi and ESP8266"
 */
#include <Bounce2.h> // Used for "debouncing" the pushbutton
#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker

const int ledPin = 2; // This code uses the built-in led for visual feedback that the button has been pressed
const int buttonPin = 13; // Connect your button to pin #13

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IAAC-WIFI";
const char* wifi_password = "enteriaac2013";

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "192.168.5.71";
const char* mqtt_topic = "test";
const char* mqtt_topic_subscribe = "animation";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "espSerhan";

bool LEDlit = false;

void ReceivedMessage(char* topic, byte* payload, unsigned int length) {
  // Output the first character of the message to serial (debug)
  Serial.print("Received Message fron Animantion");
  Serial.println((char)payload[0]);

  // Handle the message we received
  // Here, we are only looking at the first character of the received message (payload[0])
  // If it is 0, turn the led off.
  // If it is 1, turn the led on.
  if ((char)payload[0] == '0') {
    digitalWrite(ledPin, HIGH); // Notice for the HUZZAH Pin 0, HIGH is OFF and LOW is ON. Normally it is the other way around.
  }
  if ((char)payload[0] == '1') {
    digitalWrite(ledPin, LOW);
  }
}






// Initialise the Pushbutton Bouncer object
Bounce bouncer = Bounce();

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Switch the on-board LED off to start with
  digitalWrite(ledPin, HIGH);

  // Setup pushbutton Bouncer object
  bouncer.attach(buttonPin);
  bouncer.interval(5);

  // Begin Serial on 115200
  // Remember to choose the correct Baudrate on the Serial monitor!
  // This is just for debugging purposes
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //// subscribe code
  // Connect to MQTT Broker
  // setCallback sets the function to be called when a message is received.
  client.setCallback(ReceivedMessage);
  //// subscribe code

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
 // if (client.connect(clientID, mqtt_username, mqtt_password)) {
   if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

}


bool Connect() {
  // Connect to MQTT Server and subscribe to the topic
  //if (client.connect(clientID, mqtt_username, mqtt_password)) {
  if (client.connect(clientID)) {
      client.subscribe(mqtt_topic_subscribe);
      Serial.println("Subscribed to : animation");
      return true;
    }
    else {
      return false;
  }
}


void sendDataToMqtt(int data){

    client.publish(mqtt_topic, "1");
    Serial.println("Button pushed and message sent!");
    
//  if (client.publish(mqtt_topic, "1")) {
//      //char buf[4];
//      //client.publish(mqtt_topic, itoa(data,buf,10));
//      Serial.println("Button pushed and message sent!");
//  }
//
//  else {
//      Serial.println("Message failed to send. Reconnecting to MQTT Broker and trying again");
//     // client.connect(clientID, mqtt_username, mqtt_password);
//      client.connect(clientID);
//      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
//     // client.publish(mqtt_topic, "Button pressed!");
//    }
  }


void lightLed(){
    if(LEDlit){
      digitalWrite(2,LOW);
    } else {
      digitalWrite(2,HIGH);      
    }
}


int counter = 0;
int threshold = 50;

void loop() {

if(!client.connected()){
    Connect();
    Serial.println("Reconnected");
  }
  client.loop();

  int data = analogRead(A0);
  if(data > threshold){
    sendDataToMqtt(data);
    lightLed();
  }

}


