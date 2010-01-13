#ifndef BITEXT_H_
#define BITEXT_H_

#include <iostream>
#include <map>
#include <libxml/parser.h>
#include "WebFile.h"

/**
 * @class BitextData
 * @brief This class contains the data obtained from the comparison of a pair of files.
 * 
 * This class obtains and saves all the information resulting from the process of comparing a pair of files. In addition,
 * it is designed to know how many files are related with an instance of this class. In this way, the system can contorl
 * when there is no file related with an instance and it can be freed.
 * 
 * @author Miquel Esplà i Gomis
 */
class BitextData
{
	private:
		/**
		 * Number of files related with an instance of the class: 0, 1 or 2.
		 */
		int files_related;

		/**
		 * This flag indicates if both files have passed all the heuristics in comparison process.
		 */
		bool passes;

	
	public:
	
		/**
		 * This method returns the value of the flag passes.
		 * @return Returns the value of the flag passes.
		 */
		bool Passes();
		
		/**
		 * Percentage of difference between files measured in bytes.
		 */
		double byte_size_distance;
		
		/**
		 * Edit distance between files (calculated from their fingerprints).
		 */
		double edit_distance;
		
		/**
		 * Resulting alignment path obtained from edit distance algorithm.
		 */
		//string alignment_path;
		
		/**
		 * Diferència de l'array de nombres trobats.
		 */
		//unsigned int n_diff_numbers;
		
		/**
		 * Percentage of difference between files measured in characters.
		 */
		double text_difference;
		
		/**
		 * Code of the difference between the urls.
		 */
		unsigned int url_lang_rule;
		
		/**
		 * Class constructor. This class compares the bitext and obtains the resulting
		 * parametters from the comparison process. The number of related files is set
		 * to 2 if the files pass the heuristics and the obtained resulta are the best
		 * results obtained for both files.
		 * @param wf1 First web file to compare.
		 * @param wf2 Second web file to compare.
		 */
		BitextData(WebFile* wf1, WebFile* wf2);
		
		/**
		 * This method decrements the files_related counter of related files.
		 * @return Returns the new value of the variable files_related.
		 */
		int UnRelate();
		
		/**
		 * This method returns the new value of the variable files_related.
		 * @return Returns the new value of the variable files_related.
		 */
		int RelatedFiles();

		/**
		 * This method compares the information obtained from the comparison of a pair of files.
		 * @param bitext_data BitextData to comare with.
		 * @param disabled Variable que es preveu que s'active quan s'establisquen llindars de similitud excessiva.
		 */
		bool isBetterThan(BitextData* bitext_data, bool *disabled=NULL);
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
		
	static void CleanUnfrequentCasesProcessNode(xmlNode* node, wofstream &results_file, vector<unsigned int> &freq_rules, bool write);

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
	
	static bool CleanUnfrequentCases(const string &filename);
};

#endif /*BITEXT_H_*/
