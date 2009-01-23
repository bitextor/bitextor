/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004-05
*/

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <libxml/xmlreader.h>
#include "../webpage/webpage.h"
#include "../downloadmanager/downloadmanager.h"
#include "../urlhash/urlhash.h"
#include "../orderedlist/orderedlist.h"
#include "../languageguesser/langguesser.h"
#include "../languageguesser/trigramguesser.h"
#include "../languageguesser/wordguesser.h"
#include "../languageguesser/urlguesser.h"
#include "../urlgenerator/urlgenerator.h"

//#define TRAZANDO_Collector //Comentar para evitar la traza
#define HIDEMESSAGES //Comentar para no ver los mensajes de informacion sobre la descarga

#define kVeryBigValue 999999999 //Un valor enorme
#define kSizeAuxStrings 3000
#define kMaxApplications 100
#define kMaxSimilares 500
#define kSecondsBetweenRequest 0.1 /** Segundos que se esperan entre peticiones (1 segundo cada n peticiones) */
#define kMinimalContentLength 4000 /** Numero minimo de bytes que debe contenter */

#define kNumPagesperLanguage 5000 /** Tamaño inicial del vector de paginas para cada idioma */
#define kNumPagesperLanguageLevel 500 /** Tamaño inicial del vector de paginas para cada nivel de cada idioma */

//Constantes para los guessers
#define kRangeAcceptingLanguage 0.1 /** Diferencia entre los puntos asignados al idioma mas probable y al segundo para dar por bueno el resultado */
#define kNumberofGuessers 3 /** Numero de guessers configurados */
#define kIdTrigramGuesser 0 /** Identificador del guesser en cuestion */
#define kIdWordGuesser 1 /** Identificador del guesser en cuestion */
#define kIdURLGuesser 2 /** Identificador del guesser en cuestion */

//Constantes para las heuristicas
#define kRangeSize 0.1 /** Relacion del tamaño de la pagina mayor que debe ser menor la pequeña */
#define kAbsRangeSize 1000000 /** Maxima diferencia en el tamaño en texto de los ficheros para considerarlos semejantes */
#define kRangeNTags 0.1 /** Relacion entre el numero de etiquetas de la pagina con mas que debe ser menor la pequeña */
#define kAbsRangeNtags 1000000 /** Maxima diferencia en el numero de etiquetas de los ficheros para considerarlos semejantes */
#define kRangeNTagsLevel 0.4 /** Diferencia maxima entre el numero de etiquetas de un determinado nivel de la pagina con mas que debe ser menor la pequeña */
#define kAbsRangeNTagsLevel 40 /** Diferencia maxima entre el numero de etiquetas de un determinado nivel de la pagina con mas que debe ser menor la pequeña */
#define kAbsRangeTreeLevel 2 /** Diferencia de profundidad en el arbol */
#define kRangeLinks 5 /** Diferencia entre el numero de links */
#define kAbsRangeLinks 1000000 /** Maxima diferencia en el numero de enlaces de los ficheros para considerarlos semejantes */
#define kRangeImages 5 /** Diferencia entre el numero de imagenes */
#define kAbsRangeImages 1000000 /** Maxima diferencia en el numero de imagenes de los ficheros para considerarlos semejantes */

#define kNameURLList "urllist" /** Nombre base del fichero en el que almacena la lista de urls en cada idioma */
#define kNameStatsFile "resultsfile" /** Nombre del fichero en el que se almacenan las estadisticas del procesado de una web */
#define kTestLoadFile "testing.xml" /** Nombre del fichero que se utiliza para comprobar si un fichero es valido en XML */


/** Clase que representa el robot recolector de bitextos
 *  Esta clase se encarga de, dado un fichero con una lista de URLs, ir recorriendolas
 *  en busca de bitextos. 
 */ 
class Collector
 {
  DownloadManager dmanager; /** Objeto para el manager de descarga de paginas */
  bool quickmode; /** Indica si el almacenamiento de similares se hace en ambos ficheros
                      (false) o solo en uno (true). Si solo se almacena en uno, sera
                      necesario utilizar la aplicacion buildsimilars tras acabar el
                      recolector para tener un corpus consistente */
  bool onlycached; /** Indica si esta permitido el acceso a internet (false) o si solo se puede usar la cache (true)*/
  
  LangGuesser<TrigramLangGuesser> *trigramguesser; /** Adivinador de idioma basado en trigramas */
  LangGuesser<WordLangGuesser> *wordguesser; /** Adivinador de idioma basado en palabras privativas */
  LangGuesser<URLLangGuesser> *urlguesser; /** Adivinador de idioma basado en URLs */
  double guesserweight[kNumberofGuessers]; /** Peso de cada guesser si se combina su uso */
  bool applyguessers[kNumberofGuessers]; /** Guessers que se aplican */
  unsigned int urlguesserstart; /** Numero de urls leidas antes de que empiece el guesser basado en urls */
  
  URLHash *urlhash; /** Objeto para la lista de urls que recorremos */
  OrderedList2Levels *orderedlist; /** Objeto para comprobar tamaños de las paginas descargadas */
  char **langurl; /** Lista con los idiomas con los que esta trabajando el robot */
  unsigned int n_languages; /** Numero de idiomas con los que trabaja el robot */
  unsigned int *n_files; /** Numero de ficheros que se han descargado en cada uno de los idiomas */
  char **urllist; /** Lista de url que que se consideran para la busqueda de similares */
  WebPage **internalurllist; /** Auxiliar para los dominios que se consideran para comprobar si son externos */
  unsigned int n_urllist; /** Numero de url que contiene la lista de urls a recorrer */
  unsigned int n_urlactual; /** Numero de url que ya se han recorrido */
  
  unsigned int n_possiblelanguages; /** Numero de idiomas asignables a la url en proceso */
  unsigned short *possiblelanguages; /** Identificadores de los idiomas asignables */
  unsigned short *langid; /** Identificador que se asocia a cada idioma procesable */
  
  FILE **filelisturl; /** Ficheros que almacenan la lista de URLs que se van visitando */
  
  //Variables para las heuristicas
  double minimalcontentsize; /** Tamaño minimo en bytes de texto de una pagina para tenerla en cuenta */
  double rangesize; /** Diferencia maxima entre tamaños de dos fichero para considerarlos semejantes */
  double *normalizedsize; /** Tamaño normalizado asociado a cada idioma en referencia al resto para compararlos */
  double absrangesize; /** Diferencia absoluta maxima entre tamaños para considerarlos semejantes */
  double rangentags; /** Diferencia maxima entre numeros de etiquetas de dos ficheros para considerarlos semejantes */
  double absrangentags; /** Diferencia absoluta maxima entre numero de etiquetas para considerarlos semejantes */
  double rangentagslevel[kMaxLevelTags]; /** Diferencia maxima entre numeros de etiquetas por nivel entre dos ficheros para cosiderarlos semejantes */
  double absrangentagslevel[kMaxLevelTags]; /** Diferencia absoluta maxima entre numeros de etiquetas por nivel entre dos ficheros para cosiderarlos semejantes */  
  int absrangetreelevel; /** Diferencia maxima entre la profundidad en la ruta de directorios*/
  double rangenlinks; /** Diferencia entre el numero de links de las paginas */
  double absrangenlinks; /** Diferencia absoluta maxima entre numero de enlaces para considerarlos semejantes */
  double rangenimages; /** Diferencia entre el numero de imagenes de las paginas */
  double absrangenimages; /** Diferencia absoluta maxima entre numero de imagenes para considerarlos semejantes */
  int **languagepages; /** Mantiene una lista de paginas en cada idioma */
  int *n_sizelanguagepages; /** Indica el tamaño de la estructura de lista de paginas para cada idioma */
  int *n_languagepages; /** Indica el numero de paginas que tenemos para cada idioma */  
  int ***langpagesperlevel; /** Mantiene una lista de paginas en cada idioma y por nivel */
  int **n_sizelangpagesperlevel; /** Indica el tamaño de la estructura de paginas para cada idioma y nivel */
  int **n_langpagesperlevel; /** Indica el numero de paginas para cada idioma y nivel */
  
  //Peso de cada heuristica en el calculo de la distancia entre paginas
  float weightsize; /** Peso de la heuristica de tamaño de los ficheros */
  float weightntags; /** Peso de la heuristica de numero de etiquetas */
  float weightnlinks; /** Peso de la heuristica de numero de enlaces */
  float weightdepth; /** Peso de la heuristica de profundidad en el arbol de directorios */
  float weightnimages; /** Peso de la heuristica de numero de imagenes */
  float weightntagslevel; /** Peso de la heuristica de numero de etiquetas por nivel en el arbol de etiquetas */
  
  //Variables que indican si se utilizan o no las heuristicas
  bool applysize; /** Heuristica de tamaño de los ficheros */
  bool applyntags; /** Heuristica de numero de etiquetas */
  bool applynlinks;/** Heuristica de numero de enlaces */
  bool applydepth; /** Heuristica de profundidad en el arbol de directorios */
  bool applynimages; /** Heuristica de numero de imagenes */
  bool applyntagslevel; /** Heuristica de numero de etiquetas por nivel en el arbol de etiquetas */
  bool applyextension; /** Heuristica de coincidencia en la extension del fichero */
  unsigned short nappliedheuristics; /** Numero de heuristicas que se aplican */
  
  
  bool applyurlgenerator;
  URLGenerator *urlgenerator; /** Objeto que se encarga de la generacion de URL similares a una dada en base a patrones */
  unsigned short generationtype; /** Establece la forma de generar variantes (ver URLGenerator) */
  
  //Variables estadisticas
  unsigned int downloaded; /** Numero de paginas descargadas */
  unsigned int discardedlang; /** Numero de paginas descartadas por no saber su idioma */
  unsigned int discardedsize; /** Numero de paginas descartadas por su pequeño tamaño */
  unsigned int errordownloaded; /** Numero de paginas que no se han podido descargar*/
  unsigned int diffbysize; /** Numero de paginas que se han discriminado por diferencia en el tamaño */
  unsigned int diffbytags; /** Numero de paginas discriminadas por diferencia en el numero de etiquetas */
  unsigned int diffbytagslevel; /** Numero de paginas discriminadas por diferencia en el numero de etiquetas por nivel*/
  unsigned int diffbytreelevel; /** Numero de paginas discriminadas por profundidad en la ruta de directorios */
  unsigned int diffbylinks; /** Numero de paginas discriminadas por numero de enlaces */
  unsigned int diffbyimages; /** Numero de paginas discriminadas por numero de imagenes */  
  unsigned int diffbyext; /** Numero de paginas discriminadas por diferencia en su extension*/
  unsigned int n_similars; /** Numero de paginas que se han dado por similares */
  unsigned int *n_pagesperlanguage; /** Numero de paginas clasificadas en cada idioma */
  unsigned int n_repeated; /** Numero de paginas descartadas por estar repetidas */
  
  unsigned int totalurlgenerated; /** Numero total de urls que se han generado para comparar */
  unsigned int totalgoodgenerations; /** Numero de urls generadas que se encontraron en la cache */

  xmlTextReaderPtr reader; /** Variable para la carga del fichero de configuracion con la libreria LIBXML2 */  
  
  //Variables para la descarga actualizadora
  bool updatingcollection; /** Indica si la coleccion se esta actualizando (true) o si se esta generando de 0 (false) */
  char *statsfilename; /** Contiene la ruta del fichero de estadisticas que se generara o cargara */
  char *statsfilename2; /** Contiene la ruta del fichero de estadisticas que se generara como backup */
  int *oldn_languagepages; /** Indica el numero de paginas que teniamos para cada idioma en ejecuciones previas */
  unsigned int processedpreviously; /** Indica el numero de paginas descargadas y que se procesaron en anteriores ejecuciones */
  unsigned int olddiffbysize; /** Numero de paginas que se discriminaron por diferencia en el tamaño en ejecuciones previas */
  unsigned int olddiffbytags; /** Numero de paginas que se discriminaron por diferencia en el numero de etiquetas en ejecuciones previas */
  unsigned int olddiffbytagslevel; /** Numero de paginas que se discriminaron por diferencia en el numero de etiquetas por nivel en ejecuciones previas */
  unsigned int olddiffbytreelevel; /** Numero de paginas que se discriminaron por profundidad en la ruta de directorios en ejecuciones previas */
  unsigned int olddiffbylinks; /** Numero de paginas que se discriminaron por numero de enlaces en ejecuciones previas*/
  unsigned int olddiffbyimages; /** Numero de paginas que se discriminaron por numero de imagenes en ejecuciones previas */
  unsigned int olddiffbyext; /** Numero de paginas que se discriminaron por diferencia en su extension en ejecuciones previas */
  unsigned int oldn_similars; /** Numero de paginas que se han dado por similares en ejecuciones previas */
  int **oldn_langpagesperlevel; /** Numero de paginas en cada idioma que se han encontrado por nivel en ejecuciones previas */
  
  unsigned int alreadyprocessed; /** Numero de paginas descargadas que ya se habian procesado en ejecuciones previas */
  char previousdate[kSizeAuxStrings],newdate[kSizeAuxStrings]; /** Indica las fechas de la ejecucion actual y la anterior */
  
  URLHash *oldurlhash; /** Objeto para la lista de urls que recorrimos en ejecuciones previas */
  OrderedList2Levels *oldorderedlist; /** Objeto para comprobar tamaños de las paginas descargadas de ejecuciones previas */
  
  unsigned int extrastat; /** Util para evaluar y testear pruebas del robot */
    
  /** Funcion que comprueba que una url pertenezca a uno de los dominios que se estan descargando
   *  Esta funcion comprueba si la url que recibe como parametro esta contenida en 
   *  alguna de las que el robot esta descargando (urllist). De esta forma se determina
   *  si una url es externa (no se procesara) o interna.
   *
   * @param _url que se quiere comprobar
   * @returns True si la url era externa (no interesante) o False si se considera interna
   */
  bool IsExternalLink(const char* _url);
  
  /** Funcion que inserta una pagina en la lista de paginas de un determinado idioma
   *  Esta funcion recibe un numero identificador de la pagina dentro del arbol de
   *  paginas y el idioma en el que esta y se inserta en la lista de paginas en ese
   *  idioma.
   *   
   * @param _page Identificador de la pagina dentro del arbol de ficheros
   * @param _language Idioma en el que esta dicha pagina
   * @param _treelevel Nivel del arbol que ocupa la pagina
   */  
  void InsertLanguagePage(int _page,int _language,unsigned int _treelevel);
    
  /** Funcion que indica si dos paginas son similares en base a las heuristicas aplicadas
   *  Esta funcion recibe dos paginas web y se encarga de aplicar heuristicas para
   *  tratar de diferenciarlas. Si las heuristicas no dicen que las paginas no pueden
   *  ser traduccion la una de la otra, entonces es que podrian serlo. Las heuristicas
   *  buscan diferencias en base a unos rangos prefijados en:
   *    - Numero de etiquetas por cada nivel del arbol   
   *    - Numero de links
   *    - Tamaños de ficheros
   *    - Numero de etiquetas
   *    - Profundidad en la ruta de directorios
   *    - Extension de los ficheros
   *    - Numero de imagenes
   *   
   * @param _a Pagina a comparar
   * @param _b Pagina a comparar
   * @param _bplace Identificador de la segunda pagina dentro del arbol de ficheros
   * @returns Su distancia heuristica si podrian ser similares o -1 si no lo son
   */    
  float Similars(WebPage* _a,WebPage* _b,unsigned int _bplace);
  
  /** Funcion que trata de adivinar el idioma de un texto
   *   Esta funcion utiliza la clase adivinadora de idiomas para establecer el
   *   idioma asociado a un texto que recibe como parametro. La comunicacion
   *   con el guesser esta basada en un fichero temporal
   *
   * @param _webpage pagina web cuyo idioma se quiere identificar
   * @returns el identificador del idioma estimado del texto
   */  
  short GuessLanguage(WebPage* _webpage);
  
  /** Funcion que se asegura de que una pagina web exista y sino intenta reconstruirla 
   *  Esta funcion trata de reconstruir la url que corresponde al link que se
   *  recibe como primer parametro para asegurar que la url exista. Si no existiera
   *  intenta concatenar dominio y path de la pagina que recibe como segundo parametro
   *  Ademas si la url es una llamada a javascript o alguna otra directiva que
   *  no representa un enlace entonces devolvera NULL.
   *
   * @param _url Cadena con la url del link que queremos comprobar
   * @param _webpage Pagina web de la que procede el link
   * @returns una url existente, bien la original o una adaptada o NULL si no existe
   */  
  char* CheckURL(const char* _url,WebPage* _webpage);

  /** Funcion que indica si dos paginas son iguales comparando sus datos
   *  Esta funcion recibe dos paginas web y se encarga de comparar sus valores
   *  estadisticos para tratar de diferenciarlas. Si las estadisticas no permiten
   *  descartar, entonces se asumira que son iguales. Las comparaciones se
   *  realizan en el siguiente orden:
   *    - Tamaños de ficheros
   *    - Numero de etiquetas
   *    - Numero de links
   *    - Numero de imagenes
   *
   * @param _a Pagina a comparar
   * @param _b Pagina a comparar
   * @returns True si se pueden considerar iguales o False si no lo son
   */    
  bool Equals(WebPage* _a,WebPage* _b);  
  
  /** Funcion que comprueba si una pagina ha sido descargada previamente o es nueva
   *  Esta funcion permite el control de la descarga de paginas repetidas por
   *  encontrarse incluidas con mas de una URL. Para controlar esto se dispone
   *  de una estructura de listas ordenadas con los tamaños para las paginas, 
   *  de forma que solo se compararan mas en profundidad si coinciden exactamente
   *  en tamaño
   *
   * @param _a Pagina a comprobar
   * @returns True si la pagina esta repetida o False si no esta
   */
  bool IsRepeatedWebpage(WebPage* _a);
  
  /** Funcion que permite rodear el error de la libreria
   *  Esta funcion trata de paliar el error que da la libreria libXML2 en
   *  determinadas maquinas, al no poder realizar la conversion de UTF-8 a
   *  ISO8859-1. Es recomendable utilizar la propia libreria siempre que
   *  funcione.
   *
   *  @param a Cadena a transformar
   *  @returns la cadena una vez transformada
   */
  inline char* FixLibXML2Encoding(xmlChar const *a)
   {
    return(XMLToLatin1(a)); //Si no hay problemas se puede usar esta
/*    //Cuando la libreria falla se puede usar este codigo aunque no se realiza
    //la conversion
    if(a!=NULL)
      return(strdup((char*)a));
     else
       return NULL;*/
   }
   
  public:
      
    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     */
    Collector();
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~Collector();
    
    /** Funcion que activa o desactiva el modo rapido
     *  Esta funcion permite que el procesado de paginas al encontrar paginas
     *  similares sea mucho mas rapido al no actualizar la pagina similar sino
     *  tan solo la que se esta procesando. Esto significa que el corpus generdo
     *  tendra similares en un solo sentido, es decir a es similar de b, pero b
     *  no de a. Para arreglar esto se requiere el uso de la aplicacion auxiliar
     *  buildsimilars que recorre el corpus estableciendo el doble similar.
     *
     * @param value indica si se activa (True) o desactiva (False) el modo rapido
     */
    inline void QuickMode(bool value)
     { quickmode=value; }
    
    /** Funcion que activa o desactiva la generacion de una cache de paginas descargadas
     *  Esta funcion indica al sistema que mantenga todas las paginas que descarga
     *  de la web en cache para acelerar futuras ejecuciones del robot, al evitar
     *  descargar varias veces las mismas paginas.
     *
     * @param value indica si se activa (True) o desactiva (False) la generacion de cache
     */
    inline void CacheGeneration(bool value)
     { dmanager.CacheGeneration(value); }

    /** Funcion que activa o desactiva el uso de la cache de paginas descargadas
     *  Esta funcion indica al sistema que busque las paginas que va a descargar
     *  en la cache, para evitar descargarlas de nuevo si ya estuvieran.
     *
     * @param value indica si se activa (True) o desactiva (False) la utilizacion de la cache
     */
    inline void CacheRead(bool value)
     { dmanager.CacheRead(value); }
     
    /** Funcion que activa o desactiva el uso de internet en la descarga de paginas
     *  Esta funcion indica al sistema si solo debe utilizar la cache (true) o si tambien
     *  puede acceder a internet para la descarga de paginas (false)
     *
     * @param value indica si se prohibe (True) o se permite (False) el acceso a internet
     */
    inline void OnlyCache(bool value)
     { dmanager.OnlyCache(value); }
          
    /** Funcion que establece que solo se comparen paginas con URL similares
     *  Esta funcion carga el fichero de patrones que se utilizara para generar
     *  variaciones de URL y le dice al sistema que no pruebe cada pagina con
     *  el resto de las paginas sino tan solo con las variantes que se encuentren
     *  entre las paginas previamente descargadas
     *
     * @param _patternsfile Nombre del fichero que se va a cargar del disco duro
     * @returns True si el fichero existia o False si el fichero no se pudo cargar
     */
    bool CompareOnlyURLPatterns(char* _patternsfile);
    
    /** Funcion que establece el modo 1 de generacion de variantes
     *  Esta funcion establece que se utilice el modo 1 de generacion de variantes
     *  que consiste en reemplazar para cada patron todas sus ocurrencias de golpe
     *  en la variante que se genera
     */
    inline void GenerationType1()
     { generationtype=1; }
    
    /** Funcion que establece el modo 2 de generacion de variantes
     *  Esta funcion establece que se utilice el modo 2 de generacion de variantes
     *  que consiste en reemplazar para cada patron todas sus ocurrencias de una en
     *  una y por tanto generar muchas mas variantes
     */
    inline void GenerationType2()
     { generationtype=2; } 

    /** Funcion que carga el fichero de configuracion del robot del disco duro
     *  Esta funcion recibe el nombre del fichero a cargar con la configuracion
     *  por defecto del robot (que despues se podra modificar con las opciones
     *  de linea de comandos). 
     *   
     *  El formato del fichero es XML con la siguiente
     *  estructura general:
     *
     *   <collector>
     *     <languages>
     *       <language code="..." normalizedsize="..."/>
     *       <language code="..." normalizedsize="..."/>
     *       . . . 
     *     </languages>
     *     <heuristics>
     *       <size apply="..." generalsize="..." minimal="..." range="..." absrange="..."/>
     *       <tags apply="..." range="..." absrange="..."/>
     *       <links apply="..." range="..." absrange="..."/>
     *       <images apply="..." range="..." absrange="..."/>
     *       <tagslevel apply="...">
     *         <level value="..." range="..." absrange="...">
     *         <level value="..." range="..." absrange="...">
     *         . . .
     *       </tagslevel>
     *     </heuristics>
     *     <urlgeneration>
     *       <generation apply="..." type="..." filename="..."/>
     *     </urlgeneration>
     *     <languageguesser discarding="...">
     *       <guesser type="trigram" apply="..." weight="..." filename="..."/>
     *       <guesser type="word" apply="..." weight="..." filename="..."/>
     *       <guesser type="url" apply="..." weight="..." filename="..."/>
     *       . . .
     *     </languagegueser>
     *     <mode>
     *       <quickmode apply="..."/>
     *       <cache build="..." use="..."/>
     *     </mode>
     *   </collector>
     *
     *  
     * @param _file Nombre del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool LoadConfiguration(const char* _file);
    
    /** Funcion que carga el fichero de URLs del disco duro
     *  Esta funcion recibe el nombre del fichero a cargar con la lista de dominios
     *  que el robot debe recorrer en su proceso de busqueda de bitextos.
     *
     *  El formato del fichero debe ser el siguiente (reemplazando <algo> por
     *  el valor adecuado, no es XML):
     *  
     *    <lang_1> ... <lang_m>
     *    <url_principal>
     *    <url_extra_1>
     *     ...
     *    <url_extra_n>
     *
     *  En el que la url principal indica el sitio web a descargar, y los enlaces
     *  a otras webs se permiten solo si estan en la lista de urls extra. Ademas el
     *  identificador de idiomas solo distinguira entre los idiomas especificados.
     * 
     * @param _file Nombre del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool Load(const char* _file);
    
    /** Funcion que establece que la descarga se realice desde la url indicada
     *  Esta funcion establece la url de la web de la que descargara el robot. Todos
     *  los enlaces a otras web no seran permitidos en la descarga. Ademas, dado que
     *  no se han especificado idiomas. El identificador de idiomas trabajara con
     *  todos los idiomas de los que dispone.
     *
     * @param _url La url de donde descargar el corpus
     * @returns True si la url era valida
     */
    bool LoadURL(const char* _url);

    /** Funcion que una vez cargadas las url busca todos los posibles textos en ellas
     *  Esta es la funcion que implementa el funcionamiento del colector, es decir,
     *  se encarga de recorrer las url cargadas y recolectar los textos que sean
     *  relevantes.
     *  
     *  @returns true si la web se ha analizado con exito y false si hay algun error
     */
    bool Collect();
    
    /** Funcion que empieza el proceso de recopilacion de textos
     *  Esta funcion se encarga de ir recorriendo los sitios web indicados en el 
     *  fichero de urls, para ir recopilando los bitextos.
     *
     * @returns true si no se han dado errores en la recopilacion y false en caso contrario
     */
    bool Start();
    
    /** Funcion que devuelve el directorio en el que descarga las paginas el robot
     *  Esta funcion devuelve el valor de urllist[0] que especifica el directorio
     *  en el que se almacenan todas las paginas descargadas por el robot
     * 
     * @returns El directorio de descarga del robot o NULL si no esta aun definido
     */
    char* DownloadingDirectory();
    
    /** Funcion que muestra las estadisticas de funcionamiento del collector
     *  Esta funcion se encarga de mostrar por la salida estandar, las estadisticas del
     *  proceso que se llevan durante el mismo.
     */    
    void Stats();

    /** Funcion que almacena las estadisticas de funcionamiento del collector en un fichero en formato XML
     *  Esta funcion construye un fichero XML con la informacion de estadisticas del
     *  robot, de forma que permita posteriormente recomenzar la recoleccion de
     *  bitextos.
     *
     * @param _filename El fichero en el que almacenar las estadisticas
     * @returns True si no hubo problemas en la gestion del fichero o False si los hubo
     */    
    bool Stats(const char* _filename);
        
    /** Funcion que resetea las estadisticas de funcionamiento del colector
     *  Esta funcion reinicializa a 0 las estadisticas del proceso de recoleccion
     */
    void ResetStats();
    
    /** Funcion que restaura el proceso de recopilacion de textos y lo reinicia para actualizar una coleccion
     *  Esta funcion se encarga de cargar un sitio web previamente descargado y de volver a descargarlo para
     *  incorporar las novedades a la solucion.
     *
     * @param _website La web cuyo corpus se quiere actualizar
     * @returns true si no se han dado errores en la recopilacion y false en caso contrario
     */
    bool RefreshWebsite(const char* _website);
    
    /** Funcion que comprueba las paginas que han sido descargadas para ver si la pagina es nueva o ya estaba
     *  Esta funcion busca en la coleccion de paginas descargadas la pagina que
     *  recibe como parametro para ver si ha sido descargada o no. Devuelve
     *  True si la pagina ya fue descargada o sino devuelve False.
     *
     * @param _webpage La pagina a comprobar si ya habia sido descargada
     * @returns True si la pagina ya habia sido descargada o False en otro caso
     */
    bool PreviouslyDownloaded(WebPage* _webpage);

    /** Funcion que carga el fichero de resultados de una ejecucion previa del robot
     *  Esta funcion recibe el fichero de resultados que genero una ejecucion previa del robot
     *  tras completar su ejecucion. Este fichero contiene la informacion imprescindible para
     *  actualizar la coleccion generada previamente.
     *   
     *  El formato del fichero es XML con la siguiente
     *  estructura general:
     *
     *    <pacostats href="...">
     *      <nonexternalwebsites>
     *        <website href="">
     *        <website href="">
     *        . . .
     *      </nonexternalwebsites>
     *      <targetlanguages>
     *        <tlanguage code="..." webpages="...">
     *        <tlanguage code="..." webpages="...">
     *        . . .
     *       </targetlanguages>
     *      <downloading success="..." failure="..."/>
     *      <discarded unknownlang="..." insufficientsize="..." repeated="..."/>
     *      <urlgeneration type="..." coincidences="..." generated="..."/>
     *      <notsimilars tagslevel="..." links="..." size="..." tags="..." extension="..."/>
     *      <similars value="..."/>
     *    </pacostats>
     *  
     * @param _resultsfile Nombre del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
     */
    bool LoadResultsFile(const char* _file);
    
    /** Funcion que actualiza una coleccion existente con la nueva informacion disponible en la web
     *  Esta funcion implementa el funcionamiento del colector, pero en el modo de
     *  completar una coleccion previamente descargada. Inicialmente carga las cabeceras
     *  de todos los ficheros de la coleccion que ya han sido descargados y despues
     *  
     *  @returns true si la web se ha analizado con exito y false si hay algun error
     */
    bool RECollect();

    /** Funcion que recopila todos los ficheros descargados en las ejecuciones anteriores en un directorio nuevo
     *  Esta funcion se encarga de copiar todos los ficheros XML que forman parte del corpus
     *  en el directorio que se recibe como parametro. De esta forma, dichos ficheros podran
     *  alterarse sin afectar a las posteriores actualizaciones del corpus.
     *
     * @param _website El directorio en el que el robot ha recopilado la informacion desde el website
     * @param _directory El directorio en el que almacenar el corpus completo
     * @returns true si no se han dado errores en la recopilacion y false en caso contrario
     */
    bool GenerateCorpus(const char* _website,const char* _directory);
    
    /** Funcion que procesa la etiqueta <collector> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <collector> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessCOLLECTOR();

    /** Funcion que procesa la etiqueta <languages> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <languages>
     *  y de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessLANGUAGES();
    
    /** Funcion que procesa la etiqueta <language> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <language> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessLANGUAGE();
    
    /** Funcion que procesa la etiqueta <heuristics> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <heuristics> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessHEURISTICS();

    /** Funcion que procesa la etiqueta <size> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <size> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessSIZE();
    
    /** Funcion que procesa la etiqueta <tags> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <tags> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessTAGS();

    /** Funcion que procesa la etiqueta <links> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <links> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessLINKS();

    /** Funcion que procesa la etiqueta <depth> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <depth> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessDEPTH();
    
    /** Funcion que procesa la etiqueta <images> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <images> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessIMAGES();

    /** Funcion que procesa la etiqueta <extension> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <extension> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessEXTENSION();
                
    /** Funcion que procesa la etiqueta <tagslevel> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <tagslevel> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessTAGSLEVEL();
    
    /** Funcion que procesa la etiqueta <level> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <level> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessLEVEL();

    /** Funcion que procesa la etiqueta <urlgeneration> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <urlgeneration> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessURLGENERATION();

    /** Funcion que procesa la etiqueta <generation> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <generation> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessGENERATION();

    /** Funcion que procesa la etiqueta <languageguesser> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <languageguesser> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessLANGUAGEGUESSER();

    /** Funcion que procesa la etiqueta <guesser> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <guesser> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessGUESSER();

    /** Funcion que procesa la etiqueta <mode> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <mode> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessMODE();

    /** Funcion que procesa la etiqueta <quickmode> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <quickmode> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessQUICKMODE();

    /** Funcion que procesa la etiqueta <cache> durante la carga del fichero de configuracion (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <cache> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessCACHE();
    
    
    /** Funcion que procesa la etiqueta <pacostats> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <pacostats> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessPACOSTATS();

    /** Funcion que procesa la etiqueta <nonexternalwebsites> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <nonexternalwebsites> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessNONEXTERNALWEBSITES();

    /** Funcion que procesa la etiqueta <targetlanguages> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <targetlanguages> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessTARGETLANGUAGES();

    /** Funcion que procesa la etiqueta <downloading> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <downloading> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessDOWNLOADING();

    /** Funcion que procesa la etiqueta <discarded> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <discarded> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessDISCARDED();

    /** Funcion que procesa la etiqueta <urlgenerationapplied> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <urlgenerationapplied>
     *  y de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessURLGENERATIONAPPLIED();

    /** Funcion que procesa la etiqueta <notsimilars> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <notsimilars> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessNOTSIMILARS();

    /** Funcion que procesa la etiqueta <similars> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <similars> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessSIMILARS();

    /** Funcion que procesa la etiqueta <website> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <website> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessWEBSITE();

    /** Funcion que procesa la etiqueta <tlanguage> durante la carga del fichero de estadisticas (LIBXML2)
     *  Esta funcion se encarga de leer el contenido interno de la etiqueta <tlanguage> y
     *  de almacenarlo apropiadamente. Devuelve False si hay algun error en el fichero
     *  o True si todo fue correcto. La funcion asume que se esta leyendo un fichero
     *  y ya esta abierto y listo.
     * 
     * @returns False si hubo algun error o True en caso contrario
     */
    bool ProcessTLANGUAGE();
 };

#endif
