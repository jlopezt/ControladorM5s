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
  int8_t peso;
  unsigned long lectura;
  float  temperatura;
  float  humedad;
  float  luz;
  }habitacion_t;

habitacion_t habitaciones[MAX_SATELITES];

String nombres[MAX_SATELITES]; //nombre por defecto de los satelites. Cuando se registra un satelite envia su nombre
int8_t pesoSatelites[MAX_SATELITES]; //peso en el calculo de la temperatura ponderada

//////////////////////////////////////////////////Funciones de configuracion de los satelites///////////////////////////////////////////////////////
/*inicializa los satelites, pongo todo a cero*/
void inicializaSatelites(void)
  {
  //Leo el nombre del fichero o lo inicializo por defecto. Nombres y pesos
  leeFicheroNombres();
      
  for(int8_t i=0;i<MAX_SATELITES; i++)
    {
    habitaciones[i].id=NO_REGISTRADO;
    habitaciones[i].nombre=nombres[i];
    habitaciones[i].peso=pesoSatelites[i];
    habitaciones[i].lectura=0;
    habitaciones[i].temperatura=NO_LEIDO;//0.0;
    habitaciones[i].humedad=0.0;
    habitaciones[i].luz=0.0;   
    }  
  }

void leeFicheroNombres(void)
  {
  String cad="";
  
  if(leeFichero(NOMBRES_CONFIG_FILE, cad)) parseaConfiguracionNombres(cad);
  else 
    {
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

    //Valor por defecto apra el peso de los satelites
    for(int8_t i=0;i<MAX_SATELITES;i++) pesoSatelites[i]=1;
    }
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
      pesoSatelites[i]=Termometros[i]["peso"];
      }
      
    Serial.println("Nombres:"); 
    for(int8_t i=0;i<MAX_SATELITES;i++) Serial.printf("%02i: %s; peso: %i\n",i,nombres[i].c_str(),pesoSatelites[i]); 
//************************************************************************************************
    return true;
    }
  return false;
  }

//////////////////////////////////////////////////Funciones de ejecucion de los satelites///////////////////////////////////////////////////////
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
/* devuelve la lista de habitaciones                            */
/*leyendo los que tienen el id!=0                               */
/****************************************************************/
String listaSatelites(int debug)
  {
  String cad="";
  
  for(int8_t id=0;id<MAX_SATELITES;id++) //id1#nombre1|id2#nombre2|....|id_n#nombre_n n<MAX_SATELITES
    {
    if(sateliteRegistrado(id)) //if(habitaciones[id].id!=NO_REGISTRADO)
      {   
        if (cad!="") cad += SEPARADOR;
        cad += id;
        cad += SUBSEPARADOR;
        cad += habitaciones[id].nombre;
      }  
    }
    
  return cad;
  }


/****************************************************************/
/* devuelve los valores medidos para una habitacion             */
/* comprueba si el id esta registrado                           */
/****************************************************************/
String valoresSatelite(int8_t id, int debug)
  {
  String cad="";
  
  if(id>=MAX_SATELITES) return cad;
  
  if(sateliteRegistrado(id)) //if(habitaciones[id].id!=NO_REGISTRADO)
    {   
      cad = habitaciones[id].temperatura;
      cad += SEPARADOR;       
      cad += habitaciones[id].humedad;
      cad += SEPARADOR;               
      cad += habitaciones[id].luz; 
    }  
  
  return cad;
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
  habitaciones[id].peso=pesoSatelites[id];
  habitaciones[id].temperatura=NO_LEIDO; //NO_LEIDO=-100.0 significa que todavia no se ha leido y no debe tenerse en cuenta para promediar
  habitaciones[id].humedad=0.0;
  habitaciones[id].luz=0.0;

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
  habitaciones[id].peso=0;
  habitaciones[id].temperatura=NO_LEIDO; //NO_LEIDO=-100.0 significa que todavia no se ha leido y no debe tenerse en cuenta para promediar
  habitaciones[id].humedad=0.0;
  habitaciones[id].luz=0.0;
    
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
    Serial.printf("Peso: %i\n", habitaciones[id].peso);
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
  
  return habitaciones[id].luz;
  }

/*******************************************************/
/*                                                     */
/* Promedia las medidas de los satelites segun el peso */ 
/*                                                     */
/*******************************************************/
float promediaTemperatura(void)
  {
  float promedio=0;
  float pesoTotal=0;

  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    if(sateliteRegistrado(i) && habitaciones[i].temperatura!=NO_LEIDO) //if(habitaciones[i].id!=NO_REGISTRADO)
      {
      promedio+=habitaciones[i].temperatura*habitaciones[i].peso;//promedio+=habitaciones[i].temperatura;
      pesoTotal+=habitaciones[i].peso; //pesoTotal+=pesoSatelite[i];
      }
    }

  if(pesoTotal!=0 && numeroSatelites(0)) return (promedio/pesoTotal);
  
  //si llega aqui es que ha pasado algo raro
  Serial.printf("satelites registrados %i | peso total %i\n",numeroSatelites(false),pesoTotal);
  Serial.printf("************************\n");
  for (int8_t i=0;i<MAX_SATELITES;i++) if(sateliteRegistrado(i)) Serial.printf("Satelite %i, temp: %f, peso: %i\n",i,habitaciones[i].temperatura,habitaciones[i].peso);
  Serial.printf("************************\n");  
  return 9999;
  }

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
