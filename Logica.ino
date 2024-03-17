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
#define MAPA_CONFIG_BAK_FILE         "/Mapa.json.bak"
#define TEMPERATURAS_CONFIG_FILE     "/Temperaturas.json"
#define TEMPERATURAS_CONFIG_BAK_FILE "/Temperaturas.json.bak"
#define RELES_CONFIG_FILE            "/RelesConfig.json"
#define RELES_CONFIG_BAK_FILE        "/RelesConfig.json.bak"

#define TICKS_CALEFACCION_MANUAL (SEGUNDOS_EN_HORA*1000)// /(ANCHO_INTERVALO*FRECUENCIA_LOGICA_CONTROL)//1 hora

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
String valoresModoManualTxt[3]={"off \n","on  \n","auto\n"};//Todos de longitud 4+1
String topicOrdenes; //topic al que se va a enviar las ordenes de encendido y apagado de los reles, sera <topicRoot>/<topicOrdenes>
float umbral=0; //umbral para la logica de comparacion con la consigna y evitar que el regulador oscile

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

float temperaturaPromedio=NO_LEIDO;
float humedadPromedio=NO_LEIDO;
float presionPromedio=NO_LEIDO;
float altitudPromedio=NO_LEIDO;

/*********downCounter para modo manual************/
typedef struct {
  uint32_t duracion; //en ms
  uint32_t inicio;  //en ms
} downCounter_t;

downCounter_t downCounter;

void setDownCounter(int duracion)
  {
  downCounter.duracion=(duracion<0?TICKS_CALEFACCION_MANUAL:duracion);//TICKS_CALEFACCION_MANUAL;
  downCounter.inicio=millis();
  }

int getDownCounter(void)
  {
  uint32_t milis=millis();
  uint32_t salida=downCounter.inicio + downCounter.duracion;
  
  if(salida < milis) return 0;

  salida=salida - milis;
  Serial.printf("salida=(%i)=duracion(%i)+inicio(%i)-millis(%i)\n\n",salida,downCounter.duracion,downCounter.inicio, milis);
  
  return (salida/1000);
  }
  
void decrementaDownCounter(void)
  {
  //if(downCounter>0) downCounter--;//Decremento el contador
  if(downCounter.duracion<=millis()-downCounter.inicio) setModoManual(MODO_AUTO);//Si esta a cero, se pasa a modo manual
  }
/*************************************************/  

//////////////////////////////////////////////////Funciones de configuracion del mapa///////////////////////////////////////////////////////
void inicializaLogica()
  { 
  //Inicializa el modo de calefaccion  
  setDownCounter(0);//Contador de tics en modo manual. Despues de un tiempo TICKS_CALEFACCION_MANUAL vuelve a MODO_AUTO   
  setModoManual(MODO_AUTO);

  umbral=0;
    
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
  else Serial.println("Configuracion de reles por defecto");
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
    if (json.containsKey("umbral")) umbral=json.get<float>("umbral");

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

boolean leeFicheroMapa(void)
  {
  String cad="";
  
  if(leeFichero(MAPA_CONFIG_FILE, cad)) return parseaConfiguracionMapa(cad);
  else 
    {
    Serial.println("Configuracion del mapa por defecto");
    mapa[0 ]=0;mapa[1 ]=0;mapa[2 ]=0;mapa[3 ]=0;mapa[4 ]=0;mapa[5 ]=0;mapa[6 ]=0;mapa[7 ]=0;mapa[8 ]=0;mapa[9 ]=0;mapa[10]=0;mapa[11]=0;mapa[12]=0;mapa[13]=31;mapa[14]=31;mapa[15]=31;mapa[16]=31;mapa[17]=96;mapa[18]=96;mapa[19]=96;mapa[20]=96;mapa[21]=96;mapa[22]=96;mapa[23]=96;mapa[24]=96;mapa[25]=96;mapa[26]=0;mapa[27]=0;mapa[28]=0;mapa[29]=0;mapa[30]=0;mapa[31]=0;mapa[32]=0;mapa[33]=0;mapa[34]=127;mapa[35]=127;mapa[36]=127;mapa[37]=127;mapa[38]=127;mapa[39]=127;mapa[40]=127;mapa[41]=127;mapa[42]=127;mapa[43]=127;mapa[44]=127;mapa[45]=127;mapa[46]=127;//mapa[48]=127;
    return false;
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

/****************************************/
/*                                      */
/*      Salvo el mapa a fichero         */
/*                                      */
/****************************************/
boolean generaConfiguracionMapa(void)
  {
  //Genero el nuevo JSON
  const size_t capacity = JSON_OBJECT_SIZE(48);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();
  
  for(uint8_t i=0;i<48;i++)
    {
    char id[5]="0000";
    sprintf(id,"%02d%s",i/2,(i%2?"30":"00")); 
    id[4]=0;//fin de cadena
    root[id] = mapa[i]; 
    }
  
  root.printTo(Serial);
  
  String cad="";
  root.printTo(cad);
  if(!salvaFichero(MAPA_CONFIG_FILE, MAPA_CONFIG_BAK_FILE, cad)){ 
    Serial.printf("Error al salvar el fichero de Mapa de consignas\n");
    return false;
    }

  return true;
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
  uint8_t modo = getModoManual();
  switch(modo)
    {
    case MODO_OFF://Apagado manual de la caldera
      decrementaDownCounter();  //Decremento el contador de ticks en modo manual, si esta a cero paso a auto
        
      setEstadoRele(SEGURIDAD,1);
      setEstadoRele(CALDERA,0);
      break;
    case MODO_ON://Encendido manual de la caldera
      decrementaDownCounter();  //Decremento el contador de ticks en modo manual, si esta a cero paso a auto

      /*
      setEstadoRele(SEGURIDAD,1);
      setEstadoRele(CALDERA,1);
      break;
      */
    case MODO_AUTO://Logica de control en funcion de la temperatura promedio
      if(pesoSatelitesLeidos(debugGlobal))//numeroSatelitesLeidos(debugGlobal))
        {
        int estadoCaldera=getEstadoRele(CALDERA);
        setEstadoRele(SEGURIDAD,1);//si hay satelites lo enciendo

        //si responde algun satelite, promedio  
        temperaturaPromedio=promediaTemperatura();
        humedadPromedio=promediaHumedad();
        presionPromedio=promediaPresion();
        altitudPromedio=promediaAltitud();

        if(estadoCaldera==1)//calentando
          {
          if(temperaturaPromedio<getConsigna(modo)+umbral) setEstadoRele(CALDERA,1);
          else setEstadoRele(CALDERA,0);
          }
        else
          {
          if(temperaturaPromedio>getConsigna(modo)-umbral) setEstadoRele(CALDERA,0);
          else setEstadoRele(CALDERA,1);
          }        
        /*
        if(temperaturaPromedio<getConsigna()) setEstadoRele(CALDERA,1);//si esta por debajo de la consigna, enciendo el rele
        else setEstadoRele(CALDERA,0); //si no lo apago    
        */
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
/* Devuelve el estado logico del rele indicado        */ 
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
void setEstadoRele(int rele, int estado)
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
/* Devuelve la humedad promedio calculada             */ 
/*                                                    */
/******************************************************/
float getHumedadPromedio(void)
  {
  return humedadPromedio;
  }

/******************************************************/
/*                                                    */
/* Devuelve la presion promedio calculada         */ 
/*                                                    */
/******************************************************/
float getPresionPromedio(void)
  {
  return presionPromedio;
  }

/******************************************************/
/*                                                    */
/* Devuelve la altitud promedio calculada         */ 
/*                                                    */
/******************************************************/
float getAltitudPromedio(void)
  {
  return altitudPromedio;
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

float getConsigna(uint8_t modo){
  switch (modo){
    case MODO_OFF:
      return 0.0;
      break;
    case MODO_ON:
      return getConsignaDia();
      break;
    case MODO_AUTO:
      return getConsigna();
      break;
  }

  return 0.0;
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
float setConsigna(float valor){
  int8_t intervalo=0;//intervalo de 30 mintuntos en el que estamos    
  //Calculo el intervalo
  intervalo=2*hora();           //multiplico las horas por 2, intervalo por 30 minutos
  if(minuto()>=30) intervalo++; //si es mas alla de y media paso al siguiente intervalo

  ficherosModificados = ficherosModificados | FICHERO_CONSIGNAS_MODIFICADO;

  //cojo el byte del mapa y hago AND con la mascara del dia, si es 1 modifica la tem de dia y si es 0 la de noche
  if(mapa[intervalo]&diasSemana[diaSemana()-1]){
    consigna.dia=valor;
    return consigna.dia;
    }
  else {
    consigna.noche=valor;  
    return consigna.noche;
    }
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
/* Activa/Desactiva el rele especificado              */ 
/* mediante mensajes MQTT                             */
/*                                                    */
/******************************************************/
int controlaRele(int8_t id, int8_t estado)
  {
  if(id<0 || id>=MAX_RELES) return KO;

  String topic;
  String payload;

  topic=topicOrdenes;//"actuador";
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
/* MODO_OFF=0                                          */
/* MODO_ON=1                                           */
/* MODO_AUTO=2                                         */
/*                                                     */
/*******************************************************/
void setModoManual(int8_t modo) {setModoManual(modo,TICKS_CALEFACCION_MANUAL);}
void setModoManual(int8_t modo, int duracion)
  {
  Serial.printf("modo: %i | duracion: %i\n",modo, duracion);
  modoManual=modo%3;//no puede ser superior a 2
  if (modoManual!=MODO_AUTO) setDownCounter(duracion);//Si no esta en auto,cargo el contador de modo manual //era initDownCounter
  else setDownCounter(0);//era initDownCounter
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
String generaJsonDatos(void)
  {
  String cad="";

  /***********************************************************************************/
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(6) + 2*JSON_OBJECT_SIZE(3) + 7*JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["titulo"] = NOMBRE_FAMILIA;
  root["medida"] = getTemperaturaPromedio();
  root["modo"] = getModoManual();
  root["consigna"] = getConsigna(root["modo"]);
  root["humedad"] = getHumedadPromedio();
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
      habitaciones_0["presion"] = getPresion(id,debugGlobal);
      habitaciones_0["altitud"] = getAltitud(id,debugGlobal);
      habitaciones_0["luz"] = getLuz(id,debugGlobal);
      habitaciones_0["peso"] = getPeso(id,hora(),debugGlobal);
      habitaciones_0["tiempo"] = millis()-sateliteUltimaLectura(id);          
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
