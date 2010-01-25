#ifndef GLOBALPARAMS_H_
#define GLOBALPARAMS_H_

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <libxml/parser.h>
#include "Url.h"

using namespace std;

#define IRRELEVANT 1

/**
 * @class GlobalParams
 * @brief This class contains the global parameters of the application.
 * 
 * Static class that contains all the necessary global parameters for
 * the application.
 * 
 * @author Miquel Esplà i Gomis
 */
class GlobalParams
{
private:

	/**
	 * Variable that contains the path where the configuration file is
	 * placed in the system.
	 */
	static string config_file;

	/**
	 * Maximum edit distance (in absolute terms) allowed to determine if
	 * two web-file fingerpints (see documentation about WebFile) can be
	 * the same or not. The value of this variable is, by default, -1,
	 * and this means that there is no maximum absolute edit distance.
	 */
	static double max_edit_distance_length_absolute;

	/**
	 * Maximum edit distance (in percentual terms) allowed to determine
	 * if two web-file fingerpints (see documentation about WebFile) can
	 * be the same or not. To compare this parameter with the resulting
	 * value of the edit distance, it is applied the formula:
	 * edit_distance_result/MAX(LENGTH(fingerprint1),LENGTH(fingerprint2)
	 * The value of this variable is, by default, -1, and this means that
	 * there is no maximum percentual edit distance.
	 */
	static double max_edit_distance_length_percentual;

	/**
	 * Maximum difference between the depth in the directory tree of a
	 * pair of files in the website. 
	 * The value of this variable is, by default, -1, and this means that
	 * there is no maximum directory depth distance.
	 */
	static int directory_depth_distance;

	/**
	 * During fingerprints comparison, text blocks leght in the XHTML file are compared. To do
	 * this, the ratio of difference between lengths is used. This ratio can be limited for a
	 * concrete pair of languages by the user, but if there is no restriction for a given pair
	 * of languages, a default trheshold is applied. This parametter reesents that percentual
	 * threshold.
	 */
	static double text_distance_percent_differenciator;

	/**
	 * List of thresholds for difference ratio between text blocks lenths for a given pair of
	 * languages.
	 */
	static map<wstring,double> text_distance_diference_percents;

	/**
	 * Percentual threshold which limits the maximum difference of size ratio between files.
	 */
	static double file_size_difference_percent;

	/**
	 * Path where the configuration file of LibTextCat is placed.
	 */
	static wstring textcat_config_file;

	/**
	 * Maximum size (in bites) allowed to download from the target website.
	 */
	static int downloaded_size;

	/**
	 * Path where websites will be downloaded.
	 */
	static wstring download_path;

	/**
	 * If this parametter is true, bitextor will try to guess the language in which a XHMLT file
	 * is written. If it is false, it will ask the user the language of the file fore each file
	 * in the website.
	 */
	static bool guess_language;

	/**
	 * Path where the language model files used by LibTextCat are placed.
	 */
	static wstring fingerprints_dir;
	
	/**
	 * Threshold which limits the maxumum ratio of plain text length difference .
	 */
	static double max_total_text_lenght_diff;

	/*
	** 
	 * Màxima distància permesa entre els arrays d'enters, en valor absolut.
	 *
	static int max_nfingerprint_distance;*/

	/**
	 * If this variable is set to <code>true</code> all the translation units (TU) generated
	 * from the XHTML files comparison will be saved in an only translation memory (TM)
	 * (corresponding to the language pair). If it is set to <code>false</code>, a TM will be
	 * generated for each XHTML file pair compared.
	 * 
	 */
	static bool all_bitexts_in_one;

	/**
	 * File where the log information will be saved. If it is set to null, no log file will be
	 * generated.
	 */
	static wofstream log_file;

	/**
	 * Threshold which limits the minimum number of elements in a XHTML file fingerprint to be
	 * compared with the otherones.
	 */
	static int min_array_size;

	/**
	 * Variable with determines if the system will run in versbose mode or not.
	 */
	static bool verbose;

	/**
	 * If this variable is set to <code>true</code>, all the pairs of files which pass the
	 * comparison heuristics will be accepted and, in consequence, will be aligned and included
	 * in the resulting TM or TMs. If it is set to <code>false</code>, only the best pairs of
	 * files will be included in the TM generation process. In this case, if we have two files,
	 * A and B, they will a good candidate if the lowest edit distance for A is obtained in
	 * comparison with B between all the compared files, and viceversa.
	 */
	static bool create_all_candidates;

	/**
	 * This method generates the LibTextCat configuration file from the parameters defined in
	 * the configuration file of Bitextor.
	 */
	static void GenerateTextCatConfigFile();

	/**
	 * Threshold which determines if translation memories should be generated in case that a
	 * file have very similar edit distances in comparison to two (or more) different files.
	 */
	static double generate_ambiguous_bitexts;

	/**
	 * This variable determines if translation memories must be generated or only a list of
	 * pair candidates must be generated.
	 */
	static bool generate_tmx;
	
	/**
	 * Map of directories-integers in the website.
	 */
	static map<wstring,unsigned int> url_directories_code;

	/**
	 * File where the list of choosen pairs of files are saved (if this option is on).
	 */
	static wofstream results_file;
	
	/**
	 * Map which contains found URL language rules, an integer identificator and the number of
	 * times that that rule have appeared.
	 */
	static map<UrlLangRule, pair<unsigned int, unsigned int> > url_lang_rules;
	
	

public:
	/**
	 * List of language model files path (used by LibTextCat).
	 */
	static map<wstring,wstring> fingerprints;

	/**
	 * Method which cleans up all the reserved dynamic memory reserved by the class and rsets
	 * the parameters to the default values.
	 */
	static void Clear();
	
	/**
	 * This method returns the maximum edit distance (in percentual terms)
	 * allowed to determine if two web-file fingerpints (see documentation
	 * about WebFile) can be the same or not.
	 * @return Returns maximum edit distance threshold. If no value is defined, it returns -1.
	 */
	static double GetMaxEditDistancePercentual();

	/**
	 * This method returns the maximum edit distance (in absolute terms)
	 * allowed to determine if two web-file fingerpints (see documentation
	 * about WebFile) can be the same or not.
	 * @return Returns maximum edit distance threshold. If no value is defined, it returns -1.
	 */
	static double GetMaxEditDistanceAbsolute();
	
	/**
	 * This method returns the default limit of the ratio of difference between lengths of text
	 * blocks for edit distance comparison between fingerprints of files.
	 * @return Returns the default limit of the ratio of difference between lengths of text blocks.
	 */
	static double GetTextDistancePercentDifferenciator();

	/**
	 * This method returns the default limit of the ratio of difference between lengths of text
	 * blocks for edit distance comparison between fingerprints of files.
	 * @param value Valor que es vol establir.
	 */
	static void SetTextDistancePercentDifferenciator(const double &value);
	
	/**
	 * This method returns the maximum difference between the depth in the directory tree
	 * of a pair of files in the website. 
	 * @return Returns the maximum difference between the depth in the directory tree
	 * of a pair of files in the website.
	 */
	static int GetDirectoryDepthDistance();

	/**
	 * This method allows to set the maximum difference between the depth in the directory tree
	 * of a pair of files in the website. 
	 * @param New value of maximum directory distance.
	 */
	static void SetDirectoryDepthDistance(const int &value);
	
	/**
	 * This method loads all the configuration parameters from a XML configuration file.
	 * @param path Path where the configuation file is placed in the system.
	 * @throw char* The method throws an exception fi the specified file cannot be oppened.
	 * @return Returns <code>true</code> if file is correctly loaded and <code>false</code> in other case.
	 */
	static bool LoadGlobalParams(const string &path);
	
	/**
	 * Method which returns the percentual threshold which limits the maximum difference of
	 * size ratio between files.
	 * @return Returns the percentual threshold which limits the maximum difference of
	 * size ratio between files.
	 */
	static double GetFileSizeDifferencePercent();
	
	/**
	 * Method which sets the percentual threshold which limits the maximum difference of
	 * size ratio between files.
	 * @param value New value for the parametter
	 */
	static void SetFileSizeDifferencePercent(const double &value);
	
	/**
	 * Method which sets the path where the LibTextCat configuration path is placed in the system.
	 * @param path Path of the LibTextCat configuration file in the system.
	 */
	static void SetTextCatConfigFile(const wstring &path);
	
	/**
	 * Method which returns the path where the LibTextCat configuration path is placed in the system.
	 * @return Returns the path where the LibTextCat configuration path is placed in the system.
	 */
	static wstring GetTextCatConfigFile();

	/**
	 * Auxiliar method to read the configuration file.
	 * @param node Processed node.
	 * @param tagname Name of the last tag readden.
	 */
	static void ProcessNode(xmlNode* node, wstring tagname);

	/**
	 * Method which returns the maximum total size which Bitextor can download from a website.
	 * @return Returns the path where the LibTextCat configuration path is placed in the system.
	 */
	static int GetMaxDownloadedSize();

	/**
	 * Method which returns the path where downloaded files are saved.
	 * @return Returns the path where downloaded files are saved.
	 */
	static wstring GetDownloadPath();
	
	/**
	 * Method which sets the mode in which Bitextor detects the language in which analysed files
	 * is guessed: manually of authomatically.
	 * @param value If value is <code>true</code>, language of files will be guessed authomatically,
	 * and if it is <code>false</code>, Bitextor will ask the user for the language of each analysed
	 * file.
	 */
	static void SetGuessLanguage(const bool &value);
	
	/**
	 * Method which returns the mode in which Bitextor detects the language in which analysed files
	 * is guessed: manually of authomatically.
	 * @param value Returns <code>true</code> if language of files is guessed authomatically,
	 * and <code>false</code> if Bitextor asks the user for the language of each analysed
	 * file.
	 */
	static bool GetGuessLanguage();
	
	/**
	 * Method which adds an element to the list of language-pairs limit of ratio of difference
	 * between text block lengths. It is important to know that the order of languages is not
	 * important when adding a new language pair value.
	 * @param lang1 First language's code.
	 * @param lang2 Second language's code.
	 * @param percent Value of maximum ratio of difference between text block lengths for the language pair.
	 */
	static void AddTextLengthDiferencePercent(const wstring &lang1, const wstring &lang2, const double &percent);
	
	/**
	 * Method which returns an element of the list of language-pairs limit of ratio of difference
	 * between text block lengths. It is important to know that the order of languages is not
	 * important when getting a language pair value.
	 * @param lang1 First language's code.
	 * @param lang2 Second language's code.
	 * @param percent Returns the value of maximum ratio of difference between text block lengths for the language pair.
	 */
	 static double GetTextLengthDiferencePercent(const wstring &lang1, const wstring &lang2);

	/**
	 * Method which returns the maximum ratio of difference between plain text allowed by Bitextor.
	 * @return Returns the maximum ratio of difference between plain text allowed by Bitextor.
	 */
	static double GetMaxTotalTextLengthDiff();

	/**
	 * Mètode que retorna el número màxim de diferències permeses entre els arrays d'enters de dos WebFile.
	 * @return Retorna el número màxim de diferències permeses entre els arrays d'enters de dos WebFile.
	 */
	//static int GetMaxNumericFingerprintDistance();

	/**
	 * Method which indicates if all the translation memories resulting of the processment of
	 * found bitexts will be saved in a same file or in separated files.
	 * @return Returns <code>true</code> if all the translation memories resulting of the processment of
	 * found bitexts will be saved in a same file or <code>false</code> if they will be saved
	 * in separated files.
	 */
	static bool AllBitextInAFile();

	/**
	 * Method which returns the minimum length which must have a XHTML file fingerprint to be
	 * processed by Bitextot in comparisson process.
	 * @return Returns the minimum length which must have a XHTML file fingerprint to be
	 * processed by Bitextot in comparisson process.
	 */
	static int GetMinArraySize();

	/**
	 * Method which writtes a line in the file log. If there is not any log file, nothing is done.
	 * @param log_text Text which must be written in the log file.
	 */
	static void WriteLog(const wstring &log_text);

	/**
	 * Method which opens the log file to start writting.
	 * @param log_path Path where log file is placed in the system.
	 * @return Returns <code>true</code> si file has been succesfuly oppened and <code>false</code> if not.
	 */
	static bool OpenLog(const string &log_path);

	/**
	 * Method which closes the log file.
	 */
	static void CloseLog();

	/**
	 * Method which indicates if translation memorie must be created for each candidate bitext or
	 * if it must be only done with best candidates.
	 * @return Returns <code>true</code> if translation memorie must be created for each candidate bitext
	 * and <code>false</code> if it must be only done with best candidates.
	 */
	static bool GetCreateAllCandidates();

	/**
	 * Method which indicates if Bitextor is been runed in verbose mode or not.
	 * @return Returns <code>true</code> if Bitextor is been runed in verbose mode and <code>false</code> if not.
	 */
	static bool IsVerbose();

	/**
	 * Method which sets the verbose mode.
	 */
	static void SetVerbose();

	/**
	 * Method which indicates if translation memories should be generated in case that a file
	 * have very similar edit distances in comparison to two (or more) different files.
	 * 
	 * @return Reteturns <code>true</code> if translation memory should be created and <code>false</code> if not.
	 */
	static double GetGenerateAmbiguousBitexts();

	/**
	 * Method which sets the value for the flag that indicates if TMX files must be generated
	 * or not.
	 * @param generate Value for the flag that indicates if TMX files must be generated
	 * or not.
	 */
	static void GenerateTMX(bool generate);

	/**
	 * Method which returns the value for the flag that indicates if TMX files must be generated
	 * or not.
	 * @param generate Returns the value for the flag that indicates if TMX files must be generated
	 * or not.
	 */
	static bool GetGenerateTMX();

	/**
	 * Method which writtes a line in the results file, in which information about found bitexts
	 * is saved.
	 * @param result_text Text which must be written in the results file.
	 */
	static void WriteResults(const wstring &result_text);

	/**
	 * Method which opens the results file, in which information about found bitexts
	 * is saved.
	 * @param results_path Path where the results file is placed in the system.
	 * @return Returns <code>true</code> if file has been oppened succesfuly and <code>false</code> if not.
	 */
	static bool OpenResults(const string &results_path);

	/**
	 * Method which closes the results file.
	 */
	static void CloseResults();

	/**
	 * Method which adds a new URL rule to a list. These rules are substrings of difference between URLs.
	 * @param rule The rule which is added to the list.
	 * @return It returns the integer code mapped to the rule.
	 */
	static unsigned int AddUrlLangRule(UrlLangRule *rule);
	
	/**
	 * This method returns all the rules which have appeared more than a given number of times.
	 * @param min_freq Minimum number of times which the rule has to have appeared to be returned.
	 * @return Returns all the rules which have appeared more than a given number of times.
	 */
	static vector<unsigned int> * GetFreqRules(unsigned int min_freq);
	
	/**
	 * Method which returns the numeric code for a given directory in the path of the URL.
	 * @param dir_name Name of the directory for which the code will be returned.
	 * @return Returns the numeric code for the given directory in the path of a URL.
	 */
	static unsigned int GetURLDirectoryCode(const wstring &dir_name);
};

#endif /*GLOBALPARAMS_H_*/
