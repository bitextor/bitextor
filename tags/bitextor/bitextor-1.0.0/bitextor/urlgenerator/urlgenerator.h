/*
   Clase que se encarga de generar variaciones de las cadenas
   que recibe, en base a unos patrones que se establecen. Se
   puede usar como generador de urls, y los patrones ser las
   cadenas asociadas al idioma del contenido de las urls.
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#ifndef URLGENERATOR_H
#define URLGENERATOR_H

//#define probando
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include <util/languageutils.h>

#define kSizeAuxStrings 3000

#define kMaxNumberPatterns 10000
#define kMaxGenerations 100 //Indica el numero maximo de urls similares que se pueden generar
#define kMaxChangesInVariation 100 //Indica el numero maximo de cambios que se pueden aplicar por variacion

typedef struct ChangePattern
 {
  short language; //Idioma de la url en el que se puede aplicar este patron
  char *text; //Texto a intercambiar
  unsigned short nchanges; //Numero de intercambios posibles
  char **changes; //Texto con el que se reemplaza
 };

class URLGenerator
 {
  unsigned int nchangepatterns; //Numero de patrones de que se disponen
  ChangePattern *changepatterns; //Patrones a aplicar a las urls
      
  /** Funcion que libera la memoria asociada a los patrones
   *  Esta funcion libera toda la memoria reservada para conter los patrones
   *  de generacion de urls de entre los que se incluyen los que se han cargado
   *  de disco y los aprendidos.
   */
  void ResetPatterns();
    
  public:

    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     */
    URLGenerator();
        
    /** Constructor de la clase que carga el fichero 
     * Inicializa todos los atributos a los valores del fichero que recibe
     *
     * @param _file Nombre y ruta del fichero con el que se construye
     */
    URLGenerator(const char* _file);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~URLGenerator();    
      
    /** Funcion que carga los patrones de reemplazo desde un fichero
     *  Esta funcion carga los patrones almacenados en un fichero con el formato:
     *
     *  <lang_code> <source_pattern> -> <target_pattern1> <target_pattern2>...
     *  ... 
     * 
     *  <lang_code> es el numero asociado al idioma en el que esta la url en la que se busca el patron
     *  <source_pattern> es una cadena de texto entre comillas con el texto a buscar para reemplazar
     *  <target_patternX> son cadenas de texto entre comillas con el texto a reemplazar al darse el patron
     *  <
     *  Un ejemplo de fichero seria:
     *  
     *   4 "ga" -> "es" "en" "ca" "pt" ""
     *   0 "es/" -> "ga/"
     *
     *  Y un ejemplo de aplicacion de GenerateVariants con ese fichero ante la url
     *  "www.dlsi.ua.es/ga/gallego.html" devolveria:
     *   - "www.dlsi.ua.es/es/esllego.html"
     *   - "www.dlsi.ua.es/en/enllego.html"
     *   - "www.dlsi.ua.es/ca/callego.html"
     *   - "www.dlsi.ua.es/pt/ptllego.html"
     *   - "www.dlsi.ua.es//llego.html"
     *   - "www.dlsi.ua.ga/ga/gallego.html"
     *
     * @param filename nombre del fichero que contiene los patrones
     * @returns True si el fichero exisstia y su formato era coherente o False sino
     */
    bool Load(const char* _filename);
    
    /** Funcion que guarda los patrones establecidos en memorias
     *  Esta funcion guarda en disco duro toda la informacion relevante del
     *  objeto en el formato preestablecido. Si el fichero ya existiera sera
     *  sobreescrito sin preguntar.
     * 
     * @param _file Nombre y ruta del fichero que se genera en disco duro
     * @returns True si no hubo problemas al generar el fichero o False si los hubo
     *     
    */
    bool Save(const char* _filename);
    
    /** Funcion que genera variantes de la url para generar urls similares
     *  Esta funcion utiliza las heuristicas de patrones y las estadisticas de n-gramas
     *  aprendidas para tratar de generar urls similares a la que recibe como parametro.
     *  El funcionamiento es buscar ngramas frecuentes y generar urls alternativas
     *  reemplazandolos por ngramas frecuentes con el mismo n pero de diferentes idiomas.
     *  Esta version reemplaza en la url todas las veces que aparezca el patron por el
     *  cambio a realizar. Por ejemplo ante el patron y url:
     *       es "es" -> "en" ""     www.ua.es/es/esteban.html
     *  el resultado seria las siguientes 2 variaciones:
     *       www.ua.en/en/enteban.html
     *       www.ua.//teban.html
     *
     * @param language es el idioma de la url de la que se buscan similares
     * @param url es la url a la que realizar variantes
     * @param numvariants en este parametro se devuelve el numero de urls generadas
     * @returns un vector de urls generadas
     */
    char** GenerateVariants1(short _language,const char* _url,unsigned int* _numvariants);

    /** Funcion que genera variantes de la url para generar urls similares
     *  Esta funcion utiliza las heuristicas de patrones y las estadisticas de n-gramas
     *  aprendidas para tratar de generar urls similares a la que recibe como parametro.
     *  El funcionamiento es buscar ngramas frecuentes y generar urls alternativas
     *  reemplazandolos por ngramas frecuentes con el mismo n pero de diferentes idiomas.
     *  Esta version reemplaza en la url cada una de las las veces que aparezca el patron
     *  por el cambio a realizar en diferentes variaciones. Por ejemplo ante el patron
     *  y url:
     *       es "es" -> "en" ""     www.ua.es/es/esteban.html
     *  el resultado seria las siguientes variaciones:
     *       www.ua.en/es/esteban.html
     *       www.ua.es/en/esteban.html
     *       www.ua.es/es/enteban.html
     *       www.ua./es/esteban.html
     *       www.ua.es//esteban.html
     *       www.ua.es/es/teban.html
     *
     * @param language es el idioma de la url de la que se buscan similares
     * @param url es la url a la que realizar variantes
     * @param numvariants en este parametro se devuelve el numero de urls generadas
     * @returns un vector de urls generadas
     */
    char** GenerateVariants2(short _language,const char* _url,unsigned int* _numvariants);
 };

//Funcion de comparacion para el qsort
int CompareNGramFrequency(const void* a,const void* b);
 
#endif
