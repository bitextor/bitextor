#ifndef WEBSITE_H_
#define WEBSITE_H_

#include "WebPage.h"
#include <iostream>

/**
 * Classe que representa els elements continguts per un lloc web.
 * Conté informació sobre els fitxers que pengen de l'arbre de
 * directoris, de forma que es pot ordenar, fàcilment, la informació
 * dels fitxers descarregats pel DownloadMod.
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
	 
	/**
	 * Llistat de fitxers continguts al directori web.
	 */
	vector<WebFile> llista_fitxers;
	
public:
	/**
	 * Constructor per defecte de la classe.
	 */
	WebSite();
	
	/**
	 * Destructor de la classe.
	 */
	~WebSite();
	
	/**
	 * Constructor sobrecarregat de la classe.
	 * @param base_path Ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 */
	WebSite(string base_path);
	
	/**
	 * Mètode que crea un fitxer en XML amb tota la informació emmagatzemada en la classe.
	 * @param XML_path Ruta on es localitza el fitxer generat.
	 */
	void toXML(string XML_path);
	
	/**
	 * Mètode que permet establir la ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 * @param base_path Ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 */
	void setBasePath(string base_path);
	
	/**
	 * Mètode que permet obtenir la ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 * @return Retorna la ruta on es localitza el directori base del que penja tot el lloc web descarregat.
	 */
	string getBasePath();

	/**
	 * Mètode que permet afegir un fitxer web a la llista.
	 * @param wf Fitxer web que es vol afegir a la llista.
	 * @return Retorna <code>true</code> en cas de que es puga fer la inserció a la llista i <code>false</code> en cas contrari.
	 */
	bool addWebFile(WebFile wf);
	
	/**
	 * Mètode que permet eliminar un fitxer web de la llista.
	 * @param file_path Ruta del fitxer que es vol eliminar.
	 * @return Retorna <code>true</code> en cas de que es puga fer l'elimiació a la llista i <code>false</code> en cas contrari.
	 */
	bool removeWebFile(string file_path);
	
	/**
	 * Mètode que retorna les dades del fitxer emmagatzemat a la llista segons l'índex passat per paràmetre.
	 * @param pos Índex del fitxer a la llista de fitxers.
	 * @return Retorna el WebFile emmagatzemat a la posició <code>pos</code> de la llista de WebFiles.
	 */
	int getWebFile(int pos);
};

#endif /*WEBSITE_H_*/
