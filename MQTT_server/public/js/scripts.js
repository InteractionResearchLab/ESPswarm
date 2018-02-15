

// Initialize socket connection
var socket = io();
socket.on("confirmedConnection", function(){
  console.log("securely connected to the server");
});


// observe raw event stream
socket.on('raw-stream-events', function(data){
  console.log("stream-raw: " + data.message)

  var espIDselector = "esp" + data.message;
  var selectedESP = document.getElementById(espIDselector);

  selectedESP.classList.add("AgentLit");
  setTimeout(function(){
    selectedESP.classList.remove("AgentLit");
  }, 1000);

});

// add eventlisteners upon succesful window load
window.addEventListener("load", function() 
{
  displayMessageOnWebConsole("admin", "system", "initializing client connections");
  populateClients();
  setEventListeners();
});












//############# FUNCTION DEFINITIONS ####################//

// get list of agents to set
function setEventListeners(){
  
  var listOfAgents = document.getElementsByClassName("AgentClient");

  // assign click handlers
  for (i = 0; i < listOfAgents.length; i++) { 
    assignAgentClickListener(listOfAgents[i],i);
  }
}

// Assign click listener
function assignAgentClickListener(agent, index) {
  agent.addEventListener("click", handleAgentClick);
  console.log("agent #" + (index + 1) + " has click event handler assigned");
}


// Click handler
function handleAgentClick() {
  var agentIndex = this.id.slice(3) ;
  
  console.log("agent #" + agentIndex + " has been clicked");
  socket.emit("simulation-animation-hit", {agent: agentIndex});
  console.log("animation requested on agent #" + agentIndex);

  displayMessageOnWebConsole("admin", "system", "simulating hit on ESP#" + agentIndex);

}

function displayMessageOnWebConsole(author, type, message){
  console.log(author + " (" + type + "): " + message);
}

function populateClients(){
  
  var connectedClients = document.getElementsByClassName("ConnectedClients")[0]; 

  for(var i = 0; i<64; i++){
      // add the newly created element and its content into the DOM 
      connectedClients.appendChild(generateAgentUI(i)); 
      console.log("Agent #" + (i +1) + " UI element has been generated");
  }

}

function generateAgentUI(agentIndex){

  var agentIDNode = document.createElement("div");                
  
  if(agentIndex < 9){
    var agentNumeral = "0" + (agentIndex + 1);
  }else {
    var agentNumeral = agentIndex + 1;     
  }
  var agentIDText = document.createTextNode(agentNumeral);      
  agentIDNode.appendChild(agentIDText);                           
  agentIDNode.className += " AgentID";

  var agentStatusNode = document.createElement("div");            
  var agentStatusText = document.createTextNode("n/a");  
  agentStatusNode.appendChild(agentStatusText);                           
  agentStatusNode.className += " AgentStatus";

  var agentIPNode = document.createElement("div");                
  var agentIPText = document.createTextNode("x.x");       
  agentIPNode.appendChild(agentIPText);                   
  agentIPNode.className += " AgentIP";

  var agentClientNode = document.createElement("div");            
  agentClientNode.className += " AgentClient";
  agentClientNode.className += " AgentNotAvailable";
  agentClientNode.id = ("esp" + agentNumeral);
  agentClientNode.appendChild(agentIDNode);                       
  agentClientNode.appendChild(agentStatusNode);                   
  agentClientNode.appendChild(agentIPNode);                       
   
  return agentClientNode;
}