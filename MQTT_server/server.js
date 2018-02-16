// Import express framework and invoke initialization
var express = require('express');
var app = express();
// Create a http server
var http = require('http').Server(app);  
// Import the socket io connection layer
var io = require('socket.io')(http);

///app.use('*', express.static(path.join(__dirname, 'public')));
app.use(express.static('public'));

http.listen(3000, function () {
  
  console.log('');
  console.log(CrefFgGreen, 'LLUM 2018 balls installation', CrefReset);
  console.log('');
  console.log('initializing sync server...');
  console.log(CrefFgBlue, 'admin console listening on port 3000', CrefReset);
  console.log('');
});


io.on('connection', function(socket){
  console.log(CrefFgBlue, 'connected to console, adminID: ' + socket.id, CrefReset);
  
  socket.emit('confirmedConnection', { message: "socket succefully created" } );

  socket.on('disconnect', function(){
    console.log(CrefFgRed, 'disconnected from console, adminID: ' + socket.id, CrefReset);
  });

  // SIMULATION EVENTS
  socket.on('simulation-animation-hit', function(data){
    console.log(CrefDim, "admin command: <animation-hit> esp" + data.agent, CrefReset);
    mqttClient.publish('events', data.agent, { qos: 2 });
  });


});












// ******************************************
// ********* MQTT CONFIGURATION *************
// ******************************************

// Import the MQTT node package library
var mqtt = require('mqtt'); 
 
// Initialize MQTT connection, force clean flag to be false so connections are reestablished
var mqttClient = mqtt.connect('mqtt://192.168.1.150:1883', {  
    clean: false,
    clientId: 'nodeServer'
});

// Log connection status after connection is established
mqttClient.on('connect', (connack) => {  
  console.log('initializing MQTT connection log. . .');
  if (connack.sessionPresent) {
    console.log(CrefFgBlue, 'already subbed, no subbing necessary',CrefReset);
    mqttClient.publish('system', 'client reconnected', { qos: 2 });
  } else {
    console.log(CrefFgBlue, 'first session, subbing',CrefReset);
    mqttClient.subscribe('system', { qos: 2 });
    mqttClient.subscribe('events', { qos: 2 });
    //animateLEDS();
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
  console.log(CrefBright,`received message: <${topic}> '${message}'`,CrefReset);

  if(topic == "events"){
    io.emit('raw-stream-events', { message: `${message}` } );
  }
});

// animateLEDS();






// CONSOLE COLOR CONFIG 
var CrefReset = "\x1b[0m";
var CrefBright = "\x1b[1m";
var CrefDim = "\x1b[2m";
var CrefUnderscore = "\x1b[4m";
var CrefBlink = "\x1b[5m";
var CrefReverse = "\x1b[7m";
var CrefHidden = "\x1b[8m";

var CrefFgBlack = "\x1b[30m";
var CrefFgRed = "\x1b[31m";
var CrefFgGreen = "\x1b[32m";
var CrefFgYellow = "\x1b[33m";
var CrefFgBlue = "\x1b[34m";
var CrefFgMagenta = "\x1b[35m";
var CrefFgCyan = "\x1b[36m";
var CrefFgWhite = "\x1b[37m";

var CrefBgBlack = "\x1b[40m";
var CrefBgRed = "\x1b[41m";
var CrefBgGreen = "\x1b[42m";
var CrefBgYellow = "\x1b[43m";
var CrefBgBlue = "\x1b[44m";
var CrefBgMagenta = "\x1b[45m";
var CrefBgCyan = "\x1b[46m";
var CrefBgWhite = "\x1b[47m";