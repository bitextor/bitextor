/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano. La adivinacion se basa en palabras frecuentes
   caracteristicas de un solo idioma
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#ifndef WORDLANGGUESSER_H
#define WORDLANGGUESSER_H

//#define probando

//Si queremos que el entrenamiento no sea incremental comentar la siguiente linea
//y arreglar la implementacion que no funciona bien (si descomentas da SIGSEGV)
#define incrementaltraining

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "../util/memoryutils.h"
#include "../util/languageutils.h"

#define kSizeAuxStrings 3000
#define kSizeBuffer 100000
#define kAceptacionLang 0.03 //Indica la relacion entre las probabilidades del idioma mas probable y del segundo para aceptar el idioma como bueno
#define kMinPointsDifference 50 //Indica cuantos puntos mas ha de tener al menos el idioma mas probable respecto del segundo para aceptar el idioma

#define kIniSizeHash 1000000 //Tamaño inicial de la hash para el diccionario
#define kMaxFullHash 0.4 //Tamaño maximo de la hash que puede estar llena antes de redimensionarla 

/** Funcion que lee un float de un fichero y detecta errores.
 *  Esta funcion lee caracteres del fichero de texto hasta acabar de leer
 *  un float y lo devuelve. El caracter tras el float es desechado. Si el
 *  float no fuera correcto escribe un error por pantalla y devuelve 0.
 * 
 * @param _file fichero del que leer, que debera estar listo para su lectura
 * @returns el float leido de fichero o 0 si hay error.
 */
float ReadFloat(FILE* _file);

typedef struct DictionaryWord
 {
  char *word; //La palabra en si que contiene
  bool *languages; //Idiomas a los que pertenece
  unsigned int *occurences; //Veces que se ha encontrado la palabra para cada idioma  
 };
 
class WordLangGuesser
 {
  unsigned short numlanguages; /** Numero de lenguajes que se utilizan para comparar */
  bool *possiblelang; /** Indica entre que idiomas se discrimina   */
  
  DictionaryWord **hashdict; /** Hash para la lista de palabras del diccionario */
  unsigned int *wordsperlang; /** Numero total de palabras en cada idioma*/
  double *moduleperlang; /** Modulo para cada idioma que permite establecer su probabilidad para cada idioma */
  unsigned int dictsize; /** Tamaño de la tabla hash usada para el diccionario */
  unsigned int dictused; /** Numero de palabras contenidas en el diccionario */
  unsigned int dictocur; /** Numero de palabras contando repetidas que se han insertado */
  
  float *pointslang; /** Puntos que tiene cada idioma de que la pagina sea de ese idioma */
  unsigned int colissions; /** Numero de colisiones en las inserciones en la hash */
  
  char auxstrings[3][kSizeAuxStrings];/** Auxiliar para la lectura de fichero */
  
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
  
  /** Funcion que asigna a una palabra una posicion de la hash
   *  Esta funcion devuelve la posicion que le corresponde a la palabra dentro de
   *  la tabla hash asociada al diccionario
   * 
   * @param _word Palabra con la que trabajar
   * @param tamvect Tamaqo del vector hash en el que se busca la posicion
   * returns la posicion dentro de la tabla hash o -1 si ha habido algun error
   */
  int HashFunction(const char* _word,int _size);

  /** Funcion que redistribuye la palabra en caso de colision
   *  Esta funcion se encarga de redistribuir la palabra si la
   *  posicion que le asocia la funcion hash ya estuviera
   *  ocupada por otra.
   *
   * @param hash es la posicion hash a redistribuir
   * @param tries es el numero de intentos que se llevan hechos 
   * @param size es el tamaño total del vector
   * @returns la nueva posicion dentro de la tabla
   */
  inline int RedisperseFunction(int _hash,int _tries,int _size)
   {
    return((_hash+_tries)%_size);
   };
     
  /** Funcion que inserta una nueva palabra en la hash del diccionario
   *  Esta funcion se encarga de añadir una nueva palabra al diccionario
   *  e indicar a que idioma pertenece. Si la palabra ya estuviese en el diccionario
   *  pues se aumenta su frecuencia en el idioma indicado, y si no estuviese con el
   *  idioma actual pues se añade.
   *  Cuando la hash se empieza a quedar pequeña, se crea otra mayor y se reinsertan
   *  las palabras de la antigua
   *
   * @param word Palabra a insertar en el diccionario
   * @param language Idioma en el que esta la palabra
   * @param occurences Numero de ocurrencias de esta palabra a sumar a las que ya hubiera
   */  
  bool InsertDictionaryWord(const char* _word,short _language,unsigned int _occurences=1 );
  
  /** Funcion quuee busca una palabra en la tabla hash del diccionario
   *  Esta fnncion recibe unaa palabra y la busca en la tabla hash
   *  asociada al diccionario. La informacion de la palabra contenida
   *  en el diccionario se devuelve, o NULL si la palabra no se encuentra
   *
   * @param word Palabra a buscar
   * @returns la posicion de la hash ocupada por la palabra oo NULL si no esta
   */  
  const DictionaryWord* SearchWord(const char* _word);
  
  inline int hash_string(const char* s)
   {
    unsigned long long h;
    for(h=0;*s;s++)
      h=37*h+*s;
    return int(h);
   }

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
   * @param _ocur1 numero de ocurrencias del primer conteo
   * @param _total1 numero total de ocurrencias relacionadas con el primer conteo
   * @param _ocur2 numero de ocurrencias deel segundo conteo
   * @param _total2 numero total de ocurrencias relacionadas con el segundo conteo
   * @returns true si el orden de magnitud es similar o false en caso contrario.
   */
  bool SameMagnitude(int _ocur1,int _total1,int _ocur2,int _total2);  
  
  public:

    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     */
    WordLangGuesser(unsigned short _nlanguages);
        
    /** Constructor de la clase que carga el fichero 
     * Inicializa todos los atributos a los valores del fichero que recibe
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     * @param _file Nombre y ruta del fichero con el que se construye
     */
    WordLangGuesser(unsigned short _nlanguages,const char* _file);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~WordLangGuesser();    
      
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
    
    /** Funcion que recibe un fragmento te texto y devuelve su idioma
     *  Esta funcion recibe el fragmento de texto y aplica las heuristicas
     *  de conteo de palabras privativas en funcion del diccionario de palabras
     *  cargado.
     * 
     * @param _text Fragmento de texto que se va a procesar para sacar su idioma
     * @returns El identificador del idioma del fichero o -1 si no se ha podido decidir
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
    bool Train(const char* _file,short _language);
      
    /** Funcion que limpia la tabla hash para dejarla solo con las mas frecuentes
     *  Esta funcion recorre la tabla hash de palabras y se queda solo con las
     *  palabras mas frecuentes en cada uno de los idiomas. Recibe como parametro
     *  el numero de palabras mas frecuentes que se consideraran en cada idioma. Devuelve
     *  true si ha encontrado ese numero de palabras para cada idioma o false si no.
     * 
     * @param _size el numero de palabras que queremos mantener de cada idioma
     * @returns false si algun idioma no tiene las palabras que se piden
     */
    bool CleanHash(unsigned int _size);
   
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
    
    /** Funcion que carga el fichero de parametros y le añade un idioma para ser entrenado posteriormente
     *  Esta funcion permite que el formato diseñado para almacenar los parametros
     *  pueda acoger nuevos idiomas en caso de que la variable kNumLanguages se haya
     *  incrementado previamente. El funcionamiento consiste en cargar un fichero con
     *  un idioma menos de los actuales y luego inicializar el nuevo idioma y grabar
     *  el fichero con el numero de idiomas actual.
     *
     * @param _file Nombre y ruta del fichero que se va a actualizar del disco duro
     * @returns True si no hubo problemas al actualizar el fichero o False si los hubo
     */
    bool AddNewLanguage(const char* _file);
 };

#endif
