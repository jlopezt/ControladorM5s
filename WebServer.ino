/************************************************************************************************
Servicio                                  URL                                       Formato entrada         Formato salida                                              Comentario                                                                                    Ejemplo peticion                                                         Ejemplo respuesta
Configuracion de la tabla de consignas    http://IPControlador/configTabla          cadena=<cadena>         HTML                                                        Pinta la tabla de consignas. Si la recibe la carga                                            http://IPControlador/configTabla?0|0|0...|127|127                        N/A
Configuracion de las consignas de Tª      http://IPControlador/consignaTemperatura  cadena=<cadena>         HTML                                                        Pinta el formulario para introducir las consignas                                             http://IPControlador/consignaTemperatura?consignaDia=22&consignaNoche=15 N/A
Numero de habitaciones                    http://IPControlador/numeroHabitaciones   N/A                     <numHabitaciones> entero 0..255                             Devuelve el numero de habitaciones con satelite registrado                                    http://IPControlador/numeroHabitaciones                                  4
Lista de las habitaciones                 http://IPControlador/listaHabitaciones    N/A                     id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_SATELITES  Devuelve la lista de los ids y nombres de las habitaciones que se han registrado su satelite  http://IPControlador/listaHabitaciones                                   1#Salon|2#despacho|3#DormitorioPpal
Valores de las medida en las habitaciones http://IPControlador/valoresHabitaciones  id=<id>                 Temperatura|Humedad|Luz                                     Devuelve los valores de una habitacion                                                        http://IPControlador/valoresHabitaciones?id=4                            23.5|72|88
Estado de los reles                       http://IPControlador/estadoReles          id=<id>                 <estadoRele> 1/0                                            Devuelve el estado del rele indicado                                                          http://IPControlador/estadoReles?id=1                                    1
Lista de los reles                        http://IPControlador/listaReles           N/A                     id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_RELES      Devuelve la lista de los ids y nombres de los reles declarados en el controlador              http://IPControlador/listaReles?id=1                                     1#Caldera|2#Riego
Configuracion de habitaciones             http://IPControlador/configHabitaciones   id=<id>&nombre=<nombre> id|nombre                                                   Devuelve lo mismo pero de los valortes generados                                              http://IPControlador/registroHabitacion?id=1&nombre=Salon                1|Salon
Errores de comunicacion                   http://IPControlador/erroresComunicacion  N/A                     id|errores                                                  Devuelve el numero de errores de comunicacion de cada satelite                                http://IPControlador/erroresComunicacion                                 1#2|3#0
Configuracion tabala de temperatura       http://IPControlador/configTabla          cadena=<cadena>         {web configuracion}                                          Configura la tabla de temperaturas dia/noche y genera la pagina de configuracion          
Configura las consignas dia/noche         http://IPControlador/consignaTemperatura  consignaDia/consignaNoche=<valor> {web configuracion}                               Configura la consigna de dida o de noche y genera la pagina de configuracion                  
Recarga del mapa de temperaturas          http://IPControlador/recargaMapa

Reinicia el controlador                   http://IPControlador/restart
Informacion del Hw del sistema            http://IPControlador/info
Crea el fichero indicado                  http://IPControlador/creaFichero
Borra el fichero indicado                 http://IPControlador/borraFichero
Lee el fichero indicado                   http://IPControlador/leeFichero
Informacion del sistema de ficheros       http://IPControlador/infoFS
************************************************************************************************/
#define PUERTO_WEBSERVER  80
#define IDENTIFICACION "<BR><BR><BR><BR><BR>Modulo controlador. Version " + String(VERSION) + ".";

#include <WebServer.h> //#include <ESP8266WebServer.h>

WebServer server(PUERTO_WEBSERVER); //ESP8266WebServer server(PUERTO_WEBSERVER);

String cabeceraHTML = "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<meta charset=\"UTF-8\" />\n<HTML><HEAD><TITLE>Termostato domestico</TITLE></HEAD><BODY><H1>Pagina de inicio</H1>\n";
//String menuHTML = "<TABLE>\n<TR><TD>Opciones:</TD></TR>\n <TR><TD><a href=\"recargaMapa\" target=\"_blank\">Recarga del mapa de temperaturas</a></TD></TR>\n<TR><TD><a href=\"recargaFicheroNombres\" target=\"_blank\">Recarga del fichero de nombres de satelite</a></TD></TR>\n<TR><TD><a href=\"configTabla\" target=\"_blank\">Configuracion de la tabla de consignas</a></TD></TR>\n<TR><TD><a href=\"consignaTemperatura\" target=\"_blank\">Consigna de temperatura</a> </TD></TR> \n<TR><TD><a href=\"numeroHabitaciones\" target=\"_blank\">Numero de habitaciones</a> </TD></TR>\n<TR><TD><a href=\"listaHabitaciones\" target=\"_blank\">Lista de las habitaciones</a></TD></TR>\n<TR><TD><a href=\"valoresHabitaciones\" target=\"_blank\">Valores de las medida en las habitaciones</a></TD></TR>\n<TR><TD><a href=\"estadoReles\" target=\"_blank\">Estado de los reles</a></TD></TR>\n<TR><TD><a href=\"listaReles\" target=\"_blank\">Lista de los reles</a></TD></TR>\n<TR><TD><a href=\"configHabitaciones\" target=\"_blank\">Configuracion de habitaciones</a></TD></TR>\n<TR><TD><a href=\"modo\" target=\"_blank\">Modo?</a></TD><TD> | </TD><TD><a href=\"modo?modo=0 \" target=\"_blank\">Modo off</a></TD><TD> | </TD><TD><a href=\"modo?modo=1 \" target=\"_blank\">Modo on</a></TD><TD> | </TD><TD><a href=\"modo?modo=2 \" target=\"_blank\">Modo auto</a></TD></TR>\n</TABLE>\n";  
String menuHTML = "<TABLE>\n<TR><TD>Opciones:</TD></TR>\n <TR><TD><a href=\"configTabla\" target=\"_blank\">Configuracion de la tabla de consignas</a></TD></TR>\n<TR><TD><a href=\"recargaMapa\" target=\"_blank\">Recarga del mapa de temperaturas</a></TD></TR>\n<TR><TD><a href=\"consignaTemperatura\" target=\"_blank\">Consigna de temperatura</a> </TD></TR> \n<TR><TD><a href=\"numeroHabitaciones\" target=\"_blank\">Numero de habitaciones</a> </TD></TR>\n<TR><TD><a href=\"listaHabitaciones\" target=\"_blank\">Lista de las habitaciones</a></TD></TR>\n<TR><TD><a href=\"valoresHabitaciones\" target=\"_blank\">Valores de las medida en las habitaciones</a></TD></TR>\n<TR><TD><a href=\"recargaFicheroNombres\" target=\"_blank\">Recarga del fichero de nombres de satelite</a></TD></TR>\n<TR><TD><a href=\"estadoReles\" target=\"_blank\">Estado de los reles</a></TD></TR>\n<TR><TD><a href=\"listaReles\" target=\"_blank\">Lista de los reles</a></TD></TR>\n<TR><TD><a href=\"configHabitaciones\" target=\"_blank\">Configuracion de habitaciones</a></TD></TR>\n<TR><TD><a href=\"modo\" target=\"_blank\">Modo?</a></TD><TD> | </TD><TD><a href=\"modo?modo=0 \" target=\"_blank\">Modo off</a></TD><TD> | </TD><TD><a href=\"modo?modo=1 \" target=\"_blank\">Modo on</a></TD><TD> | </TD><TD><a href=\"modo?modo=2 \" target=\"_blank\">Modo auto</a></TD></TR>\n</TABLE>\n";  
String pieHTML = "</BODY>\n</HTML>";

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
  cad += "\n<BR>\n";  
  cad +="Modulo controlador. Version ";
  cad += VERSION;
  cad += "\n"; 
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
        cad += habitaciones[id].peso;
        cad += "\n";

        cad += "     Lectura: ";
        cad += habitaciones[id].lectura;
        cad += "ms, hace ";
        cad += millis()-habitaciones[id].lectura;
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
  String cad=cabeceraHTML;
  cad += IDENTIFICACION //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  
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
  cad += IDENTIFICACION //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];

  cad += "<BR>-----------------info general-----------------<BR>";
  cad += "Uptime: " + String(uptime())+ "ms";
  cad += "<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";
  cad += "IP actuador: " + IPActuador.toString();
  cad += "<BR>";  
  cad += "IP primer satelite: " + IPSatelites[0].toString();
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
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenidoFichero="";

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

  cad += "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    //inicializo el sistema de ficheros
    if (SPIFFS.begin(true)) 
      {
      Serial.println("---------------------------------------------------------------\nmounted file system");  
      //file exists, reading and loading
      if(!SPIFFS.exists(nombreFichero)) cad += "El fichero " + nombreFichero + " no existe.\n";
      else
        {
         File f = SPIFFS.open(nombreFichero, "r");
         if (f) 
           {
           Serial.println("Fichero abierto");
           size_t tamano_fichero=f.size();
           Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
           cad += "El fichero tiene un tamaño de ";
           cad += tamano_fichero;
           cad += " bytes.<BR>";
           char buff[tamano_fichero+1];
           f.readBytes(buff,tamano_fichero);
           buff[tamano_fichero+1]=0;
           Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
           cad += "El contenido del fichero es:<BR>";
           cad += buff;
           cad += "<BR>";
           f.close();
           }
         else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
        }  
      Serial.println("unmounted file system\n---------------------------------------------------------------");
      }//La de abrir el sistema de ficheros
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
    
  server.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  server.on("/infoFS", HTTP_ANY, handleInfoFS);  //URI de info del FS

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
