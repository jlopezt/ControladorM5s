var titulo = document.getElementById('Titulo');
var id = document.getElementById("Id");
var temp = document.getElementById("Temperatura");
var hum = document.getElementById("Humedad");
var luz = document.getElementById("Luz");

var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

function updateData(event)
{
  document.getElementById('valoresLeidos').innerHTML = event.data; 
		
  var msg = JSON.parse(event.data);
	
  console.log('JSON=' + event.data);
  console.log('titulo=' + msg.titulo);

  document.getElementById('Titulo').innerHTML = msg.titulo; 
  document.getElementById('JSON').innerHTML = event.data; 
}

function scheduleRequest() 
{
  console.log('Connect ' + new Date());
  connection.send("Datos");
 }
 
connection.onopen = function () 
{
  //connection.send('Connect ' + new Date());
  //console.log('Connect ' + new Date());
  //connection.send("Datos");
  scheduleRequest;
  console.log('peticion inicial enviada');
  setInterval(scheduleRequest, 5000);
  // Ejemplo 1, peticion desde cliente
  //(function scheduleRequest() {
  //   connection.send("");
  //   setTimeout(scheduleRequest, 100);
  //})();    
};

connection.onerror = function (error) 
{
  console.log('WebSocket Error ', error);
};

connection.onclose = function()
{
  console.log('WebSocket connection closed');
};

connection.onmessage = function (event) 
{
  console.log('Server: ', event.data);
  updateData(event);
};

