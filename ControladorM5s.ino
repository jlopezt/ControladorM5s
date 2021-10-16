/*
 * Termostato
 *
 * Controlador - Secuenciador del Termostato
 * 
 */
 
/***************************** Defines *****************************/
//Defines generales
#define NOMBRE_FAMILIA    "Termostatix"
#define VERSION           "v3.4.0" // (OTA|MQTT|LOGIC+|WEBSOCKETS) M5Stack v0.3.1" 
#define SEPARADOR         '|'
#define SUBSEPARADOR      '#'
#define KO                -1
#define OK                0
#define HORAS_EN_DIA      24
#define SEGUNDOS_EN_HORA  3600
#define MAX_VUELTAS       UINT16_MAX //32767
#define SATELITE_TIME_OUT 100000 //Milisegundos transcurridos entre dos peticiones del controlador antes de intentar registrarse

#define TOP_TRAZA          1
#define INC_TRAZA          14

#define MAX_SATELITES     16 //numero maximo de satelites de 0 a 15 controlado por los DIP Switch
#define MAX_RELES          2 //numero maximo de reles soportado

//Nombres de ficheros
#define GLOBAL_CONFIG_FILE     "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE "/Config.json.bak"
#define WIFI_CONFIG_FILE       "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE   "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE       "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE   "/MQTTConfig.json.bak"
#define GHN_CONFIG_FILE        "/GHNConfig.json"
#define GHN_CONFIG_BAK_FILE    "/GHNConfig.json.bak"
#define WU_CONFIG_FILE         "/WUConfig.json"
#define WU_CONFIG_BAK_FILE     "/WUConfig.json.bak"

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

// Una vuela de loop son ANCHO_INTERVALO milisegundos 
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
#define FRECUENCIA_ENVIA_DATOS      100 //cada cuantas vueltas de loop publica el estado en el broker MQTT
#define FRECUENCIA_ENVIA_WU        3000 //cada cuantas vueltas de loop publica el estado en WU
#define FRECUENCIA_SATELITE_TIMEOUT  50 //cada cuantas vueltas de loop compruebo si ha habido time out en los satelites
#define FRECUENCIA_WIFI_WATCHDOG    100 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi
#define FRECUENCIA_FREEHEAP          50 //cada cuantas vueltas publica la memoria libre

//configuracion del watchdog del sistema
#define TIMER_WATCHDOG        0 //Utilizo el timer 0 para el watchdog
#define PREESCALADO_WATCHDOG 80 //el relog es de 80Mhz, lo preesalo entre 80 para pasarlo a 1Mhz
#define TIEMPO_WATCHDOG      100*1000*1000 //Si en este numero de microsegundos no se atiende el watchdog, saltara. Esta microsegundos

//Para la pantalla
#define COLOR_FONDO TFT_DARKGREY //TFT_NAVY
#define COLOR_TITULO TFT_DARKGREY //TFT_BLUE
#define COLOR_LETRAS_TITULO TFT_WHITE
#define TEXTO_TITULO "Termostatix 2.5"
#define COLOR_MENU TFT_LIGHTGREY
#define COLOR_LETRAS_MENU TFT_WHITE

#define TEMPERATURA 0
#define CONSIGNA    1
#define REPOSO      2
#define INFO        3
#define MANUAL      4

#define INC_CONSIGNA              0.5 //Salto de la consigna cuando se ajusta con los botones

#define VETE_A_DORMIR           10000 //Se va a reposo a los 10 segundos

#define FICHERO_CONSIGNAS_MODIFICADO 1

#define NO_LEIDO      -100.0

#define DEFAULT_TIME_OUT 5000 //en milisegundos
#define DEFAULT_BRILLO_PANTALLA 10

#define LED_BUILTIN      21 //GPIO del led de la placa en los ESP32   

#define WUNDERGROUND

/***************************** Defines *****************************/

/***************************** Includes *****************************/
//Includes generales
#include <M5Stack.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <WiFi.h> //#include <ESP8266WiFi.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <SPIFFS.h> //para el ESP32
#include <WebSocketsServer.h> //Lo pongo aqui porque si lo pongo en su sitio no funciona... https://github.com/Links2004/arduinoWebSockets/issues/356
#include <rom/rtc.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <math.h>

//prototipo de funciones
String getErroresNivel(int nivel=0);
int getValorPrincipal(void);//Prototipo, declarada en Pantalla
boolean enviarMQTT(String topic, String payload) ;

//************ Valores configurables *************//
time_t TimeOut=DEFAULT_TIME_OUT;
//Valores de la pantalla
int brilloPantalla=DEFAULT_BRILLO_PANTALLA;
uint16_t colorFondo=COLOR_FONDO;
uint16_t colorTitulo=COLOR_TITULO;
//************ Fin valores configurables *************//

/*-----------------Variables comunes---------------*/
String nombre_dispositivo="";//Nombre del dispositivo, por defecto el de la familia
time_t SleepBucle=0;
time_t limiteSleep=0;

unsigned int vuelta = UINT16_MAX-50; //vueltas de loop del core 0

hw_timer_t *timer = NULL;//Puntero al timer del watchdog

int8_t debugGlobal=0; //por defecto desabilitado
int8_t ficherosModificados=0;//Inicialmente no hay ficheros modificados
/*-----------------Variables comunes---------------*/

/************************* FUNCIONES PARA EL BUITIN LED ***************************/
void configuraLed(void){pinMode(LED_BUILTIN, OUTPUT);}
void enciendeLed(void){digitalWrite(LED_BUILTIN, LOW);}//En esp8266 es al reves que en esp32
void apagaLed(void){digitalWrite(LED_BUILTIN, HIGH);}//En esp8266 es al reves que en esp32
void parpadeaLed(uint8_t veces, uint16_t espera=100)
  {
  for(uint8_t i=0;i<2*veces;i++)
    {
    delay(espera/2);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
/***********************************************************************************/  

void setup()
  {
  boolean trazaInicio=false;  //Traza sobre la pantalla del M5 Stack
  ficherosModificados=0;
  
  //Inicializo el core del M5
  M5.begin();
  //Serial.begin(115200); //No debe estar con la nueva version de la libreria de M5
  Wire.begin();

  if (M5.BtnA.isPressed()) 
    {
    debugGlobal=1; 
    trazaInicio=true;
    }
  else debugGlobal=0;

  //Traza de inicio
  pintaTrazaInicial("Iniciando setup");
  pintaTrazaInicial(String(NOMBRE_FAMILIA) + " - " + VERSION);
    
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del modulo Controlador                   *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  inicializaFicheros(debugGlobal);
  inicializaSD(debugGlobal);

  //Configuracion general
  Serial.printf("\n\nInit Config -----------------------------------------------------------------------\n");
  inicializaConfiguracion(debugGlobal);
  //Traza de inicio
  pintaTrazaInicial("Configuracion global OK");
  
  //Wifi
  Serial.printf("\n\nInit WiFi -----------------------------------------------------------------------\n");
  if (inicializaWifi(true))//debugGlobal)) No tien esentido debugGlobal, no hay manera de activarlo
    {
    //----------------Inicializaciones que necesitan red-------------
    //Traza de inicio
    String sss="Wifi OK. IP: "+getIP(0);
    pintaTrazaInicial(sss);

    //OTA
    Serial.printf("\n\nInit OTA -----------------------------------------------------------------------\n");
    inicializaOTA(debugGlobal);
    //Traza de inicio
    pintaTrazaInicial("OTA OK");

    //SNTP
    Serial.printf("\n\nInit SNTP ----------------------------------------------------------------------\n");
    inicializaReloj();
    //Traza de inicio
    pintaTrazaInicial("SNTP OK"); 

    //WebServer
    Serial.printf("\n\nInit Web --------------------------------------------------------------------------\n");
    inicializaWebServer();
    //Traza de inicio
    pintaTrazaInicial("Webserver OK"); 

    //WebSockets
    Serial.println("Init Web ------------------------------------------------------------------------");
    inicializaWebSockets();
    //Traza de inicio
    pintaTrazaInicial("Websockets OK"); 

    //MQTT
    Serial.println("Init MQTT -----------------------------------------------------------------------");
    inicializaMQTT();
    //Traza de inicio
    pintaTrazaInicial("MQTT OK"); 

#ifdef WUNDERGROUND
    //WUnderground
    Serial.println("Init WUnderground ---------------------------------------------------------------");
    inicializaWU();
    pintaTrazaInicial("WUnderground OK"); 
#endif    

    //Google Home Notifier
    Serial.println("\n\nInit Google Home Notifier -------------------------------------------------------\n");
    inicializaGHN();
    //Traza de inicio
    pintaTrazaInicial("GHN OK"); 
    }
  else 
    {
    //Traza de inicio
     pintaTrazaInicial("¡¡¡¡Wifi KO!!!!"); 
    Serial.println("No se pudo conectar al WiFi");
    }
 
  //Logica
  Serial.printf("\n\nInit Logica -----------------------------------------------------------------------\n");
  inicializaLogica();
  //Traza de inicio
  pintaTrazaInicial("Logica OK"); 
  
  //Satelites
  Serial.printf("\n\nInit Sensores ---------------------------------------------------------------------\n");
  inicializaSatelites();
  //Traza de inicio
  pintaTrazaInicial("Satelites OK");

  //Errores
  Serial.printf("\n\nInit Errores ----------------------------------------------------------------------\n");
  inicializaErrores();
  //Traza de inicio
  pintaTrazaInicial("Errores OK"); 

  //Botones
  Serial.printf("\n\nInit Botones ----------------------------------------------------------------------\n");  
  inicializaBotones();//Inicializa los botones del M5Stack
  //Traza de inicio
  pintaTrazaInicial("Botones OK"); 
  
  //Ordenes serie
  Serial.printf("\n\nInit Ordenes ----------------------------------------------------------------------\n");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC
  //Traza de inicio
  pintaTrazaInicial("Ordenes OK"); 

  //Traza de inicio
  //Paro para revisar la pantalla si hay traza de incio activada
  if (trazaInicio) 
    {
    Serial.printf("Traza inicio activada,esperando pulsacion de boton A\n");  
    while(true)
      {
      delay(1);  
      M5.update();       
      if(M5.BtnA.wasPressed()) break;
      }  
    }

  //Pantalla
  Serial.printf("\n\nInit Pantalla -----------------------------------------------------------------------\n");
  inicializaPantalla(); 

  Serial.println("");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*            Fin init del modulo Controlador                  *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  //activo el watchdog
  configuraWatchdog();
  }  

void  loop(void)
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //reinicio el watchdog del sistema
  timerWrite(timer, 0);
  
  //Logica para ir a sleep
  if(EntradaBucle-SleepBucle>limiteSleep && getValorPrincipal()!=REPOSO) 
    {
    setValorPrincipal(REPOSO);
    SleepBucle=EntradaBucle;
    }

  M5.update();

int paso=0;
//Serial.printf("paso: %i\n",paso++);
  //------------- EJECUCION DE TAREAS --------------------------------------  
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((vuelta % FRECUENCIA_OTA)==0) ArduinoOTA.handle(); //Gestion de actualizacion OTA
//Serial.printf("paso: %i\n",paso++);
  //Prioridad 1: Funciones de pantalla.
  if ((vuelta % FRECUENCIA_PANTALLA)==0) pintaPantalla(); //Pinta los datos en la pantalla
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_HORA)==0) pintaFechaHora(); //Actualiza la fecha y la hora en la pantalla principal 
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_BOTONES)==0) atiendeBotones();
//Serial.printf("paso: %i\n",paso++);  
  //Prioridad 2: Funciones de control.
  if ((vuelta % FRECUENCIA_LOGICA_CONTROL)==0) logicaControl(); //actua sobre los motores  
//Serial.printf("paso: %i\n",paso++);  
  //if ((vuelta % FRECUENCIA_LOGICA_CONTROL)==0) actualizaReles(); //actua sobre los motores   
  if ((vuelta % FRECUENCIA_MQTT)==0) atiendeMQTT();    
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_SATELITE_TIMEOUT)==0) sateliteTimeOut(SATELITE_TIME_OUT); //verifica si algun saletile no comunica hace mucho
//Serial.printf("enviaDatos paso: %i\n",paso++);  
  //Prioridad 3: Interfaces externos de consulta
  if ((vuelta % FRECUENCIA_ENVIA_DATOS)==0) enviaDatos(debugGlobal); //envia datos de estado al broker MQTT  
#ifdef WUNDERGROUND  
  if ((vuelta % FRECUENCIA_ENVIA_WU)==0) UploadDataToWU();
#endif    
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web  
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) atiendeWebSocket(debugGlobal); //atiende el servidor web 
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_SALVAR)==0) salvaConfiguracion();
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_WIFI_WATCHDOG)==0) WifiWD();  
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_FREEHEAP)==0) enviarMQTT("freeheap", "{\"IP\": \"" + String(getIP(debugGlobal)) + "\" , \"Uptime\":" + String(uptime())+ ", \"freeHeap\": " + String(ESP.getFreeHeap()) + ", \"potencia\": \"" + String(WiFi.RSSI()) + "\", \"lastResetReason0\": " + rtc_get_reset_reason(0) + ", \"lastResetReason1\": " + rtc_get_reset_reason(1) + "}");  
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  
//Serial.println("Fin");
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
  M5.Lcd.drawJpgFile(SD, "/termostatix.jpg");
  Serial.println("Logo pintado");
    
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  TimeOut=DEFAULT_TIME_OUT;
  brilloPantalla=DEFAULT_BRILLO_PANTALLA;
  limiteSleep=VETE_A_DORMIR;

  if(!leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    Serial.printf("No existe fichero de configuracion global\n");
    cad="{\"TimeOut\": 5000, \"limiteSleep\":20000, \"Brillo\": 10}";//config por defecto
    //salvo la config por defecto
    //if(salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion global creado por defecto\n"); 
    }

  return parseaConfiguracionGlobal(cad);
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
    if (json.containsKey("nombre_dispositivo")) nombre_dispositivo=String((const char *)json["nombre_dispositivo"]);    
    if(nombre_dispositivo=="") nombre_dispositivo=String(NOMBRE_FAMILIA);
 
    TimeOut = json.get<int>("TimeOut");
    limiteSleep = json.get<int>("limiteSleep");
    brilloPantalla = json.get<int>("Brillo");
    if (json.containsKey("colorFondo")) colorFondo = json.get<int>("colorFondo");
    if (json.containsKey("colorTitulo")) colorTitulo = json.get<int>("colorTitulo");
    
    Serial.printf("Configuracion leida:\nTimeOut: %i\nsleep: %i\nBrillo: %i\nFondo: %i\Titulo: %i\n",TimeOut,limiteSleep,brilloPantalla,colorFondo,colorTitulo);
    return true;     
//************************************************************************************************
    }
  return false;
  }

/*****************************************FUNCIONES COMUNES********************************************************/
/***************************************************************/
/*                                                             */
/*  Decodifica el motivo del ultimo reset                      */
/*                                                             */
/***************************************************************/
const char* reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : return ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : return ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : return ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : return ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : return ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : return ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : return ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : return ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : return ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : return ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : return ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : return ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return ("NO_MEAN");
  }
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

/***************************************************************/
/*                                                             */
/*  Devuelve el tiempo en millis que lleva el sistema activo   */
/*                                                             */
/***************************************************************/
unsigned long uptime(void) {return millis();}  

/***************************************************************/
/*                                                             */
/*  Funcion de interrupcion del watchdog                       */
/*                                                             */
/***************************************************************/
//funcion de interrupcion que reseteara el ESP si no se atiende el watchdog
void IRAM_ATTR resetModule(void) {
  ets_printf("Watchdog!!! reboot\n");
  enviarMQTT("freeheap", "{\"IP\": \"" + String(getIP(debugGlobal)) + "\" , \"Uptime\": 1970, \"freeHeap\": " + String(ESP.getFreeHeap()) + ", \"potencia\": \"" + String(WiFi.RSSI()) + "\", \"lastResetReason0\": " + rtc_get_reset_reason(0) + ", \"lastResetReason1\": " + rtc_get_reset_reason(1) + "}");  
  esp_restart();
}

/**************************************************************************/
/*                                                                        */
/*  Configuracion del watchdog del sistema                                */
/*  https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/ */
/**************************************************************************/
void configuraWatchdog(void)
{
  timer = timerBegin(TIMER_WATCHDOG, PREESCALADO_WATCHDOG, true); //timer 0, div 80 para que cuente microsegundos y hacia arriba         //hw_timer_t * timerBegin(uint8_t timer, uint16_t divider, bool countUp);
  timerAttachInterrupt(timer, &resetModule, true);                //asigno la funcion de interrupcion al contador                        //void timerAttachInterrupt(hw_timer_t *timer, void (*fn)(void), bool edge);
  timerAlarmWrite(timer, TIEMPO_WATCHDOG, false);                  //configuro el limite del contador para generar interrupcion en us    //void timerAlarmWrite(hw_timer_t *timer, uint64_t interruptAt, bool autoreload);
  timerWrite(timer, 0);                                           //lo pongo a cero                                                      //void timerWrite(hw_timer_t *timer, uint64_t val);
  timerAlarmEnable(timer);                                        //habilito el contador                                                 //void timerAlarmEnable(hw_timer_t *timer);
}

void pintaTrazaInicial(String texto)
  {
  static int x=0;
  static int y=TOP_TRAZA;
  int tamano=8;
   
  if(!debugGlobal) return;
  escribePantalla(x,y,texto,tamano);
  y += INC_TRAZA;
  }
