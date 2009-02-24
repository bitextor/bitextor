#ifndef WEBFILE_H_
#define WEBFILE_H_

#include "GlobalParams.h"
#include "FilePreprocess.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <libtagaligner/FragmentedFile.h>
#include <libtagaligner/Aligner.h>

extern "C"{
#include <textcat.h>
//#include <magic.h>
}
using namespace std;

/**
 * @class WebFile
 * @brief Classe que conté els elements d'un fitxer web.
 * 
 * Classe que representa els elements continguts per un lloc web.
 * Conté informació sobre els fitxers que pengen de l'arbre de
 * directoris diferenciats per nivells, de forma que es pot ordenar,
 * fàcilment, la informació dels fitxers descarregats pel DownloadMod.
 * 
 * @author Miquel Esplà i Gomis. 
 */
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
	wstring lang;
	
	/**
	 * Tipus de dades contingudes al fitxer.
	 */
	string file_type;
	
	/**
	 * Objecte que conté el fitxer HTML dividit en etiquetes i blocs de text.
	 */
	FragmentedFile file;

	/**
	 * Indicador que assenyala si la classe ha estat inicialitzada correctament (si està a <code>true</code>).
	 */
	bool initialized;

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
	 * Inicialitzador de la classe amb tots els paràmetres a partir de la ruta del fitxer.
	 * El mètode fa servir l'analitzador lèxic fet en Flex per a crear un vector d'etique-
	 * tes i guardar el text net (sense etiquetes) en una variable, a partir de la qual es
	 * detectarà l'idioma en què està escrit el text.
	 * @param path Ruta del fitxer a què fa referència la classe.
	 * @throw char* El mètode llança una excepció si no s'ha especificat el fitxer de configuració de TextCat.
	 */
	bool Initialize(const string &path);
	
	/**
	 * Mètode que permet obtenir el paràmetre sobre l'idioma del fitxer.
	 * @throw char* El mètode llança una excepció si no s'ha inicialitzat correctament l'objecte.
	 * @return Retorna el codi d'idioma en què està escrit el text del fitxer.
	 */
	wstring GetLang();
	
	/**
	 * Mètode que permet obtenir el paràmetre sobre la ruta on es troba el fitxer en el sistema de directoris.
	 * @throw char* El mètode llança una excepció si no s'ha inicialitzat correctament l'objecte.
	 * @return Retorna la ruta on es troba el fitxer en el sistema de directoris.
	 */
	string GetPath();
	
	/**
	 * Mètode que permet obtenir el valor del paràmetre sobre el tipus de contingut (extensió) del fitxer.
	 * @throw char* El mètode llança una excepció si no s'ha inicialitzat correctament l'objecte.
	 * @return Retorna l'extensió del fitxer.
	 */
	string GetFileType();
	
	/**
	 * Mètode que retorna un array format per la cadena d'etiquetes contingudes al fitxer web.
	 * @throw char* El mètode llança una excepció si no s'ha inicialitzat correctament l'objecte.
	 * @reutrn Retorna la llista d'etiquetes, codificades amb números enters, en un vector.
	 */
	vector<Fragment*> * GetTagArrayReference();
	
	/**
	 * Mètode que retorna l'objecte FragmentedFile que conté el fitxer XHTML.
	 * @throw char* El mètode llança una excepció si no s'ha inicialitzat correctament l'objecte.
	 * @reutrn Retorna l'objecte FragmentedFile que conté el fitxer XHTML.
	 */
	FragmentedFile* GetFragmentedFileReference();
	
	/**
	 * Mètode que indica si el fitxer està inicialitzat correctament.
	 * @return Retorna <code>true</code> si el fitxer es torba correctament inicialitzat i <code>false</code> en cas contrari.
	 */
	bool IsInitialized();
};

#endif /*WEBFILE_H_*/
