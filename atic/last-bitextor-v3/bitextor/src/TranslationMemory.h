#ifndef TRANSLATIONMEMORY_H_
#define TRANSLATIONMEMORY_H_

#include <iostream>
#include <map>
#include "WebFile.h"
#include "BitextCandidates.h"

/**
 * @class TranslationMemory
 * @brief This class contains the necessary elements to generaty translation memories (TM) from the foud bitexts in a
 * website.
 * 
 * This class contains the necessary elements to generaty translation memories (TM) from the foud bitexts in a  website.
 * It manages the TMXs files which contains the TMs. This class can work in two ways: writting each TM in a different file
 * or writting all the translation unit (TU) in an only file for a given language pair. To work in the second way, this
 * class needs to manage as many files as language pairs exist in the website and it will only close these files when the
 * application has finished the complete process.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class TranslationMemory
{
	private:
		/** Path where the TMXs files will be created and saved. */
		static string dest_path;
		
		/** 
		 * If the application is runed to create an only TM for each pair of languages, the corresponding TMX file for
		 * each TM is saved in this structure. In this way, the left element in the Map corresponds to the language
		 * pair identifier. The rigth one corresponds to a pair of values: the TMX file where the TU are been saved and
		 * the last TU identificator (each TU is identified with an integer in the TMX file). Saving the TU identificator
		 * the system can avoid the TU identification numbers repetition.
		 */
		static map< wstring, pair< FILE*,int >* > uniq_files;
	
	public:
	
		/**
		 * This method sets the destination path where the TMs will be crated and saved.
		 * @param path Assigned path.
		 */
		static void SetDestPath(const string &path);

		/**
		 * This method generates a TM from a pair of web files. When it has generated all the TU, it saves it in the
		 * file corresponding with the language pair of the files.
		 * @param wf1 First web file.
		 * @param wf2 Second web file.
		 * @param data Information generated during the web files comparison (see BitextData class documentation).
		 * @return Returns <code>true</code> if the TM has been generated and <code>false</code> if not.
		 */
		static bool WriteInSameFile(WebFile* wf1, WebFile* wf2, BitextData* data);

		/**
		 * This method generates a TM from a pair of web files. When it has generated all the TU, it saves it in an only
		 * file for this pair of files.
		 * @param wf1 First web file.
		 * @param wf2 Second web file.
		 * @param data Information generated during the web files comparison (see BitextData class documentation).
		 * @return Returns <code>true</code> if the TM has been generated and <code>false</code> if not.
		 */
		static bool WriteInDifferentFile(WebFile* wf1, WebFile* wf2, BitextData* data);

		/**
		 * This method generates a TM from a pair of web files. This is the method which chooses if the TM must be saved
		 * in an individual TMX file for the pair of files or in a general TMX file for the pair of languages.
		 * @param wf1 First web file.
		 * @param wf2 Second web file.
		 * @param data Information generated during the web files comparison (see BitextData class documentation).
		 * @return Returns <code>true</code> if the TM has been generated and <code>false</code> if not.
		 */
		static bool WriteTM(WebFile* wf1, WebFile* wf2, BitextData* data);
		
		/**
		 * This method closes all the opened files and frees the dynamic memory used by the class during the TM generation.
		 * per la classe.
		 */
		static void Reset();
};

#endif /*TRANSLATIONMEMORY_H_*/
