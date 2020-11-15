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

//Cadenas HTML precargadas
String cabeceraHTML="";
String hablaHTML="<html><head></head><body><input type=\"text\"><button>speech</button><script>var d = document;d.querySelector('button').addEventListener('click',function(){xhr = new XMLHttpRequest();xhr.open('GET','/speech?phrase='+encodeURIComponent(d.querySelector('input').value));xhr.send();});</script></body></html>";

//version de la web propia del cacharro
String pagina_a = "<!DOCTYPE html>\n<html lang=\"es\">\n <head>\n <meta charset=\"UTF-8\">\n <TITLE>Domoticae</TITLE>\n <link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\">\n </HEAD>\n <BODY>\n <table style=\"width:100%;\" cellpadding=\"10\" cellspacing=\"0\">\n  <tr style=\"height:20%; background-color:black\">\n  <th align=\"left\">\n   <span style=\"font-family:verdana;font-size:30px;color:white\">DOMOTI</span><span style=\"font-family:verdana;font-size:30px;color:red\">C</span><span style=\"font-family:verdana;font-size:30px;color:white\">AE - ";
//en medio va el nombre_dispositivo
String pagina_b = "</span>   \n  </th>\n  </tr>\n  <tr style=\"height:10%;\">\n    <td>";
String enlaces = "<table class=\"tabla\">\n<tr class=\"modo1\">\n<td><a href=\"..\" target=\"_self\" style=\"text-decoration:none; color: black;\">Home</a></td>\n<td><a href=\"configHabitaciones\" target=\"_self\" style=\"text-decoration:none; color: black;\">Estado</a></td>\n<td><a href=\"consignaTemperatura\" target=\"_self\" style=\"text-decoration:none; color: black;\">Consignas</a></td>\n<td><a href=\"listaFicheros\" target=\"_self\" style=\"text-decoration:none; color: black;\">Lista ficheros</a></td>\n<td><a href=\"info\" target=\"_self\" style=\"text-decoration:none; color: black;\">Info</a></td>\n<td><a href=\"restart\" target=\"_self\" style=\"text-decoration:none; color: black;\">Restart</a></td>\n</tr>\n</table>";
String pagina_c = "</td></tr><TR style=\"height:60%\"><TD>";
//En medio va el cuerpo de la pagina
String pieHTML = "</TD>\n</TR>\n<TR>\n<TD style=\"color:white; background-color:black\"><a href=\"https://domoticae.lopeztola.com\" target=\"_self\" style=\"text-decoration:none; color:white;\">domoticae-2020</a></TD>\n</TR>\n</table>\n</BODY>\n</HTML>";

//version para integrar en otras paginas
String cabeceraHTMLlight = "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n<meta charset=\"UTF-8\" />\n<HTML><HEAD><TITLE>Domoticae</TITLE><link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\"></HEAD><BODY>\n"; 

void inicializaWebServer(void)
  {
  cabeceraHTML = pagina_a + nombre_dispositivo + pagina_b + enlaces + pagina_c;
  
  //decalra las URIs a las que va a responder
  server.on("/", HTTP_ANY, handleRoot); //web de temperatura
  server.on("/configHabitaciones", HTTP_ANY, handleConfigHabitaciones); 

  server.on("/modo", HTTP_ANY, handleModoCalefaccion);
  
  server.on("/consignaTemperatura", HTTP_ANY, handleConfigConsignas);//Configuracion de las dos consignas (dia/noche)
  server.on("/configTabla", HTTP_ANY, handleConfigTabla);  //Configuracion de la tabla de consignas (cual aplica en cada momento)

  server.on("/recargaMapa", HTTP_ANY, handleRecargaMapa);  //Lee el fichero de la tabla de consignas segun hora y lo carga en memoria
  server.on("/recargaFicheroNombres", HTTP_ANY, handleRecargaFicheroNombres); //Lee el fichero de nombres de termometros y lo carga en memoria
  
  server.on("/restart", HTTP_ANY, handleRestart);  //URI de test
  server.on("/info", HTTP_ANY, handleInfo);  //URI de test
    
  server.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", HTTP_ANY, handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", HTTP_ANY, handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", HTTP_ANY, handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
  server.on("/infoFS", HTTP_ANY, handleInfoFS);  //URI de info del FS

  server.on("/consultaTemperatura", HTTP_ANY, handleConsultaTemperatura); //Manda una locucion al GH
   
  server.on("/speech", handleSpeechPath);
  server.on("/habla", handleHablaPath);

  server.on("/datos", handleDatos);
  server.on("/version", handleVersion);
  server.on("/listaFicheros2", handleListaFicheros2);
  

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
  
  cad = cabeceraHTML;
  cad += "<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"10;URL=/\">\n";//Para que se recargue cada 10sg

  //Valores medidos
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Valores</CAPTION>\n";    
  cad += "<TR class=\"modo2\"><TD>Temperatura:</TD><TD>" + combierteTemperaturaPromedio() + "</TD></TR>\n";
  cad += "<TR class=\"modo2\"><TD>Consigna:</TD><TD>" + String(getConsigna(),1) + "ºC</TD></TR>\n";  
  cad += "<TR class=\"modo2\"><TD>Humedad:</TD><TD>" +String(getHumedadPromedio()) + " %</TD></TR>\n";
  cad += "</TABLE>";
  cad += "\n<BR>\n";
  
  //Modo de funcionamiento
  /* MODO_ON=0   */
  /* MODO_OFF=1  */
  /* MODO_AUTO=2 */  
  cad += "<form action=\"/modo\" id=\"form_id\">\n";
  cad += "<input type=\"hidden\" id=\"modo\" name=\"modo\" value=\"0\">\n";
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<caption>Modo de funcionamiento</caption>\n";
  cad += "<TR class=\"modo2\">";
  switch (getModoManual())
    {
    case MODO_ON:
      cad += "<TR class=\"modo2\"><TD>On</TD><TD>tiempo restante: " + String(getDownCounter()) +  " sg</TD></tr>\n";
    
      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_OFF) + ";document.getElementById('form_id').submit();\">Off</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";

      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_AUTO) + ";document.getElementById('form_id').submit();\">Automatico</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";
      break;
    case MODO_OFF:
      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_ON) + ";document.getElementById('form_id').submit();\">On</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";      

      cad += "<TR class=\"modo2\"><TD>Off</TD><TD>tiempo restante: " + String(getDownCounter()) +  " sg</TD></tr>\n";

      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_AUTO) + ";document.getElementById('form_id').submit();\">Automatico</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";
      break;  
    case MODO_AUTO:
      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_ON) + ";document.getElementById('form_id').submit();\">On</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";
          
      cad += "<TR class=\"modo2\">\n";
      cad += "<TD colspan=\"2\">\n";
      cad += "<button type=\"button\" onclick=\"document.getElementById('modo').value=" + String(MODO_OFF) + ";document.getElementById('form_id').submit();\">Off</button>\n";
      cad += "</TD>\n";
      cad += "</tr>\n";

      cad += "<TR class=\"modo2\"><TD colspan'2'>Automatico</TD></tr>\n";
      break;
    }
  cad += "</TABLE>";
  cad += "</form>";
  cad += "\n<BR>\n";

  //Reles
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<caption>Reles</caption>\n";
  cad += "<TR class=\"modo2\"><TD>" + getNombreRele(CALDERA)   + "</TD><TD>" + (getEstadoRele(CALDERA)?"on":"off")  + "</TD></TR>\n";
  cad += "<TR class=\"modo2\"><TD>" + getNombreRele(SEGURIDAD) + "</TD><TD>" + (getEstadoRele(SEGURIDAD)?"on":"off") + "</TD></TR>\n";
  cad += "</TABLE>";
  cad += "\n<BR>\n";
  
  //Informacion del dispositivo
  cad += "<p style=\"font-size: 12px;color:black;\">" + nombre_dispositivo + " - Version " + String(VERSION) + "</p>";

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
  String mensaje;
  
  if(leeFicheroMapa()) mensaje="Mapa de configuracion de consignas leido";
  else mensaje="Error al leer el mapa de configuracion de consignas ";
  
  String cad = cabeceraHTML;
  cad += "<br><br><br>\n";
  cad += "<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"5;URL=/\">\n";
  cad += "<h3 style=\"font-family:verdana;font-size:30px;color:black\">" + mensaje + "</h3>\n";
  cad += "<br><br><br>\n";
  cad += pieHTML;
    
  server.send(200, "text/HTML", cad);   
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
  
  String cad = cabeceraHTML;
  cad += "<br><br><br>\n";
  cad += "<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"5;URL=/\">\n";
  cad += "<h3 style=\"font-family:verdana;font-size:30px;color:black\">" + mensaje + "</h3>\n";
  cad += "<br><br><br>\n";
  cad += pieHTML;
    
  server.send(200, "text/HTML", cad);   
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

  handleRoot();
  return;
  cad="Modo: " + getModoManualTxt();    
  server.send(200, "text/plain", cad);  
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
  boolean salir=false;
  
  //dia
  if(server.hasArg("consignaDia")){
    setConsignaD(server.arg("consignaDia").toFloat());
    salir=true;
  }
  //noche
  if(server.hasArg("consignaNoche")){
    setConsignaN(server.arg("consignaNoche").toFloat());
    salir=true;
  }

  if(salir) handleRoot(); //Si y alo he actualizado, salgo
  else{
    String cad = cabeceraHTML;
    
    cad += "<form id='formConsignas' action='/consignaTemperatura'>";
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
    cad += "<CAPTION>Consigna de temperatura</CAPTION>\n";    
    cad += "<TR class=\"modo2\"><TD aling=\"center\">día</td><td aling=\"center\"><input type='text' id='consignaDia' name='consignaDia' value='" + String(getConsignaDia(),1) + "'></TD></TR>\n";
    cad += "<TR class=\"modo2\"><TD aling=\"center\">noche</td><td aling=\"center\"><input type='text' id='consignaNoche' name='consignaNoche' value='" + String(getConsignaNoche(),1) + "'></TD></TR>\n";
    cad += "</TABLE>";    
    cad += "<BR>\n";   
    cad += "<input type=\"submit\" value=\"Enviar\">";
    cad += "</form>\n";
    cad += "<BR>\n";   

    cad += "<form id='tablaConsigna' action='/configTabla'>\n";
    cad += "<button type='button' onclick=\"document.getElementById('tablaConsigna').submit();\">Ver tabla de consignas</button>";
    cad += "</form>\n";
    cad += "<BR>\n";

    cad += "<form id='mapaConsigna' action='/recargaMapa'>\n";
    cad += "<button type='button' onclick=\"document.getElementById('mapaConsigna').submit();\">carga mapa de consignas</button>";
    cad += "</form>\n";
    cad += "<BR>\n";

    cad += "<form id='mapaSatelites' action='/recargaFicheroNombres'>\n";
    cad += "<button type='button' onclick=\"document.getElementById('mapaSatelites').submit();\">carga mapa de satelites</button>";
    cad += "</form>\n";
    cad += "<BR>\n";

    cad += pieHTML;
  
    server.send(200, "text/HTML", cad);
    }
  }
 
/*********************************************/
/*                                           */
/*  Servicio de configuracion de             */
/*  la tabla de temperaturas                 */
/*                                           */
/*********************************************/
void handleConfigTabla()
  {
  char cadLarga[192];
  for(int i=0;i<192;i++) cadLarga[i]=0;

  if(server.hasArg("cadena")) 
    {
    //String cad=server.arg("cadena"); //la cadena tiene 48 valores para la tabla
    strncpy(cadLarga,server.arg("cadena").c_str(),192);
    rellenaMapa(cadLarga);
    handleRoot();
    return;
    }

  server.send(200, "text/HTML", preparaPaginaMapa());//cad2); 
  }

String preparaPaginaMapa(void)
  {
  String cad="";
  
  if(debugGlobal || true) Serial.println("Empezamos...");
  
  cad  = cabeceraHTML;
  cad += "\n";

  cad += "<script type=\"text/javascript\">\n";
  cad += " var resultado = new Array(24)\n";
  cad += "\n";
  //cad += " function cambiaColor(int fila, int columna){}\n";
  cad += "\n";
  cad += " function reconstruye()\n";
  cad += " {\n";
  cad += " document.getElementById(\"cadena\").value=\"\";  \n";
  cad += "\n";
  cad += " for(fila=0;fila<24;fila++) {\n";
  cad += "   resultado[fila]=0;\n";
  cad += "   for(columna=0;columna<7;columna++) {\n";
  cad += "     if(document.tablaConsignas[fila*7+columna+1].value!=0)\n"; //El mas 1 es porque la columna de las horas es la 0
  cad += "       {\n";
  cad += "       resultado[fila]=resultado[fila]+2**columna;\n";
  cad += "       console.log(\"fila: \"+fila+\" columna: \"+columna+\" resultado: \" + resultado[fila]);\n";
  cad += "       }\n";  
  cad += "     }\n";
  cad += "   if(document.getElementById(\"cadena\").value!=\"\") document.getElementById(\"cadena\").value = document.getElementById(\"cadena\").value + '|';\n";  
  cad += "   document.getElementById(\"cadena\").value= document.getElementById(\"cadena\").value + resultado[fila];\n";  
  cad += "   }\n";
  cad += " }\n";
  cad += "</script>\n";  
  cad += "\n";
    
  cad += "<form id='tablaConsignas' name='tablaConsignas' action='/configTabla'>\n";
  cad += "<input type='hidden' id='cadena' name='cadena' value=''>\n";
  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
  cad += "<CAPTION>Consignas por horas</CAPTION>\n";      
  cad += "<TR>";  
  cad += "<TH></TH>";    
  cad += "<TH>lunes</TH>";  
  cad += "<TH>martes</TH>";  
  cad += "<TH>miercoles</TH>";  
  cad += "<TH>jueves</TH>";  
  cad += "<TH>viernes</TH>";  
  cad += "<TH>sabado</TH>";  
  cad += "<TH>domingo</TH>";  
  cad += "</TR>";
  
  uint8_t horas=0;
  uint8_t minutos=0;
  for(uint8_t fila=0;fila<48;fila++)
    {
    cad += "<tr>\n";
    cad += "<th>" + String(horas) + ":" + (minutos==0?String("00"):String(minutos)) + "</th>\n";
    uint8_t dia=1;//bitmap que indica el dia en el que estamos. sirve de mascara para leer el bit del dia
    for(uint8_t columna=0;columna<7;columna++)
      {
      cad += "<td align='center'>\n";  
      //cad += "<td align='center' onclick='cambiaColor(" + String(fila) + "," + String(columna) + ")'>\n";
      //cad += "<input type'text' id='dia" + String(columna) + ":hora" + String(fila) + "' value='" + (mapa[fila] & dia?"1":"0") + "'>";
      cad += "<input type'text' value='" + (mapa[fila] & dia?String(1):String(0)) + "'>";
      dia *= 2;//paso al dia siguiente
      cad += "</td>\n";
      }
    cad += "</tr>\n";
    minutos +=30;
    if(minutos>30)
      {
      minutos=0;    
      horas++;
     }
    }

  cad += "</table>\n";
  cad += "<br>";
  cad += "<button type='button' onclick=\"reconstruye();document.getElementById('tablaConsignas').submit();\">Guardar</button>\n";
  cad += "</form>\n";
  cad +="<br>\n";
  
  cad += pieHTML;  

  if(debugGlobal)
    {
    Serial.print("Cadena: ");
    Serial.println(cad);
    }
  
  return cad;
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de la               */
/*  configuracion de las habitaciones        */
/*                                           */
/*********************************************/
void handleConfigHabitaciones(void)
  {
  String cad=cabeceraHTML;

  cad += "<TABLE width=\"200px\" cellpadding=\"0\" cellspacing=\"0\" class=\"tabla\">\n";
  cad += "<caption>Temperatura promedio</caption>\n";
  cad += "<TR class=\"modo2\">\n";
  cad += "<td align='right'>" + String(getTemperaturaPromedio(),1) + " C</td>\n";
  cad += "</TR>\n"; 
  cad += "</table>\n";
  cad += "<BR>\n";

  cad += "<TABLE border=\"0\" width=\"80%\" cellpadding=\"0\" cellspacing=\"0\" class=\"tabla\">\n";
  cad += "<caption>Satelites</caption>\n";
  cad += "<TR>\n"; 
  cad += "<th width='14%'>id</th>\n";
  cad += "<th width='21%'>Nombre</th>\n";
  cad += "<th width='14%'>Temperatura</th>\n";
  cad += "<th width='14%'>Humedad</th>\n";
  cad += "<th width='7%'>Luz</th>\n";
  cad += "<th width='7%'>Peso</th>\n";
  cad += "<th width='23%'>Lectura</th>\n";
  cad += "</TR>\n"; 

  for(int8_t id=0;id<MAX_SATELITES;id++)
    {
    if(sateliteRegistrado(id))
      {
      cad += "<TR class=\"modo2\">\n";
      
      cad += "<td align=\"right\">";
      cad += habitaciones[id].id;
      cad += "</td>\n";

      cad += "<td align=\"center\">";
      cad += habitaciones[id].nombre;
      cad += "</td>\n";
      
      cad += "<td align=\"right\">";
      cad += habitaciones[id].temperatura;
      cad += " C</td>\n";
      
      cad += "<td align=\"right\">";
      cad += habitaciones[id].humedad;
      cad += " %</td>\n";

      cad += "<td align=\"right\">";
      cad += habitaciones[id].luz;
      cad += "</td>\n";

      cad += "<td align=\"right\">";
      cad += habitaciones[id].peso[hora()];
      cad += "</td>\n";

      cad += "<td align=\"right\">";
      cad += sateliteUltimaLectura(id); //habitaciones[id].lectura;
      cad += "ms, hace ";
      cad += millis()-sateliteUltimaLectura(id); //habitaciones[id].lectura;
      cad += "ms";
      cad += "</td>\n";

      cad += "</tr>\n";        
      }
    }
  cad += "</table>\n";
  cad += pieHTML;
   
  server.send(200, "text/HTML", cad);  
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

  cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";
//  cad += "<BR>-----------------info general-----------------<BR>";
  cad += "<TR clas='modo2>\n"; 
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

/*
//  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "<caption>MQTT info</caption>\n";

  cad += "<TR>\n"; 
  cad += "<td>IP broker</td>\n";
  cad += "<td>" + IPBroker.toString() + "</td>\n";
//  cad += "IP broker: " + IPBroker.toString();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>URL broker</td>\n";
  cad += "<td>" + BrokerDir.toString() + "</td>\n";
//  cad += "URL broker: " + BrokerDir.toString();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>modo</td>\n";
  cad += "<td>" + modoMQTT.toString() + "</td>\n";
//  cad += "modo: " + modoMQTT.toString();
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>modo</td>\n";
  cad += "<td>" + String(puertoBroker) + "</td>\n";
//  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>usuario</td>\n";
  cad += "<td>" + String(usuarioMQTT) + "</td>\n";
//  cad += "Usuario: " + usuarioMQTT="";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>password</td>\n";
  cad += "<td>" + String(passwordMQTT) + "</td>\n";
//  cad += "Password: " + passwordMQTT="";
  cad += "</TR>\n"; 

  cad += "<TR>\n"; 
  cad += "<td>Topic Root</td>\n";
  cad += "<td>" + String(topicRoot) + "</td>\n";
//  cad += "Topic root: " + topicRoot="";
  cad += "</TR>\n"; 
*/
    
//  cad += "<BR>-----------------info WiFi-----------------<BR>";
  cad += "<TR clas='modo2>\n"; 
  cad += "<ttd colspan='2' class=\"modo1\">info WiFi</td>\n";
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
  cad += "<TR clas='modo2>\n"; 
  cad += "<ttd colspan='2' class=\"modo1\">info hardware</td>\n";
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

  cad += "</table>";

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

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) 
      {
      handleListaFicheros();
      return;
      cad += "Fichero salvado con exito<br>";
      }
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
  String mensaje;
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(borraFichero(nombreFichero)) 
      {
      handleListaFicheros();
      return;
      mensaje = "El fichero " + nombreFichero + " ha sido borrado.\n";
      }
    else mensaje = "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else mensaje = "Falta el argumento <nombre de fichero>"; 

  cad += "<br><br><br>\n";
  cad += "<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"5;URL=/\">\n";
  cad += "<h3 style=\"font-family:verdana;font-size:30px;color:black\">" + mensaje + "</h3>\n";
  cad += "<br><br><br>\n";
  cad += pieHTML;
    
  server.send(200, "text/HTML", cad);  
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

    cad += "<table style=\"border: 0px; border-color: #FFFFFF;\"><tr style=\"border: 0px; border-color: #FFFFFF;\">";
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
    cad += "</td></tr></table>";  

    cad += "</td>";
    cad += "</tr></table>";        
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros2(void)
  {
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;
  String cad=cabeceraHTMLlight;

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

    cad += "<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 5px; text-align: left;}</style>";
    cad += "<br>\n";   
    cad += "<table style=\"border: 0px; border-color: #FFFFFF;\"><tr style=\"border: 0px; border-color: #FFFFFF;\"><td style=\"border: 0px; border-color: #FFFFFF;\">";
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";    
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
    cad += "</td>";
    
    //Para crear un fichero nuevo
    cad += "<td style=\"border: 0px; border-color: #FFFFFF;\">";
    cad += "<TABLE border=\"0\" width=\"50%\" cellpadding=\"0\" cellspacing=\"0\" width=\"300\" class=\"tabla\">\n";    
    ////cad += "<table><tr><td>";
    cad += "<table><tr><td>";
    cad += "<caption>Crear un fichero nuevo:</caption>";
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    cad += "</td>";
    cad += "</tr></table>";

    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee la temperatura promedio y manda      */
/*  una locucion al GH                       */ 
/*                                           */
/*********************************************/  
void handleConsultaTemperatura(void)
  {
  String cad=cabeceraHTML;
  char texto[255];
  
  sprintf(texto,"La temperatura actual es de %.1f grados",getTemperaturaPromedio());
  enviaNotificacion(texto);
  
  cad += "<h1>" + nombre_dispositivo + "</h1>";
  cad += "locución de temperatura enviada";
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
void handleDatos() 
  {
  String cad="";
  
  cad = cabeceraHTMLlight;  
  cad += "<TABLE>\n";
  cad += "<TR><TD>Temperatura:</TD><TD>" + combierteTemperaturaPromedio() + "</TD></TR>\n";
  cad += "<TR><TD>Consigna:</TD><TD>" + String(getConsigna(),1) + "ºC</TD></TR>\n";  
  cad += "<TR><TD>Modo:</TD><TD>" + String(getModoManualTxt()) + "</TD>";
  if(getModoManual()!=MODO_AUTO) cad += "<TD>Ticks:</TD><TD>" + String(getDownCounter()) +  "</TD><TD>Segs:</TD><TD>" + String(ticks2seg(getDownCounter())) +  "</TD></TR>\n";
  else cad += "</TR>\n";
  cad += "<TR><TD>Reles</TD></TR>\n";
  cad += "<TR><TD>Nombre:</TD><TD>" + getNombreRele(CALDERA)   + "</TD><TD>Estado:</TD><TD>" + getEstadoRele(CALDERA)   + "</TD></TR>\n";
  cad += "<TR><TD>Nombre:</TD><TD>" + getNombreRele(SEGURIDAD) + "</TD><TD>Estado:</TD><TD>" + getEstadoRele(SEGURIDAD) + "</TD></TR>\n";
  cad += "</TABLE>";
  cad += "\n<BR>\n";
  
  server.send(200, "text/HTML", cad);  
  }

void handleVersion() 
  {
  String cad="";

  cad = cabeceraHTMLlight;
  cad += "<BR>" + nombre_dispositivo + ". Version " + String(VERSION) + ".";

  cad += pieHTML;
  
  server.send(200, "text/HTML", cad);  
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
  
  if(enviaNotificacion(phrase.c_str())) server.send(200, "text / plain", "OK");
  server.send(404, "text / plain", "KO");  
  }

void handleHablaPath() 
  {
  server.send(200, "text/html", hablaHTML);
  }  

  
