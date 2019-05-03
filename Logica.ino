/*****************************************/
/*                                       */
/*  Logica de control de salidas         */
/*  para el termostato                   */
/*                                       */
/*****************************************/

#define MODO_OFF   0
#define MODO_ON    1
#define MODO_AUTO  2

#define MAPA_CONFIG_FILE             "/Mapa.json"
#define TEMPERATURAS_CONFIG_FILE     "/Temperaturas.json"
#define TEMPERATURAS_CONFIG_BAK_FILE "/Temperaturas.json.bak"
#define RELES_CONFIG_FILE            "/RelesConfig.json"
#define RELES_CONFIG_BAK_FILE        "/RelesConfig.json.bak"


#define SEGUNDOS_EN_HORA         3600
#define TICKS_CALEFACCION_MANUAL (SEGUNDOS_EN_HORA*1000)/(ANCHO_INTERVALO*FRECUENCIA_LOGICA_CONTROL)//1 hora

typedef struct{
  int8_t id;
  String nombre;
  int8_t estado;
  }rele_t; 

typedef struct{
  float dia;
  float noche;
  } consigna_t;

#define LUNES      1
#define MARTES     2
#define MIERCOLES  4
#define JUEVES     8
#define VIERNES   16
#define SABADO    32
#define DOMINGO   64

#define CALDERA   0
#define SEGURIDAD 1

int8_t diasSemana[7]={LUNES,MARTES,MIERCOLES,JUEVES,VIERNES,SABADO,DOMINGO};
int8_t modoManual; //Puede valer OFF, ON o AUTO
int8_t valoresModoManual[3]={MODO_OFF, MODO_ON, MODO_AUTO};
String valoresModoManualTxt[3]={"Off \n","On  \n","Auto\n"};//Todos de longitud 4+1

/**********************************************************************************************************************************/
/*                                                                                                                                */
/* 48 columnas correspondientes a 48 medias horas de un dia (en horizontal un dia en 48 trozos de media hora)                     */  
/* Las 8 filas se corresponden a los dias de la semana (sobra uno...) dias los 48 bist 0 son el lunes, los 48 bits 2 el martes... */
/*                                                                                                                                */
/* Los valores significan: 1->es un tramo con consigna de dia; 0->es un tiempo con consigna de noche                              */
/*
             1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20     21    22    23    24    25    26    27    28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48
          0:00  0:30  1:00  1:30  2:00  2:30  3:00  3:30  4:00  4:30  5:00  5:30  6:00  6:30  7:00  7:30  8:00  8:30  9:00  9:30  10:00 10:30 11:00 11:30 12:00 12:30 13:00 13:30 14:00 14:30 15:00 15:30 16:00 16:30 17:00 17:30 18:00 18:30 19:00 19:30 20:00 20:30 21:00 21:30 22:00 22:30 23:00 23:30
Lunes        0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Martes       0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Miércoles    0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Jueves       0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Viernes      0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Sábado       0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1      1      1    1     1     1     1     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
Domingo      0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     1     1     1      1      1    1     1     1     1     0     0     0     0     0     0     0     0     1     1     1     1     1     1     1     1     1     1     1     1     1     1
EXTRA        0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0      0      0    0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0     0
TOTAL        0     0     0     0     0     0     0     0     0     0     0     0     0    31    31    31    31    96    96    96     96     96   96    96    96    96     0     0     0     0     0     0     0     0   127   127   127   127   127   127   127   127   127   127   127   127   127   127
*/
//valor por defecto
int8_t mapa[48]={0,0,0,0,0,0,0,0,0,0,0,0,0,31,31,31,31,96,96,96,96,96,96,96,96,96,0,0,0,0,0,0,0,0,127,127,127,127,127,127,127,127,127,127,127,127,127,127};
/*                                                                                                                                */
/**********************************************************************************************************************************/

String nombre_reles[MAX_RELES];
 
rele_t reles[MAX_RELES];
consigna_t consigna={23.0,15.6}; //Limite para encender el rele de dia y de noche
float umbralLuz=10;//30; //porcentaje que se considera que hay luz en la habitacion

float temperaturaPromedio=0;

/*********downCounter para modo manual************/
int downCounter;//Contador de ticks en modo manual, se inicializa al pasar a modo manual y se decrementa en cada vuelta de logicaDeControl. Al quedar a cero se pasa  amodo auto
void setDownCounter(int valor){downCounter=valor;}
int getDownCounter(void){return downCounter;}
void initDownCounter(int duracion)
  {
  downCounter=(duracion<=0?TICKS_CALEFACCION_MANUAL:duracion);//TICKS_CALEFACCION_MANUAL;
  }  
void decrementaDownCounter(void)
  {
  if(downCounter>0) downCounter--;//Decremento el contador
  if(downCounter<=0) setModoManual(MODO_AUTO);//Si esta a cero, se pasa a modo manual
  }
/*************************************************/  

//////////////////////////////////////////////////Funciones de configuracion del mapa///////////////////////////////////////////////////////
void inicializaLogica()
  { 
  //Inicializa el modo de calefaccion  
  downCounter=0;//Contador de tics en modo manual. Despues de un tiempo TICKS_CALEFACCION_MANUAL vuelve a MODO_AUTO   
  setModoManual(MODO_AUTO);
    
  consigna.dia=22.5;
  consigna.noche=15.6;  
  
  //preconfiguracion de fabrica de los reles
  //0 Calefaccion
  nombre_reles[0]="Calefaccion";
  //1 Otros
  nombre_reles[1]="Otros";  

  //for(int i;i<MAX_RELES;i++) Serial.printf("Nombre rele[%i]=%s\n",i,nombre_reles[i].c_str());

  for(int8_t i=0;i<MAX_RELES;i++)
    {
    //inicializo la parte logica
    reles[i].id=i;
    reles[i].nombre=nombre_reles[i];
    reles[i].estado=0;    
    }
    
  leeFicheroMapa();
  leeFicheroTemperaturas();
  leeFicheroReles();
  }

/*********************************************/
/* Lee el fichero de configuracio de reles   */
/*********************************************/
void leeFicheroReles(void)
  {
  String cad="";
  
  if(leeFichero(RELES_CONFIG_FILE, cad)) parseaConfiguracionreles(cad);
  else Serial.println("Configuracion de relesa por defecto");
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de reles                     */
/*********************************************/
boolean parseaConfiguracionreles(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    JsonArray& Reles = json["Reles"];
    
    for(int8_t i=0;i<MAX_RELES;i++)
      { 
      reles[i].nombre=String((const char *)Reles[i]["nombre"]);
      }
      
    Serial.println("Reles:"); 
    for(int8_t i=0;i<MAX_RELES;i++) Serial.printf("%02i: %s\n",i,reles[i].nombre.c_str()); 
//************************************************************************************************
    return true;
    }
  return false;
  }

void leeFicheroMapa(void)
  {
  String cad="";
  
  if(leeFichero(MAPA_CONFIG_FILE, cad)) parseaConfiguracionMapa(cad);
  else 
    {
    Serial.println("Configuracion del mapa por defecto");
    mapa[0 ]=0;mapa[1 ]=0;mapa[2 ]=0;mapa[3 ]=0;mapa[4 ]=0;mapa[5 ]=0;mapa[6 ]=0;mapa[7 ]=0;mapa[8 ]=0;mapa[9 ]=0;mapa[10]=0;mapa[11]=0;mapa[12]=0;mapa[13]=31;mapa[14]=31;mapa[15]=31;mapa[16]=31;mapa[17]=96;mapa[18]=96;mapa[19]=96;mapa[20]=96;mapa[21]=96;mapa[22]=96;mapa[23]=96;mapa[24]=96;mapa[25]=96;mapa[26]=0;mapa[27]=0;mapa[28]=0;mapa[29]=0;mapa[30]=0;mapa[31]=0;mapa[32]=0;mapa[33]=0;mapa[34]=127;mapa[35]=127;mapa[36]=127;mapa[37]=127;mapa[38]=127;mapa[39]=127;mapa[40]=127;mapa[41]=127;mapa[42]=127;mapa[43]=127;mapa[44]=127;mapa[45]=127;mapa[46]=127;mapa[48]=127;
    //mapa[48]=(0,0,0,0,0,0,0,0,0,0,0,0,0,31,31,31,31,96,96,96,96,96,96,96,96,96,0,0,0,0,0,0,0,0,127,127,127,127,127,127,127,127,127,127,127,127,127,127);
    }
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio del mapa                     */
/*********************************************/
boolean parseaConfiguracionMapa(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    char id[5]="0000";
    for(int8_t i=0;i<48;i++) 
      {
      //Serial.printf("Cadena id formada: {%02d%s}\n",i/2,(i%2?"30":"00"));  
      sprintf(id,"%02d%s",i/2,(i%2?"30":"00"));  
      id[4]=0;//fin de cadena
      //Serial.printf("id=%s\n",id);

      mapa[i]=json[id];            
      }
    Serial.print("mapa[48]={"); 
    for(int8_t i=0;i<48;i++) Serial.printf("%i,",mapa[i]); 
    Serial.println("}"); 
//************************************************************************************************
    return true;
    }
  return false;
  }

void leeFicheroTemperaturas(void)
  {
  String cad="";
  
  if(leeFichero(TEMPERATURAS_CONFIG_FILE, cad)) parseaConfiguracionTemperaturas(cad);
  else 
    {
    Serial.println("Configuracion de temperatudas por defecto");
    setConsignaD(consigna.dia);
    setConsignaN(consigna.noche);
    ficherosModificados=0;//Para que no salve el fichero de configuracion de consignas
    }
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de temperaturas              */
/*********************************************/
boolean parseaConfiguracionTemperaturas(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    Serial.printf("Consigna de dia: %04.1f\nConsigna de noche: %04.1f\n",setConsignaD(json["dia"]),setConsignaN(json["noche"])); 
//************************************************************************************************
    return true;
    }
  return false;
  }

//////////////////////////////////////////////////Funciones de ejecucion de la logica /////////////////////////////////////////////////

/******************************************************/
/*                                                    */
/* Ejecuta la logica de control para actuar           */ 
/* sobre los reles de salida                          */
/*                                                    */
/******************************************************/
void logicaControl(void)
  {
  //En funcion del modo de calefaccion ejecuta una salida u otra
  switch(getModoManual())
    {
    case MODO_OFF://Apagado manual de la caldera
      decrementaDownCounter();  //Decremento el contador de ticks en modo manual, si esta a cero paso a auto
        
      setEstadoRele(SEGURIDAD,1);
      setEstadoRele(CALDERA,0);
      break;
    case MODO_ON://Encendido manual de la caldera
      decrementaDownCounter();  //Decremento el contador de ticks en modo manual, si esta a cero paso a auto

      setEstadoRele(SEGURIDAD,1);
      setEstadoRele(CALDERA,1);
      break;
    case MODO_AUTO://Logica de control en funcion de la temperatura promedio
      if(numeroSatelitesLeidos(debugGlobal))
        {
        setEstadoRele(SEGURIDAD,1);//si hay satelites lo enciendo
          
        temperaturaPromedio=promediaTemperatura(); //si responde algun satelite, promedio  
          
        if(temperaturaPromedio<getConsigna()) setEstadoRele(CALDERA,1);//si esta por debajo de la consigna, enciendo el rele
        else setEstadoRele(CALDERA,0); //si no lo apago    
        }
      else 
        {
        setEstadoRele(SEGURIDAD,0);//si no hay satelites lo apago
        temperaturaPromedio=NO_LEIDO;
        }
      break;  
    }
    
  actualizaReles();
  }

/******************************************************/
/*                                                    */
/* actua sobre los reles de salida                    */
/*                                                    */
/******************************************************/
void actualizaReles(void)
  {
  //actualizo el estado de los reles
  controlaRele(SEGURIDAD ,reles[SEGURIDAD].estado);
  controlaRele(CALDERA ,reles[CALDERA].estado);
  }

/******************************************************/
/*                                                    */
/* Devuelve el estadoi logico del rele indicado       */ 
/* No es el estado fisico en la placa de control      */
/*                                                    */
/******************************************************/
int getEstadoRele(int rele)
  {
  if (rele>=MAX_RELES) return -1;
  return reles[rele].estado;
  }

/******************************************************/
/*                                                    */
/* Establece el estado logico del rele indicado       */ 
/* No es el estado fisico en la placa de control      */
/*                                                    */
/******************************************************/
int setEstadoRele(int rele, int estado)
  {
  if (rele<MAX_RELES) reles[rele].estado=estado;
  }

/******************************************************/
/*                                                    */
/* Devuelve el nombre del rele indicado               */ 
/* No es el estado fisico en la placa de control      */
/*                                                    */
/******************************************************/
String getNombreRele(int rele)
  {
  if (rele>=MAX_RELES) return "";
  return reles[rele].nombre;
  }
      
/******************************************************/
/*                                                    */
/* Devuelve la temperatura promedio calculada         */ 
/*                                                    */
/******************************************************/
float getTemperaturaPromedio(void)
  {
  return temperaturaPromedio;
  }

/******************************************************/
/*                                                    */
/* Devuelve la temperatura de consigna actual         */ 
/*                                                    */
/******************************************************/
float getConsigna(void)
  {
  int8_t intervalo=0;//intervalo de 30 mintuntos en el que estamos    

  //Calculo el intervalo
  intervalo=2*hora();           //multiplico las horas por 2, intervalo por 30 minutos
  if(minuto()>=30) intervalo++; //si es mas alla de y media paso al siguiente intervalo

  //cojo el byte del mapa y hago AND con la mascara del dia, si es 1 devuelve la tem de dia y si es 0 la de noche
  return ((mapa[intervalo]&diasSemana[diaSemana()-1])?consigna.dia:consigna.noche);      
  }
  
float getConsignaDia(void) {return consigna.dia;}
float getConsignaNoche(void){return consigna.noche;}
    
/******************************************************/
/*                                                    */
/* Modifica la consigna                               */ 
/* FALTA SALVAR CONSIGNA A FICHERO                    */
/* {"dia": "22.3","noche":"15.1"}                     */
/*                                                    */
/******************************************************/
float setConsigna(float valor)
  {
  int8_t intervalo=0;//intervalo de 30 mintuntos en el que estamos    

  //Calculo el intervalo
  intervalo=2*hora();           //multiplico las horas por 2, intervalo por 30 minutos
  if(minuto()>=30) intervalo++; //si es mas alla de y media paso al siguiente intervalo

  //cojo el byte del mapa y hago AND con la mascara del dia, si es 1 modifica la tem de dia y si es 0 la de noche
  if(mapa[intervalo]&diasSemana[diaSemana()-1])consigna.dia=valor;
  else consigna.noche=valor;  

  ficherosModificados = ficherosModificados | FICHERO_CONSIGNAS_MODIFICADO;
  }
  
float setConsignaD(float c) 
  {
  consigna.dia=c;
  ficherosModificados = ficherosModificados | FICHERO_CONSIGNAS_MODIFICADO;
  
  return consigna.dia;
  }
  
float setConsignaN(float c) 
  {
  consigna.noche=c;
  ficherosModificados = ficherosModificados | FICHERO_CONSIGNAS_MODIFICADO;
    
  return consigna.noche;
  }

void salvaConsignaFichero(void)  
  {  
  String cad="{\"dia\": \"" + String(getConsignaDia(),1) + "\",\"noche\":\"" + String(getConsignaNoche(),1) + "\"}";
  salvaFichero(TEMPERATURAS_CONFIG_FILE,TEMPERATURAS_CONFIG_BAK_FILE,cad);  
  }
/*******************************************************/
/*                                                     */
/* Devuelve si hay mas luz del limite en la habitacion */ 
/*                                                     */
/*******************************************************/
int8_t hayLuz(int8_t id) 
  {
  if(getLuz(id,false)>=umbralLuz) return true;
  return false;
  }

/******************************************************/
/*                                                    */
/* Lee el estado de los reles en el actuador          */ 
/*                                                    */
/******************************************************/
String leeEstadoReles(void)//ESTA A MEDIAS
  {
  struct tipo_respuestaHTTP respuestaHTTP;  

  respuestaHTTP=ClienteHTTP("http://"+IPActuador.toString()+"/estado");  
  if(debugGlobal)
    {
    if(respuestaHTTP.httpCode==HTTP_CODE_OK) Serial.printf("Respuesta: %s\n",respuestaHTTP.payload.c_str());
    else if(respuestaHTTP.httpCode<0)//error no HTTP=TimeOut
        {
        Serial.println("Error en la lectura de los reles.");
        setError(ERROR_COM_RELES, "Error com. rele");
        }
    }
  return respuestaHTTP.payload;
  } 

/******************************************************/
/*                                                    */
/* Activa/Desactiva el rele especificado              */ 
/* mediante mensajes MQTT                             */
/*                                                    */
/******************************************************/
int controlaRele(int8_t id, int8_t estado)
  {
  if(id<0 || id>=MAX_RELES) return KO;

  String topic;
  String payload;

  topic="actuador";
  //{"id": "","estado": ""}
  if (estado) payload=String("{\"id\": \"")+ String(id) + String("\",\"estado\": \"on\"}");
  else payload=String("{\"id\": \"")+ String(id) + String("\",\"estado\": \"off\"}");
  
  //Serial.printf("Se envia:\ntopic: %s | payload: %s\n",topic.c_str(),payload.c_str());
  if (enviarMQTT(topic,payload)) return OK;
  
  return KO;
  }
  
/*******************************************************/
/*                                                     */
/* Devuelve si hay mas luz del limite en la habitacion */ 
/*                                                     */
/*******************************************************/
float setUmbralLuz(int umbral)
  {
  if(umbral!=0) umbralLuz=(float) umbral;
  
  return umbralLuz;
  }


/*******************************************************/
/*                                                     */
/* Recibe una cadena y la parsea para llenar la tabla  */ 
/*                                                     */
/*******************************************************/
void rellenaMapa(char* cadena)   
  {
  char* resto=cadena;
  int8_t i=0;
  
  if(debugGlobal) Serial.printf("Cadena orignial %s.\n",resto);
  while(*resto!='\0') //debe hacerlo 48 veces
    {
    char* t;
    t=parseaCad(resto, SEPARADOR);
    if(debugGlobal) Serial.printf("Trozo%i: %s - cola= %s.\n",i,resto,t);
    mapa[i++]=atoi(resto);
    resto=t;
    if(i>48) //no pongo el = porque hago el i++ y vale uno mas de lo normal
      {
      Serial.println("ERROR!!!!!");
      break;
      }
    }
  }

/*******************************************************/
/*                                                     */
/* configura el modo manual activo                     */
/* MODO_ON=0                                           */
/* MODO_OFF=1                                          */
/* MODO_AUTO=2                                         */
/*                                                     */
/*******************************************************/
void setModoManual(int8_t modo) {setModoManual(modo,TICKS_CALEFACCION_MANUAL);}
void setModoManual(int8_t modo, int duracion)
  {
  modoManual=modo%3;//no puede ser superior a 2
  if (modoManual!=MODO_AUTO) initDownCounter(duracion);//Si no esta en auto,cargo el contador de modo manual
  else initDownCounter(0);
  }  

/*******************************************************/
/*                                                     */
/* Devuuelve el modo manual activo                     */ 
/* MODO_ON=0                                           */
/* MODO_OFF=1                                          */
/* MODO_AUTO=2                                         */
/*                                                     */
/*******************************************************/
int8_t getModoManual(void)
  {
  return modoManual;
  }  

/*******************************************************/
/*                                                     */
/* Devuuelve un texto indicativo del modo              */
/* manual activo almacenado en valoresModoManualTxt    */ 
/*                                                     */
/*******************************************************/
String getModoManualTxt(void)
  {
  return valoresModoManualTxt[modoManual];
  }  

/*******************************Funcines Auxiliar****************************************************************/
/***************************************/
/* Genera el json con las medidas      */
/***************************************/
String generaJson(void)
  {
  String cad="";
  /***********************************************************************************
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["medida"] = getTemperaturaPromedio();
  root["consigna"] = getConsigna();
  root["estado"] = getModoManual();
  root["tics"] = getDownCounter();
  
  JsonArray& jsonHhabitaciones = root.createNestedArray("habitaciones");
  JsonObject& habitaciones_0 = jsonHhabitaciones.createNestedObject();
  
  for(int8_t id=0;id<MAX_SATELITES;id++) //para todas las habitaciones
    {
    if(sateliteRegistrado(id)) //Solo si esta registrada
      {   
      JsonObject& habitaciones_0_habitacion = habitaciones_0.createNestedObject(getNombre(id,debugGlobal));
      habitaciones_0_habitacion["id"] = id;
      habitaciones_0_habitacion["temperatura"] = getTemperatura(id,debugGlobal);
      habitaciones_0_habitacion["humedad"] = getHumedad(id,debugGlobal);
      habitaciones_0_habitacion["luz"] = getLuz(id,debugGlobal);
      }
    }
  
  JsonArray& jsonReles = root.createNestedArray("reles");
  JsonObject& reles_0 = jsonReles.createNestedObject();
  
  for(int8_t i=0;i<MAX_RELES;i++) //para todos los reles
    {
    JsonObject& reles_0_rele = reles_0.createNestedObject(reles[i].nombre);
    reles_0_rele["id"] = reles[i].id;
    reles_0_rele["estado"] = reles[i].estado;    
    }
  
  root.printTo(cad);//root.printTo(Serial);
  **********************************************************************************************************/  
  /***********************************************************************************/
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(3) + 2*JSON_OBJECT_SIZE(3) + 3*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["medida"] = getTemperaturaPromedio();
  root["consigna"] = getConsigna();
  root["modo"] = getModoManual();
  root["tics"] = getDownCounter();
  
  JsonArray& jsonHhabitaciones = root.createNestedArray("habitaciones");    
  for(int8_t id=0;id<MAX_SATELITES;id++) //para todas las habitaciones
    {
    if(sateliteRegistrado(id)) //Solo si esta registrada
      {   
      JsonObject& habitaciones_0 = jsonHhabitaciones.createNestedObject();
      habitaciones_0["id"] = id;
      habitaciones_0["nombre"] = getNombre(id,debugGlobal);
      habitaciones_0["temperatura"] = getTemperatura(id,debugGlobal);
      habitaciones_0["humedad"] = getHumedad(id,debugGlobal);
      habitaciones_0["luz"] = getLuz(id,debugGlobal);    
      }
    }
  
  JsonArray& jsonReles = root.createNestedArray("reles");
  for(int8_t i=0;i<MAX_RELES;i++) //para todos los reles
    {
    JsonObject& reles_0 = jsonReles.createNestedObject();      
    reles_0["id"] = reles[i].id;
    reles_0["nombre"] = reles[i].nombre;
    reles_0["estado"] = reles[i].estado;    
    }
  
  root.printTo(cad);//root.printTo(Serial);
  /**********************************************************************************************************/  
  return cad;
  }
  
/*******************************************************/
/*                                                     */
/* Convierte un numero de segundos en ticks            */
/* para los modos manuales                             */ 
/*                                                     */
/*******************************************************/
int seg2ticks(int seg) {return (seg*1000)/(ANCHO_INTERVALO*FRECUENCIA_LOGICA_CONTROL);}

/*******************************************************/
/*                                                     */
/* Convierte un numero de segundos en ticks            */
/* para los modos manuales                             */ 
/*                                                     */
/*******************************************************/
int ticks2seg(int tic) {return (tic*ANCHO_INTERVALO*FRECUENCIA_LOGICA_CONTROL)/1000;}
