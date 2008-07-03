#ifndef WEBFILE_H_
#define WEBFILE_H_

#include <libxml++/libxml++.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "TagClean.h"
extern "C"{
#include <textcat.h>
}
using namespace std;

class WebFile
{
	private:
		/**
		 * Adreça on es troba el fitxer.
		 */
		string path;
		
		/**
		 * Lengua en què està escrit el text contingut pel fitxer.
		 */
		string lang;
		
		/**
		 * Tipus de dades contingudes al fitxer.
		 */
		string file_type;

	public:
	
		/**
		 * Constructor per defecte de la classe.
		 */
		WebFile();
		
		/**
		 * Destructor de la classe.
		 */
		~WebFile();
		
		/**
		 * Constructor sobrecarregat de la classe.
		 */
		WebFile(string path);
		
		/**
		 * Mètode que permet establir el paràmetre sobre l'idioma del fitxer.
		 * @param language Codi de l'idioma en què està escrit el text del fitxer.
		 */
		void setLang(string language);
		
		/**
		 * Mètode que permet obtenir el paràmetre sobre l'idioma del fitxer. En cas què aquest no estiga definit, 
		 * @return Retorna el codi d'idioma en què està escrit el text del fitxer.
		 */
		string getLang();
		
		/**
		 * Mètode que permet establir el paràmetre sobre la ruta on es troba el fitxer en el sistema de directoris.
		 * @param path Ruta on es troba el fitxer en el sistema de directoris.
		 */
		void setPath(string path);
		
		/**
		 * Mètode que permet obtenir el paràmetre sobre la ruta on es troba el fitxer en el sistema de directoris.
		 * @return Retorna la ruta on es troba el fitxer en el sistema de directoris.
		 */
		string getPath();
		
		/**
		 * Mètode que permet obtenir el valor del paràmetre sobre el tipus de contingut (extensió) del fitxer.
		 * @return Retorna l'extensió del fitxer.
		 */
		string getFileType();
		
		/**
		 * Mètode que les dades de l'objecte a XML.
		 * @return Retorna la cadena de text de XML.
		 */
		string getXMLString();
};

#endif /*WEBFILE_H_*/
