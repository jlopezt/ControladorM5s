/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/

/************************************************/
/* Inicializa el sistema de ficheros del modulo */
/************************************************/
boolean inicializaFicheros(int debug)
{
  String s="";
  
  //inicializo el sistema de ficheros
  if (!SPIFFS.begin(true)) 
    {
    Serial.println("No se puede inicializar el sistema de ficheros");
    return (false);
    }
  
  listaFicheros(s);  
  return (true);
}

/************************************************/
/* Recupera los datos de                        */
/* de un archivo cualquiera                     */
/************************************************/
boolean leeFichero(String nombre, String &contenido)
  {
  boolean leido=false;
  
  Serial.println("Inicio de lectura de fichero " + nombre);

  if (SPIFFS.exists(nombre)) 
    {
    //file exists, reading and loading
    Serial.printf("Encontrado fichero de configuracion %s.\n",nombre.c_str());
    File configFile = SPIFFS.open(nombre, "r");
    if (configFile) 
      {
      Serial.printf("Abierto fichero de configuracion %s.\n",configFile.name());
      size_t size = configFile.size();

      // Allocate a buffer to store contents of the file.
      char *buff=NULL;
      buff=(char *)malloc(size+1);

      configFile.readBytes(buff, size);
      buff[size]=0;//pongo el fin de cadena
        
      contenido=String(buff);
      Serial.printf("Contenido del fichero: #%s#\n",contenido.c_str());
      free(buff);
      leido=true;
        
      configFile.close();//cierro el fichero
      Serial.println("Cierro el fichero");
      }//la de abrir el fichero de configuracion del WiFi
      else Serial.println("Fichero no se puede abrir");
    }//la de existe fichero
    else Serial.printf("Fichero [%s] no existe\n",nombre.c_str());
  
  return leido;
  }
/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero)
  {
  boolean salvado=false;

  //file exists, reading and loading
  if(SPIFFS.exists(nombreFichero.c_str())) 
    {
    if(nombreFicheroBak!="")
      {
      Serial.printf("El fichero %s ya existe, se copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
        
      if(SPIFFS.exists(nombreFicheroBak.c_str())) SPIFFS.remove(nombreFicheroBak.c_str());  
      SPIFFS.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
      }
    else Serial.printf("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
    }

  Serial.print("Nombre fichero: ");
  Serial.println(nombreFichero.c_str());
  Serial.print("Contenido fichero: ");
  Serial.println(contenidoFichero.c_str());
   
  File newFile = SPIFFS.open(nombreFichero.c_str(), FILE_WRITE);//abro el fichero, si existe lo borra
  if (newFile) 
    {
    Serial.printf("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
    newFile.print(contenidoFichero);
    newFile.close();//cierro el fichero
    Serial.println("Cierro el fichero");
    salvado=true;
    }
  else Serial.println("El fichero no se pudo abrir para escritura.\n");
      
  return salvado;
  }


/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean borraFichero(String nombreFichero)
  {
  boolean borrado=false;

  //file exists, reading and loading
  if(!SPIFFS.exists(nombreFichero)) Serial.println("El fichero " + nombreFichero + " no existe.");
  else
    {
    if (SPIFFS.remove(nombreFichero)) 
      {
      borrado=true;
      Serial.println("El fichero " + nombreFichero + " ha sido borrado.");
      }
    else Serial.println("No se pudo borrar el fichero " + nombreFichero + ".");
    }  

  return borrado;
  }  

/************************************************/
/* Devuelve el nombre del direcotrio del        */
/* fichro que se pasa como parametro            */
/************************************************/
String directorioFichero(String nombreFichero)
  {
  if (!nombreFichero.startsWith("/")) nombreFichero="/" + nombreFichero;
  String cad=nombreFichero.substring(0,nombreFichero.lastIndexOf("/"));
  return(cad);
  }

/************************************************/
/* Devuelve si un nombre de fichero incluye     */
/* un directorio por que encuentre mas de una / */
/************************************************/
boolean esDirectorio(String nombre)
  {
  if(nombre.startsWith("/")) nombre=nombre.substring(1);//si empieza por / se lo quito

  if(nombre.indexOf("/")!=-1) return true;
  return false;
  }

/************************************************/
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve una cadena separada    */
/* por SEPARADOR                                */
/************************************************/
String listadoFicheros(String prefix)
  {   
  String salida="";

  if(!prefix.startsWith("/")) prefix="/" + prefix;

  const size_t capacity = 2*JSON_ARRAY_SIZE(15) + JSON_OBJECT_SIZE(31);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& json = jsonBuffer.createObject();
  json["padre"] = prefix;

  JsonArray& subdirectorios = json.createNestedArray("subdirectorios");
  JsonArray& ficheros = json.createNestedArray("ficheros");

  File root = SPIFFS.open(prefix);
  File file = root.openNextFile();

  while(file)
    {
    String fichero=String(file.path()); //cambiado name por path Serial.printf("Nombre del fichero: %s |Ruta: %s\n",file.name(),file.path());
    //Si el nombre incluye el prefix, se lo quito
    uint8_t inicio=(fichero.indexOf(prefix)==-1?0:fichero.indexOf(prefix));
    fichero=fichero.substring(inicio+prefix.length());

    if(esDirectorio(fichero)) 
      {
      //verifico que el directorio no este ya en la lista
      boolean existe=false;
      String subdir=fichero.substring(0,fichero.indexOf("/"));
      for(uint8_t i=0;i<subdirectorios.size();i++)
        {
        if(subdir==subdirectorios[i]) 
          {
          existe=true;
          break;
          }
        }
      if(!existe) subdirectorios.add(fichero.substring(0,fichero.indexOf("/")));
      }
    else 
      {
      JsonObject& fichero_nuevo = ficheros.createNestedObject();
      fichero_nuevo["nombre"] = fichero;
      fichero_nuevo["tamano"] = file.size();
      fichero_nuevo["fechaEdicion"] = horaYfecha(file.getLastWrite());
      }

    file = root.openNextFile();
    }   
  json.printTo(salida);
  return (salida);
  }  

/************************************************/
/* Recupera los ficheros almacenados en el      */
/* dispositivo. Devuelve una cadena separada    */
/* por SEPARADOR                                */
/************************************************/
boolean listaFicheros(String &contenido)
  {   
  contenido="";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
 
  while(file)
    {
    Serial.print("FILE: ");
    Serial.println(file.name());

    contenido += String(file.name());
    contenido += SEPARADOR;
      
    file = root.openNextFile();
    }
    
  return (true);
  }  

/************************************************/
/* Devuelve si existe o no un fichero en el     */
/* dispositivo                                  */
/************************************************/
boolean existeFichero(String nombre)
  {  
  return (SPIFFS.exists(nombre));
  }


/************************************************/
/* Formatea el sistemas de ficheros del         */
/* dispositivo                                  */
/************************************************/
boolean formatearFS(void)
  {  
  return (SPIFFS.format());
  }
  
