//#include <M5Stack.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "VanillaExtractRegular40pt7b.h"
//#include <Orbitron_Light_32.h>
#define CF_OL32 &Orbitron_Light_32
/*
#define FM9 &FreeMono9pt7b
#define FM12 &FreeMono12pt7b
#define FM18 &FreeMono18pt7b
#define FM24 &FreeMono24pt7b

#define FMB9 &FreeMonoBold9pt7b
#define FMB12 &FreeMonoBold12pt7b
#define FMB18 &FreeMonoBold18pt7b
#define FMB24 &FreeMonoBold24pt7b

#define FMO9 &FreeMonoOblique9pt7b
#define FMO12 &FreeMonoOblique12pt7b
#define FMO18 &FreeMonoOblique18pt7b
#define FMO24 &FreeMonoOblique24pt7b

#define FMBO9 &FreeMonoBoldOblique9pt7b
#define FMBO12 &FreeMonoBoldOblique12pt7b
#define FMBO18 &FreeMonoBoldOblique18pt7b
#define FMBO24 &FreeMonoBoldOblique24pt7b

#define FF17 &FreeSans9pt7b
#define FF18 &FreeSans12pt7b
#define FF19 &FreeSans18pt7b
#define FF20 &FreeSans24pt7b

#define FF21 &FreeSansBold9pt7b
#define FF22 &FreeSansBold12pt7b
#define FF23 &FreeSansBold18pt7b
#define FF24 &FreeSansBold24pt7b
*/

#define SEPARADOR_VERTICAL 5
#define MARGEN_IZQUIERDO 4
#define MARGEN_SUPERIOR 4
#define ALTO_TITULO 35
#define ALTO_PRINCIPAL 60
#define ALTO_SECUNDARIO 90
#define ALTO_MENU 22
#define ANCHO_CERO 0
#define ALTO_CERO 0
#define ANCHO_TOTAL 320
#define ALTO_TOTAL 240
#define ALTO_LETRA_9 15
#define ALTO_LETRA_12 20
#define MAX_LEN_ALARMA 25

#define MAX_LAYOUTS 5
int sateliteEnPantalla=0;
int valorPrincipal;
void (*ptr_func_pinta[MAX_LAYOUTS])(String, int, String );

void pintaLayout(String valor, int satelite, String alarma);  //Informacion de la Tª media y los satelites uno a uno
void pintaLayout2(String valor, int satelite, String alarma); //Informacion de la Tª media y la consigna
void pintaLayout3(String valor, int satelite, String alarma); //Modo reposo
void pintaLayout4(String valor, int satelite, String alarma); //Informacion de la base y la pontencia
void pintaLayout5(String valor, int satelite, String alarma); //Informacion de la Tª media y el modo de calefaccion

/*********************************************/
/* Inicializa el modo de gestion de la       */
/* pantalla, layourts y estados             */
/*********************************************/
void inicializaPantalla(void)
  {
  //Cargo los controlados de pantalla
  ptr_func_pinta[TEMPERATURA]=pintaLayout;
  ptr_func_pinta[CONSIGNA]=pintaLayout2;
  ptr_func_pinta[REPOSO]=pintaLayout3;
  ptr_func_pinta[INFO]=pintaLayout4;
  ptr_func_pinta[MANUAL]=pintaLayout5;

  //Inicializo la pantalla
  setBrilloPantalla(brilloPantalla);//M5.Lcd.setBrightness(brilloPantalla);  
  borraPantalla();

  //Inicializo el satelite a mostrar
  sateliteEnPantalla=0;
  //Selecciono la pantalla de inicio
  setValorPrincipal(CONSIGNA);//REPOSO;//TEMPERATURA;
  Serial.println("Fin init pantalla");
  }

/*******************************************************/
/*           Funciones auxiliares                      */
/*                                                     */
/*******************************************************/
void setBrilloPantalla(void) {M5.Lcd.setBrightness(brilloPantalla);}
  
void setBrilloPantalla(int brillo)
  {  
  brilloPantalla=brillo;
  setBrilloPantalla();
  }
  
void borraPantalla(void) {M5.Lcd.fillScreen(colorFondo);}//(COLOR_FONDO);}
void pantallaNegra(void) {M5.Lcd.fillScreen(TFT_BLACK);}

/*********************************************/
/* Valida la temperatura promedio y la pasa  */
/* a cadena. Si es NO_LEIDO devuelve -.-     */
/*********************************************/
String combierteTemperaturaPromedio(void)
  {
  float TP=getTemperaturaPromedio();
  String TPS="";
  const uint8_t len=7;
  
  char temp[len]="--.- C";
  if(TP!=NO_LEIDO){
    sprintf(temp,"%2.1f C",TP);
    TPS=String(temp);
    while(TPS.length()<len-1) TPS=" " + TPS;
  }

  return TPS;
  }
  
/*********************************************/
/* Valida la temperatura promedio y la pasa  */
/* a cadena. Si es NO_LEIDO devuelve -.-     */
/*********************************************/
String combierteHumedadPromedio(void)
  {
  float TP=getHumedadPromedio();
  String TPS="";
  const uint8_t len=7;
  
  char temp[len]="---.-%";
  if(TP!=NO_LEIDO){
    sprintf(temp,"%3.1f%%",TP);
    TPS=String(temp);
    while(TPS.length()<len-1) TPS=" " + TPS;
  }

  return TPS;
  }
  
/*********************************************/
/* Valida la temperatura promedio y la pasa  */
/* a cadena. Si es NO_LEIDO devuelve -.-     */
/*********************************************/
String combiertePresionPromedio(void)
  {
  float TP=getPresionPromedio();
  String TPS="";
  const uint8_t len=10;
  
  char temp[len]="---.-hPa";
  if(TP!=NO_LEIDO){
    sprintf(temp,"%3.1fhPa",TP);
    TPS=String(temp);
    while(TPS.length()<len-1) TPS=" " + TPS;
  }

  return TPS;
  }
  
/*********************************************/
/* Pinta la cabecera de la pantalla          */
/* comun a varios layouts                    */
/*********************************************/
void pintaCabecera(void)
  {
  pintaTitulo();
  pintaFechaHora();
  } 

/*********************************************/
/* Pinta el titulo de la pantalla            */
/* comun a los layouts                       */
/*********************************************/
void pintaTitulo(void)
  {
  if(getValorPrincipal()!=REPOSO)
    {     
    M5.Lcd.fillRect(ANCHO_CERO, ALTO_CERO, ANCHO_TOTAL, ALTO_TITULO,colorTitulo);//COLOR_TITULO);
    M5.Lcd.setTextDatum(TC_DATUM);// Set text datum to top centre
    M5.Lcd.setTextColor(COLOR_LETRAS_TITULO, colorTitulo);//COLOR_TITULO);// Set text colour to orange with black background
    M5.Lcd.setFreeFont(FMBO9);                 // Select the font
    M5.Lcd.drawString(NOMBRE_FAMILIA, ANCHO_TOTAL/2, MARGEN_SUPERIOR/2, GFXFF);// Print the string name of the font  
    }
  }

/*********************************************/
/* Pinta la fecha y la hora en la            */
/* cabecera de la pantalla                   */
/*********************************************/
void pintaFechaHora(void)
  {  
  if(getValorPrincipal()!=REPOSO)
    {   
    M5.Lcd.setTextColor(COLOR_LETRAS_TITULO, colorTitulo);//COLOR_TITULO);// Set text colour to orange with black background  
    M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to top left
    M5.Lcd.drawString(getHora(), MARGEN_IZQUIERDO, ALTO_LETRA_9+MARGEN_SUPERIOR/2, GFXFF);// Print the string name of the font  
    M5.Lcd.setTextDatum(TR_DATUM);// Set text datum to top right
    M5.Lcd.drawString(getFecha(), ANCHO_TOTAL, ALTO_LETRA_9+MARGEN_SUPERIOR/2, GFXFF);// Print the string name of the font  
    }
  }

/*********************************************/
/* Pinta la temperatura media en la pantalla */
/* comun a varios layouts                    */
/*********************************************/
void pintaTemperaturaPromedio(void)
  {
  int x0,y0;
  int ancho,alto;
      
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;
  
  M5.Lcd.fillRoundRect(x0, y0, ancho, alto, 4, TFT_BLACK);  //(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO9);                 // Select the font
  M5.Lcd.drawString("Temperatura", 2*x0, y0, GFXFF);// Print the string name of the font

  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO24);                 // Select the font
  /*
  float TP=getTemperaturaPromedio();
  String TPS;
  if(TP==-100.0) TPS="-.-C";
  else TPS=String(getTemperaturaPromedio(),1)+"C";  
  */
  M5.Lcd.drawString((combierteTemperaturaPromedio()).c_str(), ANCHO_TOTAL/2, y0+alto/2, GFXFF);// Print the string name of the font
  }

/*********************************************/
/* Pinta el modo de calefaccion activo       */
/* comun a varios layouts                    */
/*********************************************/
void pintaModoCalefaccion(void)
  {
  int x0,y0;
  int ancho,alto;
      
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;
      
  if(1 || getModoManual()<MODO_AUTO)//si es ON u OFF
    {
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
    //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
    M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to middle centre
    M5.Lcd.setFreeFont(FM9);                 // Select the font    
    M5.Lcd.drawString(getModoManualTxt().c_str(), 3*ANCHO_TOTAL/4, y0+alto/2, GFXFF);// Print the string name of the font
    }
  }  

/*********************************************/
/* Piunta los menus en todas las pantalla    */
/* comun a todas las pantallas               */
/*********************************************/
void pintaMenu(int id_menu)
  {
  uint8_t a=0;
  for(uint8_t i=0;i<3;i++) a+=strcmp(menu[id_menu].texto[i],"");
  if(a==0) return;
  
  M5.Lcd.fillRect(0, ALTO_TOTAL-ALTO_MENU, ANCHO_TOTAL, ALTO_MENU, menu[id_menu].color_fondo);
  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setTextColor(menu[id_menu].color_texto, menu[id_menu].color_fondo);// Set text colour to orange with black background
  M5.Lcd.setFreeFont(FMB9);                 // Select the font  
  M5.Lcd.drawString(menu[id_menu].texto[0],  64, ALTO_TOTAL-ALTO_MENU/2, GFXFF);// Print the string name of the font
  M5.Lcd.drawString(menu[id_menu].texto[1], 160, ALTO_TOTAL-ALTO_MENU/2, GFXFF);// Print the string name of the font
  M5.Lcd.drawString(menu[id_menu].texto[2], 256, ALTO_TOTAL-ALTO_MENU/2, GFXFF);// Print the string name of the font  //M5.Lcd.drawString(menu[id_menu].texto[2], 256, 220, GFXFF);// Print the string name of the font
  }

/*********************************************/
/* Gestiona el pintado de la pantalla        */
/* Invocado desde el proceso general de loop */
/*********************************************/
void pintaPantalla(void)
  {
  float tempro=0;
  String texto;

  switch(getValorPrincipal())
    {
    case TEMPERATURA:
      tempro=getTemperaturaPromedio(); 
      texto= "Temp: " + String(tempro,1) + "C";
      if(!sateliteRegistrado(sateliteEnPantalla)) SiguienteSateliteEnPantalla(); //Inicializo al primer satelite registrado
      break;
    case CONSIGNA: 
      //consigna
      tempro=getConsigna();
      texto= "Cons: " + String(tempro,1) + "C";   
      break;
    case REPOSO:
      tempro=getConsigna();
      texto= "Cons: " + String(tempro,1) + "C";   
      break;    
    case INFO:
      texto="N/A";
      Serial.printf("PintaPantalla(%i): \n%s\n%i\n%s\n",getValorPrincipal(),texto.c_str(),sateliteEnPantalla,getMaxMensajeError().c_str());
      break;
    case MANUAL:  
      texto="N/A";
      Serial.println("Vamos al manual");
      break;
    otherwise:
      texto="";
    }
    
  ptr_func_pinta[getValorPrincipal()](texto,sateliteEnPantalla, getMaxMensajeError());      
  }
    
/*****************************************************************LAYOUTS**************************************************************************************/
//Modo Temperatura
void pintaLayout(String valor, int satelite, String alarma="")//Informacion de la Tª media y los satelites uno a uno
  {
  String nombre;
  float t;
  float h;
  float p;
  float a;
  int i;
  int x0,y0;
  int ancho,alto;
  
  //Linea principal
  pintaTemperaturaPromedio();

  //Lineas secundarias
  if(sateliteRegistrado(satelite))
    {
    nombre=getNombre(satelite,0);
    t=getTemperatura(satelite,0);
    h=getHumedad(satelite,0);
    p=getPresion(satelite,0);
    a=getAltitud(satelite,0);
    i=hayLuz(satelite);

    //Pinta el fondo
    //M5.Lcd.fillRect(MARGEN_IZQUIERDO, ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL, ANCHO_TOTAL-2*MARGEN_IZQUIERDO, ALTO_SECUNDARIO, colorFondo);
    M5.Lcd.fillRect(MARGEN_IZQUIERDO, ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL, ANCHO_TOTAL-2*MARGEN_IZQUIERDO, 5*ALTO_LETRA_12+MARGEN_SUPERIOR, TFT_LIGHTGREY);
    //pint el marco
    //M5.Lcd.drawRoundRect(MARGEN_IZQUIERDO, ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL, ANCHO_TOTAL-2*MARGEN_IZQUIERDO, ALTO_SECUNDARIO, 4, TFT_BLACK);
    M5.Lcd.drawRoundRect(MARGEN_IZQUIERDO, ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL, ANCHO_TOTAL-2*MARGEN_IZQUIERDO, 5*ALTO_LETRA_12+MARGEN_SUPERIOR, 4, TFT_LIGHTGREY);
    //Vamos conla sletras
    M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to middle centre
    M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);// Set text colour to orange with black background
    M5.Lcd.setFreeFont(FMB12);                 // Select the font  
    M5.Lcd.drawString(nombre + "("+ String(satelite+1) + "/" + String(numeroSatelites(false)) + ")", 2*MARGEN_IZQUIERDO, ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL+MARGEN_SUPERIOR, GFXFF);// Print the string name of the font
    M5.Lcd.setFreeFont(FM12);                 // Select the font  
    M5.Lcd.drawString("Temp.: " + String(t,1) + "º", 4*MARGEN_IZQUIERDO, ALTO_LETRA_12+ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL+MARGEN_SUPERIOR, GFXFF);// Print the string name of the font
    M5.Lcd.drawString("Humedad: " + String(h,1) + "%", 4*MARGEN_IZQUIERDO, 2*ALTO_LETRA_12+ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL+MARGEN_SUPERIOR, GFXFF);// Print the string name of the font
    M5.Lcd.drawString("Pres.: " + String(p,0) + "hPa (" + String(a,0) + " m)", 4*MARGEN_IZQUIERDO, 3*ALTO_LETRA_12+ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL+MARGEN_SUPERIOR, GFXFF);// Print the string name of the font
    M5.Lcd.drawString((i?"Iluminacion: si":"Iluminacion: no"), 4*MARGEN_IZQUIERDO, 4*ALTO_LETRA_12+ALTO_PRINCIPAL+ALTO_TITULO+2*SEPARADOR_VERTICAL+MARGEN_SUPERIOR, GFXFF);// Print the string name of the font  
    }

  //Modo de calefaccion
  pintaModoCalefaccion();
/*  
  if(getModoManual()<MODO_AUTO)//si es ON u OFF
    {
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
    M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
    M5.Lcd.setFreeFont(FM9);                 // Select the font
    M5.Lcd.drawString(valoresModoManualTxt[getModoManual()].c_str(), 3*ANCHO_TOTAL/4, y0+3*alto/2, GFXFF);// Print the string name of the font
    } 
*/    
  }

//Modo Consigna  
void pintaLayout2(String valor, int satelite, String alarma="")//Informacion de la Tª media y la consigna
  {
  String nombre;
  float t;
  float h;
  int i;
  int x0,y0;
  int ancho,alto;

  //Temperatura media
  pintaTemperaturaPromedio();

  //Consigna
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL+ALTO_PRINCIPAL+2*SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;  
  M5.Lcd.fillRoundRect(x0, y0, ancho, alto, 4, TFT_LIGHTGREY);  //(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO9);                 // Select the font
  M5.Lcd.drawString("Consigna", 2*x0, y0, GFXFF);// Print the string name of the font  

  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO24);                 // Select the font
  M5.Lcd.drawString((String(getConsigna(),1)+"C").c_str(), ANCHO_TOTAL/2, y0+alto/2, GFXFF);// Print the string name of the font  

  //Modo de calefaccion
  pintaModoCalefaccion();
/*  
  if(getModoManual()<MODO_AUTO)//si es ON u OFF
    {
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
    M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
    M5.Lcd.setFreeFont(FM9);                 // Select the font
    M5.Lcd.drawString(valoresModoManualTxt[getModoManual()].c_str(), 3*ANCHO_TOTAL/4, y0+3*alto/2, GFXFF);// Print the string name of the font
    }
*/    
  }

//Modo Reposo
void pintaLayout3(String valor, int satelite, String alarma="")//Modo reposo
  {
  String nombre;
  float t;
  float h;
  int i;
  int x0,y0;
  int ancho,alto;
  
  //Temperatura media
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;
  if(tempReposo!=getTemperaturaPromedio() || estadoReleReposo!=getEstadoRele(CALDERA)){
    tempReposo=getTemperaturaPromedio();
    estadoReleReposo=getEstadoRele(CALDERA);
    if (getEstadoRele(CALDERA)) M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);// Set text colour to red with black background
    else M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
    M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
    M5.Lcd.setFreeFont(&VanillaExtractRegular40pt7b);//(CF_OL32);                 // Select the font
    M5.Lcd.drawString((" "+combierteTemperaturaPromedio()+" ").c_str(), ANCHO_TOTAL/2, y0+alto/2, GFXFF);
  }
  
  //Consigna y modo
  x0=MARGEN_IZQUIERDO; 
  y0=ALTO_TITULO+3*SEPARADOR_VERTICAL+ALTO_PRINCIPAL; 
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO; 
  alto=ALTO_PRINCIPAL;   
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FF18);                 // Select the font
  M5.Lcd.drawString((String(getConsigna(),1)+"C (" + getModoManualTxt() + ")").c_str(), ANCHO_TOTAL/2, y0+alto, GFXFF);// Print the string name of the font

  //Humedad y Presion
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+4*SEPARADOR_VERTICAL+2*ALTO_PRINCIPAL;
  ancho=ANCHO_TOTAL;
  alto=ALTO_PRINCIPAL;  
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
  M5.Lcd.setFreeFont(FF18);                 // Select the font
//  M5.Lcd.setTextDatum(TL_DATUM);
//  M5.Lcd.drawString(combierteHumedadPromedio(), x0, y0+alto/2, GFXFF);
  M5.Lcd.setTextDatum(BL_DATUM);
  M5.Lcd.drawString(combierteHumedadPromedio(), 0, ALTO_TOTAL-1, GFXFF);

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to white with black background
  M5.Lcd.setFreeFont(FF18);                 // Select the font
//  M5.Lcd.setTextDatum(TR_DATUM);
//  M5.Lcd.drawString(combiertePresionPromedio(), 3.5*ANCHO_TOTAL/4, y0+alto/2, GFXFF);
  M5.Lcd.setTextDatum(BR_DATUM);
  M5.Lcd.drawString(combiertePresionPromedio(), ANCHO_TOTAL, ALTO_TOTAL-1, GFXFF);
  }

//Modo Info
void pintaLayout4(String valor, int satelite, String alarma="")//Informacion de la base y la pontencia
  {
  int x0,y0;
  int ancho,alto;

  //Temperatura media
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  alto=ALTO_PRINCIPAL;

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(ML_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FM18);//(CF_OL32);                 // Select the font
  M5.Lcd.drawString(String("Base: " + String(WiFi.SSID())).c_str(), x0,y0, GFXFF);// Print the string name of the font
  M5.Lcd.drawString(String("Potencia: " + String(WiFi.RSSI())).c_str(), x0, y0+alto, GFXFF);// Print the string name of the font
  M5.Lcd.drawString(String("IP: " + String(getIP(0))).c_str(), x0, y0+2*alto, GFXFF);// Print the string name of the font

  M5.Lcd.setTextDatum(BR_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FM12);//(CF_OL32);                 // Select the font
  M5.Lcd.drawString(String("Vueltas: " + String(vuelta)).c_str(), x0, y0+3*alto, GFXFF);// Print the string name of the font
  }

//Modo Tipo Calefaccion
void pintaLayout5(String valor, int satelite, String alarma="")//Informacion de la Tª media y el modo de calefaccion
  {
  String nombre;
  float t;
  float h;
  int i;
  int x0,y0;
  int ancho,alto;

  //Temperatura media
  pintaTemperaturaPromedio();
/*  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;
  M5.Lcd.fillRoundRect(x0, y0, ancho, alto, 4, TFT_BLACK);  //(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO9);                 // Select the font
  M5.Lcd.drawString("Temperatura", 2*x0, y0, GFXFF);// Print the string name of the font

  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO24);                 // Select the font
  M5.Lcd.drawString((String(getTemperaturaPromedio(),1)+"C").c_str(), ANCHO_TOTAL/2, y0+alto/2, GFXFF);// Print the string name of the font
*/
  //Modo de calefaccion
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL+ALTO_PRINCIPAL+2*SEPARADOR_VERTICAL;
  ancho=ANCHO_TOTAL-2*MARGEN_IZQUIERDO;
  alto=ALTO_PRINCIPAL;  
  M5.Lcd.fillRoundRect(x0, y0, ancho, alto, 4, TFT_LIGHTGREY);  //(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_LIGHTGREY);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO9);                 // Select the font
  M5.Lcd.drawString("Modo calefaccion", 2*x0, y0, GFXFF);// Print the string name of the font  

  M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FMBO24);                 // Select the font
  M5.Lcd.drawString(getModoManualTxt().c_str(), ANCHO_TOTAL/2, y0+alto/2, GFXFF);// Print the string name of the font
  }
/***************************************************************FIN LAYOUTS************************************************************************************/      

//Modo WifiManager
void pintaWifiManager(String miSSID, String miIP, String miNombre)//Activado como base wifi esperando configuracion. Informa del SSID, IP y del nombre mDNS
  {
  int x0,y0;
  int ancho,alto;

  //Temperatura media
  x0=MARGEN_IZQUIERDO;
  y0=ALTO_TITULO+SEPARADOR_VERTICAL;
  alto=ALTO_PRINCIPAL;

  pantallaNegra();

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);// Set text colour to orange with black background
  M5.Lcd.setTextDatum(ML_DATUM);// Set text datum to top centre //M5.Lcd.setTextDatum(MC_DATUM);// Set text datum to middle centre
  M5.Lcd.setFreeFont(FM12);                 // Select the font
  M5.Lcd.drawString("Configuracion WiFi", x0,y0, GFXFF);// Print the string name of the font
  M5.Lcd.setFreeFont(FM9);                 // Select the font
  M5.Lcd.drawString((String("SSID: ") + miSSID).c_str(), x0, y0+alto, GFXFF);// Print the string name of the font
  M5.Lcd.drawString((String("IP: ") + miIP).c_str(), x0, y0+2*alto, GFXFF);// Print the string name of the font
  M5.Lcd.drawString((String("mDNS: ") + miNombre).c_str(), x0, y0+2*alto, GFXFF);// Print the string name of the font
  }


    
void escribePantalla(int x, int y, String texto, int tamano)
  {
  switch(tamano)
    {
    case 9:
      M5.Lcd.setFreeFont(FM9);
      break;
    case 12:
      M5.Lcd.setFreeFont(FM12);
      break;
    case 18:
      M5.Lcd.setFreeFont(FM18);
      break;
    case 24:
      M5.Lcd.setFreeFont(FM24);
      break;
    otherwise:
      M5.Lcd.setFreeFont(FM9);
    }
  M5.Lcd.setTextDatum(TL_DATUM);// Set text datum to TL: top left, TM: top middle, TR: top right, ML: middle left,... BR: bottom right
  M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY);// Set text colour to orange with black background
  M5.Lcd.drawString(texto, x, y, GFXFF);// Print the string name of the font        
  }

/********************************************************************************/
/*                                                                              */
/*                  Funciones para los menus de los botones                     */ 
/*                                                                              */
/********************************************************************************/
void SiguienteSateliteEnPantalla(void)
  {   
  for(int i=1;i<MAX_SATELITES;i++)
    {    
    if (sateliteRegistrado((sateliteEnPantalla+i)%MAX_SATELITES))//esta registrado, salgo con el
      {
      sateliteEnPantalla=(sateliteEnPantalla+i)%MAX_SATELITES;      
      break;
      }
    }
  }

void AnteriorSateliteEnPantalla(void)
  {   
  for(int i=1;i<MAX_SATELITES;i++)
    {    
    if (sateliteRegistrado((MAX_SATELITES+sateliteEnPantalla-i)%MAX_SATELITES))//esta registrado, salgo con el
      {
      sateliteEnPantalla=(MAX_SATELITES+sateliteEnPantalla-i)%MAX_SATELITES;
      break;
      }
    }
  }

int getValorPrincipal(void) 
  {
  return(id_menu);  
  //return(valorPrincipal);
  }

void setValorPrincipal(int valor)
  {
  //valorPrincipal=valor;
  id_menu=valor; 
  
  switch(valor)
    {
    case TEMPERATURA:
    case CONSIGNA:      
    case MANUAL:
      borraPantalla();
      pintaCabecera(); 
      pintaMenu(id_menu); 
      break;
    case REPOSO:
      pantallaNegra();
      break;
    case INFO:
      pantallaNegra();
      break;
    otherwise: 
      break;
    }
  }
