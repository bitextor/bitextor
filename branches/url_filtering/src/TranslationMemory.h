#ifndef TRANSLATIONMEMORY_H_
#define TRANSLATIONMEMORY_H_

#include <iostream>
#include <map>
#include "WebFile.h"
#include "BitextCandidates.h"

/**
 * @class TranslationMemory
 * @brief Classe que conté els elements necessaris per generar els fitxers de traducció.
 * 
 * Bàsicament, la classe enera el contingut i gestiona els fitxers,
 * de forma que escolleix quin contingut va en quin fitxer.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class TranslationMemory
{
	private:
		/** Ruta en què es generaran les memòries de traducció. */
		static string dest_path;
		
		/** 
		 * Array emprat quan s'executa l'aplicació generant només una memòria
		 * de traducció per parell de llengües. Gestiona el conjunt de fitxers
		 * per anar desant els nous TU's.
		 */
		static map< wstring, pair< FILE*,int >* > uniq_files;
	
	public:
	
		/**
		 * Mètode que permet assignar la ruta de destí de les memòries de traducció.
		 * @param path Nova ruta a assignar.
		 */
		static void SetDestPath(const string &path);

		/**
		 * Mètode que genera una memòria de traducció a partir de dues pàgines web
		 * i les desa en un sol fitxer, per al parell de llengües, on s'hi desaran
		 * la resta de TU's dels altres fitxers web analitzats.
		 * @param wf1 Un dels fitxers web del qual s'obtindrà la memòria de traducció.
		 * @param wf2 L'altre fitxer web des del qual s'obtindrà la memòria de traducció.
		 * @param data Informació resultant de la comparació dels fitxers.
		 * @return Retorna <code>true</code> si s'ha generat la memòria de traducció i <code>false</code> en cas contrari.
		 */
		static bool WriteInSameFile(WebFile* wf1, WebFile* wf2, BitextData* data);

		/**
		 * Mètode que genera una memòria de traducció a partir de dues pàgines web
		 * i la desa en un fitxer pròpi per al parell de fitxers comparats en qüestió.
		 * @param wf1 Un dels fitxers web del qual s'obtindrà la memòria de traducció.
		 * @param wf2 L'altre fitxer web des del qual s'obtindrà la memòria de traducció.
		 * @param data Informació resultant de la comparació dels fitxers.
		 * @return Retorna <code>true</code> si s'ha generat la memòria de traducció i <code>false</code> en cas contrari.
		 */
		static bool WriteInDifferentFile(WebFile* wf1, WebFile* wf2, BitextData* data);

		/**
		 * Mètode que genera una memòria de traducció a partir de dues pàgines web. Depenent
		 * del mode en què s'estiga executant Bitextor, crida el mètode WriteInSameFile o
		 * WriteInDifferentFile.
		 * @param wf1 Un dels fitxers web del qual s'obtindrà la memòria de traducció.
		 * @param wf2 L'altre fitxer web des del qual s'obtindrà la memòria de traducció.
		 * @param data Informació resultant de la comparació dels fitxers.
		 * @return Retorna <code>true</code> si s'ha generat la memòria de traducció i <code>false</code> en cas contrari.
		 */
		static bool WriteTM(WebFile* wf1, WebFile* wf2, BitextData* data);
		
		/**
		 * Mètode que tanca tots els fitxers oberts i elimina la memòria ocupada pels objectes instanciats
		 * per la classe.
		 */
		static void Reset();
};

#endif /*TRANSLATIONMEMORY_H_*/
