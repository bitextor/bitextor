#ifndef BITEXT_H_
#define BITEXT_H_

#include <iostream>
#include <map>
#include "WebFile.h"

/**
 * @class BitextData
 * @brief Classe que conté i calcula les dades per comparar dos fitxers web.
 * 
 * Aquesta classe obté i emmagatzema tota la informació resultant de la comparació entre dos fitxers web. A més, està
 * dissenyada per a saber quants fitxers n'hi ha relacionats amb una instància. D'aquesta forma, es pot saber quan
 * dos fitxers són els idonis per a ser considerats paral·lels i també quan una instància ja no està relacionada amb
 * cap fitxer i, per tant, pot ser eliminada.
 * 
 * @author Miquel Esplà i Gomis
 */
class BitextData
{
	private:
		/**
		 * Nombre de fitxers relacionats a aquesta informació. Pot valdre:
		 * 0: La informació ha de ser esborrada.
		 * 1: Un fitxer té una parella òptima, però aquesta situació no és recíproca.
		 * 2: Una parella de fitxers és òptima i, per tant, són candidats a ser fitxers paral·lels.
		 */
		int files_related;

		/**
		 * Indicador que marca si els fitxerrs comparats han superat totes les heurístiques.
		 */
		bool passes;
		
		
		static void CleanUnfrequentCasesProcessNode(xmlNode* node, wofstream &results_file, vector<unsigned int> &freq_rules, bool write);

	
	public:
	
		/**
		 * Mètode que retorna el valor de l'indicador passes.
		 * @return Retorna el valor de l'indicador passes.
		 */
		bool Passes();
	
		/**
		 * Variable que es troba a <code>true</code> si ambdós fitxers tenen la mateixa extensió i
		 * <code>false</code> en cas contrari.
		 */
		bool same_extension;
		
		/**
		 * Percentatge de diferència de mida en bytes entre ambdós fitxers.
		 */
		double byte_size_distance;
		
		/**
		 * Percentatge de distància d'edició entre els arrays d'etiquetes/text d'ambdós fitxers.
		 */
		double edit_distance;
		
		/**
		 * Ruta d'aliniament per als bitextos.
		 */
		string alignment_path;
		
		/**
		 * Diferència de l'array de nombres trobats.
		 */
		unsigned int n_diff_numbers;
		
		/**
		 * Diferència en el total de text.
		 */
		double text_difference;
		
		unsigned int url_lang_rule;
		
		/**
		 * Constructor de la classe que inicialitza els valors amb l'aplicació de les heurístiques als fitxers web passats.
		 * @param wf1 Primer fitxer web a comparar.
		 * @param wf2 Segon fitxer web a comparar.
		 */
		BitextData(WebFile* wf1, WebFile* wf2);
		
		/**
		 * Mètode que desrelaciona un dels fitxers amb la informació, decrementant en ú la variable files_related.
		 * @return Retorna el valor de la variable files_related actualitzat.
		 */
		int UnRelate();
		
		/**
		 * Mètode que retorna el valor de la variable files_related actualitzat.
		 * @return Retorna el valor de la variable files_related actualitzat.
		 */
		int RelatedFiles();

		/**
		 * Mètode que compara la informació de dues parelles de fitxers web.
		 * @param bitext_data Objecte BitextData amb que es compararà.
		 * @param disabled Variable que es preveu que s'active quan s'establisquen llindars de similitud excessiva.
		 */
		bool isBetterThan(BitextData* bitext_data, bool *disabled=NULL);
		
		static bool CleanUnfrequentCases(const string &filename);
		

};

/**
 * @class BitextCandidates
 * @brief Classe que representa un fitxer web i tots els candidats a ser traduccions del mateix a altres idiomes.
 * 
 * Aquesta classe conté un fitxer web, i l'enllaça amb tots els fitxers que són candidats a ser fitxers paral·lels en altres
 * idiomes. Així mateix, es guarda, per a cadascun d'aquests candidats, la llista de dades de comparació entre fitxer en
 * un objecte BitextData, mitjançant el qual es podran comparar els possibles mútiples candidats per a quedar-se només amb
 * el millor.
 * 
 * @author Miquel Esplà i Gomis
 */
class BitextCandidates
{
private:
	/**
	 * Indicador que controla si el bitext ha estat inicialitzat correctament.
	 */
	bool is_initialized;

	/**
	 * El primer fitxer web del bitext.
	 */
	WebFile *wf;

	/**
	 * Llista de candidats a ser la millor parella per a una llengua determinada.
	 */
	map <wstring, pair<WebFile*,BitextData*>* > candidates;

	/**
	 * Apuntador a la última inserció en la llista.
	 */
	map <wstring, pair<WebFile*,BitextData*>* >::iterator last_insertion;

public:
	/**
	 * Constructor de la classe Bitext.
	 */
	BitextCandidates(WebFile* wf);

	/**
	 * Destructor de la classe Bitext.
	 */
	virtual ~BitextCandidates();
	
	/**
	 * Mètode que retorna el primer fitxer web del bitext.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna el primer fitxer web del bitext.
	 */
	WebFile* GetWebFile();
	
	/**
	 * Mètode que retorna les dades de comparació del fitxer web principal amb el seu millor candidat per a una llengua donada.
	 * @param lang Llengua del candidat que es busca.
	 * @return Retorna l'objecte BitextData amb les dades de comparació.
	 */
	BitextData* GetBitextData(const wstring &lang);
	
	/**
	 * Mètode que crea el bitext a partir dels dos fiters web introduïts en la inicialització.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna <code>true</code> si s'ha pogut generar el bitext i <code>false</code>
	 * en cas contrari.
	 */
	bool GenerateBitexts();

	/**
	 * Mètode que afegeix un candidat a la llista de millors candidats. Aquest només s'afegirà si no n'hi ha cap
	 * candidat per a la llengua donada o si és millor que el candidat actual per a la mateixa llengua.
	 * @param c Nou candidat a comparar.
	 * @return Retorna <code>true</code> si el candidat és afegit i <code>false</code> en cas contrari.
	 */
	bool Add(BitextCandidates* c);
	
	/**
	 * Mètode que afegeix un candidat a la llista de millors candidats. Aquest només s'afegirà si no n'hi ha cap
	 * candidat per a la llengua donada o si és millor que el candidat actual per a la mateixa llengua. En aquest cas
	 * la comparació entre fitxers web ja està feta i es disposa de les dades. Per tant, no cal fer les operacions de
	 * comparació entre fitxers.
	 * @param lang Llengua del candidat.
	 * @param d Dades de comparació entre fitxers web a afegir.
	 * @return Retorna <code>true</code> si el candidat és afegit i <code>false</code> en cas contrari.
	 */
	bool Add(const wstring &lang, BitextData* d);
	
	/**
	 * Mètode que esborra el darrer candidat afegit.
	 */
	void EraseLastAdded();
	
	/**
	 * Mètode que genera la memòria de traducció amb el darrer candidat afegit.
	 * @return Retorna <code>true</code> si el candidat és afegit i <code>false</code> en cas contrari.
	 */
	bool GenerateLastAddedBitext();
	
	/**
	 * Mètode que retorna el fitxer web corresponent al candidat per a una llengua donada.
	 * @param lang Llengua que identifica al fitxer web que es vol recuperar.
	 * @return Retorna el fitxer web indicat.
	 */
	WebFile* GetWebFile(const wstring &lang);

	/**
	 * Mètode que retorna el fitxer web corresponent al darrer candidat afegit.
	 * @return Retorna el fitxer web corresponent al darrer candidat afegit.
	 */
	WebFile* GetLastAddedWebFile();
};

#endif /*BITEXT_H_*/
