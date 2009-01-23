#ifndef DOWNLOADMOD_
#define DOWNLOADMOD_

#include <cstdio>
#include <cstdlib>

#include "GlobalParams.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/stat.h>

using namespace std;

/**
 * @class DonloadMod
 * @brief Mòdul de descàrrega web.
 * 
 * Mòdul encarregat de la descàrrega de les diverses pàgines descarregades. Està basat en l'aplicació
 * Wget, de la qual pren les funcionalitats que permeten la descàrrega de fitxers. La cridada a aquesta
 * aplicació es fa mitjançant un <code>system</code> i s'hi permet la incorporació de diversos paràmetres
 * per a refinar la cerca.
 * 
 * @author Miquel Esplà i Gomis
 */
class DownloadMod
{
	private:
	/**
	 * Paràmetre que conté la duració màxima (en segons) de la descàrrega de fitxers.
	 */
	long max_downloaded_size;
	 
	/**
	 * Directori de destí en què es guardaran els fitxers descarregats.
	 */
	string dest_path;
	
	public:  
	/**
	 * Constructor per defecte de la classe. Quan es construeix l'objecte, tant la llista d'extenssions admeses com el nombre màxim de bytes a descarregar són presos del fitxer de configuració. Posteriorment poden ser modificats.
	 */
	DownloadMod();
	   
	/**
	 * Destructor de la classe.
	 */
	~DownloadMod();
	
	/**
	 * Mètode que permet establir la mida màxima del conjunt d'elements descarregats.
	 * @param mds Valor de la mida màxima de descàrrega.
	 */
	void SetMaxDownloadedSize(long mdt);

	/**
	 * Mètode que permet definir el directori on es realitzaran les descàrregues dels llocs web.
	 * @param path Ruta que defineix el directori on es descarregaran els llocs web.
	 * @return El mètode retorna <code>true</code> quan el directori existeix i <code>false</code> si no (en aquest segon cas, l'assignació de l'adresa no es realitzarà). 
	 */
	void SetDestPath(string path);

	/**
	 * Mètode que realitza la descàrrega de webs mitjançant els paràmetres inclosos a la classe.
	 * @param website Adreça URL del lloc web que es vol descarregar.
	 * @return El mètode retorna <code>true</code> si el mètode ha funcionat correctament i <code>false</code> en cas contrari.
	 */
	bool StartDownload(string website);
};

#endif /*DOWNLOADMOD_*/
