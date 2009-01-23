/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2005
   
   Este codigo fuente contiene la recopilacion de funciones
   especificas no incluibles dentro de una clase determinada.
*/

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include "../util/genericutils.h"

//#define TRAZANDO_DownloadManager //Comentar para evitar la traza
#define HIDEMESSAGES //Comentar para ver los mensajes de informacion sobre la descarga

#define kSizeBufferUtils 50000
#define kSizeInt 30
#define kTimeWaitingWGET 25
#define kSizeAuxStrings 3000
#define kMaxSizeNameFile 1000

#define kLocalDirectory "cache/" //Directorio en el que se almacena la cache de paginas descargadas

#define kDefaultFileName "index.html" 

#define kMaxSizeCacheURL 250 //Tamaño maximo de una URL para que se almacene con su nombre real en la cache
#define kSizeCacheURLHash 500000
#define kRedisperseHash 10001 
#define kURLRenamePrefix "renamed" //Prefijo con el que se guardan los ficheros renombrados
#define kHashfilename "hashinfo" //Nombre del fichero con los datos de la hash de urls largas

//Definir para no mostrar la traza de lo que esta haciendo wget
//#define HIDEMESSAGES

class DownloadManager
 {
  unsigned int cachefaults; //Numero de fallos de cache
  unsigned int cachehits; //Numero de aciertos de cache
  unsigned int downloadedpages; //Numero de paginas descargadas
  unsigned int notfoundpages; //Numero de paginas no encontradas
  
  bool cacheread; //Indica si se busca las paginas en la cache o no
  bool cachegeneration; //Indica si se almacena la url en la cache de urls o no
  bool onlycache; //Indica si esta prohibido el acceso a internet o si se permite
 
  char **bigurlhash; //Hash de urls que son tan largas que hay que guardarlas en cache con otro nombre
  unsigned int sizebigurlhash; //Tamaño de la hash de renombrado
  unsigned int n_urlsinhash; //Numero de urls que se han introducido en la tabla
  char *bigurlhashfilename; //Nombre del fichero que se utiliza para cargar/guardar la tabla hash
  
  char *scriptspath; //Directorio en el que se encuentran los scripts
  
  /** Funcion hash que transforma una cadena de caracteres en un entero
   *  Esta es la funcion hash que se utiliza para la tabla hash de ficheros cuya url
   *  es demasiado larga como para guardarla tal cual en el sistema de ficheros.
   *
   * @param str Cadena a la que aplicarle la funcion hash
   * @returns el resultado de la funcion hash
   */
  int HashFunction(const char* _str);
  
  /** Funcion que inserta una url en la hash
   *  Esta funcion asigna un identificador de fichero a la url y la
   *  introduce en la hash
   *
   * @param _url URL del fichero a insertar en la hash
   * @returns el identificador asociado a la url o 0 si hay error
   */
  unsigned int InsertHashCache(const char* _url);
  
  /** Funcion que busca en la hash una determinada url y devuelve su posicion
   *  Esta funcion busca la url que recibe como parametro en la hash
   *  y si la encuentra devuelve su identificador, y sino devuelve 0
   *
   * @param _url URL del fichero a buscar en la hash
   * @returns el identificador asociado a la url o 0 si no estaba
   */
  unsigned int SearchHashCache(const char* _url);
  
  /** Funcion que almacena en disco la hash de urls largas
   *  Esta funcion guarda en el disco duro la hash de urls largas que se
   *  esta utilizando para poder restaurarla en la proxima ejecucion.
   *  El formato del fichero debera tener en cada linea el identificador
   *  de la url (posicion en la hash), un espacio y por ultimo la url.
   *
   * @param _filename Fichero en disco en el que se grabara el fichero
   * @returns True si no hubo problemas o False en caso contrario
   */
  bool SaveHashCache(const char* _filename);
  
  /** Funcion que carga de disco la hash de urls largas
   *  Esta funcion carga del disco duro la hash de urls largas que se
   *  almaceno previamente.
   *
   * @param _filename Fichero en disco en el que se grabara el fichero
   * @returns True si no hubo problemas o False en caso contrario
   */
  bool LoadHashCache(const char* _filename);
  
  public:
  
    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     */
    DownloadManager();
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~DownloadManager();
    
    /** Funcion que reinicia los valores estadisticos del download manager    
     *  Esta funcion pone a 0 todos los contadores que constituyen las estadisticas
     *  del proceso de descarga de paginas web
     */
    void Reset();
    
    /** Esta funcion le indica al downloadmanager si debe almacenar paginas nuevas en cache o no
     *  Esta funcion establece el valor de la variable cachegeneration, que indica si se genera
     *  una cache de las paginas que se descargan o si una vez descargadas y utilizadas se borran.
     *
     * @param activation Indica si se activa o se desactiva este modo de funcionamiento
     */
    void CacheGeneration(bool activation)
     { cachegeneration=activation; }
    
    /** Esta funcion le indica al downloadmanager si debe utilizar la cache o si siempre descarga paginas
     *  Esta funcion establece si al descargar una pagina se busca la pagina en la cache o si se hace como
     *  si no existiera la cache y directamente se descarga la pagina web de internet.
     *
     * @param activation Indica si se activa o se desactiva este modo de funcionamiento
     */
    void CacheRead(bool activation)
     { cacheread=activation; }
     
    /** Esta funcion le indica al downloadmanager si debe utilizar el acceso a internet o no
     *  Esta funcion establece si al descargar una pagina se accede a internet o si simplemente
     *  se devuelve que no está cuando no se encuentra en la cache.
     *
     * @param activation Indica si se activa o se desactiva este modo de funcionamiento
     */
    void OnlyCache(bool activation)
     { onlycache=activation; }
    
    
    /** Funcion que devuelve el tamaño del fichero que indica una url o -1 si no existe
     *  Esta funcion recibe una url y calcula el tamaño del fichero asociado, si el
     *  fichero no existiese entonces su tamaño pasara a ser -1. Las urls que empiezan
     *  por "java..." o "mailto:" o "https" no se procesaran y se devolvera que no se encuentran.
     *  Ademas esta funcion solo trabaja con ficheros de texto, de forma que si el fichero no
     *  es del tipo [text/html] o [text/plain] entonces nunca encontrara el fichero.
     *
     * @param _url indica el origen del fichero cuyo tamaño queremos conocer
     * @returns el tamaño del fichero o -1 si no se ha podido acceder o si no existe
     */
    int SizeURL(const char* _url);

    /** Funcion que almacena en disco una url utilizando wget para descargarla
     *  Esta funcion recibe la url a descargar y la almacena en el disco local en
     *  la ruta que se especifica como segundo parametro. La funcion devuelve -1 si
     *  el fichero no existiese o no se pudiese descargar.
     *
     * @param _url indica el origen del fichero cuyo tamaño queremos conocer
     * @param _localfile indica se guardara el fichero en disco duro
     * @returns true si no hubo problemas o false si no se pudo descargar y/o guardar en disco
     */
    bool DownloadURL(const char* _url,const char* _localfile);
 
    /** Funcion que comprueba que la extension que se recibe como parametro sea de un fichero de texto
     *  Esta funcion se encarga de discriminar las extensiones que interesan al robot de las
     *  que no interesan. Entre las que interesan se encuentran ("asc", "asp", "cgi", "html",
     *  "htm", "jsp", "php", "txt", "xml") y tambien los ficheros sin extension ("").
     *  
     * @param _ext la extension a validar.
     * @returns True si la extension es aceptada o False si no lo es.
     */
    bool IsGoodExtension(const char* _ext);
    
    /** Funcion que devuelve el numero de fallos de cache que se han producido
     *  Esta funcion tan solo devuelve el valor de la variable cachefaults, que contiene
     *  el numero de fallos de cache que se han producido durante la ejecucion actual del
     *  programa.
     *
     * @returns el numero de fallos de cache que se han producido
     */
    unsigned int CacheFaults()
     { return(cachefaults); }
    
    /** Funcion que devuelve el numero de aciertos de cache que se han producido
     *  Esta funcion tan solo devuelve el valor de la variable cachehits, que contiene
     *  el numero de aciertos de cache que se han producido durante la ejecucion actual del
     *  programa.
     *
     * @returns el numero de fallos de cache que se han producido
     */
    unsigned int CacheHits()
     { return(cachehits); }
    
    /** Funcion que devuelve el numero de paginas que no estaban ni en cache ni en internet
     *  Esta funcion tan solo devuelve el valor de la variable notfoundpages, que contiene
     *  el numero de paginas que no se han podido devolver puesto que no han podido ser
     *  localizadas.
     *
     * @returns el numero de fallos de cache que se han producido
     */
    unsigned int NotFoundPages()
     { return(notfoundpages); }
     
    /** Funcion que devuelve una medida de la eficacia de la cache de paginas
     *  Esta funcion calcula la eficacia de la cache en funcion de los valores 
     *  de fallos y aciertos de cache.
     *
     * @returns la eficacia de la cache
     */
    float CacheAccuracy()
     {
      if(cachefaults+cachehits>0) 
        return(((float)cachehits)/(cachehits+cachefaults)); 
       else
         return(0);
     }
 };
 
#endif
