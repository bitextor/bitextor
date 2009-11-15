#ifndef WEBFILE_H_
#define WEBFILE_H_

#include "GlobalParams.h"
#include "FilePreprocess.h"
#include "Url.h"
#include <string>
#include <iostream>
#include <vector>


extern "C"{
#include <textcat.h>
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
	vector<int> file;

	/**
	 * Indicador que assenyala si la classe ha estat inicialitzada correctament (si està a <code>true</code>).
	 */
	bool initialized;
	
	/**
	 * Vector de nombres enters trobats al text
	 */
	vector<int> numbers_vec;

	/**
	 * Mida total del text (en caràcters) continguda al document.
	 */
	unsigned int text_size;
	
	Url *url;

	/**
	 * Mètode que indica si un caràcter és alfabètic o no.
	 * @param car Caràcter a analitzar
	 * @return Retorna <code>true</code> si el caràcter és alfabètic i <code>false</code> en cas contrari.
	 */
	bool IsAlphabetic(const wchar_t& car);
	
	/**
	 * Method wich obtains the url of the webfile from a comment in the HTML code (if it has been downloaded by using
	 * HTTrack)
	 */
	//void ObtainURL();

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
	bool Initialize(const string &path, Url *url=NULL);
	
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
	 * Mètode que indica si el fitxer està inicialitzat correctament.
	 * @return Retorna <code>true</code> si el fitxer es torba correctament inicialitzat i <code>false</code> en cas contrari.
	 */
	bool IsInitialized();

	/**
	 * Mètode que retorna un punter al vector d'enters continguts al text.
	 * @return Retorna un punter al vector d'enters continguts al text.
	 */
	vector<int>* GetNumbersVector();

	/**
	 * Mètode que retorna la mida total en caràcters del text del document.
	 * @return Retorna la mida total en caràcters del text del document.
	 */
	unsigned int GetTextSize();

	/**
	 * Mètode que retorna un punter a l'array d'etiquetes-blocs de text.
	 * @return Retorna un punter a l'array d'etiquetes-blocs de text.
	 */
	vector<int>* GetTagArray();

	/**
	 * Mètode que carrega a l'array d'enters els números enters continguts al text.
	 * @param text Text del qual s'hi volen extreure els números.
	 */
	void GetNonAplha(wstring text);

	Url* GetURL();
	
	wstring toXML();
};

#endif /*WEBFILE_H_*/
