#ifndef WEBSITE_H_
#define WEBSITE_H_

#include "WebFile.h"
#include "GlobalParams.h"
#include "Heuristics.h"
#include "BitextCandidates.h"
#include <iostream>
#include <vector>

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
	 * Ruta del directori base on es troba descarregat el lloc web.
	 */
	string base_path;

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

	/**
	 * Mètode que retorna el nom d'un fitxer a partir del seu path complet.
	 * @param path Path del qual s'hi vol obtenir el nom del fitxer.
	 * @return Retorna el nom del fitxer.
	 */
	static string GetFileName(string path);
	
	/**
	 * Mètode que inicialitza la llista de fitxers continguts en el directori arrel proporcionat de forma recursiva.
	 * @param dest_path Directori base del qual es llegeix la informació sobre els fitxers a procesar.
	 * @return Retorna <code>true</code> si s'ha trobat alguna coincidència o <code>false</code> en cas contrari.
	 */
	bool GenerateBitexts(const string &dest_path);
	
	/**
	 * Mètode que compara, segons els límits establerts als paràmetres globals, els fitxers web
	 * continguts al lloc web per a obtenir una llista de conjunts de fitxers possibles candi-
	 * dats a tractar-se del mateix fitxer en diferents idiomes.
	 * @param dest_dir Directori de destinació per la generació dels bitextos.
	 * @param file_list Llista de fitxers per nivells ja carregats.
	 * @param size Nombre de nivells de la cerca.
	 * @throw char* El mètode llança una excepció si l'objecte no ha estat inicialitzat correctament.
	 * @return Retorna una estructura <code>vector</code>. Cada node d'aquesta estructura és
	 * un altre vector compost per les rutes dels fitxers candidats. 
	 */
	bool GetMatchedFiles(const string &dest_dir, vector< BitextCandidates* > **file_list, unsigned int size);
};

#endif /*WEBSITE_H_*/
