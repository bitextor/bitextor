/*
   Source code of the aligner 2-1. It is designed to allow webpage alignment after
   turning them into XHTML files. The alignment is performed in a single step, where
   sentences and tags are never aligned between them.

   Source code of the aligner 2 steps with alignment distance. It is designed
   to allow webpage alignment after turning them into XHTML files. The alignment
   is performed in two steps: 
    - In the first one, tags and text chunks are aligned between them.
    - In the second one, aligned text chunks are splitted in sentences and are aligned
      at the sentence-level.

   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

#ifndef TAGALIGNER2_1_H
#define TAGALIGNER2_1_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <time.h>
#include "../util/xhtmltags.h"
#include "../util/sentencesplitter.h"

//Uncomment the following line to use the number of words inside a text chunk instead of the chunk length
//#define DistanceInWords 

#define kSizeAuxStrings 3000
#define kFirstLength 1000 //Number that is added to a text segment to distinguish it from the tags
#define kBigValue 999999999 //It represent an enormous value

class TagAligner2_1
 {
  double EDInsStructural; /** Insertion cost of a structural tag */
  double EDInsFormat; /** Insertion cost of a format tag */
  double EDInsContent; /** Insertion cost of a content tag */
  double EDInsText; /** Insertion cost of each byte of text */
  double EDDelStructural; /** Deletion cost of a structural tag */
  double EDDelFormat; /** Deletion cost of a format tag */
  double EDDelContent; /** Deletion cost of a content tag */
  double EDDelText; /** Deletion cost of each byte of text */
  double EDSubsStrucStruc; /** Substitution cost of a structural cost for a different structural tag */
  double EDSubsStrucFormat; /** Substitution cost of a structural tag for a format tag and vice versa */
  double EDSubsStrucContent; /** Substitution cost of a structural tag for a content tag and vice versa */
  double EDSubsFormatFormat; /** Substitution cost of a format tag for a different format tag */
  double EDSubsFormatContent; /** Substitution cost of a format tag for a content tag and vice versa */
  double EDSubsContentContent; /** Substitution cost of a content tag for a different format tag */
  double EDSubsText; /** Substitution cost of each byte of text of the sizes */ 
   
  unsigned int ALtextsizemin; /** Diferencia minima de tamaño entre segmentos de texto para poder descartar por grandes diferencias de tamaño */
  float ALtextsizeminrelation; /** Diferencia maxima de porcentajes de tamaño entre segmentos de texto para poder considerarlos paralelos */
 
  char **lphrases,**rphrases;/** Frases del texto izquierdo y del derecho */
  unsigned int n_lphrases,n_rphrases; /** Numero de frases de cada texto */
  short *lvector,*rvector; /** Vector con etiquetas y longitudes de frases, si es una longitud el valor ira incrementado en kFirstLongitud */
  unsigned int *bestalignment; /** Ruta que marca el mejor alineamiento posible */
  unsigned int n_bestalignment; /** Numero de pasos que tiene el mejor alineamiento posible */
  char **lsegments,**rsegments; /** Segmentos una vez realizado el alineamiento */
  unsigned int n_segments; /** Numero de segmentos resultantes del alineamiento */

  
  /** Function that given a text or a tag obtains its numerical representation
   *  This function generates a number for the input phrase depending if it is
   *  a tag or a text chunk. If the input is a opening tag, then its associated
   *  number is returned. If the input is a closing tag then its associated
   *  number is a returned but in negative. Finally if the input is a text chunk
   *  then the result is its length plus kFirstLength.
   *
   * @param _phrase The input phrase
   * @return The associated number
   */
  short GetAssociatedNumber(const char* _phrase);
  
  /** Function that calculates the alignment distance between two sequences of text chunks
   *  This function receives as an input two sequences of text chunks and their
   *  respective sizes and returns the alignment distance between them. This 
   *  distance represents the proximity of aligned sentences of the best
   *  possible alignment.
   *
   * @param _leftchunks The text chunks of the first sequence
   * @param _nleftchunks The number of text chunks included in the first sequence
   * @param _rightchunks The text chunks of the second sequence
   * @param _nrightchunks The number of text chunks included in the second sequence
   * @returns The alignment distance between the two sequences
   */
  double AlignmentDistanceChunks(const char** _leftchunks,unsigned short _nleftchunks,const char** _rightchunks,unsigned short _nrightchunks);

  /** Function that counts the number of words contained in a text string
   *  This function receives an input string and returns the number of words that
   *  it contained.
   *
   * @param _text The text to process
   * @returns The number of words contained in that text
   */
  unsigned short WordsNumber(const char* _text);
   
  public:
      
    /** Constructor de la clase
     *  Inicializa todos los atributos a los valores por defecto
     */
    TagAligner2_1();
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~TagAligner2_1();    

    /** Funcion que libera la memoria usada durante el alineamiento
     *  Esta funcion elimina toda la memoria asociada al alineamiento y a las
     *  conversiones de datos que se realizan previamente al mismo
     */
    void Reset();
    
    /** Function that splits a text in tags and text chunks and discards content of style and script tags
     *  This function processes a text and splits it in chunks whenever
     *  a tag is found. Irrelevants tags are filtered from the text and
     *  do not constitute splitting points. The function returns the
     *  number of resulting chunks in the output parameter _number
     *
     * @param _text Text to split
     * @param _number Number of resulting chunks (output parameter)
     * @return The chunks obtained as a result
     */
    char** SplitFilterText(const char* _text,unsigned int* _number);
    
    /** Function that splits text chunks into sentences 
     *  This function receives a split text in tags and text chunks and processes
     *  text chunks looking for splitting points and splits them.
     * 
     * @param _splittext The tags and text chunks sequence
     * @param _sizesplittext The number of input items
     * @param resultsnumber The number of items returned
     * @returns The tags and the text chunks split into sentences
     */
    char** SplitInSentences(const char** _splittext,unsigned int _sizesplittext,unsigned int* _resultsnumber);
    
    /** Funcion que genera un fichero TMX en funcion de las listas de alineamientos
     *  Esta funcion genera un fichero TMX en el que se relacionan todos los
     *  segmentos cuyo resultado del alineamiento se recibe como parametro. Tambien
     *  es necesario decirle el idioma de cada uno de los grupos de segmentos para
     *  incluir dicha informacion en el tmx
     *
     * @param _leftparts El vector de partes izquierdas
     * @param _llang El identificador del idioma en el que estan las partes izquierdas
     * @param _rightparts El vector de partes derechas
     * @param _rlang El identificador del idioma en el que estan las partes derechas
     * @param _n_segments El numero total de segmentos de los vectores
     */
    char* GenerateTMX(const char** _leftparts,const char* _llang,const char** _rightparts,const char* _rlang,unsigned int _n_segments);
    
    /** Funcion que genera un fichero TMX en funcion de las listas de alineamientos
     *  Esta funcion genera un fichero TMX en el que se relacionan todos los
     *  segmentos cuyo resultado del alineamiento se recibe como parametro. Tambien
     *  es necesario decirle el idioma de cada uno de los grupos de segmentos para
     *  incluir dicha informacion en el tmx
     *
     * @param _llang El identificador del idioma en el que estan las partes izquierdas
     * @param _rlang El identificador del idioma en el que estan las partes derechas
     * @param _n_segments El numero total de segmentos de los vectores
     */
    inline char* GenerateTMX(const char* _llang,const char* _rlang)
     {
      return(GenerateTMX((const char**)lsegments,_llang,(const char**)rsegments,_rlang,n_segments));
     }
    
    /** Funcion que genera un fichero de texto que muestra el resultado del alineamientos
     *  Esta funcion genera un fichero de texto en el que se escribe en cada linea un segmento,
     *  de forma que la linea 1 esta alineada con la 2 y asi sucesivamente
     *
     * @param _leftparts El vector de partes izquierdas
     * @param _rightparts El vector de partes derechas
     * @param _n_segments El numero total de segmentos de los vectores
     */
    char* GenerateTextAligned(const char** _leftparts,const char** _rightparts,unsigned int _n_segments);
    
    /** Funcion que genera un fichero de texto que muestra el resultado del alineamientos
     *  Esta funcion genera un fichero de texto en el que se escribe en cada linea un segmento,
     *  de forma que la linea 1 esta alineada con la 2 y asi sucesivamente
     *
     * @param _n_segments El numero total de segmentos de los vectores
     */
    inline char* GenerateTextAligned()
     {
      return(GenerateTextAligned((const char**)lsegments,(const char**)rsegments,n_segments));
     }
    
    /** Function that given two tag and text sequences, returns the sequence of steps of the minimal path of edition cost
     *  This function calculates the minimal edit distance path according to the established
     *  weights. If at any moment the minimal edit distance is over the value of _maxvalue,
     *  the process is stopped and NULL is returned. The sequence of steps is returned as
     *  a sequence of characters in which:
     *          's': substitution
     *          'd': deletion
     *          'i': insertion
     *
     *  This function only takes into account estructural, format and content tags, any other
     *  type tag should be discarded first. Text segments can not be substituted
     *  for tags, only for another text segment, and its cost is the edit distance of their
     *  corresponding alignment.
     * 
     * @param _tts1 the first of the sequences
     * @param _ttssize1 the size of the first sequence
     * @param _tts2 the second of the sequences
     * @param _ttssize2 the size of the second sequence
     * @param _maxvalue the maximum value that allow to stop the processs if the edit distance is high
     * @returns The sequence of characters that indicate the minimal edit distance path or NULL if there is an error or it was stopped because of _maxvalue
     */
    char* EditDistancePath(const char** _tts1,unsigned int _ttssize1,const char** _tts2,unsigned int _ttssize2,double _maxvalue);

    /** Function that given two text lengths sequences, returns the edit distance between them
     *  This function calculates the minimal edit distance according to the
     *  established weights. Text segments can not be substituted between them
     *  if the difference between their distances is higher than ALtextsizeminrelation
     *  and the distance of one of them is at least ALtextsizemin. Addittionally, using 
     *  the maxvalue parameter a maximum edit distance can be set, so that the edit
     *  distance algorithm would discard high cost paths (making it faster) or even abort
     *  the whole process if the distance is too high.
     * 
     * @param _ts1 the first of the text sequences
     * @param _tssize1 the size of the first text sequence
     * @param _ts2 the second of the text sequences
     * @param _tssize2 the size of the second text sequence
     * @param _maxvalue Indicates when to discard a path
     * @returns The edit distance between the sequences or -1 if there was an error
     */
    double EditDistance(const short* _ts1,unsigned int _tssize1,const short* _ts2,unsigned int _tssize2,double _maxvalue=kBigValue);
         
    /** Funcion que alinea dos textos en base a etiquetas y longitudes de frases
     *  Esta funcion segmenta los textos que recibe en frases y etiquetas y
     *  posteriormente realiza el alineamiento a frases entre ellos. El resultado
     *  se almacena en las variables lsegments y rsegments y el numero de segmentos
     *  resultantes en n_segments.
     *
     * @param _ltext El primero de los textos a alinear
     * @param _rtext El segundo de los textos a alinear
     * @returns True si el alineamiento tuvo exito o False si hubo error
     */
    bool Align(const char* _ltext,const char* _rtext);    
 };

#endif
