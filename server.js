var mqtt = require('mqtt');  
var mqttClient = mqtt.connect('mqtt://localhost:1883', {  
    clean: false,
    clientId: 'nodeServer'
});

// suzi was here

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

function animateLEDS() {
    setInterval(
        function(){
            mqttClient.publish('animation', '0', { qos: 1 });
            setTimeout(
                function(){
                    mqttClient.publish('animation', '1', { qos: 1 });
                }
                , 1000
            );
        }
        , 2000
    );
}

mqttClient.on('message', (topic, message) => {  
  console.log(`Received message: '${message}'`);
});

