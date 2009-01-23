/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano o etiquetado. La adivinacion se basa en trigramas
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#ifndef TRIGRAMLANGGUESSER_H
#define TRIGRAMLANGGUESSER_H

//#define probando

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include "../util/memoryutils.h"
#include "../util/languageutils.h"

#define TRAZANDO_TrigramLangGuesser

#define kSizeAuxStrings 3000
#define kSizeBuffer 100000
#define kNumFrequents 300 //Numero de frecuentes que se tienen en cuenta
#define kAjusteLG 10 //Constante para la ordenacion de trigramas
#define kAceptacionLang 0.03 //Indica la relacion entre las probabilidades del idioma mas probable y del segundo para aceptar el idioma como bueno
#define kMaxTrigramNumber 500000 //Numero maximo de trigramas que se analizan de un fichero


/** Funcion que lee un float de un fichero y detecta errores.
 *  Esta funcion lee caracteres del fichero de texto hasta acabar de leer
 *  un float y lo devuelve. El caracter tras el float es desechado. Si el
 *  float no fuera correcto escribe un error por pantalla y devuelve 0.
 * 
 * @param _file fichero del que leer, que debera estar listo para su lectura
 * @returns el float leido de fichero o 0 si hay error.
 */
float ReadFloat(FILE* _file);

/** Estructura que contiene un trigrama y su frecuencia
 * en cada idioma
 */ 
typedef struct Trigrama
 {
  char *a,*b,*c; //Caracteres del trigrama
  float frequency; //Frecuencia del trigrama en cada idioma
 };

class TrigramLangGuesser
 {
  unsigned short numlanguages; /** Numero de lenguajes que se utilizan para comparar */
  bool *possiblelang; /** Indica entre que idiomas se discrimina   */
  int numfrequents; /** Numero de trigramas mas frecuentes que se conservan para comparar */
  Trigrama **frequents; /** Almacena los mas frecuentes ordenados por frecuencia, el primero
                        de cada lengua es el que representa el resto de trigramas (los que
                        no estan) */
  int *ntrigrams; /** Numero de trigramas en la estructura para cada idioma */
  
  float *pointslang; /** Puntos que tiene cada idioma de que la pagina sea de ese idioma */
  
  char auxstrings[3][kSizeAuxStrings]; /** Auxiliares para lectura de los trigramas */
  
    
  /** Funcion que lee un caracter html (incluidas entidades) y lo devuelve
   *  Esta funcion lee un caracter y devuelve la cadena con el caracter. El
   *  caracter puede tambien estar formado por un &...;, pero ignora las
   *  etiquetas y los espacios en blanco, en cuyo caso devuelve NULL.
   * 
   * @param _file Nombre y ruta del fichero del que se lee el caracter
   * @returns la cadena con el caracter que ha leido o NULL si no es un caracter
   *     
   */  
  char* ReadCharacter(FILE* _file);
  
  /** Funcion que lee un caracter html (incluidas entidades) y lo devuelve
   *  Esta funcion lee un caracter y devuelve la cadena con el caracter. El
   *  caracter puede tambien estar formado por un &...;, pero ignora las
   *  etiquetas y los espacios en blanco, en cuyo caso devuelve NULL.
   * 
   * @param _file Puntero al texto del que se quiere leer un caracter
   * @param _increment Indica el numero de caracteres que se ha desplazado el texto
   * @param _where Indica en que posicion del trigrama almacenarlo
   * @returns la cadena con el caracter que ha leido o NULL si no es un caracter
   *     
   */  
  char* ReadCharacter(const char* _text,short* _increment,short where=0);
  
  /** Funcion que normaliza el contenido de pointslang para que este entre 0 y 1
   *  Esta funcion suma todos los valores de las posiciones de pointslang y luego
   *  divide todos los valores por el valor de la suma, para de esa forma normalizar
   */  
  void Normalize();  
  
  public:

    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     */
    TrigramLangGuesser(unsigned short _nlanguages);
        
    /** Constructor de la clase que carga el fichero 
     * Inicializa todos los atributos a los valores del fichero que recibe
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     * @param _file Nombre y ruta del fichero con el que se construye
     */
    TrigramLangGuesser(unsigned short _nlanguages,const char* _file);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~TrigramLangGuesser();
      
    /** Funcion que guarda los parametros en disco duro
     *  Esta funcion guarda en disco duro toda la informacion relevante del
     *  objeto en el formato XML establecido. Si el fichero ya existiera sera
     *  sobreescrito sin preguntar.
     * 
     * @param _file Nombre y ruta del fichero que se genera en disco duro
     * @returns True si no hubo problemas al generar el fichero o False si los hubo
     *     
    */
    bool Save(const char* _file);
    
    /** Funcion que carga el fichero de parametros del disco duro
     * Esta funcion recibe la ruta completa y el nombre del fichero
     * que corresponde a la pagina web que queremos cargar.
     * 
     * @param _file Nombre y ruta del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
    */
    bool Load(const char* _file);        
    
    /** Funcion que devuelve el idioma del texto que recibe o aplicando trigramas de letras
     * Esta funcion recibe un fragmento de texto y aplica las heuristicas basadas en los
     * trigramas de letras para deducir el idioma del texto
     * 
     * @param _file texto que se va a procesar para sacar su idioma
     * @returns El identificador del idioma del texto o -1 si no se ha podido decidir
    */
    short GuessText(const char* _text);
    
    /** Funcion que permite entrenar el guesser de trigramas de forma no iterativa
     * Esta funcion recibe el nombre de un fichero y el idioma al que pertenece
     * para aprender la frecuencia de sus trigramas. 
     *
     * @param _file nombre del fichero de aprendizaje
     * @param _language identificador del idioma
     * @returns false si ha habido un error o sino true
     */
    bool Train(const char* _file,unsigned short _language);
    
    /** Funcion que muestra por pantalla los trigramas y frecuencias asociadas
     * Esta funcion recibe un identificador de idioma y imprime por pantalla
     * la estructura de frecuencias de trigramas para dicho idioma
     *
     * @param _language identificador del idioma
     */    
    void PrintTrigrams(short _language);
   
   /** Funcion que fija los idiomas en los que pueden estar las paginas a discriminar su idioma
    *  Esta funcion establece los idiomas que se van a tener en cuenta a la hora al
    *  adivinar el idioma de las paginas. Recibe una cadena en la que cada caracter representa
    *  si se acepta o no cada idioma, de forma que si fuera "11111" se aceptarian todos, pero
    *  si fuera "01010" solo se aceptarian el segundo y el cuarto.
    * 
    * @param _languagestring cadena con los idiomas que se admiten expresada en ceros y unos
    * @returns false si la cadena no era valida o true en caso contrario
    */    
    bool SetPossibleLanguages(const char* _languagestring);
    
    /** Funcion que devuelve los idiomas en los que trabajan los guessers
     *  Esta funcion devuelve los identificadores de los idiomas que reconoce el guesser
     *  y el numero de ellos en el parametro que recibe como referencia
     *
     * @param _n_possiblelanguages es el parametro en el que se devuelve el numero de idiomas
     * @returns los identicadores de los idiomas en los que se trabajass
     */
    unsigned short* GivePossibleLanguages(unsigned int* n_possiblelanguages);    
    
    /** Funcion que devuelve la puntuacion que ha obtenido cada idioma
     *  Esta funcion devuelve un vector con las probabilidades de pertenencia a
     *  cada idioma.
     *
     * @returns la puntuacion obtenida por cada uno de los idiomas
     */
    const float* PointsPerLanguage();
 };

//Funcion de comparacion para el qsort
int CompareTrigramFrequency(const void* a,const void* b);
 
#endif
