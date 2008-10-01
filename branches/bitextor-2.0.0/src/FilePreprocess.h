#ifndef FILEPREPROCESS_H_
#define FILEPREPROCESS_H_

#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <tidy/platform.h>
#include <tidy/tidyenum.h>
#include <fstream>
#include <enca.h>
#include "GlobalParams.h"

using namespace std;

/**
 * @class FilePreprocess
 * @brief Mòdul de preprocessament de fitxers.
 * 
 * Classe que conté el mètode de preprocessament de fitxers.
 * Es basa en les llibreries de TidyHTML i Enca. La primera
 * s'encarrega de "netejar" l'etiquetatge del document HTML
 * d'entrada, així com de la seua conversió a UTF-8. El se-
 * gon s'encarrega de detectar la codificació de caracters
 * del fitxer d'entrada per a poder realitzar l'esmentada
 * conversió.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class FilePreprocess
{
public:
	/**
	 * Mètode que s'encarrega de realitzar el preprocessament dels fitxers d'entrada de l'aplicació.
	 * Després de l'execució d'aquest mètode, el fitxer d'entrada queda corregit, des d'un punt de
	 * vista de coherència de l'etiquetatge HTML, i convertit a UTF-8.
	 * @param file_path Aquest paràmetre indica la ruta del fitxer d'entrada en el sistema.
	 * @return El mètode retorna <code>true</code> si el preprocessament s'ha efectuat de forma satisfactòria o <code>false</code> en cas contrari.
	 * @throw char* El mètode llança un missatge d'error si no s'ha especificat la ruta al fitxer de configuració de la llibreria TextCat.
	 */
	static bool PreprocessFile(string file_path);
};

#endif /*FILEPREPROCESS_H_*/

