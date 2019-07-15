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
#define VERSION           "2.5.0 M5Stack (OTA|MQTT|LOGIC+) lib v0.2.2"
#define SEPARADOR         '|'
#define SUBSEPARADOR      '#'
#define KO                -1
#define OK                0
#define MAX_VUELTAS       32767
#define SATELITE_TIME_OUT 100000 //Milisegundos transcurridos entre dos peticiones del controlador antes de intentar registrarse

//Organizacion de pines de E/S
#define RELES_PIN          8 //Pîn del primer rele, los demas consecutivos
#define MAX_SATELITES     16 //numero maximo de satelites de 0 a 15 controlado por los DIP Switch
#define MAX_RELES          2 //numero maximo de reles soportado


//Nombres de ficheros
#define FICHERO_CANDADO        "/Candado"
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
#define FRECUENCIA_ENVIA_DATOS      100 //cada cuantas vueltas de loop publica el estado en el broker MQTT
#define FRECUENCIA_SATELITE_TIMEOUT  50 //cada cuantas vueltas de loop compruebo si ha habido time out en los satelites
#define FRECUENCIA_WIFI_WATCHDOG    100 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

//configuracion del watchdog del sistema
#define TIMER_WATCHDOG        0 //Utilizo el timer 0 para el watchdog
#define PREESCALADO_WATCHDOG 80 //el relog es de 80Mhz, lo preesalo entre 80 para pasarlo a 1Mhz
#define TIEMPO_WATCHDOG      4*ANCHO_INTERVALO //Si en N ANCHO_INTERVALO no se atiende el watchdog, saltara

//Para la pantalla
#define COLOR_FONDO TFT_NAVY
#define COLOR_TITULO TFT_BLUE
#define COLOR_LETRAS_TITULO TFT_WHITE
#define TEXTO_TITULO "Termostatix 2.4"
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
time_t TimeOut=DEFAULT_TIME_OUT;
//Valores de la pantalla
int brilloPantalla=DEFAULT_BRILLO_PANTALLA;
//************ Fin valores configurables *************//

/*-----------------Variables comunes---------------*/
String nombre_dispositivo(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
time_t SleepBucle=0;
time_t limiteSleep=0;

unsigned int vuelta = 32700;//0; //vueltas de loop del core 0

hw_timer_t *timer = NULL;//Puntero al timer del watchdog

int8_t debugGlobal=1; //por defecto desabilitado
int8_t ficherosModificados=0;//Inicialmente no hay ficheros modificados
boolean candado=false; //Candado de configuracion. true implica que la ultima configuracion fue mal
/*-----------------Variables comunes---------------*/

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

  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  inicializaFicheros(debugGlobal);

  //Compruebo si existe candado, si existe la ultima configuracion fue mal
  if(existeFichero(FICHERO_CANDADO)) 
    {
    Serial.printf("Candado puesto. Configuracion por defecto");
    candado=true; 
    debugGlobal=1;
    }
  else
    {
    candado=false;
    //Genera candado
    if(salvaFichero(FICHERO_CANDADO,"","JSD")) Serial.println("Candado creado");
    else Serial.println("ERROR - No se pudo crear el candado");
    }
 
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

  //Si ha llegado hasta aqui, todo ha ido bien y borro el candado
  if(borraFichero(FICHERO_CANDADO))Serial.println("Candado borrado");
  else Serial.println("ERROR - No se pudo borrar el candado");
  
  Serial.println("");
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*            Fin init del modulo Controlador                  *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  //activo el watchdog
  //configuraWatchdog();
  }  

void  loop(void)
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=0;
  EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //reinicio el watchdog del sistema
  //timerWrite(timer, 0);
  
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
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_SERVIDOR_WEB)==0) webServer(debugGlobal); //atiende el servidor web  
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_ORDENES)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_SALVAR)==0) salvaConfiguracion();
//Serial.printf("paso: %i\n",paso++);  
  if ((vuelta % FRECUENCIA_WIFI_WATCHDOG)==0) WifiWD();  
//Serial.printf("paso: %i\n",paso++);  
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

  if(!leeFicheroConfig(GLOBAL_CONFIG_FILE, cad))
    {
    Serial.printf("No existe fichero de configuracion global\n");
    cad="{\"TimeOut\": 5000, \"limiteSleep\":20000, \"Brillo\": 10}";//config por defecto
    //salvo la config por defecto
    if(salvaFicheroConfig(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion global creado por defecto\n"); 
    }

  return parseaConfiguracionGlobal(cad);

/*if(leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    if (!parseaConfiguracionGlobal(cad)) Serial.println("¡¡¡Error al cargar la configuracion general!!!");    
    }
  return true;*/
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
    TimeOut = json.get<int>("TimeOut");
    limiteSleep = json.get<int>("limiteSleep");
    brilloPantalla = json.get<int>("Brillo");
    
    Serial.printf("Configuracion leida:\nTimeOut: %i\nsleep: %i\nBrillo: %i\n",TimeOut,limiteSleep,brilloPantalla);
    return true;     
//************************************************************************************************
    }
  return false;
  }

///////////////FUNCIONES COMUNES/////////////////////
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

/***************************************************************/
/*                                                             */
/*  Devuelve el tiempo en millis que lleva el sistema activo   */
/*                                                             */
/***************************************************************/
time_t uptime(void) {return millis();}  

/***************************************************************/
/*                                                             */
/*  Funcion de interrupcion del watchdog                       */
/*                                                             */
/***************************************************************
//funcion de interrupcion que reseteara el ESP si no se atiende el watchdog
void IRAM_ATTR resetModule() {
  Serial.printf("Watchdog!!!");
  //ets_printf("reboot\n");
  //esp_restart();
}
*/
/***************************************************************/
/*                                                             */
/*  Configuracion del watchdog del sistema                     */
/*                                                             */
/***************************************************************
void configuraWatchdog(void)
{
  timer = timerBegin(TIMER_WATCHDOG, PREESCALADO_WATCHDOG, true); //timer 0, div 80 para que cuente microsegundos y hacia arriba
  timerAttachInterrupt(timer, &resetModule, true);                //asigno la funcion de interrupcion al contador
  timerAlarmWrite(timer, TIEMPO_WATCHDOG, false);                 //configuro el limite del contador para generar interrupcion
  timerWrite(timer, 0);                                           //lo pongo a cero
  timerAlarmEnable(timer);                                        //habilito el contador 
}
*/
