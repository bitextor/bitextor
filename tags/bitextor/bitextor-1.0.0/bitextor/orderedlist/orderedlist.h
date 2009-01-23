/*
   Codigo fuente de un tipo de dato auxiliar para hacer
   busquedas eficientes de numeros enteros. Esta basada
   en listas ordenadas a dos niveles y a su vez balanceadas
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2005
*/

#ifndef orderedlist_H
#define orderedlist_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <math.h>

//#define TRAZANDO_OrderedList //Comentar para evitar la traza

#define kIniListNumber 5 //Numero inicial de listas de primer nivel (upper)
#define kGrowing 5 //Tamaño que crece el numero de sublistas al balancear
                   //Se balancea cuando totalements>()upperlistsize+kGrowing)^2)
#define kIniMaxValue 500000 //Valor maximo inicial (util para division inicial de listas de primer nivel)

typedef struct ItemList
 {
  unsigned int value; //Valor a comparar
  unsigned int data; //Valor a recuperar
  ItemList *next;
 };

/** Estructura para permitir busquedas mas eficientes en la lista
 *  Esta estructura permite tener una lista ordenada de sublistas
 *  ordenadas a su vez, de forma que se optimice
 */
typedef struct OrderedList
 {
  unsigned int value; //Valor a comparar del primer subelemento (minimo)
  ItemList *inside; //Enlace a la sublista
  unsigned int sizeinside; //Tamaño de la sublista
  OrderedList *next;
 };

/** Clase que representa una estructura de listas ordenadas a dos niveles
 *  Esta clase se utiliza para buscar un valor dentro de la estructura y
 *  obtener un dato asociado si se encuentra (o conjunto de datos si el
 *  valor estaba repetido).
 */ 
class OrderedList2Levels
 {
  OrderedList *orderedlist; //Enlace a la estructura
  unsigned int upperlistsize; //Numero de sublistas de segundo nivel
  unsigned int totalelements; //Numero total de elementos en la estructura
  
  unsigned int *searchresult; //Vector con el resultado de una busqueda
  unsigned int growing; //Numero de elementos que crece la estructura al balancearla
  unsigned int inimaxvalue; //Valor maximo inicial para partir las sublistas inicialmente
  
  OrderedList *actual; //Establece la lista que se esta recorriendo
  ItemList *nextitem; //Establece el siguiente elemento que se recorrera
  
  /** Funcion que libera toda la estructura
   *  Esta funcion libera la memoria asociada al objeto que exista
   *  actualmente.
   */
  void Reset();
  
  /** Funcion que balancea las listas para igualar su tamaño
   *  Esta funcion optimiza el rendimiento de las busquedas mediante creacion
   *  de nuevas sublistas y ademas reequilibra los elementos de las mismas
   *  para tener el mismo numero de elementos en cada una de ellas.
   */
  void Balance();
  
  public:
      
    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     */
    OrderedList2Levels();
    
    /** Constructor de la clase
     * Inicializa todos los atributos a los valores por defecto
     *
     * @param _growing Especifica cuanto crece el numero de sublistas al balancear
     * @param _sublistnumber Indica el numero inicial de sublistas
     * @param _inimaxvalue Valor maximo a insertar, util para inicializar las sublistas
     */
    OrderedList2Levels(unsigned int _growing,unsigned int _sublistnumber,unsigned int _inimaxvalue);
    
    /** Destructor de la clase
     *  Elimina toda la memoria reservada por el objeto
     */
    ~OrderedList2Levels();    
        
    /** Funcion que inserta un valor en la estructura
     *  Esta funcion inserta un valor y su dato asociado en la estructura
     *
     * @param _value Valor por el que se ordena
     * @param _data Dato asociado al valor
     */
    void Insert(unsigned int _value,unsigned int _data);
    
    /** Funcion que busca en la estructura y devuelve el numero de veces que aparece
     *  Esta funcion busca la estructura y devuelve el numero de veces que
     *  aparece y ademas deja en la variable de clase searchresult los datos
     *  asociados a la ocurrencia
     *
     * @param _value el valor a buscar en la estructura
     * @returns el numero de ocurrencias que se han encontrado
     */
    unsigned int Search(unsigned int _value);
    
    /** Funcion que permite el acceso a las ocurrencias que se han encontrado en la busqueda
     *  Esta funcion devuelve un puntero a la estructura de los datos que se han
     *  buscado con la funcion search, es decir, la variable searchresult
     *
     * @returns el resultado de la ultima busqueda realizada    
     */
    const unsigned int* SearchResults();
    
    /** Funcion que devuelve el tamaño de la estructura
     *  Esta funcion tan solo devuelve el valor de totalelements.
     *
     * @returns el tamaño de la estructura 
     */    
    unsigned int Size();
    
    /** Funcion que reinicia el recorrido de la estructura
     *  Esta funcion coloca la posicion del recorrido al inicio de la
     *  estructura
     */
    void GoStart();
    
    /** Funcion que devuelve el siguiente dato contenido dentro de la lista
     *  Esta funcion recupera el dato siguiente en el recorrido de la estructura
     *  y avanza un paso en dicho recorrido
     *
     * @returns el dato siguiente en el recorrido de la estructura o -1 si ya se acabaron
     */
    int GoNext();
 };

#endif
