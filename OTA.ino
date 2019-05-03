/*********************************************************************
Funciones d egestion de la actualizacion OTA

Manual: http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html
Libreria: https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

Funcines que provee al libreria:  
  void setPort(uint16_t port);//Sets the service port. Default 8266

  void setHostname(const char *hostname);  //Sets the device hostname. Default esp8266-xxxxxx
  String getHostname();
  
  void setPassword(const char *password);  //Sets the password that will be required for OTA. Default NULL
  void setPasswordHash(const char *password);//Sets the password as above but in the form MD5(password). Default NULL
  void setRebootOnSuccess(bool reboot);  //Sets if the device should be rebooted after successful update. Default true
  void onStart(THandlerFunction fn);  //This callback will be called when OTA connection has begun
  void onEnd(THandlerFunction fn);  //This callback will be called when OTA has finished
  void onError(THandlerFunction_Error fn);  //This callback will be called when OTA encountered Error
  void onProgress(THandlerFunction_Progress fn);  //This callback will be called when OTA is receiving data
  void begin();  //Starts the ArduinoOTA service
  void handle();  //Call this in loop() to run the service
  int getCommand();  //Gets update command type after OTA has started. Either U_FLASH or U_SPIFFS
**********************************************************************/
boolean flagOTA=false; //flag de si hay una actualizacion en curso

boolean inicializaOTA(boolean debug)
  {    
  flagOTA=false; //flag de si hay una actualizacion en curso
  
  ArduinoOTA.setPassword((const char *)"88716");// No authentication by default

  //Configuramos las funciones CallBack
  ArduinoOTA.onStart(inicioOTA);
  ArduinoOTA.onEnd(finOTA);
  ArduinoOTA.onProgress(progresoOTA);
  ArduinoOTA.onError(errorOTA);
  
  //iniciamos la gestion OTA
  ArduinoOTA.begin();
  }

void inicioOTA(void)
  {
  flagOTA=true;  
  Serial.println("Actualizacion OTA");
  borraPantalla();
  escribePantalla(10,50,"Actualizacion OTA",12);
  }
  
void finOTA(void)
  {
  flagOTA=false;  
  Serial.println("Fin actualizacion");
  escribePantalla(10,200,"Fin OK",12);
  }

void progresoOTA(unsigned int progress, unsigned int total)
  {
  String cad="";
  float avance=100*(float)progress/total;

  Serial.printf("actualizando: %5.1f %  :  ",avance);

  //int escribePantalla(int columna, int fila, String texto)
  cad = "progreso: ";
  cad += String(avance,1);
  cad += "%";
  escribePantalla(10,100,cad,12);
  }

void errorOTA(ota_error_t error)
  {
  Serial.printf("Error en actualizacion OTA ");    Serial.printf("Error[%u]: ", error);
  
  flagOTA=false; //Para liberar los bucles principales y el watchdog
  
  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR) Serial.println("End Failed");
  }

boolean actualizacionOTAEnCurso(void) {return flagOTA;}
