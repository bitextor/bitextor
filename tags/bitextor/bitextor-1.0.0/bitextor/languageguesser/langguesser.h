/*
   Clase virtual que representa la funcionalidad basica de un adivinador 
   de idiomas. Todos los adivinadores de idiomas deberan implementar las
   funciones de esta clase abstracta
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2005
*/

#ifndef LANGGUESSER_H
#define LANGGUESSER_H

#include <stdio.h>

#define kMaxSizeFile 500000 //Tamaño maximo del fichero a adivinar su idioma que se utiliza

//#define probando
template <class Guesser>
class LangGuesser
 {
  unsigned short numlanguages; /** Numero de idiomas que se utilizan para comparar */
  Guesser *guesser; /** Enlace al guesser en cuestion */

  public:

    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     */
    LangGuesser(unsigned short _nlanguages);
        
    /** Constructor de la clase que carga el fichero 
     * Inicializa todos los atributos a los valores del fichero que recibe
     *
     * @param _nlanguages numero de idiomas con los que trabajar el guesser
     * @param _file Nombre y ruta del fichero con el que se construye
     */
    LangGuesser(unsigned short _nlanguages,const char* _file);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~LangGuesser();    
      
    /** Funcion que guarda los parametros en disco duro
     *  Esta funcion guarda en disco duro los parametros previamente obtenidos
     *  tras el entrenamiento para poder cargarlos posteriormente sin reentrenar
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
    
    /** Funcion que recibe un fichero y devuelve el idioma del fichero
     * Esta funcion recibe el nombre del fichero y aplica heuristicas para
     * deducir el idioma del texto
     * 
     * @param _file Fichero que contiene el corpus de entrenamiento
     * @param _language Idioma asociado al fichero de entrenamiento
     * @returns True si el entrenamiento tuvo exito o False en caso contrario
    */
    bool Train(const char* _file,unsigned short _language);
    
    /** Funcion que recibe un texto y devuelve el idioma asociado
     * Esta funcion recibe un texto y aplica heuristicas para
     * deducir el idioma en el que esta escrito
     * 
     * @param _text Fragmento de texto que se va a procesar para sacar su idioma
     * @returns El identificador del idioma del fichero o -1 si no se ha podido decidir
    */
    short GuessText(const char* _text);
    
    /** Funcion que recibe un fichero y devuelve el idioma del fichero
     * Esta funcion recibe el nombre del fichero y aplica heuristicas para
     * deducir el idioma del texto
     * 
     * @param _file Fragmento de texto que se va a procesar para sacar su idioma
     * @returns El identificador del idioma del fichero o -1 si no se ha podido decidir
    */
    short GuessFile(const char* _text);
    
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
    unsigned short* GivePossibleLanguages(unsigned int* _npossiblelanguages);    
    
    /** Funcion que devuelve la puntuacion que ha obtenido cada idioma aplicando un guesser
     *  Esta funcion devuelve un vector con las probabilidades de pertenencia a cada
     *  idioma, en funcion del guesser cuyo identificador recibe como parametro
     *
     * @param _idguesser Identificador del guesser cuya puntuacion se requiere
     */
    
    const float* PointsPerLanguage();
 };

template <class Guesser>
LangGuesser<Guesser>::LangGuesser(unsigned short nlanguages)
 {
  numlanguages=nlanguages;
  guesser=new Guesser(nlanguages);
 }

template <class Guesser>
LangGuesser<Guesser>::LangGuesser(unsigned short nlanguages,const char *file)
 {
  numlanguages=nlanguages;
  guesser=new Guesser(nlanguages,file);
 }
 
template <class Guesser>
LangGuesser<Guesser>::~LangGuesser()
 {
  if(guesser!=NULL)
   {
    delete guesser;
    guesser=NULL;
   }
 }

template <class Guesser>
bool LangGuesser<Guesser>::Load(const char* file)
 {
  return(guesser->Load(file));
 }

template <class Guesser>
bool LangGuesser<Guesser>::Save(const char* file)
 {
  return(guesser->Save(file));
 }
 
template <class Guesser>
bool LangGuesser<Guesser>::Train(const char* file,unsigned short language)
 {
  return(guesser->Train(file,language));
 }
 
template <class Guesser> 
short LangGuesser<Guesser>::GuessText(const char* text)
 {
  short result;
  
  result=guesser->GuessText(text);
  return(result);
 }

template <class Guesser> 
short LangGuesser<Guesser>::GuessFile(const char* file)
 {
  short result;
  FILE *fent;
  char buffer[kMaxSizeFile+1];
  int bytesleidos;

  if((fent=fopen(file,"r")))
   {
    bytesleidos=fread(buffer,1,kMaxSizeFile,fent);
    buffer[bytesleidos]='\0';
    fclose(fent);
    
    result=guesser->GuessText(buffer);
   }
   else
    {
     result=-1;
     fprintf(stderr,"Error: The input file could not be opened\n");
    }
  return(result);
 }

template <class Guesser>
bool LangGuesser<Guesser>::SetPossibleLanguages(const char* languagestring)
 {
  return(guesser->SetPossibleLanguages(languagestring));
 }

template <class Guesser>
unsigned short* LangGuesser<Guesser>::GivePossibleLanguages(unsigned int* npossiblelanguages)
 {
  return(guesser->GivePossibleLanguages(npossiblelanguages));
 }

template <class Guesser>
const float* LangGuesser<Guesser>::PointsPerLanguage()
 {
  return(guesser->PointsPerLanguage());
 }
  
#endif
