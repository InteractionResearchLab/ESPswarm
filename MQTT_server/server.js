// Import the MQTT node package library
var mqtt = require('mqtt');  

// Initialize MQTT connection, force clean flag to be false so connections are reestablished
var mqttClient = mqtt.connect('mqtt://localhost:1883', {  
    clean: false,
    clientId: 'nodeServer'
});

// Log connection status after connection is established
mqttClient.on('connect', (connack) => {  
  if (connack.sessionPresent) {
    console.log('Already subbed, no subbing necessary');
    mqttClient.publish('test', 'client reconnected', { qos: 1 });
  } else {
    console.log('First session! Subbing.');
    mqttClient.subscribe('test', { qos: 1 });
    animateLEDS();
  }
});   

// Function to animate the LEDs
function animateLEDS() {
    setInterval(
        function() {console.log("test script");}
        , 2000
    );
}

// Log every message that's received on the broker
mqttClient.on('message', (topic, message) => {  
  console.log(`Received message: '${message}'`);
});

animateLEDS();


