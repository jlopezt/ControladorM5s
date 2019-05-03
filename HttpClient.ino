/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <HTTPClient.h> //#include <ESP8266HTTPClient.h>

struct tipo_respuestaHTTP 
  {
  String payload="";
  long httpCode=-1;
  };

struct tipo_respuestaHTTP ClienteHTTP(String URL) 
  {  
  struct tipo_respuestaHTTP respuestaHTTP;
  HTTPClient http;

  if(URL=="") return respuestaHTTP;//Si esta vacio salgo rapido

  http.begin(URL);//HTTP
  http.setTimeout(5000);//Milisegundos!!!//2000 aprox 18sg. 300 aprox 2,6sg. Tiempo medio de ok< ,8 sg

  // start connection and send HTTP header
  respuestaHTTP.httpCode= http.GET();

  if(respuestaHTTP.httpCode > 0) //httpCode will be negative on error
    {
    // HTTP header has been send and Server response header has been handled
    if(getErrorNivel(ERROR_COM_SATELITES)) eliminaError(ERROR_COM_SATELITES);
    if(respuestaHTTP.httpCode == HTTP_CODE_OK) 
      {

      respuestaHTTP.payload=http.getString(); //return http.getString();
      }
    } 
  else if (debugGlobal) Serial.printf("Error comunicacion. \n Peticion: %s\n error: %i : %s\n", URL.c_str(), respuestaHTTP.httpCode, http.errorToString(respuestaHTTP.httpCode).c_str());
    

  http.end();

  return respuestaHTTP;
  }
