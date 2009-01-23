/*
   Implemented by Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
   
   Source code of the functions oriented to manage languages,
   that is, associating language identifications to each of
   the languages.
*/

#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H
#include <stdio.h>
#include <string>

#define kNumLanguages 5 //Number of languages that can handle the application

/* Function that returns the associated string to a language number
 *
 * @param language the language id
 * @returns The language string associated to that language id
 */
const char* LanguageName(short _language);

/** Function that returns the language id of a certain language string
 *  This function receives a string that should cointain a language string
 *  such as 'es', 'ca' 'en', etc, and returns the id number associated to
 *  that language. If the string is unknown then the application will return -1
 * 
 * @param language the string associated to a certain language
 * @returns The id of the correspondent language or -1 if it was not found
 */
short LanguageCode(const char* _language);

#endif
