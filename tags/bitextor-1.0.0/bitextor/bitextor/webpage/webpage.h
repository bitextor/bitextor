/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
   
   Codigo fuente de la clase encargada de la gestion de paginas web, descargadas
   utilizando la aplicacion wget. Se encarga de transformarlas a XML, asi como de
   extraer estadisticas diversas.
*/

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include <libxml/xmlreader.h>
#include "../util/genericutils.h"
#include "../util/languageutils.h"

//#define TRAZANDO_WebPage //Comentar para evitar la traza

#define kSizeAuxStrings 3000
#define kSizeFileBuffer 500000
#define kMaxGuessers 100 //Maximo numero de guessers que se pueden incluir en el xml
#define kMaxLanguages 100 //Maximo numero de idiomas que se pueden incluir en los resultados de guessers de los xml
#define kMaxApplications 100 //Maximo numero de aplicaciones que se pueden incluir en el xml
#define kMaxSimilars 5000 //Numero maximo de similares que puede tener una pagina en el xml
#define kMaxLevelTags 30 //Nivel maximo de etiquetas que se tienen en cuenta en las estadisticas

/* El siguiente define indica si se busca la fecha de ultima modificacion en el
   fichero o no. Si no se busca o no se encuentra se le asigna el valor de tiempo
   actual*/
//#define SearchLastModified

/** Clase que representa una pagina web que se ha descargado de la web
 * El formato del fichero que se utiliza es el siguiente
 * Page Storage for Translation Automata (PSBR):
 *	<psbr>
 *	 <metainfo>
 *	  <source>
 *	   <location domain="..." path="..." file="..." ext="..." param="..."/>
 *	   <date type="generation" value="..."/> 
 *	  </source>
 *	  <local>
 *	   <date type="retrieval" value="..."/>  
 *	   <size value="..." text="..."/>
 *	   <tagsnumber value="..."/>
 *	   <taglevelnumber value="..."/>
 *	  </local>
 *	  <language code="...">
 *	   <guesser name="...">
 *	    <lang code="..." probability="..."/>
 *	    <lang code="..." probability="..."/>
 *	      . . .
 *	   </guesser>
 *	   <guesser name="...">
 *	    <lang code="..." probability="..."/>
 *	    <lang code="..." probability="..."/>
 *	   </guesser>
 *	     . . .
 *	  </language>
 *	  <process> <!-- Aplicaciones que han modificado el xml -->
 *	   <application name="..." version="...">
 *	   <application name="..." version="...">
 *	    . . .
 *	  </process>
 *	  <similar> <!-- Ficheros que podrian ser traducciones del actual -->
 *	   <id langcode="..." value="..." distance="..."/>
 *	   <id langcode="..." value="..." distance="..."/>
 *	    . . .
 *	  </similar>
 *	 </metainfo>
 *	 <content>
 *	 ...
 *	 </content>
 *	</psbr>  
 */ 
class WebPage
 {
  char *localfile; /** Nombre del fichero local en el que esta almacenada la pagina web */
  char *domain; /** Nombre de dominio del que se bajo la pagina web */
  char *path; /** Directorio dentro del dominio en el que estaba la pagina web */
  char *filename; /** Nombre del fichero en internet de la pagina web */
  char *fileext; /** Extension del fichero en internet de la pagina web*/
  char *fileparam; /** Parametros que se le han pasado a la pagina por GET */
  unsigned int date; /** Fecha en la que se almaceno la pagina web */
  unsigned int origdate; /** Fecha en la que se publico la pagina web */
  short language; /** Idioma en el que se estima que esta la pagina */
  unsigned int size; /** Tamaño en bytes que ocupa la pagina web */
  unsigned int textsize; /** Numero de bytes de texto de la pagina */
  char *content; /** Contenido de la pagina web*/
  int n_tags; /** Numero de etiquetas que contiene la pagina */
  int n_leveltags[kMaxLevelTags]; /** Numero de etiquetas de cada nivel contenidas en la pagina */
  int *similars; /** Vector con los identificadores de los archivos que se presuponen 
                     compatibles con esta pagina web */
  short *langsimilars; /** Vector con el idioma de cada uno de los similares */
  float *distancesimilars; /** Vector con la distancia a cada uno de los similares segun las heuristicas utilizadas */
  int n_similars; /** Numero de ficheros compatibles con esta pagina web */
  
  char **applications; /** Vector con las aplicaciones que han generado/modificado el xml */
  char **appversions; /** Vector con las versiones de cada una de dichas aplicaciones */
  int n_applications; /** Numero de aplicaciones que han generado/modificado el xml */
  char **guessers; /** Guessers que se han aplicado para averiguar el idioma de la pagina */
  int n_guessers; /** Numero de guessers que se han aplicado */
  int *n_languagesperguesser; /** Numero de idiomas con los que ha trabajado cada guesser */
  int **id_languagesperguesser; /** Identificador de los idiomas con los que ha trabajado cada guesser */
  float **prob_languagesperguesser; /** Probabilidad de cada idioma de entre los que trabajado cada guesser */  
  
  /* Estadisticas obtenidas al procesar la pagina (al cargarla solo se cargan n_links y n_images) */
  unsigned int n_links; /** Numero de enlaces que contiene la pagina */
  char **links; /** Lista de links que tiene la pagina */
  unsigned int n_outlinks; /** Numero de enlaces a otras webs */
  unsigned int n_images; /** Numero de imagenes que contiene la pagina */  
  
  xmlTextReaderPtr reader; /** Variable para la carga del fichero con la libreria LIBXML2 */
  
  /** Funcion que recibe una cadena con la fecha y hora y la adapta a la estructura tm
   *   Esta funcion recibe una cadena del tipo: "Mon, 15 Mar 2004 07:04:09 GMT" y
   *   rellena la estructura tm que contiene un campo para cada uno de esos datos
   *
   * @param _date contiene la cadena a transformar
   * @returns NULL si la cadena no es valida o la estructura tm rellena si es valida
   */ 
  
  struct tm* GetDateString(const char* _date);
    
  /** Funcion que almacena una url en la lista de enlaces de la pagina
   *  Esta funcion recibe una url de un enlace y la inserta en el vector de
   *  enlaces de la pagina, incrementando el total de enlaces
   *
   * @param _url cadena con la url a comprobar
   * @returns true si la pagina era distinta de NULL o false en otro caso
   *
   */
  bool InsertLink(const char* _url);

  /** Funcion que procesa la etiqueta <psbr> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <psbr> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessPSBR();
  
  /** Funcion que procesa la etiqueta <metainfo> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <metainfo> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessMETAINFO();
  
  /** Funcion que procesa la etiqueta <source> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <source> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessSOURCE();
  
  /** Funcion que procesa la etiqueta <local> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <local> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessLOCAL();
  
  /** Funcion que procesa la etiqueta <language> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <language> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessLANGUAGE();
  
  /** Funcion que procesa la etiqueta <process> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <process> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessPROCESS();
  
  /** Funcion que procesa la etiqueta <similar> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <similar> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessSIMILAR();
  
  /** Funcion que procesa la etiqueta <location> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta >location> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessLOCATION();
  
  /** Funcion que procesa la etiqueta <date> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <date> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessDATE();
  
  /** Funcion que procesa la etiqueta <size> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <size> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessSIZE();
  
  /** Funcion que procesa la etiqueta <tagnumber> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <tagnumber> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessTAGNUMBER();
  
  /** Funcion que procesa la etiqueta <taglevelnumber> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <taglevelnumber>
   *  y de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessTAGLEVELNUMBER();

  /** Funcion que procesa la etiqueta <linksnumber> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <linksnumber>
   *  y de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessLINKSNUMBER();

  /** Funcion que procesa la etiqueta <imagesnumber> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <imagesnumber>
   *  y de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessIMAGESNUMBER();
    
  /** Funcion que procesa la etiqueta <guesser> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <guesser> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessGUESSER();
  
  /** Funcion que procesa la etiqueta <lang> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <lang> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessLANG();
  
  /** Funcion que procesa la etiqueta <application> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <application> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessAPPLICATION();
  
  /** Funcion que procesa la etiqueta <id> durante la carga del fichero (LIBXML2)
   *  Esta funcion se encarga de leer el contenido interno de la etiqueta <id> y
   *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
   *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
   *  y ya esta abierto y listo.
   * 
   * @returns False si hubo algun error o True en caso contrario.
   */
  bool ProcessID();
  
  public:
      
    /** Constructor de la clase
     *  Inicializa todos los atributos a los valores por defecto
     */
    WebPage();
    
    /** Constructor de la clase que inicializa la pagina con el fichero asociado
     *  Carga todos los campos del fichero indicado. Si el fichero no fuese
     *  accesible entonces equivaldria al constructor sin parametros
     * 
     * @param _localfile Nombre y ruta del fichero almacenado en el disco duro
     *     
     */
    WebPage(const char* _localfile);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~WebPage();    

    /** Funcion que libera la pagina cargada si existiera
     *  Esta funcion libera la memoria asociada al objeto pagina cargado
     *  actualmente, en caso de que se haya cargado uno.
     */
    void Reset();
       
    /** Funcion que guarda el fichero en disco duro
     *  Esta funcion guarda en disco duro toda la informacion relevante del
     *  objeto en el formato XML establecido. Si el fichero ya existiera sera
     *  sobreescrito sin preguntar.
     * 
     * @param _file Nombre y ruta del fichero que se genera en disco duro
     * @returns True si no hubo problemas al generar el fichero o False si los hubo
     *     
    */
    bool Save(const char* _file);
     
    /** Funcion que guarda el fichero en disco duro de nuevo
     *  Esta funcion guarda en disco duro toda la informacion relevante del
     *  objeto en el formato XML establecido. Si el fichero ya existiera sera
     *  sobreescrito sin preguntar. El fichero de destino tiene que haber sido
     *  establecido previamente, y se tomara de la variable localfile
     *
     * @returns True si no hubo problemas al generar el fichero o False si los hubo
     *
     */
    bool ReSave();
    
    /** Funcion que actualiza la cabecera de una pagina en disco
     *  Esta funcion carga el fichero y lo graba de nuevo con los datos de la
     *  cabecera actuales, por lo que se perderan los anteriores
     */
    bool Refresh();
    
    /** Funcion que recibe una url y que la fragmenta y almacena en los atributos
     *  Esta funcion separa una cadena que contiene una url en domain, path,
     *  filename, fileext  y fileparam. Esta funcion asume que la url es valida
     *  por lo que si no lo es, no funcionara bien
     *
     * @param _url Cadena con la url a transformar 
     */  
    void FijarURL(const char* _url);
    
    /** Funcion que carga un fichero PSBR del disco duro
     *  Esta funcion recibe la ruta completa y el nombre del fichero
     *  que corresponde a la pagina web que queremos cargar.
     * 
     * @param _file Nombre y ruta del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool Load(const char* _file);

    /** Funcion que carga la cabecera PSBR del fichero del disco duro
     *  Esta funcion recibe la ruta completa y el nombre del fichero
     *  que corresponde a la pagina cuya cabecera queremos cargar.
     * 
     * @param _file Nombre y ruta del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool LoadHeader(const char* _file);
      
    /** Funcion que carga el contenido de la pagina PSBR pero no su cabecera
     *  Esta funcion recibe la ruta completa y el nombre del fichero
     *  que corresponde a la pagina web cuyo contenido queremos cargar.
     * 
     * @param _file Nombre y ruta del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool LoadContent(const char* _file);    
    
    /** Funcion que añade un fichero similar a este fichero
     *  Esta funcion introduce un fichero similar en forma de identificador
     *  del fichero y el idioma en el que esta la equivalencia, e introduce la
     *  distancia que separa a ambas paginas.
     *
     * @param _language indica el idioma en el que esta el archivo similar
     * @param _numfile es el numero del fichero que se considera archivo similar
     * @param _distance es la distancia segun las heuristicas aplicadas con el similar
     * @returns true si no hay problemas, false si esta repetido el similar
     */
    bool NewSimilar(short _language,int _numfile,float _distance);
    
    /** Funcion que cambia el numero de un fichero similar por otro (util al renumerar)
     *  Esta funcion busca el fichero similar que recibe en forma de identificador
     *  del fichero y el idioma en el que esta, y sustituye el identificador por el
     *  otro valor que recibe o lo borra de la lista si es negativo
     *
     * @param _language indica el idioma en el que esta el archivo similar
     * @param _numorigfile es el numero del fichero similar a buscar
     * @param _numnewfile es el nuevo numero que se asocia a dicho similar o -1 si ya no hay
     * @returns true si no hay problemas o false si el similar a reemplazar no existia
     */
    bool ChangeSimilar(short _language,int _numorigfile,int _numnewfile);
    
    /** Funcion que cambia la distancia a un fichero similar por otro valor
     *  Esta funcion busca el fichero similar que recibe en forma de identificador
     *  del fichero y el idioma en el que esta, y sustituye la distancia que hubiera
     *  por la nueva.
     *
     * @param _language indica el idioma en el que esta el archivo similar
     * @param _numorigfile es el numero del fichero similar a buscar
     * @param _newdistance es la nueva distancia que se asocia a dicho similar
     * @returns true si no hay problemas o false si el similar a actualizar no existia
     */
    bool RefreshSimilar(short _language,int _numorigfile,float _newdistance);
    
    /** Funcion que borra un similar (util al renumerar)
     *  Esta funcion busca el fichero similar que recibe en forma de identificador
     *  del fichero y el idioma en el que esta, y lo elimina.
     *
     * @param _language indica el idioma en el que esta el archivo similar
     * @param _numfile es el numero del fichero similar a borrar
     * @returns true si no hay problemas o false si el similar a borrar no existia
     */
    inline bool DeleteSimilar(short _language,int _numfile)
     {
      return(ChangeSimilar(_language,_numfile,-1));
     }
    
    /** Funcion que elimina todos los similares de la pagina que no esten en la lista
     *  Esta funcion recorre todos los similares de la pagina y elimina todos los que
     *  no esten incluidos en la lista de similares que se recibe como parametros.
     *
     * @param _sizelist Tamaño de la lista de similares que permaneceran
     * @param _languagelist Lista de idiomas de cada uno de los similares que permaneceran
     * @param _numberlist Lista con los identificadores de los similares que permaneceran
     * @returns El numero de similares que se han borrado
     */
    unsigned int ResetSimilarListTo(unsigned int _sizelist,unsigned short* _languagelist,unsigned int* _numberlist);
    
    /** Funcion que importa un fichero del wget y lo transforma al estandar PSBR
     *  Esta funcion carga un fichero wget (con cabecera HTTP) y extrae los
     *  campos adecuados de la misma para integrarlo al estandar PSBR. Tambien
     *  se encarga de pasar el tidy a los documentos mediante un fichero temporal
     * 
     * @param _file indica el fichero a importar
     * @param _href indica el origen del fichero que se importa
     * @returns false si hay algun error en la carga del fichero
     */    
    bool ImportWget(const char* _file,const char* _href);
    
    /** Funcion que importa un fichero obtenido utilizando wget+tidy+unicode2ascii
     *  Esta funcion carga un fichero de texto normal, que podra ser o no html y se
     *  encarga de la construccion del fichero en el estandar psbr.
     * 
     * @param _file indica el fichero a importar
     * @param _href indica el origen del fichero que se importa
     * @returns false si hay algun error en la carga del fichero
     */
    bool ImportConvertedPage(const char* _file,const char* _href);
  
    /** Funcion que procesa y analiza la pagina web para extraer muchas caracteristicas
     *  Esta funcion recorre la pagina web cargada y realiza los conteos de enlaces,
     *  enlaces a paginas externas, numero de paginas y numero de etiquetas
     *
     * @returns True si no hubo problemas o False si el fichero no estaba cargado o habia errores
     */
    bool ProcessPage();
  
    /** Funcion que devuelve el idioma en el que esta la pagina cargada
     *  Esta funcion tan solo devuelve el idioma al que se ha estimado que pertenece
     *  la pagina cargada.
     * 
     * @returns El identificador del idioma asociado
     */ 
    short WhatLanguage();
    
    /** Funcion que devuelve el numero de etiquetas de la pagina cargada
     *  Esta funcion tan solo devuelve el numero de etiquetas que se han encontrado
     *  en la pagina.
     * 
     * @returns El numero de etiquetas que contiene la pagina
     */ 
    unsigned int N_Tags();

    /** Funcion que devuelve el numero de etiquetas por nivel de la pagina cargada
     *  Esta funcion tan solo devuelve el numero de etiquetas que se han encontrado
     *  por cada nivel en la pagina, en forma de vector const.
     * 
     * @returns El numero de etiquetas por nivel en el arbol que contiene la pagina
     */ 
    const unsigned int* N_LevelTags();
        
    /** Funcion que devuelve el numero de enlaces de la pagina cargada
     *  Esta funcion tan solo devuelve el numero de enlaces que se han encontrado en la 
     *  pagina, es decir, el numero de <a href="..."> que contiene.
     * 
     * @returns El numero de enlaces que contiene la pagina
     */ 
    unsigned int N_Links();
    
    /** Funcion que devuelve un vector con los enlaces que contiene la pagina
     *  Esta funcion tan solo permite el acceso a la lista de enlaces que contiene
     *  la pagina cargada.
     *
     * @returns El vector que contiene los enlaces a la pagina
     */ 
    const char** Links();
        
    /** Funcion que devuelve el numero de similares de la pagina cargada
     *  Esta funcion tan solo devuelve el numero de ficheros similares que tiene la 
     *  pagina, es decir, el valor de la variable n_similars.
     * 
     * @returns El numero de similares que contiene la pagina
     */ 
    unsigned int N_Similars();
    
    /** Funcion que devuelve los similares a la pagina cargada
     *  Esta funcion tan solo devuelve un vector con los ficheros que son
     *  similares a la pagina cargada, es decir, el valor de la variable similars
     *
     * @returns El vector que contiene los similares a la pagina
     */
    const int* Similars();
    
    /** Funcion que devuelve el idioma de cada uno de los similares de la pagina cargada
     *  Esta funcion tan solo devuelve un vector con el idioma de cada uno de los ficheros
     *  similares a la pagina cargada, es decir, el valor de la variable langsimilars
     *
     * @returns El vector que contiene el idioma de cada uno de los similares a la pagina
     */
    const short* LangSimilars();
    
    /** Funcion que devuelve la distancia de cada uno de los similares de la pagina cargada
     *  Esta funcion tan solo devuelve un vector con la distancia de cada uno de los ficheros
     *  similares a la pagina cargada, es decir, el valor de la variable distancesimilars
     *
     * @returns El vector que contiene la distancia de cada uno de los similares a la pagina
     */
    const float* DistanceSimilars();
     
    /** Funcion que devuelve el numero de similares de un idioma determinado
     *  Esta funcion cuenta los similares en el idioma que recibe como parametro
     *  y devuelve cuantos habia.
     *
     * @param _language El idioma en el que contar sus similares
     * @returns El numero de similares en ese idioma
     */
    unsigned int CountSimilars(short _language);
    
    /** Funcion que devuelve el numero de imagenes de la pagina cargada
     *  Esta funcion tan solo devuelve el numero de imagenes que se han encontrado en la 
     *  pagina, es decir, el numero de <img src="..."> que contiene.
     * 
     * @returns El numero de imagenes que contiene la pagina
     */ 
    unsigned int N_Images();
    
    /** Funcion que devuelve el identificador asociado al fichero en disco de esta pagina
     *  Esta funcion devuelve el numero de fichero que representa esta pagina dentro
     *  de las que comparten el idioma
     *
     * @returns el identificador de fichero o 0 si no existiera
     */
    unsigned int LocalFileId();
    
    /** Funcion que devuelve el tamaño de la pagina cargada
     *  Esta funcion se limita a devolver el tamaño de la variable size
     *
     * @returns el tamaño del fichero
     */
    unsigned int Size();
    
    /** Funcion que devuelve el tamaño de la pagina cargada contando solo bytes de texto
     *  Esta funcion se limita a devolver el tamaño de la variable textsize, que contiene
     *  el numero de bytes del contenido de la pagina no asociados a etiquetas y distintos
     *  de blancos
     *
     * @returns el tamaño del fichero
     */
    unsigned int TextSize();
    
    /** Funcion que devuelve la URL de la pagina cargada
     *  Esta funcion se encarga de concatenar los campos asociados a la url (domain,
     *  path, filename, fileext y fileparam) y devolver la URL completa.
     *
     * @returns la url asociada al fichero o NULL si no hay un fichero cargado
     */
    char* PageFullURL();
        
    /** Funcion que devuelve el contenido de la pagina web cargada
     *  Esta funcion se encarga de devolver el contenido de la pagina si estuviese
     *  cargada o NULL si no lo estuviera. El contenido no podra ser modificado.
     *
     * @returns el contenido de la pagina web asociada
     */
    const char* Content();
    
    /** Funcion que fija el idioma que corresponde a la pagina cargada
     *  Esta funcion tan solo fija el idioma en que se ha estimado que esta la pagina 
     *  web
     *
     * @param _language identifica el idioma estimado de la pagina cargada
     */  
    void SetLanguage(short _language);
  
    /** Funcion que devuelve el domain asociado al objeto
     *  Esta funcion se limita a devolver un puntero a la variable domain del objeto
     *
     * @returns el puntero del domain del objeto
     */
    const char* Domain();
    
    /** Funcion que devuelve el path asociado al objeto
     *  Esta funcion se limita a devolver un puntero a la variable path del objeto
     *
     * @returns el path del objeto
     */
    const char* Path();
    
    /** Funcion que devuelve el filename asociado al objeto
     *  Esta funcion se limita a devolver un puntero a la variable filename del objeto
     *
     * @returns el puntero al filename del objeto
     */
    const char* Filename();
    
    /** Funcion que devuelve la extension asociada al objeto
     *  Esta funcion se limita a devolver un puntero a la variable fileext del objeto
     *
     * @returns la extension de la url asociada al objeto
     */
    const char* Extension();
 
    /** Funcion que devuelve los parametros asociados a la url del objeto
     *  Esta funcion se limita a devolver un puntero a la variable fileparam del objeto
     *
     * @returns los parametroe de la url asociados al objeto
     */
    const char* Parameters();    
    
    /** Funcion que recibe una url e indica si la url pertenece a la misma que la pagina
     *  Esta funcion recibe una url y comprueba si dicha url pertenece al mismo dominio
     *  en el que esta la pagina web cargada, en cuyo caso devolvera false, o si pertenece
     *  a otro dominio, caso en el que devolvera true
     *
     * @param _url cadena con la url a comprobar
     * @returns true si la url es externa o false si es interna
     *
     */
    bool ExternalLink(const char* _url);

    /** Funcion que simplemente elimina el contenido de la pagina web
     *  Esta funcion libera la memoria reservada para el contenido de la pagina web
     */    
    void CleanContent();
    
    /** Funcion que elimina los datos de la cabecera que no se almacenan en disco
     *  Esta funcion libera la memoria reservada al procesar la pagina que no es
     *  relevante para compararla con otras y que no se almacena en disco
     *  Los campos que libera son: links, pero sin poner n_links a 0. Si se van
     *  a utilizar estos campos es NECESARIO procesar de nuevo la pagina.
     */
    void FreeUselessFields();
     
    /** Funcion que añade un guesser a la lista de guessers cuyo nombre se indica
     *  Esta funcion añade un guesser al vector de guessers con el nombre indicado
     *  y rellena sus datos sobre los idiomas con los que trabajo y sus resultados
     * 
     * @param _guessername Nombre del guesser cuya puntuacion se esta fijando
     * @param _nlanguages numero de idiomas que usa el guesser
     * @param _languages los identificadores de los idiomas que usa el guesser
     * @param _points las probabilidades asignadas a dichos idiomas
     */
    void AddNewGuesser(const char* _guessername,unsigned int _nlanguages,unsigned short* _languages,const float* _points);
    
    /** Funcion que indica cuantos idiomas tiene en cuenta determinado guesser y cuales son sus probabilidades
     *  Esta funcion establece el numero de idiomas que tiene en cuenta un guesser determinado
     *  y cuales son esos idiomas, asi como las probabilidades que se han asignado a cada uno
     *  de ellos
     *
     * @param _guessername identificador del guesser cuya puntuacion se esta fijando
     * @param _nlanguages numero de idiomas que usa el guesser
     * @param _languages los identificadores de los idiomas que usa el guesser
     * @param _points las probabilidades asignadas a dichos idiomas
     */
    void SetProbsGuesser(const char* _guessername,unsigned int _nlanguages,unsigned short* _languages,const float* _points);
    
    /** Funcion añade una nueva aplicacion a la lista de aplicaciones
     *  Esta funcion incrementa el numero de aplicaciones que han procesado la pagina
     *  mediante la insercion de una nueva aplicacion cuyo nombre y version se
     *  pasan como parametros
     *
     * @param _appname nombre de la aplicacion a insertar
     * @param _appversion version de la aplicacion a insertar
     */
    void AddApplication(const char* _appname,const char* _appversion);
    
    /** Funcion que devuelve el contenido de la pagina tras filtrar todo lo que no es texto
     *  Esta funcion filtra todas las etiquetas y comentarios asi como el
     *  contenido de las etiquetas style y script, ya que no se considera 
     *  texto. Después devuelve el resultado de dicho filtrado.
     */
    char* FilterContent();
 };

#endif
