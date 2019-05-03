/***************************************************************/
/*   Gestion de errores solo los almacena y prioriza           */
/*   Niveles de error:                                         */
/*    -Interno: Wifi(0), SPIFFS(1), SNTP(2)...(4)              */
/*    -Comunicaiones: Satelites(6), Reles(7)...(9)             */
/***************************************************************/

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

typedef struct 
  {
  boolean estado;
  String mensaje;
  }t_error;

t_error error[MAX_ERROR];  //El indice es el nivel de error. Creo el contenedor vacio

/***************************************************************/
/*   Devuelve el mensaje del error mas critico del sistema     */
/***************************************************************/
void inicializaErrores(void)
  {
  for(int i=0;i<MAX_ERROR;i++) error[i].mensaje="";
  }  

/***************************************************************/
/*   Devuelve el mensaje del error mas critico del sistema     */
/***************************************************************/
String getMaxMensajeError()
  {
  for(int i=0;i<MAX_ERROR;i++) 
    {
    if(error[i].estado) return error[i].mensaje;
    }
  return "";
  }  

/***************************************************************/
/*   Devuelve el nivel del error mas critico del sistema       */
/***************************************************************/
int getMaxNivelError()
  {
  for(int i=0;i<MAX_ERROR;i++) if(error[i].estado) return i;   
  return NO_ERROR;
  }  

/***************************************************************/
/*   Devuelve si hay error de ese nivel en el sistema          */
/***************************************************************/
boolean getErrorNivel(int nivel)
  {    
  return error[nivel].estado;  
  /*
  if(error[nivel].estado) return true;   
  return false;
  */
  }  

/***************************************************************/
/*   Almacena un mansaje de error                              */
/***************************************************************/
void setError(int nivel, String mensaje)
  {
  error[nivel].estado=true;
  error[nivel].mensaje=mensaje;
  }
  
/***************************************************************/
/*   Elimina un mansaje de error                               */
/***************************************************************/
void eliminaError(int nivel)
  {
  error[nivel].estado=false;
  error[nivel].mensaje="";
  }

/***************************************************************/
/*   Muestra los errores de nivel igual o superior al idicado  */
/***************************************************************/
String getErroresNivel(int nivel)
  {
  String salida="";
  
  for(int i=nivel;i<MAX_ERROR;i++) 
    {
    if(error[i].estado) 
      {
      salida+="\n";
      salida+=error[i].mensaje;
      }
    }

  return salida;   
  }

/***************************************************************/
/*   Muestra todos los errores                                 */
/***************************************************************/
String getTodosErrores(void) {return getErroresNivel(0);}


