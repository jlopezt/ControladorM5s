/**********************************************/
/*                                            */
/*  Gestion de la conexion MQTT               */
/*  Incluye la conexion al bus y la           */
/*  definicion del callback de suscripcion    */
/*                                            */
/* Librria de sooprte del protocolo MQTT      */
/* para arduino/ESP8266/ESP32                 */
/*                                            */
/* https://pubsubclient.knolleary.net/api.html*/
/**********************************************/
//Includes MQTT
//#define MQTT_KEEPALIVE 60
#include <PubSubClient.h>
#define CLEAN_SESSION TRUE

//definicion de los comodines del MQTT
#define WILDCARD_ALL      "#"
#define WILDCARD_ONELEVEL "+"

//definicion de constantes para WILL
#define WILL_TOPIC  "will"
#define WILL_QOS    1
#define WILL_RETAIN false
#define WILL_MSG    "¡" + ID_MQTT + " caido!" //String("¡" + ID_MQTT + " caido!")

//Definicion de variables globales
IPAddress IPBroker; //IP del bus MQTT
uint16_t puertoBroker; //Puerto del bus MQTT
uint16_t timeReconnectMQTT; //Tiempo de espera en la reconexion al bus
String usuarioMQTT; //usuario par ala conxion al broker
String passwordMQTT; //password parala conexion al broker
String topicRoot; //raiz del topic a publicar. Util para separar mensajes de produccion y prepropduccion
String ID_MQTT; //ID del modulo en su conexion al broker
String topicMedidas; //topic al que se va a suscribir para recibir las medidas, sera <topicRoot>/<topicMedidas>
String topicReles; //topic al que se va a suscribir para recibir el estado de los reles, sera <topicRoot>/<topic_Reles>
//String topicOrdenes; //topic al que se va a enviar las ordenes de encendido y apagado de los reles, sera <topicRoot>/<topicOrdenes>
int8_t publicarEstado; //Flag para determinar si se envia el json con los valores de las salidas

WiFiClient espClient;
PubSubClient clienteMQTT(espClient);

/************************************************/
/* Inicializa valiables y estado del bus MQTT   */
/************************************************/
void inicializaMQTT(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosMQTT(false)) Serial.printf("error al recuperar config MQTT.\nConfiguracion por defecto.\n");

  //Si va bien inicializo con los valores correstoc, si no con valores por defecto  
  //confituro el servidor y el puerto
  clienteMQTT.setServer(IPBroker, puertoBroker);
  //configuro el callback, si lo hay
  clienteMQTT.setCallback(callbackMQTT);

  if (conectaMQTT()) Serial.println("connectado al broker");  
  else Serial.printf("error al conectar al broker con estado %i\n",clienteMQTT.state());
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de MQTT                          */
/************************************************/
boolean recuperaDatosMQTT(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  IPBroker.fromString("0.0.0.0");
  puertoBroker=0;
  timeReconnectMQTT=0;
  ID_MQTT="";  
  usuarioMQTT="";
  passwordMQTT="";
  topicRoot="";
  topicMedidas="";
  topicOrdenes="";
  topicReles="";
  publicarEstado=1;//por defecto publico

  if(!leeFicheroConfig(MQTT_CONFIG_FILE, cad)) 
    {
    //Confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion MQTT\n");
    cad="{\"IPBroker\": \"0.0.0.0\", \"puerto\": 1883, \"timeReconnectMQTT\": 500, \"ID_MQTT\": \"controlador\", \"usuarioMQTT\": \"usuario\", \"passwordMQTT\": \"password\", \"topicRoot\": \"casa\", \"topicMedidas\": \"/+/medidas\", \"topicOrdenes\": \"actuador\", \"topicReles\": \"actuador\estado\", \"publicarEstado\": 1}";
    if(salvaFicheroConfig(MQTT_CONFIG_FILE, MQTT_CONFIG_BAK_FILE, cad)) Serial.printf("Fichero de configuracion MQTT creado por defecto\n");    
    }

  return parseaConfiguracionMQTT(cad);
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio MQTT                         */
/*********************************************/
boolean parseaConfiguracionMQTT(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    IPBroker.fromString(json.get<String>("IPBroker"));
    puertoBroker=json.get<uint16_t>("puerto");
    timeReconnectMQTT=json.get<uint16_t>("timeReconnectMQTT");
    ID_MQTT=json.get<String>("ID_MQTT");
    usuarioMQTT=json.get<String>("usuarioMQTT");
    passwordMQTT=json.get<String>("passwordMQTT");
    topicRoot=json.get<String>("topicRoot");
    topicMedidas=json.get<String>("topicMedidas"); if(!topicMedidas.startsWith("/")) topicMedidas = "/" + topicMedidas;
    topicOrdenes=json.get<String>("topicOrdenes"); if(!topicOrdenes.startsWith("/")) topicOrdenes = "/" + topicOrdenes;
    topicReles=json.get<String>("topicReles"); if(!topicReles.startsWith("/")) topicReles = "/" + topicReles;    
    publicarEstado=json.get<int8_t>("publicarEstado");
    
    Serial.printf("Configuracion leida:\nIP broker: %s\nIP Puerto del broker: %i\ntimeReconnectMQTT: %i\nID_MQTT: %s\nUsuario: %s\nPassword: %s\nTopic root: %s\nTopic medidas: %s\nTopic Ordenes: %s\nTopic Reles: %s\nPublicar estado: %i\n",IPBroker.toString().c_str(),puertoBroker,timeReconnectMQTT, ID_MQTT.c_str(),usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),topicMedidas.c_str(),topicOrdenes.c_str(),topicReles.c_str(),publicarEstado);
//************************************************************************************************
    return true;
    }
  return false;
  }

/***********************************************Funciones de gestion de mensajes MQTT**************************************************************/
/***************************************************/
/* Funcion que recibe el mensaje cuando se publica */
/* en el bus un topic al que esta subscrito        */
/***************************************************/
void callbackMQTT(char* topic, byte* payload, unsigned int length)
  {
  if(debugGlobal) Serial.printf("Entrando en callback: \n Topic: %s\nPayload %s\nLongitud %i\n", topic, payload, length);
  
  /**********compruebo el topic*****************/ 
  //Identifica si el topic del mensaje es uno de los suscritos (deberia ser siempre que si)
  //Compara el topic recibido con los que tiene suscritos para redirigir a la funcion de gestion correspondiente   
  String cad=String(topic);
  
  //Para cada topic suscrito...  
  if(comparaTopics(topicRoot + topicMedidas, topic)) procesaTopicMedidas(topic,payload,length);  
  else if(comparaTopics(topicRoot + topicReles, topic)) procesaTopicReles(topic,payload,length);
  //elseif(comparaTopics(topicRoot + <topicSuscrito>, topic)) <funcion de gestion>(topic,payload,length);  
  //Si no machea el topic recibido con los sucritos lo tira (no deberia ocurrir)
  else Serial.printf("topic no reconocido: \ntopic: %s\nroot: %s\n", cad.c_str(),cad.substring(0,cad.indexOf("/")).c_str());  
  }

/******************************************************/
/* Funcion que compara los topic recibido y suscrito. */
/* Devuelve true si coinciden, false si no.           */
/******************************************************/
boolean comparaTopics(String topicSuscrito, String topicRecibido)
  {
  //para suscrito
  int8_t finS=0;
  String subS;  
  //para recibido
  int8_t finR=0;
  String subR;

  do
    {
    if(debugGlobal) Serial.printf("topicSuscrito: %s\nsubS: %s\nfinS: %i\n",topicSuscrito.c_str(),subS.c_str(),finS);
    if(debugGlobal) Serial.printf("topicRecibido: %s\nsubR: %s\nfinR: %i\n",topicRecibido.c_str(),subR.c_str(),finR);
    
    finS=topicSuscrito.indexOf("/");    
    finR=topicRecibido.indexOf("/");  
    
    if(finS!=-1) subS=topicSuscrito.substring(0,finS);
    else subS=topicSuscrito;
    if(finR!=-1) subR=topicRecibido.substring(0,finR);
    else subR=topicRecibido;

    //Si en suscrito hay un comodin total, salgo con OK. Si ha llegado hasta aqui es ok, el resto no importa
    if(subS.equals(WILDCARD_ALL)) return true;
    
    //si en suscrito no es un comodin de un nivel, comparo con lo recibido
    if(!subS.equals(WILDCARD_ONELEVEL))
      {
      //si no son iguales, salgo con falso
      if(!subR.equalsIgnoreCase(subS)) return false;
      }

    if(finS==-1 && finR==-1) return true; //Si llego al final de los dos, son iguales
    if(finS==-1 || finR==-1) return false; //Si llego al final de uno y no del otro, no son iguales
      
    //Si son iguales, me quedo con el resto de la cadena
    topicSuscrito=topicSuscrito.substring(finS+1);
    topicRecibido=topicRecibido.substring(finR+1);
    }while(1);
    
  return true;
  }

/***************************************************/
/* Funcion que interpreta el mensaje de medidas de */
/* un satelite. Si no esta registrado, lo registra */
/***************************************************/
void procesaTopicMedidas(char* topic, byte* payload, unsigned int length)
  {  
  char mensaje[length];    
  int id;  
  int estado;

  Serial.printf("topic: %s\nPayload: %s\nlongitud: %i\n",topic,(const char*)payload,length);

  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//acabo la cadena

  /**********************Leo el JSON***********************/
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonBuffer jsonBuffer(bufferSize);     
  JsonObject& root = jsonBuffer.parseObject(mensaje);

  if (!root.success()) 
    {
    Serial.printf("¡¡KO en el parseo del JSON!!\nJSON: %s",mensaje);    
    return; //si el mensaje es incorrecto sale  
    }
    
  //Registro el satelite y copio sobre la habitacion correspondiente del array los datos recibidos
  id=atoi(root["id"]);
  
  //Si no esta registrado, lo registro  
  if(!sateliteRegistrado(id)) 
    {
    String cad=String(topic);
    addSatelite(id, cad.substring(cad.indexOf("/")+1,cad.indexOf("/",cad.indexOf("/")+1)));  
    }

  //Leo los valores
  sateliteLeido(id); //apunto la hora de la lectura del mensaje
  habitaciones[id].temperatura = root["Temperatura"];
  habitaciones[id].humedad = root["Humedad"]; 
  habitaciones[id].luz = root["Luz"];
  Serial.printf("Medida leida:\nSatelite: %i\nTemperatura: %l0.2f\nHmedad: %l0.2f\nLuz: %l0.2f\n",id,habitaciones[id].temperatura,habitaciones[id].humedad,habitaciones[id].luz);
  /**********************Fin JSON***********************/    
  }

/***************************************************/
/* Funcion que interpreta el mensaje de estado de  */
/* los reles                                       */
/***************************************************/
void procesaTopicReles(char* topic, byte* payload, unsigned int length)
  {  
  char mensaje[length];    
    
  //copio el payload en la cadena mensaje
  for(int8_t i=0;i<length;i++) mensaje[i]=payload[i];
  mensaje[length]=0;//acabo la cadena

  setEstadoRelesLeido(String(mensaje));
  }

/********************************************/
/* Funcion que gestiona la conexion al bus  */
/* MQTT del broker                          */
/********************************************/
boolean conectaMQTT(void)  
  {
  int8_t intentos=0;
  String topic;
  if(debugGlobal) Serial.println("Inicio conectaMQTT.");

  if(IPBroker==IPAddress(0,0,0,0)) 
    {
    Serial.println("IP del broker = 0.0.0.0, no se intenta conectar.");
    return (false);//SI la IP del Broker es 0.0.0.0 (IP por defecto) no intentaq conectar y sale con error
    }
  
  while (!clienteMQTT.connected()) 
    {    
    if(debugGlobal) Serial.println("No conectado, intentando conectar.");
  
    // Attempt to connect
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);    
    Serial.printf("Conectando con:\nid=%s\nuser=%s\npassword=%s\nwill topic=%s\nwillQoS=%i\nwillRetain=%i\nwillMessage=%s\ncleanSession=%i\n",ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, String(WILL_MSG).c_str(), CLEAN_SESSION);    
    if (clienteMQTT.connect(ID_MQTT.c_str(), usuarioMQTT.c_str(), passwordMQTT.c_str(), (topicRoot+"/"+String(WILL_TOPIC)).c_str(), WILL_QOS, WILL_RETAIN, String(WILL_MSG).c_str(), CLEAN_SESSION))
      {
      if(debugGlobal) Serial.println("conectado");

      //Suscripcion a todos los topics que aplican a este componente
      //topicMedidas: topic en el que los satelites publican las medidas
      topic=topicRoot + topicMedidas;
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());       

      //topicReles: topic en el que el actuador publica el estado de los reles
      topic=topicRoot + topicReles;
      if (clienteMQTT.subscribe(topic.c_str())) Serial.printf("Subscrito al topic %s\n", topic.c_str());
      else Serial.printf("Error al subscribirse al topic %s\n", topic.c_str());       

      return(true);
      }

    Serial.printf("Error al conectar al broker(%i). Estado: %s\n",intentos,stateTexto().c_str());
    if(intentos++>=2) 
      {
      clienteMQTT.disconnect();
      return (false);
      }
      
    delay(timeReconnectMQTT);      
    } 
  }

/********************************************/
/* Funcion que envia un mensaje al bus      */
/* MQTT del broker                          */
/* Eliminado limite del buffer de envio     */
/********************************************/
boolean enviarMQTT(String topic, String payload)
  {
  //si no esta conectado, conecto
  if(debugGlobal) Serial.println("Verifico si esta conectado");
  if (!clienteMQTT.connected()) 
  {
    if(debugGlobal) Serial.println("No esta conectado");
    conectaMQTT();
  }

  //si y esta conectado envio, sino salgo con error
  if(debugGlobal) Serial.println("Vuelvo a verificar si esta conectado");
  if (clienteMQTT.connected()) 
    {
    if(!topic.startsWith("/")) topic = "/" + topic;  
    topic=topicRoot + topic;
      
    //Serial.printf("Enviando:\ntopic:  %s\n payload: (%i) %s\n",topic.c_str(),payload.length(),payload.c_str());
  
    if(clienteMQTT.beginPublish(topic.c_str(), payload.length(), false))//boolean beginPublish(const char* topic, unsigned int plength, boolean retained)
      {      
      for(uint16_t i=0;i<payload.length();i++) clienteMQTT.write((uint8_t)payload.charAt(i));//virtual size_t write(uint8_t);
      return(clienteMQTT.endPublish()); //int endPublish();
      }
    }
  else return (false);
  }

/********************************************/
/* Funcion que revisa el estado del bus y   */
/* si se ha recibido un mensaje             */
/********************************************/
void atiendeMQTT(void)
  {      
  clienteMQTT.loop();
  }

/********************************************/
/*                                          */
/* Funcion que envia datos de estado del    */
/* controlador al broker                    */
/*                                          */
/********************************************/
void enviaDatos(boolean debug)
  {
  String topic= "/" +   ID_MQTT + "/estado";  
  String payload;

  if(publicarEstado==1)
    {
    payload=generaJsonEstado();//genero el json de estado
    
    if(debug)Serial.println("Inicio envio de json al broker.");
    
    //Lo envio al bus    
    if(enviarMQTT(topic, payload)) if(debug)Serial.println("Enviado json al broker con exito.");
    else if(debug)Serial.println("¡¡Error al enviar json al broker!!");
    }
  else if(debugGlobal) Serial.printf("No publico estado. Publicar estado es %i\n",publicarEstado);  
  }
  
/******************************* UTILIDADES *************************************/
/********************************************/
/* Funcion que devuleve el estado           */
/* de conexion MQTT al bus                  */
/********************************************/
String stateTexto(void)  
  {
  int r = clienteMQTT.state();

  String cad=String(r) + " : ";
  
  switch (r)
    {
    case -4:
      cad += "MQTT_CONNECTION_TIMEOUT";
      break;
    case -3:
      cad += "MQTT_CONNECTION_LOST";
      break;
    case -2:
      cad += "MQTT_CONNECT_FAILED";
      break;
    case -1:
      cad += "MQTT_DISCONNECTED";
      break;
    case  0:
      cad += "MQTT_CONNECTED";
      break;
    case  1:
      cad += "MQTT_CONNECT_BAD_PROTOCOL";
      break;
    case  2:
      cad += "MQTT_CONNECT_BAD_CLIENT_ID";
      break;
    case  3:
      cad += "MQTT_CONNECT_UNAVAILABLE";
      break;
    case  4:
      cad += "MQTT_CONNECT_BAD_CREDENTIALS";
      break;     
    case  5:
      cad += "MQTT_CONNECT_UNAUTHORIZED";
      break;
    default:
      cad += "????";
    }
      
  return (cad);
  }  
