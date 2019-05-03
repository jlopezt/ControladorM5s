/*
 * ordenes.ino
 *
 * Comandos para la actualizacion de la hora del reloj
 *
 * Permite la puesta en hora del reloj a traves de comandos enviados por el puesto
 * serie desde el PC.
 *
 * Para actualizar la hora <comado> <valor>; Ejemplo: "hora 3;"
 * Se pueden anidar: "hora 2;minuto 33;"
 *
 */

#define LONG_COMANDO   40
#define LONG_PARAMETRO 30
#define LONG_ORDEN     22 //Comando (espacio) Parametros (fin de cadena)
#define MAX_COMANDOS   45

#include <Time.h>

char ordenRecibida[LONG_ORDEN]="";
int lonOrden=0;

typedef struct 
  {
  String comando;
  String descripcion;
  void (*p_func_comando) (int, char*, float)=NULL;
  }tipo_comando;
tipo_comando comandos[MAX_COMANDOS];

int HayOrdenes(int debug)
  {
  char inChar=0;
  
  while (Serial.available())
    {
    inChar=(char)Serial.read();    
    switch (inChar)
      {
      case ';':
        //Recibido final de orden
        if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
        return(1);
        break;
      default:
        //Nuevo caracter recibido. Añado y sigo esperando
        ordenRecibida[lonOrden++]=inChar;
        ordenRecibida[lonOrden]=0;
        break;
      }
    }    
  return(0); //No ha llegado el final de orden
  }

int EjecutaOrdenes(int debug){
  String comando="";
  String parametros="";
  int iParametro=0;
  char sParametro[LONG_PARAMETRO]="";//LONG_PARAMETRO longitud maxmima del parametro
  float fParametro=0;
  int inicioParametro=0;

  if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
  
  for(int i=0;i<LONG_COMANDO;i++)
    {
    switch (ordenRecibida[i])
      {
      case ' ':
        //fin del comando, hay parametro
        inicioParametro=i+1;
        
        //Leo el parametro
        for (int j=0;j<LONG_ORDEN;j++)
          {  //Parsea la orden      
          if(ordenRecibida[j+inicioParametro]==0) 
            {
            strncpy(sParametro,ordenRecibida+inicioParametro,j+1);//copio el parametro como texto
            break;//j=LONG_ORDEN;//Cuando encuentro el final de la cadena
            }
          else iParametro=(iParametro*10)+(int)ordenRecibida[j+inicioParametro]-48; //hay que convertir de ASCII a decimal
          }
        fParametro=String(sParametro).toFloat();
        
        i=LONG_COMANDO;
        break;
      case 0:
        //fin de la orden. No hay parametro
        i=LONG_COMANDO;
        break;
      default:
        comando+=ordenRecibida[i];
        break;
      }
    }

  //Limpia las variables que3 recogen la orden
  lonOrden=0;
  ordenRecibida[0]=0;

  if (debug) Serial.printf("comando: %s\niParametro: %i\nsParametro: %s\nfParametro: %f\n",comando.c_str(),iParametro,sParametro,fParametro);
    
/**************Nueva funcion ***************************/
  int8_t indice=0;
  for(indice=0;indice<MAX_COMANDOS;indice++)
    {
    if (debug) Serial.printf("Comando[%i]: {%s} - {%s}\n",indice,comando.c_str(),comandos[indice].comando.c_str());

    if (comandos[indice].comando==comando) 
      {
      //Ejecuta la funcion asociada
      comandos[indice].p_func_comando(iParametro, sParametro, fParametro);
      return(0);
      }    
    }

  //Si llega aqui es que no ha encontrado el comando
  Serial.println("Comando no encontrado");
  return(-1);//Comando no encontrado  
/*******************************************************/
}

void limpiaOrden(void)
  {
  lonOrden=0;
  ordenRecibida[0]=0;
  }
  
void inicializaOrden(void)
  { 
  int i =0;  

  limpiaOrden(); 
  //Comando 1
  comandos[i].comando="help";
  comandos[i].descripcion="Listado de comandos";
  comandos[i++].p_func_comando=func_comando_help;
  //Comando 2    
  comandos[i].comando="IP";
  comandos[i].descripcion="Devuelve la direccion IP del equipo";
  comandos[i++].p_func_comando=func_comando_IP;
  //Comando 3  
  comandos[i].comando="errores";
  comandos[i].descripcion="Devuelve la descripcion de todos los errores";
  comandos[i++].p_func_comando=func_comando_errores;
  //Comando 4  
  comandos[i].comando="restart";
  comandos[i].descripcion="Reinicia el modulo";
  comandos[i++].p_func_comando=func_comando_restart;
  //Comando 5  
  comandos[i].comando="info";
  comandos[i].descripcion="Devuelve la informacion del hardware del equipo";
  comandos[i++].p_func_comando=func_comando_info;
  //Comando 6  
  comandos[i].comando="fexist";
  comandos[i].descripcion="Comprueba la existencia del fichero indicado";
  comandos[i++].p_func_comando=func_comando_fexist;
  //Comando 7
  comandos[i].comando="finfo";
  comandos[i].descripcion="Devuelve informacion sobre el sistema de ficheros del equipo";
  comandos[i++].p_func_comando=func_comando_finfo;
  //Comando 8
  comandos[i].comando="fopen";
  comandos[i].descripcion="Devuelve el contenido del fichero indicado";
  comandos[i++].p_func_comando=func_comando_fopen;
  //Comando 9
  comandos[i].comando="fremove";
  comandos[i].descripcion="Borra el fichero indicado";
  comandos[i++].p_func_comando=func_comando_fremove;
  //Comando 10
  comandos[i].comando="format";
  comandos[i].descripcion="Formatea el sistema de ficheros del equipo";
  comandos[i++].p_func_comando=func_comando_format;
  //Comando 11
  comandos[i].comando="sLuz";
  comandos[i].descripcion="Dewvuelve el umbral de lus configurado";
  comandos[i++].p_func_comando=func_comando_sLuz;
  //Comando 12
  comandos[i].comando="reloj";
  comandos[i].descripcion="Devuelve la hora actual";
  comandos[i++].p_func_comando=func_comando_reloj;
  //Comando 13
  comandos[i].comando="PM";
  comandos[i].descripcion="Devuelve si la hora actual es AM o PM";
  comandos[i++].p_func_comando=func_comando_PM;
  //Comando 14
  comandos[i].comando="gDow";
  comandos[i].descripcion="Devuelve el dia de la semana";
  comandos[i++].p_func_comando=func_comando_gDow;
  //Comando 15
  comandos[i].comando="numSat";
  comandos[i].descripcion="Devuelve el numero de satelites registrados";
  comandos[i++].p_func_comando=func_comando_numSat;
  //Comando 16
  comandos[i].comando="leeIP";
  comandos[i].descripcion="Devuleve la lista de IPs del sistema configuradas en el equipo";
  comandos[i++].p_func_comando=func_comando_leeIP;
  //Comando 17
  comandos[i].comando="consigna";
  comandos[i].descripcion="Devuleve la consigna de la hora actual";
  comandos[i++].p_func_comando=func_comando_consigna;
  //Comando 18
  comandos[i].comando="consignas";
  comandos[i].descripcion="Devuelve las consignas configuradas";
  comandos[i++].p_func_comando=func_comando_consignas;
  //Comando 19
  comandos[i].comando="mapa";
  comandos[i].descripcion="Devuelve el mapa de consignas configuradas";
  comandos[i++].p_func_comando=func_comando_mapa;
  //Comando 20
  comandos[i].comando="setConD";
  comandos[i].descripcion="Configura la consigna de dia";
  comandos[i++].p_func_comando=func_comando_setConD;
  //Comando 21
  comandos[i].comando="setConN";
  comandos[i].descripcion="Configura la consigna de noche";
  comandos[i++].p_func_comando=func_comando_setConN;
  //Comando 22
  comandos[i].comando="temPro";
  comandos[i].descripcion="Devuelve la temperatura promedio";
  comandos[i++].p_func_comando=func_comando_temPro;
  //Comando 23
  comandos[i].comando="staRele";
  comandos[i].descripcion="Devuleve el estado de los reles";
  comandos[i++].p_func_comando=func_comando_staRele;
  //Comando 24
  comandos[i].comando="actRele";
  comandos[i].descripcion="Activa el rele indicado";
  comandos[i++].p_func_comando=func_comando_actRele;
  //Comando 25
  comandos[i].comando="desactRele";
  comandos[i].descripcion="Desactiva el rele indicado";
  comandos[i++].p_func_comando=func_comando_desactRele;
  //Comando 26
  comandos[i].comando="qSat";
  comandos[i].descripcion="Consulta el satelite indicado";
  comandos[i++].p_func_comando=func_comando_qSat;
  //Comando 27
  comandos[i].comando="hora";
  comandos[i].descripcion="Devuelve la hora de la hora actual del sistema";
  comandos[i++].p_func_comando=func_comando_hora;
  //Comando 28
  comandos[i].comando="minuto";
  comandos[i].descripcion="Devuelve los minutos de la hora actual del sistema";
  comandos[i++].p_func_comando=func_comando_minuto;
  //Comando 29
  comandos[i].comando="segundo";
  comandos[i].descripcion="Devuelve los segundos de la hora actual del sistema";
  comandos[i++].p_func_comando=func_comando_segundo;
  //Comando 30
  comandos[i].comando="echo";
  comandos[i].descripcion="Confirma el echo del comando";
  comandos[i++].p_func_comando=func_comando_echo;
  //Comando 31
  comandos[i].comando="debug";
  comandos[i].descripcion="Activa/desactiva la traza extendida";
  comandos[i++].p_func_comando=func_comando_debug;
  //Comando 32
  comandos[i].comando="mqtt";
  comandos[i].descripcion="Info sobre MQTT";
  comandos[i++].p_func_comando=func_comando_mqtt; 
  //Comando 33
  comandos[i].comando="tipoPantalla";
  comandos[i].descripcion="Selecciona el modo de pantalla (0=normal|1=consigna|2=reposo)";
  comandos[i++].p_func_comando=func_comando_tipoPantalla;  
  //Comando 34
  comandos[i].comando="brilloPantalla";
  comandos[i].descripcion="Selecciona el brillo de la pantalla";
  comandos[i++].p_func_comando=func_comando_brilloPantalla;  
  //Comando 35
  comandos[i].comando="sleep";
  comandos[i].descripcion="Selecciona el numero de milisegundos para ir a sleep";
  comandos[i++].p_func_comando=func_comando_limiteSleep;  
  //Comando 36
  comandos[i].comando="uptime";
  comandos[i].descripcion="Tiempo desde el ultimo reinicio";
  comandos[i++].p_func_comando=func_comando_uptime;  
  //Comando 37
  comandos[i].comando="json";
  comandos[i].descripcion="Activar IF JSON";
  comandos[i++].p_func_comando=func_comando_if_json;  
  //Comando 38
  comandos[i].comando="lisHab";
  comandos[i].descripcion="Listado de habitaciones";
  comandos[i++].p_func_comando=func_comando_habitaciones;  
  //Comando 39
  comandos[i].comando="vueltas";
  comandos[i].descripcion="valores de vueltas";
  comandos[i++].p_func_comando=func_comando_vueltas; 
  //Comando 40
  comandos[i].comando="pesos";
  comandos[i].descripcion="Peso de los satelites";
  comandos[i++].p_func_comando=func_comando_pesos; 
  //Comando 41
  comandos[i].comando="modo";
  comandos[i].descripcion="Modo Calefaccion(OFF|ON|AUTO)";
  comandos[i++].p_func_comando=func_comando_modo; 
  //resto
  for(;i<MAX_COMANDOS;i++) 
    {
    comandos[i].comando="vacio";
    comandos[i].descripcion="Comando vacio";
    comandos[i].p_func_comando=func_comando_vacio;  
    }

  func_comando_help(0,"",0.0);  
  //for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!="vacio") Serial.printf("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  }


/*********************************************************************/
/*  Funciones para los comandos                                      */
/*  void (*p_func_comando) (int, char*, float)                       */
/*********************************************************************/  
void func_comando_vacio(int iParametro, char* sParametro, float fParametro) //"vacio"
{}

void func_comando_help(int iParametro, char* sParametro, float fParametro) //"help"
  {
  Serial.printf("\n\nComandos:");  
  for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!="vacio") Serial.printf("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  //for (int8_t indice=0;indice<MAX_COMANDOS;indice++) Serial.printf("\n%s : %s",comandos[indice].comando.c_str(),comandos[indice].descripcion.c_str());
  Serial.printf("\n------------------------------------------------------------------------------\n");
  }

void func_comando_IP(int iParametro, char* sParametro, float fParametro) //"IP"
  {
  boolean debug=false;
  Serial.println(getIP(debug));  
  }  

void func_comando_errores(int iParametro, char* sParametro, float fParametro)//"errores")
  {
  Serial.printf("Errores: %s\n",getTodosErrores().c_str());
  }

void func_comando_restart(int iParametro, char* sParametro, float fParametro)//"restart")
  {
  ESP.restart();
  }

void func_comando_info(int iParametro, char* sParametro, float fParametro)//"info")
  {
  Serial.printf("\n----------------------------------------------\n");
  Serial.printf("Version: %s", VERSION);
  Serial.printf("\n-----------------Hardware info-----------------\n");
  //Serial.printf("Vcc: %i\n",ESP.getVcc());
  Serial.printf("FreeHeap: %i\n",ESP.getFreeHeap());
  Serial.printf("ChipId: %i\n",ESP.getChipRevision());
  Serial.printf("SdkVersion: %s\n",ESP.getSdkVersion());
  //Serial.printf("CoreVersion: %s\n",ESP.getCoreVersion().c_str());
  //Serial.printf("FullVersion: %s\n",ESP.getFullVersion().c_str());
  //Serial.printf("BootVersion: %i\n",ESP.getBootVersion());
  //Serial.printf("BootMode: %i\n",ESP.getBootMode());
  Serial.printf("CpuFreqMHz: %i\n",ESP.getCpuFreqMHz());

  //Serial.printf("FlashChipId: %i\n",ESP.getFlashChipId());
      //gets the actual chip size based on the flash id
  //Serial.printf("FlashChipRealSize: %i\n",ESP.getFlashChipRealSize());

      //gets the size of the flash as set by the compiler
  Serial.printf("FlashChipSize: %i\n",ESP.getFlashChipSize());
  Serial.printf("FlashChipSpeed: %i\n",ESP.getFlashChipSpeed());
      //FlashMode_t ESP.getFlashChipMode());
      
  //Serial.printf("FlashChipSizeByChipId: %i\n",ESP.getFlashChipSizeByChipId());  
  Serial.printf("-----------------------------------------------\n");

  String cad;
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
  Serial.println(cad);
  }  

void func_comando_fexist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    if(SPIFFS.exists(sParametro)) Serial.printf("El fichero %s existe.\n",sParametro);
    else Serial.printf("NO existe el fichero %s.\n",sParametro);
    }
  }

void func_comando_finfo(int iParametro, char* sParametro, float fParametro)//"finfo")
  {
  Serial.printf("totalBytes: %i\nusedBytes: %i\n",SPIFFS.totalBytes(), SPIFFS.usedBytes());
  }

/*NO CONTROLADO EL DESBORDE DEL BUFFER DE sParametro*/
void func_comando_fcreate(int iParametro, char* sParametro, float fParametro)//"fcreate")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    { 
    String p=String(sParametro); 
    int i =p.indexOf(',');
    if (i!=-1)
      {
      String nombreFichero=p.substring(0,i);
      String contenidoFichero=p.substring(i+1,p.length());
      Serial.printf("Nombre: %s\nContenido: %s\n",nombreFichero.c_str(),contenidoFichero.c_str());
    
      if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) Serial.printf("Fichero salvado con exito.\n");
      }
    else Serial.printf("No se pudo salvar el fichero\nParametro: %s\n",p.c_str()); 
    }
  }

void func_comando_fopen(int iParametro, char* sParametro, float fParametro)//"fopen")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    File f = SPIFFS.open(sParametro, "r");
    if (f)
      { 
      Serial.println("Fichero abierto");
      size_t tamano_fichero=f.size();
      Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
      char buff[tamano_fichero+1];
      f.readBytes(buff,tamano_fichero);
      buff[tamano_fichero+1]=0;
      Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
      f.close();
      }
    else Serial.printf("Error al abrir el fichero %s.\n", sParametro);
    } 
  } 

void func_comando_fremove(int iParametro, char* sParametro, float fParametro)//"fremove")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    { 
    if (SPIFFS.remove(sParametro)) Serial.printf("Fichero %s borrado\n",sParametro);
    else Serial.printf("Error al borrar el fichero%s\n",sParametro);
    } 
 }

void func_comando_format(int iParametro, char* sParametro, float fParametro)//"format")
  {     
  if (SPIFFS.format()) Serial.println("Sistema de ficheros formateado");
  else Serial.println("Error al formatear el sistema de ficheros");
  } 

void func_comando_sLuz(int iParametro, char* sParametro, float fParametro)//"sLuz") 
  {
  Serial.print("El umbral de hayLuz es : ");
  Serial.println(setUmbralLuz(iParametro));
  }

void func_comando_reloj(int iParametro, char* sParametro, float fParametro)//"reloj") 
  {
  imprimeDatosReloj();  
  if(cambioHorario()==1) Serial.println("Horario de verano");
  else Serial.println("Horario de invierno");
  }

void func_comando_PM(int iParametro, char* sParametro, float fParametro)//"PM") 
  {
  if(relojPM()) Serial.println("Es por la tarde");
  else Serial.println("Es por la mañana");  
  }

void func_comando_gDow(int iParametro, char* sParametro, float fParametro)//"gDow") 
  {
  Serial.printf("El dia de la seman es %i\n",diaSemana());
  }

void func_comando_numSat(int iParametro, char* sParametro, float fParametro)//"numSat") 
  {
  boolean debug=false;  
  Serial.println(numeroSatelites(debug));  
  }

void func_comando_leeIP(int iParametro, char* sParametro, float fParametro)//"leeIP") 
  {
  Serial.println(leerIPs());
  }

void func_comando_consigna(int iParametro, char* sParametro, float fParametro)//"consigna") 
  {
  Serial.printf("La temperatura consigna es %04.1f\n",getConsigna());  
  }

void func_comando_consignas(int iParametro, char* sParametro, float fParametro)//"consignas") 
  {
  Serial.printf("La temperatura consigna de dia es %04.1f\nLa temperatura consigna de noche es %04.1f\n",getConsignaDia(),getConsignaNoche());  
  }

void func_comando_mapa(int iParametro, char* sParametro, float fParametro)//"mapa") 
  {
  Serial.println("***********MAPA***********");  
  for(int8_t i=0;i<48;i++)
    {
    Serial.printf("mapa[%i]= %i\n",i,mapa[i]);
    }
  Serial.println("***********MAPA***********");  
  }

void func_comando_setConD(int iParametro, char* sParametro, float fParametro)//"setConD") 
  {
  Serial.printf("Consigna de dia configurada a %04.1f\n",setConsignaD(fParametro));  
  }

void func_comando_setConN(int iParametro, char* sParametro, float fParametro)//"setConN") 
  {
  Serial.printf("Consigna de noche configurada a %04.1f\n",setConsignaN(fParametro));    
  }

void func_comando_temPro(int iParametro, char* sParametro, float fParametro)//"temPro") 
  {
  Serial.println(getTemperaturaPromedio());
  }

void func_comando_staRele(int iParametro, char* sParametro, float fParametro)//"staRele") 
  {
  Serial.println(leeEstadoReles());
  }

void func_comando_actRele(int iParametro, char* sParametro, float fParametro)//"actRele") 
  {
  if(controlaRele(iParametro,1)==OK) Serial.println("OK");    
  else Serial.println("KO");
  }

void func_comando_desactRele(int iParametro, char* sParametro, float fParametro)//"desactRele") 
  {
  if(controlaRele(iParametro,0)==OK) Serial.println("OK");    
  else Serial.println("KO");
  }

void func_comando_qSat(int iParametro, char* sParametro, float fParametro)//"qSat") 
  {
  Serial.println(consultaSatelite(iParametro));  
  }

void func_comando_hora(int iParametro, char* sParametro, float fParametro)//"hora") 
  {
  if (sParametro=="") Serial.println("Es necesario indicar la hora a configurar");
  else
    {        
    //Tomo la hora actual
    time_t ahora=now();
    int hora=hour(ahora); 
    int minuto=minute(ahora);
    int segundo=second(ahora);
    int dia=day(ahora);
    int mes=month(ahora);
    int anno=year(ahora);    

    hora =iParametro;
    setTime(hora, minuto,segundo,dia, mes, anno); //Aplico la nueva hora
    }
  }

void func_comando_minuto(int iParametro, char* sParametro, float fParametro)//"minuto")
  {
  if (sParametro=="") Serial.println("Es necesario indicar los minutos a configurar");
  else
    {
    //Tomo la hora actual
    time_t ahora=now();
    int hora=hour(ahora); 
    int minuto=minute(ahora);
    int segundo=second(ahora);
    int dia=day(ahora);
    int mes=month(ahora);
    int anno=year(ahora);    

    minuto=iParametro;
    setTime(hora, minuto,segundo,dia, mes, anno); //Aplico la nueva hora
    }
  }

void func_comando_segundo(int iParametro, char* sParametro, float fParametro)//"segundo")
  {
  if (sParametro=="") Serial.println("Es necesario indicar los segundos a configurar");
  else
    {
    //Tomo la hora actual
    time_t ahora=now();
    int hora=hour(ahora); 
    int minuto=minute(ahora);
    int segundo=second(ahora);
    int dia=day(ahora);
    int mes=month(ahora);
    int anno=year(ahora);    

    segundo=iParametro;    
    setTime(hora, minuto,segundo,dia, mes, anno); //Aplico la nueva hora
    }
  }

void func_comando_echo(int iParametro, char* sParametro, float fParametro)//"echo") 
  {
  Serial.println(sParametro);
  }

void func_comando_debug(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  ++debugGlobal=debugGlobal % 2;
  if (debugGlobal) Serial.println("debugGlobal esta on");
  else Serial.println("debugGlobal esta off");
  }

void func_comando_tipoPantalla(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  //Serial.printf("sParametro = [%s]\n",sParametro);  
  if (sParametro[0]!=0) //if (sParametro!="")
    {
    setValorPrincipal(iParametro);
    //borraPantalla();
    //pintaTitulo();
    }
  Serial.printf("tipo pantalla = %i\n",getValorPrincipal());
  }

void func_comando_brilloPantalla(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  if (sParametro[0]!=0) setBrilloPantalla(iParametro);  
  Serial.printf("Brillo pantalla = %i\n",brilloPantalla);
  }
  
void func_comando_limiteSleep(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  if (sParametro[0]!=0) limiteSleep=iParametro;  
  Serial.printf("Limite para ir a pantalla de sleep = %i\n",limiteSleep);
  }

void func_comando_uptime(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  Serial.printf("Uptime: %i ms\n",uptime());
  }

void func_comando_if_json(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  if(iParametro==0) IF_JSON=false;
  else IF_JSON=true;

  if(IF_JSON)Serial.println("El interfaz JSON esta ON");
  else Serial.println("El interfaz JSON esta OFF");
  }

void func_comando_habitaciones(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  for(int i=0;i<MAX_SATELITES;i++)  Serial.printf("Satelite %i: id: %i:temperatura:%f\n",i,habitaciones[i].id,habitaciones[i].temperatura);
  }

void func_comando_vueltas(int iParametro, char* sParametro, float fParametro)//tipoPantalla
  {
  Serial.printf("vueltas loop %i\n",vuelta);
  }

void func_comando_pesos(int iParametro, char* sParametro, float fParametro) 
  {
  for(int8_t i=0;i<MAX_SATELITES;i++) Serial.printf("%02i: %s; id: %i; peso: %i\n",i,nombres[i].c_str(),habitaciones[i].id,pesoSatelites[i]); 
  }

void func_comando_mqtt(int iParametro, char* sParametro, float fParametro)   
  {
  Serial.printf("Info MQTT:\nIPBroker: %s\nPuerto del broker: %i\nID MQTT: %s\nUsuario MQTT: %s\nPassword: %s\nTopic root: %s\n",IPBroker.toString().c_str(),puertoBroker,ID_MQTT.c_str(),usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str());
  }

void func_comando_modo(int iParametro, char* sParametro, float fParametro) //"modo"
  {
  if(iParametro>=0) setModoManual(iParametro);
  Serial.printf("Modo calefaccion: %s\n",getModoManualTxt().c_str());
  }
