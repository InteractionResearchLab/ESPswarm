
console.log("lets see if our script is working");


// Initialize socket connection
var socket = io();
socket.on("confirmedConnection", function(){
  console.log("securely connected to the server");
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
  console.log("Agent #" + index + " has click event handler assigned");
}


// Click handler
function handleAgentClick() {
  var agentIndex = this.childNodes[1].innerHTML.slice(1) ;
  console.log(agentIndex);
  console.log("Agent #" + agentIndex + " clicked");
  socket.emit("simulation-animation-hit", {agent: agentIndex});
  console.log("Simulating Animation on Agent #" + agentIndex);

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
      console.log("Agent #" + i + " UI element has been generated");
  }

}

function generateAgentUI(agentIndex){

  var agentIDNode = document.createElement("div");                // Create a <div> node
  var agentIDText = document.createTextNode(agentIndex);          // Create a text node
  agentIDNode.appendChild(agentIDText);                           // Append the text to <div>
  agentIDNode.className += " AgentID";

  var agentStatusNode = document.createElement("div");            // Create a <div> node
  var agentStatusText = document.createTextNode("init");  // Create a text node
  agentStatusNode.appendChild(agentStatusText);                           // Append the text to <div>
  agentStatusNode.className += " AgentStatus";

  var agentIPNode = document.createElement("div");                // Create a <div> node
  var agentIPText = document.createTextNode("x.x");       // Create a text node
  agentIPNode.appendChild(agentIPText);                           // Append the text to <div>
  agentIPNode.className += " AgentIP";

  var agentClientNode = document.createElement("div");            // Create a <div> node
  agentClientNode.className += " AgentClient";
  agentClientNode.appendChild(agentIDNode);                       // Append the children to main div
  agentClientNode.appendChild(agentStatusNode);                   // Append the children to main div
  agentClientNode.appendChild(agentIPNode);                       // Append the children to main div
   
  return agentClientNode;
}