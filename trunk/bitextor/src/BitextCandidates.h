#ifndef BITEXT_H_
#define BITEXT_H_

#include <iostream>
#include <map>
#include "WebFile.h"


class BitextData
{
	private:
		int files_related;
		
		bool passes;
	
	public:
	
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
		
		double percent_text_distance;
		
		double percent_text_distance_variation;
		
		BitextData(WebFile* wf1, WebFile* wf2);
		
		int UnRelate();
		
		int RelatedFiles();

		bool isBetterThan(BitextData* bitext_data, bool *disabled=NULL);
};

/**
 * @class BitextCandidates
 * @brief Classe que conté els elements dels bitextos.
 * 
 * Classe que conté els elements derivats i necessaris d'un bitext. Açò són, molt ressumidament,
 * els dos textos a aparellar i la capacitat per a generar l'esmentat bitext. A més, en aquest
 * cas, també s'inclouen mètodes i paràmetres necessaris per a la comparació entre els fitxers
 * d'entrada.
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

	 map <wstring, pair<WebFile*,BitextData*>* > candidates;

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
	
	BitextData* GetBitextData(const wstring &lang);
	
	/**
	 * Mètode que crea el bitext a partir dels dos fiters web introduïts en la inicialització.
	 * @param main_fout Fitxer on s'emmagatzemaran els bitextos generats.
	 * @param starting_tuid TUID des de la qual s'han de començar a numerar els bitexts (TU's) generats.
	 * @param last_tuid En cas què aquest punter siga diferent de NULL, s'hi emmagatzemarà la darrera tuid assignada a un TU.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna <code>true</code> si s'ha pogut generar el bitext i <code>false</code>
	 * en cas contrari.
	 */
	//bool GenerateBitexts(map<wstring,FILE *> *main_fout, const string &dest_path, unsigned int starting_tuid, unsigned int *last_tuid);

	bool GenerateBitexts(/*const string &dest_dir*/);
	
	bool Add(BitextCandidates* c);
	
	bool Add(const wstring &lang, BitextData* d);
	
	void EraseLastAdded();
	
	//bool GenerateLastAddedBitext(map<wstring,FILE *> *main_fout, const string &dest_path, unsigned int starting_tuid=0, unsigned int *last_tuid=NULL);
	
	bool GenerateLastAddedBitext(/*FILE* main_fout*/);
	
	WebFile* GetWebFile(const wstring &lang);

	WebFile* GetLastAddedWebFile();
};

#endif /*BITEXT_H_*/
