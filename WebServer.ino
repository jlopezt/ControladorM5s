/************************************************************************************************
Servicio                                  URL                            Formato entrada         Formato salida                                              Comentario                                                                                    Ejemplo peticion                                              Ejemplo respuesta
Configuracion de la tabla de consignas    http://IP/configTabla          cadena=<cadena>         HTML                                                        Pinta la tabla de consignas. Si la recibe la carga                                            http://IP/configTabla?0|0|0...|127|127                        N/A
Configuracion de las consignas de Tª      http://IP/consignaTemperatura  cadena=<cadena>         HTML                                                        Pinta el formulario para introducir las consignas                                             http://IP/consignaTemperatura?consignaDia=22&consignaNoche=15 N/A
Numero de habitaciones                    http://IP/numeroHabitaciones   N/A                     <numHabitaciones> entero 0..255                             Devuelve el numero de habitaciones con satelite registrado                                    http://IP/numeroHabitaciones                                  4
Lista de las habitaciones                 http://IP/listaHabitaciones    N/A                     id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_SATELITES  Devuelve la lista de los ids y nombres de las habitaciones que se han registrado su satelite  http://IP/listaHabitaciones                                   1#Salon|2#despacho|3#DormitorioPpal
Valores de las medida en las habitaciones http://IP/valoresHabitaciones  id=<id>                 Temperatura|Humedad|Luz                                     Devuelve los valores de una habitacion                                                        http://IP/valoresHabitaciones?id=4                            23.5|72|88
Estado de los reles                       http://IP/estadoReles          id=<id>                 <estadoRele> 1/0                                            Devuelve el estado del rele indicado                                                          http://IP/estadoReles?id=1                                    1
Lista de los reles                        http://IP/listaReles           N/A                     id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_RELES      Devuelve la lista de los ids y nombres de los reles declarados en el controlador              http://IP/listaReles?id=1                                     1#Caldera|2#Riego
Configuracion de habitaciones             http://IP/configHabitaciones   id=<id>&nombre=<nombre> id|nombre                                                   Devuelve lo mismo pero de los valortes generados                                              http://IP/registroHabitacion?id=1&nombre=Salon                1|Salon
Errores de comunicacion                   http://IP/erroresComunicacion  N/A                     id|errores                                                  Devuelve el numero de errores de comunicacion de cada satelite                                http://IP/erroresComunicacion                                 1#2|3#0
Configuracion tabala de temperatura       http://IP/configTabla          cadena=<cadena>         {web configuracion}                                          Configura la tabla de temperaturas dia/noche y genera la pagina de configuracion          
Configura las consignas dia/noche         http://IP/consignaTemperatura  consignaDia/consignaNoche=<valor> {web configuracion}                               Configura la consigna de dida o de noche y genera la pagina de configuracion                  
Recarga del mapa de temperaturas          http://IP/recargaMapa

Reinicia el controlador                   http://IP/restart
Informacion del Hw del sistema            http://IP/info
Crea el fichero indicado                  http://IP/creaFichero
Borra el fichero indicado                 http://IP/borraFichero
Lee el fichero indicado                   http://IP/leeFichero
Informacion del sistema de ficheros       http://IP/infoFS
************************************************************************************************/
#define PUERTO_WEBSERVER  80
#define IDENTIFICACION "<BR><BR><BR><BR><BR>Modulo controlador. Version " + String(VERSION) + ".";

#include <WebServer.h> //#include <ESP8266WebServer.h>

WebServer server(PUERTO_WEBSERVER); //ESP8266WebServer server(PUERTO_WEBSERVER);

String cabeceraHTML = "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<meta charset=\"UTF-8\" />\n<HTML><HEAD><TITLE>Termostato domestico</TITLE></HEAD><BODY><h1><a href=\"../\" target=\"_self\">" + nombre_dispositivo + "</a><br></h1>\n";
String menuHTML = "<TABLE>\n<CAPTION>Opciones:</CAPTION>\n <TR><TD><a href=\"configTabla\" target=\"_blank\">Configuracion de la tabla de consignas</a></TD></TR>\n<TR><TD><a href=\"recargaMapa\" target=\"_blank\">Recarga del mapa de temperaturas</a></TD></TR>\n<TR><TD><a href=\"consignaTemperatura\" target=\"_blank\">Consigna de temperatura</a> </TD></TR> \n<TR><TD><a href=\"numeroHabitaciones\" target=\"_blank\">Numero de habitaciones</a> </TD></TR>\n<TR><TD><a href=\"listaHabitaciones\" target=\"_blank\">Lista de las habitaciones</a></TD></TR>\n<TR><TD><a href=\"valoresHabitaciones\" target=\"_blank\">Valores de las medida en las habitaciones</a></TD></TR>\n<TR><TD><a href=\"recargaFicheroNombres\" target=\"_blank\">Recarga del fichero de nombres de satelite</a></TD></TR>\n<TR><TD><a href=\"estadoReles\" target=\"_blank\">Estado de los reles</a></TD></TR>\n<TR><TD><a href=\"listaReles\" target=\"_blank\">Lista de los reles</a></TD></TR>\n<TR><TD><a href=\"configHabitaciones\" target=\"_blank\">Configuracion de habitaciones</a></TD></TR>\n<TR><TD><a href=\"modo\" target=\"_blank\">Modo?</a></TD><TD> | </TD><TD><a href=\"modo?modo=0 \" target=\"_blank\">Modo off</a></TD><TD> | </TD><TD><a href=\"modo?modo=1 \" target=\"_blank\">Modo on</a></TD><TD> | </TD><TD><a href=\"modo?modo=2 \" target=\"_blank\">Modo auto</a></TD></TR>\n</TABLE>\n";  
String pieHTML = "</BODY>\n</HTML>";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_blank\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_blank\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_blank\">Restart</a></TD></TR>\n<TR><TD><a href=\"estado\" target=\"_blank\">Estado</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_blank\">Lista ficheros</a></TD></TR>\n<TR><TD><a href=\"estadoSalidas\" target=\"_blank\">Estado salidas</a></TD></TR>\n<TR><TD><a href=\"estadoEntradas\" target=\"_blank\">Estado entradas</a></TD></TR>\n<TR><TD><a href=\"planes\" target=\"_blank\">Planes del secuenciador</a></TD></TR></TABLE>\n"; 
String hablaHTML="<html><head></head><body><input type=\"text\"><button>speech</button><script>var d = document;d.querySelector('button').addEventListener('click',function(){xhr = new XMLHttpRequest();xhr.open('GET','/speech?phrase='+encodeURIComponent(d.querySelector('input').value));xhr.send();});</script></body></html>";

void inicializaWebServer(void)
  {
  //decalra las URIs a las que va a responder
  server.on("/", HTTP_ANY, handleRoot); //web de temperatura
  server.on("/medida", HTTP_ANY, handleMedida); //web de temperatura
  server.on("/numeroHabitaciones", HTTP_ANY, handleNumeroHabitaciones); 
  server.on("/listaHabitaciones", HTTP_ANY, handleListaHabitaciones); 
  server.on("/valoresHabitaciones", HTTP_ANY, handleValoresHabitaciones); 
  server.on("/configHabitaciones", HTTP_ANY, handleConfigHabitaciones); 

  server.on("/modo", HTTP_ANY, handleModoCalefaccion);
   
  server.on("/estadoReles", HTTP_ANY, handleEstadoReles);
  server.on("/listaReles", HTTP_ANY, handleListaReles);

  server.on("/configTabla", HTTP_ANY, handleConfigTabla);  
  server.on("/consignaTemperatura", HTTP_ANY, handleConfigConsignas);

  server.on("/recargaMapa", HTTP_ANY, handleRecargaMapa);  
  server.on("/recargaFicheroNombres", HTTP_ANY, handleRecargaFicheroNombres);
  server.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  server.on("/info", HTTP_ANY, handleInfo);  //URI de test
    
  server.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
  server.on("/infoFS", HTTP_ANY, handleInfoFS);  //URI de info del FS

  server.on("/speech", handleSpeechPath);
  server.on("/habla", handleHablaPath);

  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", ""); 
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
  



void handleRoot() 
  {
  String cad="";
  
  //Rutas absolutas: menuHTML= "<TABLE> \n<TR><TD>Opciones:</TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/configTabla\" target=\"_blank\">Configuracion de la tabla de consignas</a></TD></TR>  \n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/consignaTemperatura\" target=\"_blank\">Consigna de temperatura</a> </TD></TR> \n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/numeroHabitaciones\" target=\"_blank\">Numero de habitaciones</a> </TD></TR>  \n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/listaHabitaciones  \" target=\"_blank\">Lista de las habitaciones</a></TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/valoresHabitaciones\" target=\"_blank\">Valores de las medida en las habitaciones</a></TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/estadoReles        \" target=\"_blank\">Estado de los reles</a></TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/listaReles         \" target=\"_blank\">Lista de los reles</a></TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/configHabitaciones \" target=\"_blank\">Configuracion de habitaciones</a></TD></TR>\n<TR><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/modo \" target=\"_blank\">Modo?</a></TD><TD> | </TD><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/modo?modo=0 \" target=\"_blank\">Modo off</a></TD><TD> | </TD><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/modo?modo=1 \" target=\"_blank\">Modo on</a></TD><TD> | </TD><TD><a href=\"http://" + String(getIP(debugGlobal)) + "/modo?modo=2 \" target=\"_blank\">Modo auto</a></TD></TR>\n</TABLE>\n";    

  cad = cabeceraHTML;
  cad += "<TABLE>\n";
  cad += "<TR><TD>Temperatura:</TD><TD>" + String(getTemperaturaPromedio(),1) + "ºC</TD></TR>\n";
  cad += "<TR><TD>Consigna:</TD><TD>" + String(getConsigna(),1) + "ºC</TD></TR>\n";  
  cad += "<TR><TD>Modo:</TD><TD>" + String(getModoManualTxt()) + "</TD>";
  if(getModoManual()!=MODO_AUTO) cad += "<TD>Ticks:</TD><TD>" + String(getDownCounter()) +  "</TD><TD>Segs:</TD><TD>" + String(ticks2seg(getDownCounter())) +  "</TD></TR>\n";
  else cad += "</TR>\n";
  cad += "<TR><TD>Reles</TD></TR>\n";
  cad += "<TR><TD>Nombre:</TD><TD>" + getNombreRele(CALDERA)   + "</TD><TD>Estado:</TD><TD>" + getEstadoRele(CALDERA)   + "</TD></TR>\n";
  cad += "<TR><TD>Nombre:</TD><TD>" + getNombreRele(SEGURIDAD) + "</TD><TD>Estado:</TD><TD>" + getEstadoRele(SEGURIDAD) + "</TD></TR>\n";
  cad += "</TABLE>";
  cad += "\n<BR>\n";
  cad += menuHTML;
    
  //Enlaces
  cad += "<BR><BR>\n";
  cad += enlaces;
  cad += "<BR><BR>" + nombre_dispositivo + " . Version " + String(VERSION) + ".";
  
  cad += pieHTML;
  
  server.send(200, "text/HTML", cad);  
  }

/*********************************************/
/*                                           */
/*  Servicio de recarga del                  */
/*  mapa de configuracion de consignas       */
/*                                           */
/*********************************************/
void handleRecargaMapa(void) 
  {
  leeFicheroMapa();
  
  String cad = "Mapa de configuracion de consignas leido";
  server.send(200, "application/json", cad);   
   
  Serial.println("Medidas requeridas ok"); 
  }

/*********************************************/
/*                                           */
/*  Servicio de recarga del fichero          */
/*  de nombres de satelites                  */
/*                                           */
/*********************************************/
void handleRecargaFicheroNombres(void) 
  {
  inicializaSatelites();//leeFicheroNombres();
  
  String cad = "Mapa de configuracion de consignas leido";
  server.send(200, "application/json", cad);   
   
  Serial.println("Medidas requeridas ok"); 
  }

/*********************************************/
/*                                           */
/*  Servicio de configuracion del            */
/*  modo manual de la calefaccion            */
/*                                           */
/*********************************************/
void handleMedida(void)
  {
  String cad=generaJson();
  
  server.send(200, "application/json", cad);   
   
  Serial.println("Medidas requeridas ok");  
  }
  
/*********************************************/
/*                                           */
/*  Servicio de configuracion del            */
/*  modo manual de la calefaccion            */
/*                                           */
/*********************************************/
void handleModoCalefaccion(void)
  {
  String cad;
  int duracion=0;
  
  if(server.hasArg("modo")) 
    {    
    int modo=server.arg("modo").toInt(); 
    //se puede dart al duracion en ticks o en segundos
    if(server.hasArg("duracion")) duracion=server.arg("duracion").toInt(); //si va en ticks tiene prefecrencia
    else if(server.hasArg("tiempo")) duracion=seg2ticks(server.arg("tiempo").toInt()); //si no, en segundos
    else if(server.hasArg("horas")) duracion=seg2ticks(3600*server.arg("horas").toInt()); //si no, en horas. Hecho para google home
    if (modo>=0) setModoManual(modo, duracion);    
    //Serial.printf("modo: %i | duracion: %i\n",modo,duracion);
    }  
     
  cad="Modo: " + getModoManualTxt();    
  server.send(200, "text/plain", cad);  
  }

/*********************************************/
/*                                           */
/*  Servicio de configuracion de             */
/*  la tabla de temperaturas                 */
/*                                           */
/*********************************************/
void handleConfigTabla() //A MEDIAS
  {
  char cadLarga[192];
  for(int i=0;i<192;i++) cadLarga[i]=0;

  if(server.hasArg("cadena")) 
    {
    //String cad=server.arg("cadena"); //la cadena tiene 48 valores para la tabla
    strncpy(cadLarga,server.arg("cadena").c_str(),192);
    rellenaMapa(cadLarga);
    }

  server.send(200, "text/HTML", preparaPaginaMapa());//cad2); 
  }

/*********************************************/
/*                                           */
/*  Servicio de configuracion de             */
/*  las consignas de temperaturas            */
/*                                           */
/*********************************************/
void handleConfigConsignas(void)
  {
  Serial.printf("Metodo HTTP: %i\nURI: %s\n",server.method(),server.uri().c_str());//enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };  
  for(int8_t i=0;i<server.args();i++) Serial.printf("argumento %i: nombre: %s - valor: %f\n",i,server.argName(i).c_str(),server.arg(i).toFloat());
  
  //dia
  if(server.hasArg("consignaDia")) setConsignaD(server.arg("consignaDia").toFloat());
  //noche
  if(server.hasArg("consignaNoche")) setConsignaN(server.arg("consignaNoche").toFloat());
    
  server.send(200, "text/HTML", preparaPaginaConsigna());
  }
 
/*********************************************/
/*                                           */
/*  Servicio de consulta del                 */
/*  Numero de habitaciones                   */
/*                                           */
/*********************************************/
void handleNumeroHabitaciones() 
  {
  String cad="";
  
  cad=numeroSatelites(debugGlobal);
  server.send(200, "text/plain", cad);      
  }
  
/*********************************************/
/*                                           */
/*  Servicio de consulta de la               */
/*  lista de habitaciones                    */
/*                                           */
/*********************************************/
void handleListaHabitaciones() 
  {
  String cad="";

  cad=listaSatelites(debugGlobal);
  server.send(200, "text/plain", cad);       
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de los              */
/*  valores medidos de las habitaciones      */
/*                                           */
/*********************************************/
void handleValoresHabitaciones() 
  {
  String cad="";
  int id;

  if(!server.hasArg("id")) 
    {
    String cad="";

    cad = "\n\nTemperatura promedio: ";
    cad += String(getTemperaturaPromedio(),1)+"C\n\n";
    
    for(int8_t id=0;id<MAX_SATELITES;id++)
      {
      if(sateliteRegistrado(id))
        {
        cad += "Satelite id: ";
        cad += habitaciones[id].id;
        cad += "\n";

        cad += "     Nombre: ";
        cad += habitaciones[id].nombre;
        cad += "\n";
        
        cad += "     Temperatura: ";
        cad += habitaciones[id].temperatura;
        cad += "\n";
        
        cad += "     Humedad: ";
        cad += habitaciones[id].humedad;
        cad += "\n";

        cad += "     Luz: ";
        cad += habitaciones[id].luz;
        cad += "\n";  
        }
      }
    server.send(200, "text/plain", cad);
    }
  else
    {
    id=server.arg("id").toInt();

    cad=valoresSatelite(id,debugGlobal);
    server.send(200, "text/plain", cad);       
    }
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de la               */
/*  configuracion de las habitaciones        */
/*                                           */
/*********************************************/
void handleConfigHabitaciones(void)
  {
  String cad="";
  int id;

  if(!server.hasArg("id")) 
    {
    String cad="";

    cad = "\n\nTemperatura promedio: ";
    cad += String(getTemperaturaPromedio(),1)+"C\n\n";
    
    for(int8_t id=0;id<MAX_SATELITES;id++)
      {
      if(sateliteRegistrado(id))
        {
        cad += "Satelite id: ";
        cad += habitaciones[id].id;
        cad += "\n";

        cad += "     Nombre: ";
        cad += habitaciones[id].nombre;
        cad += "\n";
        
        cad += "     Temperatura: ";
        cad += habitaciones[id].temperatura;
        cad += "\n";
        
        cad += "     Humedad: ";
        cad += habitaciones[id].humedad;
        cad += "\n";

        cad += "     Luz: ";
        cad += habitaciones[id].luz;
        cad += "\n";  

        cad += "     Peso: ";
        cad += habitaciones[id].peso[hora()];
        cad += "\n";

        cad += "     Lectura: ";
        cad += sateliteUltimaLectura(id); //habitaciones[id].lectura;
        cad += "ms, hace ";
        cad += millis()-sateliteUltimaLectura(id); //habitaciones[id].lectura;
        cad += "ms";
        cad += "\n"; 
        }
      }
    server.send(200, "text/plain", cad);
    }
  else
    {
    id=server.arg("id").toInt();

    cad=valoresSatelite(id,debugGlobal);
    server.send(200, "text/plain", cad);       
    }    
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta del                 */
/*  estado de los reles                      */
/*                                           */
/*********************************************/
void handleEstadoReles(void) 
  {
  server.send(200, "text/plain", leeEstadoReles());
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de la               */
/*  lista de reles                           */
/*                                           */
/*********************************************/
void handleListaReles() 
  {
  String cad="";

  for(int8_t i=0;i<MAX_RELES;i++) //id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_SATELITES
    {
      if (cad!="") cad += SEPARADOR;
      cad += i;
      cad += SUBSEPARADOR;
      cad += reles[i].nombre;
    }

  server.send(200, "text/plain", cad);           
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad=cabeceraHTML;
  cad += IDENTIFICACION

  cad += "<BR>-----------------info general-----------------<BR>";
  cad += "Uptime: " + String(uptime())+ "ms";
  cad += "<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";    
  cad += "Brillo: " + String(brilloPantalla);
  cad += "<BR>";
  cad += "Limite sleep: " + String(limiteSleep)+ "ms";
  cad += "<BR>";
  cad += "----------------------------------<BR>";
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/    
  cad += "<BR>-----------------info WiFi-----------------<BR>";
  /*
  byte mac[6]; 
  WiFi.macAddress(mac);
  cad += "MAC M5Stack: " + String(WiFi.macAddress());
  cad += "<BR>";  
  */
  cad += "Base: " + String(WiFi.SSID());
  cad += "<BR>";
  /*
  byte bssid[6];
  WiFi.BSSID(bssid);
  cad += "MAC AP: " + String(WiFi.BSSID());
  cad += "<BR>";
  */
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";
  cad += "----------------------------------<BR>";
  
  
  cad += "<BR>-----------------info Hardware-----------------<BR>";
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipRevision());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  

  cad += pieHTML;
  server.send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;

  cad += "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) cad += "Fichero salvado con exito<br>";
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
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
  String contenidoFichero="";
  String cad=cabeceraHTML;

  cad += "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(borraFichero(nombreFichero)) cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTML;
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

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee info del FS                          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfoFS(void)
  {
  String cad=cabeceraHTML;

  cad += "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  //inicializo el sistema de ficheros
  if (SPIFFS.begin(true)) 
    {
    Serial.println("---------------------------------------------------------------\nmounted file system");  
    cad += "totalBytes: ";
    cad += SPIFFS.totalBytes();
    cad += "<BR>usedBytes: ";
    cad += SPIFFS.usedBytes();
    Serial.println("unmounted file system\n---------------------------------------------------------------");
    }
    else cad += "Error al leer info";

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

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

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado de los     */
/*  ficheros en el sistema a traves de una   */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(void)
  {
  String nombreFichero="";
  String contenido="";
  String cad=cabeceraHTML;
  
  cad += "<h1>" + nombre_dispositivo + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

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

      cad += "<table><tr><td>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "  </p>";
      cad += "</form>";
      cad += "</td></tr></table>";
      
      cad += "<table>Modificar fichero<tr><td>";      
      cad += "<form action=\"creaFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    contenido del fichero: <br><textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    <BR>";
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "  </p>";
      cad += "</td></tr></table>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
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
  String cad=cabeceraHTML;

  cad += "<h1>" + nombre_dispositivo + "</h1>";
  cad += "<h2>Lista de ficheros</h2>";
  
  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD></TR>";
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/**********************************************************************/
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) 
  { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  
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
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
  }

void handleFileUpload()
  {
  File fsUploadFile;
  HTTPUpload& upload = server.upload();
  String path;
 
  if(upload.status == UPLOAD_FILE_START)
    {
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.startsWith("/www")) path = "/www"+path;
    if(!path.endsWith(".gz")) 
      {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
      }
      
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
    }
  else if(upload.status == UPLOAD_FILE_WRITE)
    {
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } 
  else if(upload.status == UPLOAD_FILE_END)
    {
    if(fsUploadFile) 
      {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      }
    else 
      {
      server.send(500, "text/plain", "500: couldn't create file");
      }
    }
  }

/******************************************************************************************************************************/
String preparaPaginaMapa(void)
  {
  String cad="";
  
  if(debugGlobal) Serial.println("Empezamos...");
  
  cad +="<!DOCTYPE html>\n";
  cad +="<html lang=\"es\">\n";
  cad +="<head>\n";
  cad +="    <meta charset=\"UTF-8\" />\n";
  cad +="\n";
  cad +="    <title>Crear celdas tabla HMTL con JavaScript</title>\n";
  cad +="\n";
  cad +="    <script type=\"text/javascript\">\n";
  cad +="     var total = new Array(48)\n";
  cad +="\n";
  cad +="     function Envia()\n";
  cad +="     {\n";
  cad +="     var cadena=\"\";\n";
  cad +="\n";
  cad +="     for(i=0;i<48;i++)\n";
  cad +="        {   \n";
  cad +="        cadena = cadena + document.salida[i].value;\n";
  cad +="        if(i<47) cadena = cadena + \"|\";\n";
  cad +="        }\n";
  cad +="      document.location.href=document.location.origin+document.location.pathname+\"?cadena=\" + cadena;\n";
  cad +="      //location.assign\n";
  cad +="      }\n";
  cad +="\n";
  cad +="     function Inicializa()\n";
  cad +="     {\n";
  cad +="     total=[";
  
  for(int i=0;i<48;i++)
    {
    cad +=mapa[i];
    if(i<47) cad += ",";
    }
  
  cad +="];\n";     
  cad +="     for(i=0;i<48;i++) document.salida[i].value=total[i];\n";
  cad +="     }\n";
  cad +="    </script>\n";
  cad +="</head>\n";
  cad +="<body onload=\"Inicializa()\">\n";
  cad +="    <form name='boton'> \n";
  cad +="          <input type=\"button\" value=\"Envia\" onClick=\"Envia()\">\n";
  cad +="    </form>\n";
  cad +="    <table id=\"tabla\" border=\"0\">\n";
  cad +="        <tbody>\n";
  cad +="            <tr>\n";
  cad +="              <form name=\"salida\">\n";
  cad +="                <td>Salida</td>\n";
  cad +="                <td>\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                    <input type=\"text\" size=\"1\">\n";
  cad +="                </td>\n";
  cad +="               </form>\n";
  cad +="            </tr>\n";
  cad +="        </tbody>\n";
  cad +="    </table>\n";
  cad +="    </body>\n";
  cad +="</html>\n";
  
  if(debugGlobal)
    {
    Serial.print("Cadena: ");
    Serial.println(cad);
    }
  
  return cad;
  }

String preparaPaginaConsigna(void)  
  {
  String cad="";
  
  if(debugGlobal) Serial.println("Empezamos...");
  
  cad +="<!DOCTYPE html>\n";
  cad +="<html lang=\"es\">\n";
  cad +="<head>\n";
  cad +="    <meta charset=\"UTF-8\" />\n";
  cad +="\n";
  cad +="    <title>Crear celdas tabla HMTL con JavaScript</title>\n";
  cad +="\n";
  cad +="    <script type=\"text/javascript\">\n";
  cad +="     var total = new Array(48)\n";
  cad +="\n";
  cad +="     function Envia()\n";
  cad +="     {\n";
  cad +="     var cadena=\"\";\n";
  cad +="\n";
  cad +="     cadena=\"consignaDia=\" + document.consigna[0].value + \"&consignaNoche=\" + document.consigna[1].value;";
  cad +="\n";
  cad +="      document.location.href=document.location.origin+document.location.pathname+\"?\" + cadena;\n";
  cad +="      //location.assign\n";
  cad +="      }\n";
  cad +="\n";
  cad +="     function Inicializa()\n";
  cad +="     {\n";
  cad +="     document.consigna[0].value=" + String(getConsignaDia(),1) + ";\n";
  cad +="     document.consigna[1].value=" + String(getConsignaNoche(),1) + ";\n";
  cad +="     }\n";
  cad +="    </script>\n";
  cad +="</head>\n";
  cad +="<body onload=\"Inicializa()\">\n";
  cad +="    <table id=\"tabla\" border=\"0\">\n";
  cad +="        <tbody>\n";
  cad +="            <tr>\n";
  cad +="              <form name=\"consigna\">\n";
  cad +="            <tr>\n";  
  cad +="                <td>Consignas</td>\n";
  cad +="            </tr>\n";  
  cad +="            <tr>\n";    
  cad +="                <td>\n";
  cad +="                    Dia  :<input name=\"Dia  :\" type=\"text\" size=\"4\">\n";
  cad +="                </td>\n";
  cad +="            </tr>\n";  
  cad +="            <tr>\n";    
  cad +="                <td>\n";    
  cad +="                    Noche:<input name=\"Noche:\" type=\"text\" size=\"4\">\n";
  cad +="                </td>\n";
  cad +="            </tr>\n";  
  cad +="               </form>\n";
  cad +="            </tr>\n";
  cad +="        </tbody>\n";
  cad +="    </table>\n";
  cad +="    <form name='boton'> \n";
  cad +="          <input type=\"button\" value=\"Envia\" onClick=\"Envia()\">\n";
  cad +="    </form>\n";
  cad +="    </body>\n";
  cad +="</html>\n";
  
  if(debugGlobal)
    {
    Serial.print("Cadena: ");
    Serial.println(cad);
    }
  
  return cad;
  }

/****************************Google Home Notifier ******************************/
void handleSpeechPath() 
  {
  String phrase = server.arg("phrase");
  
  if (phrase == "") 
    {
    server.send(401, "text / plain", "query 'phrase' is not found");
    return;
    }
  
  if(enviaNotificacion((char*)phrase.c_str())) server.send(200, "text / plain", "OK");
  server.send(404, "text / plain", "KO");  
  }

void handleHablaPath() 
  {
  server.send(200, "text/html", hablaHTML);
  }  

  
