/**********************************************/
/*                                            */
/*  Control de satelites desde el controlador */
/*  reles para el termostato                  */
/*                                            */
/**********************************************/
#define MAX_ERRORES    3
#define NO_REGISTRADO -1
#ifndef NO_LEIDO
#define NO_LEIDO      -100.0
#endif
#define TEMPERATURA    1
#define HUMEDAD        2
#define LUZ            3

#define NOMBRES_CONFIG_FILE "/TermometrosConfig.json"

boolean IF_JSON=true;

typedef struct{
  int8_t id;
  String nombre;
  int8_t peso[HORAS_EN_DIA];
  unsigned long lectura;
  float  temperatura;
  float  humedad;
  float  luz;
  float  presion;
  float  altitud;
  }habitacion_t;

habitacion_t habitaciones[MAX_SATELITES];

String nombres[MAX_SATELITES]; //nombre por defecto de los satelites. Cuando se registra un satelite envia su nombre
int8_t pesoSatelites[MAX_SATELITES][HORAS_EN_DIA]; //peso en el calculo de la temperatura ponderada

//////////////////////////////////////////////////Funciones de configuracion de los satelites///////////////////////////////////////////////////////
/*inicializa los satelites, pongo todo a cero*/
boolean inicializaSatelites(void)
  {
  //Leo el nombre del fichero o lo inicializo por defecto. Nombres y pesos
  boolean salida=leeFicheroNombres();
  
  //Inicializo el valor de los satelites   
  for(int8_t i=0;i<MAX_SATELITES; i++)
    {
    habitaciones[i].id=NO_REGISTRADO;
    habitaciones[i].nombre=nombres[i];
    for(int8_t h=0;h<HORAS_EN_DIA;h++) habitaciones[i].peso[h]=pesoSatelites[i][h];
    habitaciones[i].lectura=0;
    habitaciones[i].temperatura=NO_LEIDO;//0.0;
    habitaciones[i].humedad=NO_LEIDO;
    habitaciones[i].luz=NO_LEIDO;
    habitaciones[i].presion=NO_LEIDO;  
    habitaciones[i].altitud=NO_LEIDO;
    }  
  return salida;  
  }

boolean leeFicheroNombres(void)
  {
  String cad="";
  
  if(leeFichero(NOMBRES_CONFIG_FILE, cad)) return parseaConfiguracionNombres(cad); 

  //Si falla la lectura del fichero, inicializo por defecto y devuelvo falso
  Serial.println("Configuracion de los satelites por defecto");
  //preconfiguracion de fabrica de las habitaciones
  //0 Salon
  nombres[0]="Salon_def";
  //1 Despacho
  nombres[1]="Despacho_def";  
  //2 Dormitorio Ppal
  nombres[2]="DormitorioPpal_def";  
  //3 Jorge
  nombres[3]="Jorge_def";  
  //4 Sara
  nombres[4]="Sara_def";  
  //5 Diego
  nombres[5]="Diego_def";  
  //6 Buhardilla
  nombres[6]="Buhardilla_def";  
  //7 Bodega
  nombres[7]="Bodega_def";  
  //8 Lavanderia
  nombres[8]="Lavanderia_def"; 
  //9 Salon
  nombres[9]="Exterior_def";
  //10 Despacho
  nombres[10]="Despacho2_def";  
  //11 Dormitorio Ppal
  nombres[11]="DormitorioPpal2_def";  
  //12 Jorge
  nombres[12]="Jorge2_def";  
  //13 Sara
  nombres[13]="Sara2_def";  
  //14 Diego
  nombres[14]="Diego2_def";  
  //15 Buhardilla
  nombres[15]="Buhardilla2_def";  
  
Serial.printf("A por el for\n");
  //Valor por defecto apra el peso de los satelites
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
Serial.printf("i=%i\n",i);        
    for(int8_t j=0;j<HORAS_EN_DIA;j++)
      {
Serial.printf("j=%i\n",j);                  
      pesoSatelites[i][j]=1;
      }
    }
Serial.printf("Fin\n");              
  return false;
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio del mapa                     */
/*********************************************/
boolean parseaConfiguracionNombres(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    JsonArray& Termometros = json["Termometros"];
    
    for(int8_t i=0;i<MAX_SATELITES;i++)
      { 
      nombres[i]=String((const char *)Termometros[i]["nombre"]);
      JsonArray& peso = Termometros[i]["peso"];
      for(int8_t h=0;h<HORAS_EN_DIA;h++) pesoSatelites[i][h]=peso[h];
      }
      
    Serial.println("Nombres:"); 
    for(int8_t i=0;i<MAX_SATELITES;i++) 
      {
      Serial.printf("%02i: %s; peso:\n",i,nombres[i].c_str()); 
      for(int8_t h=0;h<HORAS_EN_DIA;h++) Serial.printf(" %i:%i | ",h,pesoSatelites[i][h]);
      Serial.println();
      }
//************************************************************************************************
    return true;
    }
  return false;
  }

//////////////////////////////////////////////////Funciones de ejecucion de los satelites///////////////////////////////////////////////////////
/***************************************************** Inicio satelites ***************************************************************************/  
/****************************************************************/
/* devuelve si el satelite esta registrado                      */
/* leyendo los que tienen el id!=0                              */
/****************************************************************/
int sateliteRegistrado(int8_t id)
  { 
  if(habitaciones[id].id!=NO_REGISTRADO) return 1;
  return 0;
  }
  
/****************************************************************/
/* devuelve el numero de satelites,                             */
/*leyendo los que tienen el id!=0                               */
/****************************************************************/
int numeroSatelites(int debug)
  {
  int temp=0;
  
  for(int i=0;i<MAX_SATELITES; i++) 
    {
    //if(habitaciones[i].id!=NO_REGISTRADO) temp++;
    if(sateliteRegistrado(i)) temp++;
    }

  return temp;
  }

/****************************************************************/
/* devuelve el numero de satelites con valores leidos,          */
/*leyendo los que tienen el id!=0 y medida de Tª != de -100     */
/****************************************************************/
int numeroSatelitesLeidos(int debug)
  {
  int temp=0;
  
  for(int i=0;i<MAX_SATELITES; i++) 
    {
    //if(habitaciones[i].id!=NO_REGISTRADO) temp++;
    if(sateliteRegistrado(i) && habitaciones[i].temperatura!=NO_LEIDO) temp++;
    }

  return temp;
  }
  
/****************************************************************/
/* devuelve el peso de los satelites con valores leidos,        */
/* leyendo los que tienen el id!=0 y medida de Tª != de -100    */
/****************************************************************/
int pesoSatelitesLeidos(int debug)
  {
  int peso=0;
  int hora_actual=hora();
  
  for(int i=0;i<MAX_SATELITES; i++) 
    {     
    if(sateliteRegistrado(i) && habitaciones[i].temperatura!=NO_LEIDO) peso += habitaciones[i].peso[hora_actual];
    }

  return peso;
  }
  
/**********************************************************/
/*añade un satelite que se ha identificado con su id      */
/*devuelve el id del satelite                             */
/**********************************************************/
int addSatelite(int8_t id, String nombre="")
  {
  habitaciones[id].id=id;//lo pongo en servicio
  
  if(nombre!="") habitaciones[id].nombre=nombre;
  else habitaciones[id].nombre=nombres[id];//le pongo el nombre por defecto //"id_" + id;

  //inicializo los datos
  for(int8_t h=0;h<HORAS_EN_DIA;h++) habitaciones[id].peso[h]=pesoSatelites[id][h];
  habitaciones[id].temperatura=NO_LEIDO; //NO_LEIDO=-100.0 significa que todavia no se ha leido y no debe tenerse en cuenta para promediar
  habitaciones[id].humedad=NO_LEIDO;
  habitaciones[id].luz=NO_LEIDO;
  habitaciones[id].presion=NO_LEIDO;
  habitaciones[id].altitud=NO_LEIDO;

  return id;
  }

/**********************************************************/
/*elimina un satelite del registro de satelites           */
/*devuelve el id del satelite                             */
/**********************************************************/
int8_t delSatelite(int8_t id)
  {
  Serial.printf("Desregistrando satelite %i\n",id);
  
  habitaciones[id].id=NO_REGISTRADO;
  habitaciones[id].lectura=0;
  habitaciones[id].nombre="";
  for(int8_t h=0;h<HORAS_EN_DIA;h++) habitaciones[id].peso[h]=0;
  habitaciones[id].temperatura=NO_LEIDO; //NO_LEIDO=-100.0 significa que todavia no se ha leido y no debe tenerse en cuenta para promediar
  habitaciones[id].humedad=NO_LEIDO;
  habitaciones[id].luz=NO_LEIDO;
  habitaciones[id].presion=NO_LEIDO;
  habitaciones[id].altitud=NO_LEIDO;
    
  return id;
  }
  
/***************************************************************/
/*Devuelve una cadena con el estado del satelite identificado  */
/*devuelve id del satelite=OK | -1=KO                          */
/***************************************************************/
int consultaSatelite(int8_t id)
  {
  if(id<0 || id>= MAX_SATELITES) return -1;

  if(sateliteRegistrado(id)) //if(habitaciones[id].id!=NO_REGISTRADO)
    {  
    Serial.printf("Satelite id: %i\n", id);
    Serial.print("Nombre: "); Serial.println(habitaciones[id].nombre);
    Serial.printf("Temperatura: %f\n", habitaciones[id].temperatura);
    Serial.printf("Humedad: %f\n", habitaciones[id].humedad);
    Serial.printf("Luz: %f\n", habitaciones[id].luz);
    Serial.printf("Presion: %f\n", habitaciones[id].presion);
    Serial.printf("Altitud: %f\n", habitaciones[id].altitud);

    Serial.print("peso:"); 
    for(int8_t h=0;h<HORAS_EN_DIA;h++) Serial.printf(" %i:%i | ",h,habitaciones[id].peso[h]);
    Serial.println();
    
    Serial.printf("Lectura: %i: tiempo: %i\n", habitaciones[id].lectura,millis()-habitaciones[id].lectura);
    }
   else Serial.printf("El satelite %i no se ha identificado.\n",id);

  return id; 
  }

/***************************************************************/
/*Registra la hora (milisegundos) en la que se recibio una     */
/*lectura en bus desde el satelite                             */
/***************************************************************/
void sateliteLeido(int8_t id)
  {
  habitaciones[id].lectura=millis();//apunto la hora de la lectura del mensaje en el bus
  }

/***************************************************************/
/*Devuelve la hora (milisegundos) en la que se recibio la      */
/*ultima lectura en bus desde el satelite                      */
/* -1 si no se ha leido nunca
/***************************************************************/
unsigned long sateliteUltimaLectura(int8_t id)
  {
  if(!sateliteRegistrado(id)) return -1;
  
  return habitaciones[id].lectura;
  }

/****************************************************************/
/* Devuelve el nombre de una habitacion                         */
/* comprueba si el id esta registrado                           */
/****************************************************************/
void sateliteTimeOut(unsigned long time_out)
  {
  unsigned long ahora=millis();
  
  for(int8_t id=0;id<MAX_SATELITES;id++)
    {      
    if(sateliteRegistrado(id) && (ahora-sateliteUltimaLectura(id)>time_out)) delSatelite(id);//se ha producido time-out y lo borro del registro
    }
  }
/***************************************************** Fin satelites ***************************************************************************/  
/***************************************************** Inicio get/set ***************************************************************************/
/****************************************************************/
/* Devuelve el nombre de una habitacion                         */
/* comprueba si el id esta registrado                           */
/****************************************************************/
String getNombre(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return "ERROR";
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return "ERROR";
  
  return habitaciones[id].nombre;
  }
  
/****************************************************************/
/* devuelve la temperatura de una habitacion                    */
/* comprueba si el id esta registrado                           */
/****************************************************************/
float getTemperatura(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  if(debug) Serial.printf("temperatura: %.1f",habitaciones[id].temperatura);
  
  return habitaciones[id].temperatura;
  }

/****************************************************************/
/* devuelve la humedad de una habitacion                        */
/* comprueba si el id esta registrado                           */
/****************************************************************/
float getHumedad(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  if(debug) Serial.printf("humedad: %.1f",habitaciones[id].humedad);
  
  return habitaciones[id].humedad;
  }

/****************************************************************/
/* devuelve la luz de una habitacion                            */
/* comprueba si el id esta registrado                           */
/****************************************************************/
float getLuz(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  if(debug) Serial.printf("luz: %.1f",habitaciones[id].luz);
  
  return habitaciones[id].luz;
  }

/****************************************************************/
/* devuelve la presion de una habitacion                            */
/* comprueba si el id esta registrado                           */
/****************************************************************/
float getPresion(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  if(debug) Serial.printf("presion: %.1f",habitaciones[id].presion);
  
  return habitaciones[id].presion;
  }

/****************************************************************/
/* devuelve la altitud de una habitacion                            */
/* comprueba si el id esta registrado                           */
/****************************************************************/
float getAltitud(int8_t id, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  if(debug) Serial.printf("altitud: %.1f",habitaciones[id].altitud);
  
  return habitaciones[id].altitud;
  }

/****************************************************************/
/* Devuelve el peso de una habitacion a la hora indicada        */
/* comprueba si el id esta registrado                           */
/****************************************************************/
uint8_t getPeso(int8_t id, int hora, int debug)
  {
  //if(id>=MAX_SATELITES || habitaciones[id].id==NO_REGISTRADO) return KO;
  if(id>=MAX_SATELITES || !sateliteRegistrado(id)) return KO;
  
  return habitaciones[id].peso[hora];
  }
/*******************************************************/
/*                                                     */
/* Promedia las medidas de temperatura de los          */
/* satelites segun el peso                             */ 
/*                                                     */
/*******************************************************/
float promediaTemperatura(void)
  {
  float promedio=0;
  float pesoTotal=0;
  int hora_actual=hora();
  
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    if(sateliteRegistrado(i) && habitaciones[i].temperatura!=NO_LEIDO)
      {
      promedio+=habitaciones[i].temperatura*habitaciones[i].peso[hora_actual];
      pesoTotal+=habitaciones[i].peso[hora_actual];
      }
    }

  if(pesoTotal!=0 && numeroSatelites(0)) return roundf((promedio/pesoTotal)*10)/10;
  
  //si llega aqui es que ha pasado algo raro
  Serial.printf("satelites registrados %i | peso total %i\n",numeroSatelites(false),pesoTotal);
  Serial.printf("************************\n");
  for (int8_t i=0;i<MAX_SATELITES;i++) if(sateliteRegistrado(i)) Serial.printf("Satelite %i, temp: %f, peso: %i\n",i,habitaciones[i].temperatura,habitaciones[i].peso[hora_actual]);
  Serial.printf("************************\n");  
  return 9999;
  }

/*******************************************************/
/*                                                     */
/* Promedia las medidas de humedad de los              */
/* satelites segun el peso                             */ 
/*                                                     */
/*******************************************************/
float promediaHumedad(void)
  {
  float promedio=0;
  float pesoTotal=0;
  int hora_actual=hora();
  
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    if(sateliteRegistrado(i) && habitaciones[i].humedad>0)
      {
      promedio+=habitaciones[i].humedad*habitaciones[i].peso[hora_actual];
      pesoTotal+=habitaciones[i].peso[hora_actual];
      }
    }

  if(pesoTotal!=0 && numeroSatelites(0)) return roundf((promedio/pesoTotal)*10)/10;
  
  //si llega aqui es que ha pasado algo raro
  Serial.printf("satelites registrados %i | peso total %i\n",numeroSatelites(false),pesoTotal);
  Serial.printf("************************\n");
  for (int8_t i=0;i<MAX_SATELITES;i++) if(sateliteRegistrado(i)) Serial.printf("Satelite %i, humedad: %f, peso: %i\n",i,habitaciones[i].humedad,habitaciones[i].peso[hora_actual]);
  Serial.printf("************************\n");  
  return NO_LEIDO;
  }

/*******************************************************/
/*                                                     */
/* Promedia las medidas de presion de los              */
/* satelites segun el peso                             */ 
/*                                                     */
/*******************************************************/
float promediaPresion(void)
  {
  float promedio=0;
  float pesoTotal=0;
  int hora_actual=hora();
  
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    if(sateliteRegistrado(i) && habitaciones[i].presion>0)
      {
      //promedio+=habitaciones[i].presion*habitaciones[i].peso[hora_actual];
      //pesoTotal+=habitaciones[i].peso[hora_actual];
      promedio+=habitaciones[i].presion;
      pesoTotal++;
      }
    }

  if(pesoTotal!=0 && numeroSatelites(0)) return roundf((promedio/pesoTotal)*10)/10;
  
  //si llega aqui es que ha pasado algo raro
  Serial.printf("satelites registrados %i | peso total %i\n",numeroSatelites(false),pesoTotal);
  Serial.printf("************************\n");
  for (int8_t i=0;i<MAX_SATELITES;i++) if(sateliteRegistrado(i)) Serial.printf("Satelite %i, presion: %f, peso: %i\n",i,habitaciones[i].presion,habitaciones[i].peso[hora_actual]);
  Serial.printf("************************\n");  
  return NO_LEIDO;
  }

/*******************************************************/
/*                                                     */
/* Promedia las medidas de altitud de los              */
/* satelites segun el peso                             */ 
/*                                                     */
/*******************************************************/
float promediaAltitud(void)
  {
  float promedio=0;
  float pesoTotal=0;
  int hora_actual=hora();
  
  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    if(sateliteRegistrado(i) && habitaciones[i].altitud>0)
      {
      //promedio+=habitaciones[i].altitud*habitaciones[i].peso[hora_actual];
      //pesoTotal+=habitaciones[i].peso[hora_actual];
      promedio+=habitaciones[i].altitud;
      pesoTotal++;
      }
    }

  if(pesoTotal!=0 && numeroSatelites(0)) return roundf((promedio/pesoTotal)*10)/10;
  
  //si llega aqui es que ha pasado algo raro
  Serial.printf("satelites registrados %i | peso total %i\n",numeroSatelites(false),pesoTotal);
  Serial.printf("************************\n");
  for (int8_t i=0;i<MAX_SATELITES;i++) if(sateliteRegistrado(i)) Serial.printf("Satelite %i, presion: %f, peso: %i\n",i,habitaciones[i].presion,habitaciones[i].peso[hora_actual]);
  Serial.printf("************************\n");  
  return NO_LEIDO;
  }

/******************************************/
/* Convierte grados Celsius en Farenheit  */
/* Temperatura en farenheit               */   
/*  (32 °C × 9 / 5) + 32 = 89,6 °F        */ 
/******************************************/
float tempCtoF(float tempC){ return ((tempC *9.0/5.0)+32.0);}
/******************************************/
/* Convierte grados Farenheit en Celsius  */
/* Temperatura en farenheit               */   
/*  (89.6 °F - 32)  × 5 / 9) = 32 °F      */ 
/******************************************/
float tempFtoC(float tempF){ return ((tempF - 32.0) *5.0/9.0);}
/******************************************/
/*  Temperatura de rocio en celsius       */   
/*  TR = TCelsius- ((100- R)/5.0)         */ 
/******************************************/
float getTemperaturaRocio(float temperatura, float humedad){ return (temperatura - ((100.0 - humedad)/5.0));} 
/***************************************************** Fin get/set ***************************************************************************/
   
/********************************************************/
/*recibe una cedena que responde el controlador y       */
/*la parsea buscando el dato solicitado                 */
/*la cadena es una secuencia de numeros separados por | */
/********************************************************/
float parsearDatosf(String cadena, int8_t dato, char separador)
  {
  int8_t posicion=0;
  int8_t posicion_anterior=0;
  String valor_cadena="";
  
  for(int8_t i=0;i<dato;i++) 
    {
    posicion_anterior=posicion;
    if(posicion_anterior>0) posicion_anterior++;//para saltar el separador si no es la primera
    posicion=cadena.indexOf(separador,posicion+1);  
    }
    
  valor_cadena=cadena.substring(posicion_anterior,posicion);

  return valor_cadena.toFloat();
  }
