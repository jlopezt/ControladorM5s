/**************************************************************/
/*                                                            */
/* Servidor web embebido en el el termostato para su gestion  */ 
/* sirve paginas almnacenadas internamente y paginas generadas*/
/* de manera dinamica a partir de la informacion actual del   */
/* servicio                                                   */
/*                                                            */
/* Version de la remodelacion: 3.1.1                          */
/*                                                            */
/***************************************************************/
#define PUERTO_WEBSERVER  80

#include <WebServer.h>

WebServer server(PUERTO_WEBSERVER); //ESP8266WebServer server(PUERTO_WEBSERVER);

//version para integrar en otras paginas
const String cabeceraHTMLlight = "<!DOCTYPE html>\n<head>\n<meta charset=\"UTF-8\" />\n<TITLE>Domoticae</TITLE><link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\"></HEAD><html lang=\"es\">\n<BODY>\n"; 
const String pieHTMLlight="</body>\n</HTML>\n";

const String hablaHTML="<html><head></head><body><input type=\"text\"><button>speech</button><script>var d = document;d.querySelector('button').addEventListener('click',function(){xhr = new XMLHttpRequest();xhr.open('GET','/speech?phrase='+encodeURIComponent(d.querySelector('input').value));xhr.send();});</script></body></html>";

void inicializaWebServer(void)
  {
  //decalra las URIs a las que va a responder
  server.on("/", HTTP_ANY, handleMain); //layout principal
  server.on("/nombre", handleNombre); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  
  server.on("/root", HTTP_ANY, handleRoot); //devuleve el frame con la informacion principal
  server.on("/estado", handleEstado); //devuelve un JSON con las medidas, reles y modo para actualizar la pagina de datos
  server.on("/modo", HTTP_ANY, handleModoCalefaccion);

  server.on("/configHabitaciones", HTTP_ANY, handleConfigHabitaciones); 
  server.on("/datos", handleDatos); //devuelve el JSON con todos los datos operacionales

  server.on("/consignaTemperatura", HTTP_ANY, handleConfigConsignas);//Configuracion de las dos consignas (dia/noche)
  server.on("/configTabla", HTTP_ANY, handleConfigTabla);  //Configuracion de la tabla de consignas (cual aplica en cada momento)
  server.on("/recargaMapa", HTTP_ANY, handleRecargaMapa);  //Lee el fichero de la tabla de consignas segun hora y lo carga en memoria
  server.on("/recargaFicheroNombres", HTTP_ANY, handleRecargaFicheroNombres); //Lee el fichero de nombres de termometros y lo carga en memoria

  server.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero

  server.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero 

  server.on("/info", HTTP_ANY, handleInfo);  //URI de test
  server.on("/restart", HTTP_ANY, handleRestart);  //URI de test

  server.on("/habla", HTTP_ANY,handleHabla); //Se introduce una cadena por pantalla y la envia al GHN configurado
/**************ACTUALIZADO****************/  
  
  server.on("/consultaTemperatura", HTTP_ANY, handleConsultaTemperatura); //Manda una locucion al GH
   
  //load editor
  server.on("/upload", HTTP_GET, []() {
    if (!handleFileRead("/upload.html")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  
  //first callback is called after the request has ended with all parsed arguments, second callback handles file uploads at that location  
  server.on("/upload",  HTTP_POST, []() {  // If a POST request is sent to the /upload.html address,
    server.send(200, "text/plain", "Subiendo..."); 
  }, handleFileUpload);                       // go to 'handleFileUpload'
  
  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();
  Serial.println("HTTP server started");
  }

void webServer(int debug)
  {
  server.handleClient();
  }

void reinicializaWebServer(void)
  {
  server.begin();  
  }

/**************************** Handels ***************************************/
void handleMain() 
  {
  server.sendHeader("Location", "/main.html",true); //Redirect to our html web page 
  server.send(302, "text/html","");    
  }

void handleRoot() 
  {
  server.sendHeader("Location", "/root.html", true); //Redirect to our html web page */
  server.send(302, "text/html","");    
  }

 void handleConfigHabitaciones()
  {
  server.sendHeader("Location", "/configHabitaciones.html", true); //Redirect to our html web page */
  server.send(302, "text/html","");      
  }
  
 void handleDatos()
  {
  String cad=generaJsonDatos();
  //String cad="{\"titulo\":\"Controlador_termostato\",\"medida\":22.82,\"consigna\":23,\"modo\":2,\"tics\":720,\"habitaciones\":[{\"id\":0,\"nombre\":\"Salon\",\"temperatura\":23.5,\"humedad\":44.1,\"luz\":0,\"peso\": 1,\"tiempo\":33},{\"id\":2,\"nombre\":\"DormitorioPpal\",\"temperatura\":22.8,\"humedad\":48.3,\"luz\":0,\"peso\": 1,\"tiempo\":33},{\"id\":4,\"nombre\":\"Sara\",\"temperatura\":22.9,\"humedad\":66.3,\"luz\":2,\"peso\": 1,\"tiempo\":33},{\"id\":6,\"nombre\":\"Buhardilla\",\"temperatura\":21.4,\"humedad\":48.7,\"luz\":35,\"peso\": 1,\"tiempo\":33},{\"id\":7,\"nombre\":\"Bodega\",\"temperatura\":20.6,\"humedad\":46.4,\"luz\":0,\"peso\": 1,\"tiempo\":33},{\"id\":9,\"nombre\":\"Exterior\",\"temperatura\":23.4,\"humedad\":44,\"luz\":0,\"peso\": 1,\"tiempo\":33}],\"reles\":[{\"id\":0,\"nombre\":\"Calefaccion\",\"estado\":1},{\"id\":1,\"nombre\":\"Seguridad\",\"estado\":1}]}";
  server.send(200,"text/json",cad);
  }
 
void handleEstado() 
  {
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  
  JsonObject& medidas = root.createNestedObject("medidas");
  medidas["temperatura"] = getTemperaturaPromedio();
  medidas["consigna"] = getConsigna();
  medidas["humedad"] = getHumedadPromedio();
  
  JsonObject& salidas = root.createNestedObject("salidas");
  salidas["caldera"] = getEstadoRele(CALDERA);
  salidas["seguridad"] = getEstadoRele(SEGURIDAD);
  
  JsonArray& modo = root.createNestedArray("modo");
  modo.add(getModoManual());
  modo.add(getDownCounter());

  String cad="";
  root.printTo(cad);
  server.send(200,"text/json",cad);
  }

void handleNombre()
  {
  const size_t capacity = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["nombre"] = nombre_dispositivo;
  root["version"] = VERSION;
  
  String cad="";
  root.printTo(cad);
  server.send(200,"text/json",cad);
  }
  
void handleConfigConsignas(void)
  {
  //dia
  if(server.hasArg("consignaDia")) setConsignaD(server.arg("consignaDia").toFloat());  
  //noche
  if(server.hasArg("consignaNoche")) setConsignaN(server.arg("consignaNoche").toFloat());
  
  server.sendHeader("Location", String("/consignas.html?dia=") +String(getConsignaDia()) + \
                                         "&noche=" + String(getConsignaNoche()), \
                                         true); //Redirect to our html web page 
  server.send(302, "text/html","");        
  }
  
void handleConfigTabla()
  {
  char cadLarga[192];
  for(int i=0;i<192;i++) cadLarga[i]=0;

  if(server.hasArg("cadena")) 
    {
    strncpy(cadLarga,server.arg("cadena").c_str(),192);
    rellenaMapa(cadLarga);
    generaConfiguracionMapa();//saco el mapa a fichero 
    handleDatos();//handleRoot();
    return;
    }
    
  //manda la pagina del mapa
  //calculo la cadena
  String cad="";
  for(uint8_t i=0;i<48;i++) 
    {
    if(cad!="") cad += SEPARADOR;
    cad += mapa[i];
    }
  server.sendHeader("Location", String("/configTabla.html?entrada=") + cad,true); //Redirect to our html web page 
  server.send(302, "text/html","");  
  }

/*********************************************/
/*                                           */
/*  Servicio de recarga del                  */
/*  mapa de configuracion de consignas       */
/*                                           */
/*********************************************/
void handleRecargaMapa(void) 
  {
  String mensaje;
  
  if(leeFicheroMapa()) mensaje="Mapa de configuracion de consignas leido";
  else mensaje="Error al leer el mapa de configuracion de consignas ";
      
  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");
  }

/*********************************************/
/*                                           */
/*  Servicio de recarga del fichero          */
/*  de nombres de satelites                  */
/*                                           */
/*********************************************/
void handleRecargaFicheroNombres(void) 
  {
  String mensaje;
  
  if(inicializaSatelites()) mensaje="Mapa de termometros leido";
  else mensaje="Error al leer el mapa de termometros";
  
  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String mensaje="Reiniciando...";

  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");
  
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Servicio de configuracion del            */
/*  modo manual de la calefaccion            */
/*                                           */
/*********************************************/
void handleModoCalefaccion(void)
  {
  int duracion=-1;
  
  if(server.hasArg("modo")) 
    {    
    int modo=server.arg("modo").toInt(); 
    //se puede dart al duracion en ticks, en segundos o en horas
    if(server.hasArg("duracion")) duracion=server.arg("duracion").toInt(); //si va en ticks (ms) tiene prefecrencia
    else if(server.hasArg("tiempo")) duracion=seg2ticks(server.arg("tiempo").toInt()); //si no, en segundos
    else if(server.hasArg("horas")) duracion=seg2ticks(3600*server.arg("horas").toInt()); //si no, en horas. Hecho para google home
    if (modo>=0) setModoManual(modo, duracion);    
    //Serial.printf("modo: %i | duracion: %i\n",modo,duracion);
    }  

  handleRoot();
  return;
  }

/************************* INFO *********************************************/
/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad=cabeceraHTMLlight;

  cad += "<TABLE border=\"1\" width=\"50%\" cellpadding=\"1\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
//  cad += "<BR>-----------------info general-----------------<BR>";
  cad += "<TR clas='modo2'>\n"; 
  cad += "<td colspan='2' class=\"modo1\">info general</td>\n";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Hora actual</td>\n";
  cad += "<td>" + String(getHora()) + "</td>\n";
//  cad += "Hora actual: " + getHora(); 
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Fecha actual</td>\n";
  cad += "<td>" + String(getFecha()) + "</td>\n";
//  cad += "Fecha actual: " + getFecha(); 
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Uptime</td>\n";
  cad += "<td>" + String(uptime())+ " ms</td>\n";
//  cad += "Uptime: " + String(uptime())+ "ms";
  cad += "</TR>\n"; 
  
  cad += "<TR>\n"; 
  cad += "<td>IP</td>\n";
  cad += "<td>" + String(getIP(debugGlobal)) + "</td>\n";
//  cad += "IP: " + String(getIP(debugGlobal));
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Brillo</td>\n";
  cad += "<td>" + String(brilloPantalla) + "</td>\n";
//  cad += "Brillo: " + String(brilloPantalla);
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Limite sleep</td>\n";
  cad += "<td>" + String(limiteSleep)+ "ms</td>\n";
//  cad += "Limite sleep: " + String(limiteSleep)+ "ms";
  cad += "</TR>\n"; 


//  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "<TR clas='modo2'>\n"; 
  cad += "<td colspan='2' class=\"modo1\">info MQTT</td>\n";
  cad += "</TR>\n"; 
  
  cad += "<TR>\n"; 
  cad += "<td>IP broker</td>\n";
  cad += "<td>" + getIPBroker() + "</td>\n";
//  cad += "IP broker: " + getIPBroker();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>URL broker</td>\n";
  cad += "<td>" + getBrokerDir() + "</td>\n";
//  cad += "URL broker: " + BrokerDir.toString();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>modo</td>\n";
  cad += "<td>" + getPuertoBroker() + "</td>\n";
//  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>modo</td>\n";
  cad += "<td>" + getModoMQTT() + "</td>\n";
//  cad += "modo: " + modoMQTT.toString();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>usuario</td>\n";
  cad += "<td>" + getUsuarioMQTT() + "</td>\n";
//  cad += "Usuario: " + usuarioMQTT="";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>password</td>\n";
  cad += "<td>" + getPasswordMQTT() + "</td>\n";
//  cad += "Password: " + passwordMQTT="";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Topic Root</td>\n";
  cad += "<td>" + getTopicRoot() + "</td>\n";
//  cad += "Topic root: " + topicRoot="";
  cad += "</TR>\n"; 
    
//  cad += "<BR>-----------------info WiFi-----------------<BR>";
  cad += "<TR clas='modo2'>\n"; 
  cad += "<td colspan='2' class=\"modo1\">info WiFi</td>\n";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>base</td>\n";
  cad += "<td>" + String(WiFi.SSID()) + "</td>\n";
//  cad += "Base: " + String(WiFi.SSID());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>potencia</td>\n";
  cad += "<td>" + String(WiFi.RSSI()) + "</td>\n";
//  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "</TR>\n"; 
  
  
//  cad += "<BR>-----------------info Hardware-----------------<BR>";
  cad += "<TR clas='modo2'>\n"; 
  cad += "<td colspan='2' class=\"modo1\">info hardware</td>\n";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>base</td>\n";
  cad += "<td>" + String(ESP.getFreeHeap()) + "</td>\n";
//  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>ChipId</td>\n";
  cad += "<td>" + String(ESP.getChipRevision()) + "</td>\n";
//  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>SdkVersion</td>\n";
  cad += "<td>" + String(ESP.getSdkVersion()) + "</td>\n";
//  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>CpuFreqMHz</td>\n";
  cad += "<td>" + String(ESP.getCpuFreqMHz()) + " Mhz</td>\n";
//  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>FlashChipSize</td>\n";
  cad += "<td>" + String(ESP.getFlashChipSize()) + "</td>\n";
//  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>FlashChipSpeed</td>\n";
  cad += "<td>" + String(ESP.getFlashChipSpeed()) + "</td>\n";
//  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "</TR>\n"; 

//  cad += "<BR>-----------------info FileSystem-----------------<BR>";
  cad += "<TR clas='modo2'>\n"; 
  cad += "<td colspan='2' class=\"modo1\">info filesystem</td>\n";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Bytes totales</td>\n";
  cad += "<td>" + String(SPIFFS.totalBytes()) + "</td>\n";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Bytes usados</td>\n";
  cad += "<td>" + String(SPIFFS.usedBytes()) + "</td>\n";
  cad += "</TR>\n"; 

  cad += "</table>";

  cad += pieHTMLlight;
  server.send(200, "text/html", cad);     
  }
/**********************************************************************/

/************************* FICHEROS *********************************************/
/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String mensaje="";
  String nombreFichero="";
  String contenidoFichero="";
  
  boolean salvado=false;

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
      {
      handleListaFicheros();
      return;
      }
    else mensaje = "No se pudo salvar el fichero"; 
    }
  else mensaje = "Falta el argumento <nombre de fichero>"; 

  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");  
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String nombreFichero="";
  String mensaje="";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(!borraFichero(nombreFichero)) mensaje = "No se pudo borrar el fichero " + nombreFichero + ".\n";   
    else
      {
      handleListaFicheros();
      return;
      }
    }
  else  mensaje = "Falta el argumento <nombre de fichero>"; 
  
  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");        
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTMLlight;
  String nombreFichero="";
  String contenido="";
  
  cad += "<h1>" + String(NOMBRE_FAMILIA) + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";   
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado del        */
/*  fichero indicado, a traves de una        */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(void)
  {
  String nombreFichero="";
  String contenido="";
  String cad=cabeceraHTMLlight;
   
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {            
      cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";

    cad += "<form id=\"borrarFichero\" action=\"/borraFichero\">\n";
    cad += "  <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">\n";
    cad += "</form>\n";

    cad += "<form action=\"creaFichero\" target=\"_self\">";
      
      cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";      
      cad += "<caption>Fichero: " + nombreFichero + "(" + contenido.length() + ")</caption>";
      cad += "<tr><td colspan=\"2\">";      

      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    </td>";
      cad += "    </tr>"; 
           
      cad += "    <tr>";
      cad += "    <td>"; 
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "    </td>";

      cad += "    <td align=\"right\">\n";  
      cad += "    <button type=\"button\" onclick=\"document.getElementById('borrarFichero').submit();\">Borrar</button>\n";
      cad += "    </td>\n";
                  
      cad += "    </tr>"; 
      
      cad += "</table>";
      cad += "</form>";     
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(void)
  {
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;
  String cad=cabeceraHTMLlight;

  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";
    cad += "<br>\n";

    cad += "<table style=\"border: 0px; border-color: #FFFFFF;\">\n";
    cad += "<tr style=\"border: 0px; border-color: #FFFFFF;\">";
    cad += "<td style=\"vertical-align: top; border: 0px; border-color: #FFFFFF;\">";
    
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
    cad += "<caption>Lista de ficheros</caption>\n";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR>";
      cad += "<TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD>";
      cad += "<TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD>";
      cad += "</TR>";
      }
    cad += "</TABLE>\n";
    cad += "</td>";
    
    //Para crear un fichero nuevo
    cad += "<td style=\"vertical-align: top; border: 0px; border-color: #FFFFFF;\">";    
    //Para crear un fichero nuevo
    cad += "<table>";
    cad += "<caption>Crear un fichero nuevo:</caption>";
    cad += "<tr><td>";    
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</form>";
    cad += "</td></tr>";
    cad += "</table>";  

    cad += "</td>";
    cad += "</tr></table>";        
    }
  else cad += "<H2>No se pudo recuperar la lista de ficheros</H2>"; 

  cad += pieHTMLlight;
  server.send(200, "text/html", cad); 
  }
/**********************************************************************/

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  if(handleFileRead(server.uri()))return;
    
  String message = "";//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  message = "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
  server.send(404, "text/html", message);
  }

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) 
  { // send the right file to the client (if it exists)
  //Serial.println("handleFileRead: " + path);
  
  if (!path.startsWith("/")) path += "/";
  path = "/www" + path; //busco los ficheros en el SPIFFS en la carpeta www
  //if (!path.endsWith("/")) path += "/";
  
  String contentType = getContentType(path);               // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))    // If the file exists, either as a compressed archive, or normal
    { 
    if (SPIFFS.exists(pathWithGz)) path += ".gz";          // If there's a compressed version available, Use the compressed verion
                                               
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
    }
  //Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
  }

void handleFileUpload()
  {
  static File fsUploadFile;
  HTTPUpload& upload = server.upload();

  if(upload.status == UPLOAD_FILE_START)
    {
    String path = upload.filename;
    if(!path.startsWith("/")) path = "/" + path;
    
    Serial.printf("handleFileUpload Name: %s",path.c_str());
    fsUploadFile = SPIFFS.open(path.c_str(), "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    if(!fsUploadFile) Serial.printf("Error al crear el fichero\n");
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    else Serial.printf("Error al escribir en el fichero\n");
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    String mensaje="";  

    if(fsUploadFile) // If the file was successfully created
      {                                    
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      mensaje="Fichero subido con exito (" + String(upload.totalSize) + "bytes)";  
      }
    else mensaje="Se produjo un error al subir el fichero";  

    server.sendHeader("Location","/resultadoUpload.html?mensaje=" + mensaje, true);      // Redirect the client to the success page
    server.send(302);  
    }
  }
/******************************************************************************************************************************/

/****************************Google Home Notifier ******************************/
/*********************************************/
/*                                           */
/*  Lee la temperatura promedio y manda      */
/*  una locucion al GH                       */ 
/*                                           */
/*********************************************/  
void handleConsultaTemperatura(void)
  {
  String cad=cabeceraHTMLlight;
  char texto[255];
  
  sprintf(texto,"La temperatura actual es de %.1f grados",getTemperaturaPromedio());
  enviaNotificacion(texto);

  String mensaje="locución de temperatura enviada";
  
  server.sendHeader("Location", String("/mensaje.html?mensaje=") + mensaje,true); //Redirect to our html web page 
  server.send(302, "text/html","");
  }
  
/*********************************************/
/*                                           */
/*  Pide un texto por pantalla y lo envia    */
/*  al GHN para que lo lea en alto           */ 
/*                                           */
/*********************************************/  
void handleHabla() 
  {
  String phrase ="";
  
  if(server.hasArg("phrase")) 
    {
    phrase = server.arg("phrase");
  
    if (phrase == "") 
      {
      server.send(401, "text / plain", "query 'phrase' is not found");
      return;
      }
  
    if(enviaNotificacion(phrase.c_str())) server.send(200, "text / plain", "OK");
    server.send(404, "text / plain", "KO");  
    }
    
  server.sendHeader("Location", String("/habla.html"), true); //Redirect to our html web page 
  server.send(302, "text/html",""); 
  }
/**********************************************************************/
