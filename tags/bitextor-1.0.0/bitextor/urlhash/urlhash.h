/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#ifndef urlhash_H
#define urlhash_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

//#define TRAZANDO_URLHash //Comentar para evitar la traza

#define kSizeAuxStrings 3000
#define kSizeHash 500000
#define kRedisperseHash 10001
#define kIncSizeTree 10000
#define kMaxDeepTree 200
#define kFirstSizeQueue 25000

typedef struct URLContent
 {
  bool visited; /** Indica si la URL ha sido ya descargada o no */
  char *urlcontent; /** Parte de la URL que contiene el nodo */
  void *content; /** Enlace al contenido del nodo */
 };

typedef struct NodeQueue
 {
  int place; /** Lugar dentro de la tabla hash que ocupa el nodo actual */
  NodeQueue *sig; /** Nodo siguiente de la cola */
 };
 
/** Clase que representa una estructura hash de urls dentro de una web
 *  Esta clase se utiliza para asignar identificadores a cada fichero, de forma
 *  que dada cualquier url sepamos si ya se le ha asociado un fichero en disco o no
 *  La destruccion de objetos de esta clase no borra el content de los nodos del
 *  arbol.
 */ 
class URLHash
 {
  unsigned int sizevector; //Tamaño de memoria reservado para el vector
  URLContent **hash; //Vector hash con las paginas
  unsigned int urlnumber; //Numero de paginas incluidas en el vector hash de paginas
  
  NodeQueue *queuehead; //Cabeza de la cola de urls
  NodeQueue *queuetail; //Final de la cola de urls (util para recorridos en anchura)
  unsigned int urlsinqueue; //Numero de nodos que quedan en la cola
  
  NodeQueue *levelshead[kMaxDeepTree]; //Punteros a los lugares en los que se inserta para cada nivel de profundidad en el arbol de directorios (para recorridos en base al nivel de profundidad)
  unsigned int depthhead; //Profundidad a la que se encuentra el primer elemento
  
  /** Funcion que libera toda la estructura hash y la cola
   *  Esta funcion libera la memoria asociada al objeto que exista
   *  actualmente.
   */
  void Reset();

  /** Funcion hash que transforma una cadena de caracteres en un entero
   *  Esta es la funcion hash que se utiliza para la insercion de paginas
   *  en la web
   *
   * @param cad Cadena a la que aplicarle la funcion hash
   * @returns el resultado de la funcion hash
   */
  int HashFunction(const char* _cad);
  
  /** Funcion que arregla una url para quitar directorios incorrectos como "." o ".."
   *  Esta funcion reeemplaza la cadena "/./" por "/" y "/_/../" por "/" siendo "_" un
   *  nombre cualquiera de directorio. La idea es no almacenar esas rutas parciales
   *  dentro de las url puesto que no representan nada.
   *
   * @param _cad URL a arreglar
   * @returns la URL corregida o una copia de la original si estaba bien
   */
  char* FixURL(const char* _cad);
  
  public:
      
    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     */
    URLHash();
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~URLHash();    
        
    /** Funcion que inserta una url a la hash y lo mete en la cola
     *  Esta funcion asigna un identificador de fichero a la url y lo 
     *  inserta en la cola de urls por nivel
     *
     * @param _url URL del fichero a insertar en el arbol
     * @param _content contenido asociado al nodo
     * @returns el identificador asociado a la url
     */
    unsigned int InsertPerDepth(const char* _url,void* _content);

    /** Funcion que inserta una url a la hash y lo mete al final de la cola
     *  Esta funcion asigna un identificador de fichero a la url y lo 
     *  inserta al final de la cola
     *
     * @param _url URL del fichero a insertar en el arbol
     * @param _content contenido asociado al nodo
     * @returns el identificador asociado a la url
     */
    unsigned int InsertEnd(const char* _url,void* _content);
        
    /** Funcion que busca en la hash una determinada url y devuelve su posicion
     *  Esta funcion busca la url que recibe como parametro en la hash
     *  y si la encuentra devuelve su identificador, y sino devuelve 0
     *
     * @param _url URL del fichero a buscar en el arbol
     * @returns el identificador asociado a la url o 0 si no estaba
     */
    unsigned int Search(const char* _url);
  
    /** Funcion que accede en la hash a una determinada posicion y devuelve su url
     *  Esta funcion accede a la posicion de la hash indicada y devuelve
     *  la url de dicha posicion si es valida o 0 si no es valida
     *
     * @param _url posicion en la hash a la que se quiere acceders
     * @returns la url asociada al identificador o NULL si no existia
     */
    char* Search(unsigned int _idurl);
    
    /** Funcion que devuelve el primer nodo recorriendo por anchura que este sin procesar
     *  Esta funcion realiza un recorrido en anchura del arbol hasta encontrar un
     *  nodo que corresponda a una url completa y que no se haya procesado aun. Si
     *  el arbol ya se hubiese procesado por completo, entonces el resultado seria
     *  0.
     *
     * @returns el identificador del siguiente nodo a procesar o 0 si ya no quedan
     */    
    unsigned int NextNode();    
    
    /** Funcion que devuelve el contenido asociado a un nodo en concreto
     *  Esta funcion recibe un identificador de url y devuelve la url que
     *  esta asociada a dicho identificador o NULL si no existiera
     * 
     * @param _idurl el identificador del nodo cuyo contenido se quiere usar
     * @returns la URL del fichero buscado o NULL si no estaba
     */
    void* Content(unsigned int _idurl);
    
    /** Funcion que pone a NULL el contenido de una url determinada
     *  Esta funcion tan solo pone a NULL el contenido de una url del arbol que ha sido
     *  previamente borrada (o deberia sino se quiere dejar memoria sin liberar)
     *
     * @param _idurl el identificador del nodo cuyo contenido se quiere usar
     */    
    bool SetContentNULL(unsigned int _idurl);
    
    /** Funcion que devuelve el numero de urls que contiene la hash
     *  Esta funcion tan solo devuelve el valor de urlnumber, para saber el 
     *  numero de urls que se han insertado en el mismo
     *
     * @returns el numero de urls contenidas en la hash
     */    
    unsigned int NURLs();
    
    /** Funcion que devuelve el tamaño de la tabla hash
     *  Esta funcion tan solo devuelve el valor de sizevector, para saber el 
     *  tamaño reservado para la tabla hash
     *
     * @returns el tamaño de la hash 
     */    
    unsigned int Size();
    
    /** Funcion que devuelve el tamaño actual de la cola de urls
     *  Esta funcion tan solo devuelve el valor de urlsinqueue, para saber el 
     *  numero de urls que quedan por recorrer
     *
     * @returns el tamaño de cola de urls por recorrer
     */    
    unsigned int QueueSize();
    
    /** Funcion que devuelve la profundidad en el arbol de una determinada url
     *  Esta fundion realiza la busqueda del elemento en el arbol y devuelve
     * su profundidad en la ruta de directorios que representa el arbol
     *
     * @param _url URL del fichero a determinar su profundidad
     * @returns la profundidad o 0 si no estaba
     */    
    unsigned int Depth(const char* _url);

    /** Funcion que devuelve la profundidad en el arbol de una determinada url
     *  Esta fundion realiza la busqueda del elemento en el arbol y devuelve
     * su profundidad en la ruta de directorios que representa el arbol
     *
     * @param _idurl posicion del nodo a comprobar su profundidad
     * @returns la profundidad o 0 si no estaba
     */    
    unsigned int Depth(unsigned int _idurl);        
 };

#endif
