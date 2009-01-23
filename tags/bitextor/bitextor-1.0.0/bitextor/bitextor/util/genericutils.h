/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
   
   Este codigo fuente contiene la recopilacion de funciones
   especificas no incluibles dentro de una clase determinada.
*/

#ifndef GENERICUTILS_H
#define GENERICUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <libxml/xmlreader.h>

//#define TRAZANDO_GenericUtils //Comentar para evitar la traza

/** Funcion que transforma todas las entidades de una cadena del tipo &amp; en &
 *  Esta funcion recorre la cadena en busca de entidades &amp; para transformarlas en
 *  su equivalente &, dado que para acceder a los enlaces no se utiliza dicha entidad
 *
 * @param _string cadena a transformar su fuera preciso
 * @returns la cadena equivalente tras realizar la transformacion
 */
char* RemoveAmpEntities(const char* _stringamp);

/** Funcion que transforma todos los & de una cadena en entidades del tipo &amp;
 *  Esta funcion recorre la cadena en busca de "&" para transformarlos en su equivalente
 *  &amp;, dado que en el contenido de los xmls se requiere realizar este cambio
 *
 * @param _string cadena a transformar su fuera preciso
 * @returns la cadena equivalente tras realizar la transformacion
 */
char* InsertAmpEntities(const char* _string);

/** Funcion que transforma todas las entidades de una cadena del tipo &amp;, &lt; y &gt; en &, < y >
 *  Esta funcion recorre la cadena en busca de entidades &amp;, &lt; y &gt; para
 *  transformarlas en sus equivalentes &, < y >, dado que para acceder a los enlaces 
 *  no se utiliza dicha entidad
 *
 * @param _string cadena a transformar su fuera preciso
 * @returns la cadena equivalente tras realizar la transformacion
 */
char* RemoveAmpLtGtEntities(const char* _stringamp);

/** Funcion que transforma todos los &, <, y > de una cadena en entidades del tipo &amp; &lt; y &gt;
 *  Esta funcion recorre la cadena en busca de "&", de "<" y de ">" para transformarlos
 *  en su equivalente &amp;, &lt; y &gt;, dado que en el contenido de los xmls se requiere
 *  realizar este cambio
 *
 * @param _string cadena a transformar su fuera preciso
 * @returns la cadena equivalente tras realizar la transformacion
 */
char* InsertAmpLtGtEntities(const char* _string);

/** Funcion que transforma del formato de la libreria LIBXML2 al estandar de C
 *  Esta funcion se encarga de la transformacion del formato interno de la libreria
 *  (xmlChar) al formato normal (char) de C. Devuelve un puntero con la memoria reservada
 *  que debera ser liberado posteriormente, o NULL si la entrada era una cadena no valida
 *
 * @param _entrada la cadena a convertir
 * @returns la cadena convertida o NULL si hubo algun error
 */
char* XMLToLatin1(const xmlChar* _entrada);

/** Function that given an URL removes its ending reference if it exists
 *  This function removes the ending "#..." of a certain URL, so for an url like
 *  "http://www.myweb.es/index.html#news" it would return "http://www.myweb.es/index.html".
 *  If the input url does not have this reference the same url is returned.
 *
 * @param _inputurl the url to be processed
 * @returns the url without the ending reference of the input one if it did not exist
 */
char* RemoveURLReference(const char* _inputurl);
#endif
