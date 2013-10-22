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
 * @brief This class represents a web file and all the candidates to a translation in another language.
 * 
 * This class contains a web file (main file of the class) and the related files which are cnadidates to be a
 * translation in another language. In addition, for each candidate this class saves the data resulting from the
 * comparison between the file and the main file and the candidate in a BitextData object. This object is used to
 * compare the possibe candidates to discard the unlike ones and keep only the best candidate.
 * 
 * @author Miquel Esplà i Gomis
 */
class BitextCandidates
{
private:
	/**
	 * Flag which indicates if the object has been initialized (default=false).
	 */
	bool is_initialized;

	/**
	 * Main web file of the class.
	 */
	WebFile *wf;

	/**
	 * List of the best candidates to be translations in each language. The information is structured in a map in which
	 * the left element of each entry is the language code and the rigth element is a pair
	 * web-file-candidate/comparison-information. 
	 */
	map <wstring, pair<WebFile*,BitextData*>* > candidates;

	/**
	 * This iterator points to the last insertion in the candidates structure.
	 */
	map <wstring, pair<WebFile*,BitextData*>* >::iterator last_insertion;

	/**
	 * 
	 */
	static void CleanUnfrequentCasesProcessNode(xmlNode* node, wofstream &results_file, vector<unsigned int> &freq_rules, bool write);

public:
	/**
	 * Default class constructor.
	 * @param wf Main web file of the class.
	 */
	BitextCandidates(WebFile* wf);

	/**
	 * Class destructor.
	 */
	virtual ~BitextCandidates();
	
	/**
	 * This method returns the main web file of the class.
	 * @throw char* The method throws an exception when the method is called and the class has not been initialised.
	 * @return Main web file of the class.
	 */
	WebFile* GetWebFile();
	
	/**
	 * This method returns the comparison data between a the main web file and its candidate for a given language.
	 * @param lang Language of the searched candidate.
	 * @return Returns the comparison data between a the main web file and its candidate for a given language. If there
	 * is no candidate for the given language, the method returns NULL.
	 */
	BitextData* GetBitextData(const wstring &lang);
	
	/**
	 * This method generates the corresponding translation memory (TM) for the main web file and each candidate of
	 * translation in the class. The results will be saved in individual TM or in an only TM memory (for each language
	 * pairs) deppending on the choosen options in the configuration file.
	 * @throw char* The method throws an exception when the method is called and the class has not been initialised.
	 * @return Returns <code>true</code> if the TMs are generated successfuly and <code>false</code> in other case.
	 */
	bool GenerateBitexts();

	/**
	 * This method adds a new candidate in the translation candidates list. The candiate will only be added if it is
	 * a better candidate than the previous assigned candidate for the corresponding language. If ther is no candidate
	 * for the corresponding language, it will be added if it passes all the defined heuristics.
	 * @param c New candidate to add.
	 * @return Returns <code>true</code> if the candidate has been added and <code>false</code> if not.
	 */
	bool Add(BitextCandidates* c);
	
	/**
	 * This method compares a given BitextData object with the assigned BitextData object assigned to an object language
	 * pair. If it is higher (or if there is no BitextData object assigned to this language) the existing candidate will
	 * be removed but the BitextData will be assigned as the better comparison data obtained for any candidate and the
	 * main web file for that language. This system is used to avoid the generation of non optimal bitexts. For
	 * example, if we are comparing two files A and B for a language pair and we assign the file B as the best candidate
	 * to be a translation of A, but in the process of comparison we realize that C is a better candidate for B for the
	 * same language pair, we must remove B from the candidates list of A. To do this, we use this method with the
	 * BitextData object generated in the comparison between B and C.
	 * @param lang BitextData corresponding language.
	 * @param d Given comparison data for comparison.
	 * @return Returns <code>true</code> if the candidate is added to the list (and, in consequence, it is better than the existing
	 * BitextCandidate assigned for the given language) and <code>false</code> if not.
	 */
	bool Add(const wstring &lang, BitextData* d);
	
	/**
	 * This method removes the last added candidate in the list of candidates.
	 */
	void EraseLastAdded();
	
	/**
	 * This method generates the translation memory corresponding to the main web file of the class and the last added
	 * candidate to the candidates list.
	 * @return Returns <code>true</code> if the candidate has been added and <code>false</code> if not.
	 */
	bool GenerateLastAddedBitext();
	
	/**
	 * This method returns the web file corresponding to the language specified by the argument lang from the candidates
	 * list.
	 * @param lang Language of the file which the method will return.
	 * @return The candidate file returns the web file corresponding to the specified language. If there is no candidate
	 * for that language, the method returns NULL.
	 */
	WebFile* GetWebFile(const wstring &lang);

	/**
	 * This method returns the last added web file in the candidates list.
	 * @return Returns the last added web file in the candidates list. If there is no added file, the method returns
	 * NULL.
	 */
	WebFile* GetLastAddedWebFile();
	
	/**
	 * EXEPEIMENTAL!!!
	 * Method which removes those pairs that doesn't correspond with the most repeated URL language rules.
	 * @param filename Name of the file where the 
	 */
	static bool CleanUnfrequentCases(const string &filename);
};

#endif /*BITEXT_H_*/
