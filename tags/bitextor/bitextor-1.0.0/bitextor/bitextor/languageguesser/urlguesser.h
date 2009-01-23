/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto en base tan solo a su url. La adivinacion se basa en
   n-gramas tras un entrenamiento con una lista de ficheros en cada idioma
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#ifndef URLLANGGUESSER_H
#define URLLANGGUESSER_H

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

#define kSizeAuxStrings 3000
#define kMaxSizeNGrams 5 //Tamaño maximo de los n-gramas que se tienen en cuenta
#define kMinSizeNGrams 2 //Tamaño minimo de los n-gramas que se tienen en cuenta

#define kNumNgrams 100 //Numero de n-gramas frecuentes que se tienen en cuenta
#define kAjusteLG 10 //Constante para la ordenacion de n-gramas
#define kAceptacionLang 0.03 //Indica la relacion entre el idioma mas probable y el segundo para aceptar el idioma como bueno
#define kGenerateVariants 10 //Indica el numero de ngramas que se tienen en cuenta para generar variantes (debe ser menor de kNumNgrams)

#define kMaxNumberPatterns 10000
#define kMaxGenerations 100 //Indica el numero maximo de urls similares que se pueden generar
#define kMaxChangesInVariation 100 //Indica el numero maximo de cambios que se pueden aplicar por variacion

//Ctes para los idiomas
#define kLangES 0 //Español
#define kLangEN 1 //Ingles
#define kLangCA 2 //Catalan
#define kLangPO 3 //Portugues
#define kLangGA 4 //Gallego

/** Estructura que contiene un trigrama y su frecuencia en cada idioma */ 
typedef struct NGram
 {
  char *ngram; //Caracteres del n-grama
  float frequency; //Frecuencia del n-grama en cada idioma
 };

class URLLangGuesser
 {
  unsigned short numlanguages; /** Numero de lenguajes que se utilizan para comparar */
  unsigned short maxsizengrams; /** Hasta que tamaño se tienen en cuenta los n-gramas */
  bool *possiblelang; /** Indica entre que idiomas se discrimina */
  unsigned short numfrequents; /** Numero de ngramas mas frecuentes que se conservan para comparar */
  NGram ***frequents; /** Almacena los mas frecuentes ordenados por frecuencia, el primero
                        de cada lengua es el que representa al resto de trigramas (la frecuencia de los que
                        no estan), el primer indice es el idioma y el segundo el tamaño del
                        n-grama */
  int **numngrams; /** Numero de ngramas en la estructura para cada idioma y tamaño */
  int **totalngrams; /** Numero total de ngramas que se han usado para entrenar por cada idioma y tamaño */
  
  float *pointslang; /** Puntos que tiene cada idioma de que la pagina sea de ese idioma */
  
  char auxstrings[kMaxSizeNGrams][kSizeAuxStrings]; /** Auxiliares para lectura de los ngramas */
  
  /** Funcion que normaliza el contenido de pointslang para que este entre 0 y 1
   *  Esta funcion suma todos los valores de las posiciones de pointslang y luego
   *  divide todos los valores por el valor de la suma, para de esa forma normalizar
   */  
  void Normalize();
  
  /** Funcion que compara dos conteos de ocurrencias para ver si tienen el mismo orden de magnitud
   *  Esta funcion compara que dos numeros ocurrencias en funcion del total sean
   *  comparables en el mismo orden de magnitud, es decir, que su frecuencia relativa sea
   *  similar.
   *
   * @param _freq1 primera frecuencia a comparar
   * @param _freq2 segunda frecuencia a comparar
   * @returns true si el orden de magnitud es similar o false en caso contrario.
   */
  bool SameMagnitude(float _freq1,float _freq2);  
  
  public:

    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     */
    URLLangGuesser(unsigned short _nlanguages);
        
    /** Constructor de la clase que carga el fichero 
     * Inicializa todos los atributos a los valores del fichero que recibe
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     * @param _file Nombre y ruta del fichero con el que se construye
     */
    URLLangGuesser(unsigned short _nlanguages,const char* _file);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~URLLangGuesser();    
      
    /** Funcion que guarda los ngramas calculados en el disco duro
     *  Esta funcion guarda en disco duro toda la informacion relevante del
     *  objeto en el formato preestablecido. Si el fichero ya existiera sera
     *  sobreescrito sin preguntar.
     * 
     * @param _file Nombre y ruta del fichero que se genera en disco duro
     * @returns True si no hubo problemas al generar el fichero o False si los hubo
     *     
    */
    bool Save(const char* _file);
    
    /** Funcion que carga el fichero de ngramas del disco duro
     *  Esta funcion recibe la ruta completa y el nombre del fichero
     *  que corresponde a los ngramas a cargar.
     * 
     * @param _file Nombre y ruta del fichero que se va a cargar del disco duro
     * @returns True si no hubo problemas al cargar el fichero o False si los hubo
    */
    bool Load(const char* _file);        
    
    /** Funcion que recibe una url y devuelve el idioma de la misma
     *  Esta funcion recibe una url y estima en que idioma estara su pagina
     *  asociada.
     * 
     * @param _file URL que se va a procesar para sacar su idioma
     * @returns El identificador del idioma del fichero o -1 si no se ha podido decidir
    */
    short GuessText(const char* _url);
    
    /** Funcion que entrena el guesser de ngramas de forma no iterativa
     *  Esta funcion recibe el nombre de un fichero que contendra una lista
     *  de URLs y el idioma al que pertenecen, y se encarga de aprender los
     *  n-gramas mas frecuentes.
     *
     * @param _file nombre del fichero de aprendizaje
     * @param _language identificador del idioma
     * @returns false si ha habido un error o sino true
     */
    bool Train(const char* _file,short _language);
    
    /** Funcion que reduce el numero de ngramas de cada lengua para quedarse con los significativos
     *  Esta funcion extrae los ngramas que pudieran discriminar y descarta aquellos cuya
     *  frecuencia es similar en varios idiomas.
     */     
    void ReduceNgrams();
        
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

    //Incremental training (ir aprendiendo segun llegan urls)
    
    
    /** Funcion que analiza los n-gramas mas frecuentes y genera posibles variaciones en las url
     *  Esta funcion analiza los n-gramas mas probables de cada idioma y genera patrones
     *  de reemplazo entre ellos, para que sean probados por el generador de urls.
     *
     * @param filename nombre del fichero de patrones que se genera
     * @returns True si el fichero se genero correctamente o False en otro caso
     */
    bool GeneratePatterns(const char* _filename);
    
    /** Funcion que devuelve la puntuacion que ha obtenido cada idioma
     *  Esta funcion devuelve un vector con las probabilidades de pertenencia a
     *  cada idioma.
     *
     * @returns la puntuacion obtenida por cada uno de los idiomas
     */    
    const float* PointsPerLanguage();

 };

//Funcion de comparacion para el qsort
int CompareNGramFrequency(const void* a,const void* b);
 
#endif

//Ultimo cambio: quitar numngrams para recorrer los ngramas en disco
