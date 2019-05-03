/***************************************************************/
/*   Gestion de los botones del M5Stack                        */
/*   Gestion del menu                                          */
/***************************************************************/
#define MAX_MENUS  5
#define BOTON_A    1
#define BOTON_B    2
#define BOTON_C    3
#define BOTON_NONE 0

typedef struct 
  {
  char texto[3][7];
  int mueveA[3]; //a que menu me muevo cuando se pulsa esa tecla A=0, B=1 y C=2
  int16_t color_fondo;
  int16_t color_texto;
  void (*p_func_BtnA) (void)=NULL;
  void (*p_func_BtnB) (void)=NULL;
  void (*p_func_BtnC) (void)=NULL;
  }tipo_menu;

tipo_menu menu[MAX_MENUS]; //menus a mostrar
int id_menu=0; //id de menu activo
  
void inicializaBotones(void)
  {
  Serial.println("Init menus");
  //Inicializo los menus
  //menu 0: Temperatura
  int8_t id=0;
  strcpy(menu[id].texto[0],"subir");
  strcpy(menu[id].texto[1],"consig");
  strcpy(menu[id].texto[2],"bajar");
  menu[id].color_fondo=COLOR_MENU;
  menu[id].color_texto=COLOR_LETRAS_MENU;
  menu[id].p_func_BtnA=SiguienteSateliteEnPantalla;
  menu[id].p_func_BtnB=muestraConsigna;
  menu[id].p_func_BtnC=AnteriorSateliteEnPantalla;
  //menu 1: Consigna
  id=1;
  strcpy(menu[id].texto[0],"subir");
  strcpy(menu[id].texto[1],"modo");
  strcpy(menu[id].texto[2],"bajar");
  menu[id].color_fondo=COLOR_MENU;
  menu[id].color_texto=COLOR_LETRAS_MENU;
  menu[id].p_func_BtnA=subeConsigna;
  menu[id].p_func_BtnB=muestraManual;
  menu[id].p_func_BtnC=bajaConsigna;
  //menu 2: Reposo
  id=2;
  strcpy(menu[id].texto[0],"");
  strcpy(menu[id].texto[1],"");
  strcpy(menu[id].texto[2],"");
  menu[id].color_fondo=TFT_BLACK;
  menu[id].color_texto=TFT_BLACK;
  menu[id].p_func_BtnA=Auxiliar;
  menu[id].p_func_BtnB=muestraConsigna;
  menu[id].p_func_BtnC=muestraInfo;
  //menu 3: Info
  id=3;
  strcpy(menu[id].texto[0],"");
  strcpy(menu[id].texto[1],"");
  strcpy(menu[id].texto[2],"");
  menu[id].color_fondo=TFT_BLACK;
  menu[id].color_texto=TFT_BLACK;
  menu[id].p_func_BtnA=muestraConsigna;
  menu[id].p_func_BtnB=muestraConsigna;
  menu[id].p_func_BtnC=muestraConsigna;
  //menu 4: Manual
  id=4;
  strcpy(menu[id].texto[0],"subir");
  strcpy(menu[id].texto[1],"temp.");
  strcpy(menu[id].texto[2],"bajar");
  menu[id].color_fondo=COLOR_MENU;
  menu[id].color_texto=COLOR_LETRAS_MENU;
  menu[id].p_func_BtnA=subirModoManual;
  menu[id].p_func_BtnB=muestraTemperatura;
  menu[id].p_func_BtnC=bajarModoManual;

  Serial.println("Fin init menus");
  }

void atiendeBotones(void)
  {
  //M5.update();
    
  if (M5.BtnA.wasPressed())
    {
    Serial.println("Presionado boton A");
    SleepBucle=millis();
    if(menu[id_menu].p_func_BtnA!=NULL) menu[id_menu].p_func_BtnA();
    }
  else if (M5.BtnB.wasPressed())
    {
    Serial.println("Presionado boton B");
    SleepBucle=millis();
    if(menu[id_menu].p_func_BtnB!=NULL) menu[id_menu].p_func_BtnB();
    }
  else if (M5.BtnC.wasPressed()) 
    {
    Serial.println("Presionado boton C");
    SleepBucle=millis();
    if(menu[id_menu].p_func_BtnC!=NULL) menu[id_menu].p_func_BtnC();
    }

  pintaMenu(id_menu);  
  }


/********************************************************************************/
/*                                                                              */
/*                  Funciones para los menus de los botones                     */ 
/*                                                                              */
/********************************************************************************/
void muestraTemperatura(void)  
  {
  setValorPrincipal(TEMPERATURA);
  //id_menu=0;
  }
  
void muestraConsigna(void)  
  {
  setValorPrincipal(CONSIGNA);
  //id_menu=1;
  }

void muestraReposo(void)  
  {
  setValorPrincipal(REPOSO);
  //id_menu=2;
  }

void muestraInfo(void)
  {
  setValorPrincipal(INFO);
  //id_menu=3;  
  }    

void muestraManual(void)
  {
  setValorPrincipal(MANUAL);
  //id_menu=4;  
  }    

void Auxiliar(void)//funcion para usos varios al pulsar el boton A en la pantalla de reposo
  {
  reinicializaWebServer();
  }  
  
void subeConsigna(void)
  {
  setConsigna(getConsigna()+INC_CONSIGNA); 
  }

void bajaConsigna(void)
  {
  setConsigna(getConsigna()-INC_CONSIGNA); 
  }

void subirModoManual(void)
  {
  setModoManual(getModoManual()+1);  
  }

void bajarModoManual(void)  
  {
  setModoManual(getModoManual()+2);//sumo dos que es lo mismo que restar 1 en modulo 3
  }
