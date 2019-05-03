 /*
 * Termostato
 *
 * Controlador - Secuenciador del Termostato
 * 
 * Dos reles pines 8 y 9
 *
 */
 
//Defines generales
#define NOMBRE_FAMILIA    "Controlador_termostato"
#define VERSION           "2.4.0 M5Stack (OTA|MQTT|LOGIC+) lib v0.2.2"
#define SEPARADOR         '|'
#define SUBSEPARADOR      '#'
#define KO                -1
#define OK                0
#define MAX_VUELTAS       32767
#define SATELITE_TIME_OUT 60000 //Milisegundos transcurridos entre dos peticiones del copntrolador antes de intentar registrarse

//Organizacion de pines de E/S
#define RELES_PIN          8 //Pîn del primer rele, los demas consecutivos
#define MAX_SATELITES     16 //numero maximo de satelites de 0 a 15 controlado por los DIP Switch
#define MAX_RELES         2 //numero maximo de reles soportado


//Nombres de ficheros
#define GLOBAL_CONFIG_FILE     "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE "/Config.json.bak"
#define WIFI_CONFIG_FILE       "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE   "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE       "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE   "/MQTTConfig.json.bak"

//Definicion de codigos de error
#ifndef ERRORES //si no esta definido ERRORES
#define ERRORES //ya esta definido...
#define MAX_ERROR            10
#define NO_ERROR             99
#define ERROR_INTERNO_CONFIG  0
#define ERROR_INTERNO_WIFI    1
#define ERROR_INTERNO_SPIFFS  2
#define ERROR_INTERNO_SNTP    3
#define ERROR_COM_SATELITES   4
#define ERROR_COM_RELES       5
#endif

//Definiciopn de pines
//#define PIN_DESBORDE_TIEMPO 14 //GPIO del Pin del led de desborde de tiempo

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define ANCHO_INTERVALO             100 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA                5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_PANTALLA          10 //20 //cada cuantas vueltas de loop pinta la pantalla 
#define FRECUENCIA_LOGICA_CONTROL    50 //300 //cada cuantas vueltas de loop actualiza el control de las salidas
#define FRECUENCIA_SERVIDOR_WEB       1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_ORDENES            2 //cada cuantas vueltas de loop atiende las ordenes via serie 
#define FRECUENCIA_RTC              600 //cada cuantas vueltas de loop atiende el RTC 
#define FRECUENCIA_HORA             600 //cada cuantas vueltas de loop pinta el reloj 
#define FRECUENCIA_BOTONES            1 //cada cuantas vueltas de loop atiende los botones
#define FRECUENCIA_SALVAR           600 //cada cuantas vueltas de loop salva la configuracion a los ficheros
#define FRECUENCIA_MQTT              10 //cada cuantas vueltas de loop envia y lee del broket MQTT
#define FRECUENCIA_SATELITE_TIMEOUT  50 //cada cuantas vueltas de loop compruebo si ha habido time out en los satelites
#define FRECUENCIA_WIFI_WATCHDOG    100 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

#define INC_CONSIGNA              0.5 //Salto de la consigna cuando se ajusta con los botones

#define VETE_A_DORMIR           10000 //Se va a reposo a los 10 segundos

#define COLOR_FONDO TFT_NAVY
#define COLOR_TITULO TFT_BLUE
#define COLOR_LETRAS_TITULO TFT_WHITE
#define TEXTO_TITULO "Termostatix 2.3"
#define COLOR_MENU TFT_LIGHTGREY
#define COLOR_LETRAS_MENU TFT_WHITE

#define TEMPERATURA 0
#define CONSIGNA    1
#define REPOSO      2
#define INFO        3
#define MANUAL      4

#define FICHERO_CONSIGNAS_MODIFICADO 1

#define NO_LEIDO      -100.0

#define DEFAULT_TIME_OUT 5000 //en milisegundos
#define DEFAULT_BRILLO_PANTALLA 10

#include <M5Stack.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <WiFi.h> //#include <ESP8266WiFi.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <SPIFFS.h> //para el ESP32

//prototipo de funciones
String getErroresNivel(int nivel=0);
int getValorPrincipal(void);//Prototipo, declarada en Pantalla
boolean enviarMQTT(String topic, String payload) ;

//************ Valores configurables *************//
//IPs de los diferentes dispositivos 
IPAddress IPControlador;
IPAddress IPActuador;
IPAddress IPSatelites[MAX_SATELITES];
IPAddress IPGateway;
//TimeOut HTTP
time_t TimeOut=DEFAULT_TIME_OUT;
//Valores de la pantalla
int brilloPantalla=DEFAULT_BRILLO_PANTALLA;
//************ Fin valores configurables *************//

/*-----------------Variables comunes---------------*/
String nombre_dispoisitivo(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
time_t SleepBucle=0;
time_t limiteSleep=VETE_A_DORMIR;

unsigned int vuelta = 32700;//0; //vueltas de loop del core 0

int debugGlobal=0; //por defecto desabilitado
int8_t ficherosModificados=0;//Inicialmente no hay ficheros modificados

void setup()
  {
  ficherosModificados=0;
  
  //Inicializo el core del M5
  M5.begin();
  Serial.begin(115200);
  Wire.begin();//Lo he visto en los ejemplos...

  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del modulo Controlador                   *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  //Configuracion general
  Serial.printf("\n\nInit Config -----------------------------------------------------------------------\n");
  inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.printf("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (inicializaWifi(true))//debugGlobal)) No tien esentido debugGlobal, no hay manera de activarlo
    {
    //----------------Inicializaciones que necesitan red-------------
    //OTA
    Serial.printf("\n\nInit OTA -----------------------------------------------------------------------\n");
    inicializaOTA(debugGlobal);
    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------\n");
    inicializaReloj();
    //MQTT
    Serial.println("Init MQTT -----------------------------------------------------------------------");
    inicializaMQTT();
    //WebServer
    Serial.printf("\n\nInit Web --------------------------------------------------------------------------\n");
    inicializaWebServer();
    }
  else Serial.println("No se pudo conectar al WiFi");
 
  //Logica
  Serial.printf("\n\nInit Logica -----------------------------------------------------------------------\n");
  inicializaLogica();
  
  //Satelites
  Serial.printf("\n\nInit Sensores ---------------------------------------------------------------------\n");
  inicializaSatelites();

  //Errores
  Serial.printf("\n\nInit Errores ----------------------------------------------------------------------\n");
  inicializaErrores();

  //Pantalla
  Serial.printf("\n\nInit Pantalla -----------------------------------------------------------------------\n");
  inicializaPantalla(); 

  //Botones
  Serial.printf("\n\nInit Botones ----------------------------------------------------------------------\n");  
  inicializaBotones();//Inicializa los botones del M5Stack
  
  //Ordenes serie
  Serial.printf("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  Serial.println("");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*            Fin init del modulo Controlador                  *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  }  

void  loop(void)
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //Logica para ir a sleep
  if(EntradaBucle-SleepBucle>limiteSleep && getValorPrincipal()!=REPOSO) 
    {
    setValorPrincipal(REPOSO);
    SleepBucle=EntradaBucle;
    }
  //else Serial.printf("Entrada %i, Sleep %i, Umbral %i\n",EntradaBucle,SleepBucle,limiteSleep);

  M5.update();

  //------------- EJECUCION DE TAREAS --------------------------------------  
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((vuelta % FRECUENCIA_OTA)==0) ArduinoOTA.handle(); //Gestion de actualizacion OTA
  //Prioridad 1: Funciones de pantalla.
  if ((vuelta % FRECUENCIA_PANTALLA)==0) pintaPantalla(); //Pinta los datos en la pantalla
  if ((vuelta % FRECUENCIA_HORA)==0) pintaFechaHora(); //Actualiza la fecha y la hora en la pantalla principal
  if ((vuelta % FRECUENCIA_BOTONES)==0) atiendeBotones();
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_LOGICA_CONTROL)==0) logicaControl(); //actua sobre los motores 
  //if ((vuelta % FRECUENCIA_LOGICA_CONTROL)==0) actualizaReles(); //actua sobre los motores   
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT();    
  if ((vuelta % FRECUENCIA_SATELITE_TIMEOUT)==0) sateliteTimeOut(SATELITE_TIME_OUT); //verifica si algun saletile no comunica hace mucho
  //Prioridad 3: Interfaces externos de consulta
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web  
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
  if ((vuelta % FRECUENCIA_SALVAR)==0) salvaConfiguracion();
  if ((vuelta % FRECUENCIA_WIFI_WATCHDOG)==0) WifiWD();  
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  
  
  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop
  if (vuelta>=MAX_VUELTAS) vuelta=0;
  
  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+ANCHO_INTERVALO) 
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    delayMicroseconds(1000);
    }
  }

///////////////CONFIGURACION GLOBAL/////////////////////
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo global                           */
/************************************************/
boolean inicializaConfiguracion(boolean debug)
  {
  String cad="";

  Serial.println("Inicializando pantalla M5Stack");  
  //M5.Lcd.drawBitmap(0, 0, 320, 240, (uint16_t *)gImage_logoM5);
  M5.Lcd.drawJpgFile(SD, "/termostatix.jpg");
  Serial.println("Logo pintado");
    
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  TimeOut=DEFAULT_TIME_OUT;
  brilloPantalla=DEFAULT_BRILLO_PANTALLA;
  IPControlador.fromString("0.0.0.0");
  IPActuador.fromString("0.0.0.0");
  IPGateway.fromString("0.0.0.0");
  for(int8_t id=0;id<MAX_SATELITES;id++) IPSatelites[id].fromString("0.0.0.0");  
    
  if(leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    if (!parseaConfiguracionGlobal(cad)) Serial.println("¡¡¡Error al cargar la configuracion general!!!");    
    }
  return true;
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio global                       */
/*********************************************/
boolean parseaConfiguracionGlobal(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************    
    if (json.containsKey("TimeOut")) TimeOut = (int)json["TimeOut"];             
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar TimeOut");
    
    if (json.containsKey("limiteSleep")) limiteSleep = (int)json["limiteSleep"];             
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar limiteSleep");
    
    if (json.containsKey("Brillo")) brilloPantalla = (int)json["Brillo"];             
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar Brillo");
    
    if (json.containsKey("IPControlador")) IPControlador.fromString((const char *)json["IPControlador"]);
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar IPControlador");
   
    if (json.containsKey("IPActuador"))IPActuador.fromString((const char *)json["IPActuador"]);          
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar IPActuador");

    if (json.containsKey("IPGateway"))IPGateway.fromString((const char *)json["IPGateway"]);             
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar IPGateway"); 
    
    if (json.containsKey("IPPrimerTermometro"))
      {
      IPSatelites[0].fromString((const char *)json["IPPrimerTermometro"]);          
    
      for(int8_t id=1;id<MAX_SATELITES;id++)
        {
        IPSatelites[id]=IPSatelites[id-1];//copio la anterior
        IPSatelites[id][3]++;//paso a la siguiente
        }            
      }
    else setError(ERROR_INTERNO_CONFIG, "Error al cargar IPPrimerTermometro");

    if (getErrorNivel(ERROR_INTERNO_CONFIG))
      {
      Serial.printf("Errores en la configuracion:%s\n",getErroresNivel(ERROR_INTERNO_CONFIG).c_str());
      return false;//si ahy errores retorno false
      }
    else //si no lo hay pinto la traza y retorno true
      {
      Serial.printf("Configuracion leida:\nTimeOut: %i\nsleep: %i\nBrillo: %i\nIP controlador: %s\nIP actuador: %s\nIP primer satelite: %s\nIP Gateway: %s\n",TimeOut,limiteSleep,brilloPantalla,IPControlador.toString().c_str(),IPActuador.toString().c_str(),IPSatelites[0].toString().c_str(),IPGateway.toString().c_str());
      return true;
      }
//************************************************************************************************
    }
  return false;
  }

///////////////FUNCIONES COMUNES/////////////////////
/********************************************/
/*  Genera una cadena con todas las IPs     */
/********************************************/
String leerIPs(void)
  {
  String cad="";

  cad =  IPControlador.toString();
  cad += "\n";
  cad += IPActuador.toString();
  cad += "\n";
  
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    cad += "IP " + String(i) + ": ";
    cad += IPSatelites[i][0];
    cad += ".";
    cad += IPSatelites[i][1];
    cad += ".";
    cad += IPSatelites[i][2];
    cad += ".";
    cad += IPSatelites[i][3];
    cad += "\n";
    }
  return cad;
  }

/*************************************************/
/*  Dado un long, lo paso a binario y cambio los */
/*  bits pares. Devuelve el nuevo valor          */ 
/*************************************************/
int generaId(int id_in)
  {
  const long mascara=43690;
  return (id_in^mascara);
  }

/***************************************************************/
/*                                                             */
/*  Parsea la cenaorigen hasta encontrar un separador,         */
/*  lo reemplaza por un '\0' y apunta cola a la siguiente      */
/*   posicion. Devuelve la cola                                */   
/*   ¡MODIFICA LA CADENA ORIGEN EN EL PROCESO!                 */
/***************************************************************/
 char* parseaCad(char* origen, char separador)
  {
  char* cola=origen; //apunto la cola a la cabeza

  while(*cola!='\0') //mientras no encuentre un final de cadena
    {
    if(*cola==separador) //si este caracter es el separador
      {
      *cola='\0'; //lo cambio por un final de cadena
      cola++; //paso la cola al primero del resto de la cadena
      break;
      }
    else cola++;
    }

  return (cola); //devuelve la cadena a la que apunta cola
  }

/***************************************************************/
/*                                                             */
/*  Salva la configuracion a los ficheros                      */
/*                                                             */
/***************************************************************/
void salvaConfiguracion(void)
  {
  //Serial.printf("ficherosModificados = %i\n",ficherosModificados);
  if(ficherosModificados & FICHERO_CONSIGNAS_MODIFICADO) salvaConsignaFichero();  

  ficherosModificados =0;
  }

time_t uptime(void) {return millis();}  
