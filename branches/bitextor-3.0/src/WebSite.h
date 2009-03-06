#ifndef WEBSITE_H_
#define WEBSITE_H_

#include "WebFile.h"
#include "GlobalParams.h"
#include "Heuristics.h"
#include "Bitext.h"

#include <libtagaligner/ConfigReader.h>
#include <dirent.h> 
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stack>



/**
 * @class WebSite
 * @brief Classe que conté els elements d'un lloc web.
 * 
 * Classe que representa els elements continguts per un lloc web.
 * Conté informació sobre els fitxers que pengen de l'arbre de
 * directoris diferenciats per nivells, de forma que es pot ordenar,
 * fàcilment, la informació dels fitxers descarregats pel DownloadMod.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class WebSite
{
private:

	/**
	 * Indicador que assenyala si la classe ha estat inicialitzada correctament (si està a <code>true</code>).
	 */
	//bool initialized;

	/**
	 * Ruta del directori base on es troba descarregat el lloc web.
	 */
	string base_path;

	/**
	 * Llistat de fitxers continguts al directori web.
	 */
	//vector< vector< WebFile* > > file_list;

public:
	/**
	 * Constructor per defecte de la classe.
	 */
	WebSite(const string &path);
	
	/**
	 * Destructor de la classe.
	 */
	~WebSite();
	
	/**
	 * Mètode que permet obtenir la ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @return Retorna la ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 */
	string GetBasePath();
	
	string GetFileName(string path);
	
	/**
	 * Mètode que retorna les dades del fitxer emmagatzemat a la llista segons l'índex passat per paràmetre.
	 * @param pos Índex del fitxer a la llista de fitxers.
	 * @param level Nivell en què es troba el fitxer en l'arbre de directoris.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @throw char* El mètode llança una excepció si s'intenta accedir a un nivell que no existeix mitjançant el paràmetre level.
	 * @throw char* El mètode llança una excepció si s'intenta accedir a una posició del nivell especificat que no existeix mitjançant el paràmetre pos.
	 * @return Retorna el WebFile emmagatzemat a la posició <code>pos</code> de la llista de WebFiles.
	 */
	//WebFile* GetWebFile(const unsigned int &pos, const unsigned int &level);
	
	/**
	 * Mètode que retorna el nombre de fitxers continguts al directori inicialitzat.
	 * @throw char* El mètode llança una excepció si s'intenta accedir a un nivell que no existeix mitjançant el paràmetre level.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @return Retorna el nombre de fitxers continguts al directori inicialitzat.
	 */
	//unsigned int WebFileCount(const unsigned int &level);
	
	/**
	 * Mètode que retorna el nombre de nivells de l'arbre de directoris representat.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @return Retorna el nombre de nivells de l'arbre de directoris representat.
	 */
	//unsigned int LevelCount();
	
	/**
	 * Mètode que inicialitza la llista de fitxers continguts en el directori arrel proporcionat de forma recursiva.
	 * @param base_path Directori base del qual es llegeix la informació sobre els fitxers a procesar.
	 */
	bool GenerateBitexts(const string &dest_path);
	
	/**
	 * Mètode que compara, segons els límits establerts als paràmetres globals, els fitxers web
	 * continguts al lloc web per a obtenir una llista de conjunts de fitxers possibles candi-
	 * dats a tractar-se del mateix fitxer en diferents idiomes.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @return Retorna una estructura <code>vector</code>. Cada node d'aquesta estructura és
	 * un altre vector compost per les rutes dels fitxers candidats. 
	 */
	bool GetMatchedFiles(const string &dest_dir, vector< WebFile* > **file_list, const unsigned int &size, FILE * main_fout=NULL, unsigned int starting_tuid=0, unsigned int *last_tuid=NULL);
};

#endif /*WEBSITE_H_*/
