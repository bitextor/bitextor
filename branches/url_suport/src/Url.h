#ifndef URL_H_
#define URL_H_

#include <iostream>
#include <map>
#include <vector>

using namespace std; 

//#define DEBUG_URL_INIT
//#define DEBUG_URL_CMP

/**
 * @class UrlLangRule
 * @brief This class represents a rule which indicates the language of the webpage in the url.
 * 
 * This class treates the segment of the URL which identifies the language in the URL. It is used to check the differences
 * between URLs.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class UrlLangRule
{
	/**
	 * There are three types of differences between URLs: differenecs in the directory structure, differences between the
	 * filenames and differences between the values of the variables.
	 */
	public: enum LangRuleType{DIRECTORY, FILENAME, VARIABLE};
	
	private:
		/**
		 * Rule type.
		 */
		LangRuleType rule_type;
		
		/**
		 * Segment of the first URL.
		 */
		wstring value1;

		/**
		 * Segment of the second URL.
		 */
		wstring value2;
	
	public:
		/**
		 * Class constructor using the type of the difference, the segment of the first URL and the segment of the second
		 * URL. It is only applied when the difference is a variable or the file name.
		 * @param type The type of language rule in the URL.
		 * @param val1 The segment of difference the first URL.
		 * @param val2 The segment of difference the second URL.
		 */
		UrlLangRule(const LangRuleType &type, const wstring &val1, const wstring &val2);
		
		/**
		 * Class constructor using the type of the difference, the segment of the first URL and the segment of the second
		 * URL. This method is used with differences in the directory structure. The directory names are maped to
		 * integers, so this method compares a pair of integers considering them directory names.
		 * @param type The type of language rule in the URL.
		 * @param val1 The segment of difference the first URL.
		 * @param val2 The segment of difference the second URL.
		 */
		UrlLangRule(const LangRuleType &type, const unsigned int &val1, const unsigned int &val2);

		/**
		 * This method returns the rule type.
		 * @return Returns the rule type.
		 */
		LangRuleType GetRuleType();

		/**
		 * This method returns the first difference value.
		 * @return Returns the first difference value.
		 */
		wstring GetValue1();

		/**
		 * This method returns the second difference value.
		 * @return Returns the second difference value.
		 */
		wstring GetValue2();

		/**
		 * The equality operator. This method compares two UrlLanguageRule objects to know if they are equal.
		 * @param rule The right side of the comparison.
		 * @return Returns <code>true</code> if the objects are equal and <code>false</code> if they are different.
		 */
		bool operator==(const UrlLangRule &rule);

		/**
		 * The "lower than" operator. This method compares two UrlLanguageRule objects to know if the left one is lower
		 * than the right one.
		 * @param rule The right side of the comparison.
		 * @return This method compares, firstly, the type of rule. If the types are different, the method returns
		 * <code>false</code>. If the types are the same, then, the segment of the first URL are compared. If the left
		 * one is lower than the right one, the method returns <code>true</code> and if it is higher it returns 
		 * <code>false</code>. If they are equal, the second URL segment is compared and if the left side segment is
		 * lower the method returns <code>true</code> and if it is equal or higher returns <code>false</code>.
		 */
		bool operator<(const UrlLangRule &rule) const;
};

/**
 * @class URL
 * @brief This class represents a URL and it is able to perform some operations on it.
 * 
 * This class saves all the information related with the URL: the directories, the name of the file and the variables.
 * From this moment, it can perform different calculations to compare them and use them as a comparison parameter to
 * valorate the similarity.
 * 
 * @author Miquel Esplà i Gomis
 */
class Url
{
	private:
		/**
		 * The complete URL.
		 */
		wstring url;
	
		/**
		 * List of directories in the URL. These directories are maped to integers to make easier the comparison between
		 * directories.
		 */
		vector<unsigned int> directories;

		/**
		 * Filename in the URL.
		 */
		wstring filename;

		/**
		 * List of variables. The name of the variable is saved in the left side of the map and the value of the variable
		 * is saved in the rigth side.
		 */
		map<wstring,wstring> variables;
	
	public:
	
		/**
		 * Class constructor.
		 * @param url The complete URL represented.
		 */
		Url(wstring url);
		
		/**
		 * This method returns the complete URL represented by the object.
		 * @return Returns the complete URL.
		 */
		wstring GetCompleteURL();
		
		/**
		 * This method returns a directory in the directories list.
		 * @param index Index of the element which must be returned.
		 * @return Returns the directory code placed in the position indexed. If there is no element placed in the
		 * indicated position the method return -1.
		 */
		unsigned int GetDirectoriy(unsigned int &index);

		/**
		 * This method returns a variable value in the variables list.
		 * @param var_name Name of the searched variable.
		 * @return Returns the variable value corresponding to the given variable name. If there is no value
		 * corresponding to the variable name, the method returns an empty string.
		 */
		wstring GetVariableValue(wstring &var_name);
		
		/**
		 * The method returns the filename in the URL.
		 * @return Returns the filename in the URL.
		 */
		wstring GetFilename();
		
		/**
		 * The method returns the number of directories in the URL.
		 * @return Returns the number of directories in the URL.
		 */
		unsigned int GetNumberDirectories();
		
		unsigned int GetNumberVariables();
		
		bool VariableExists(wstring &var_name);
		
		unsigned int Differences(Url *url, vector<UrlLangRule*> *rules=NULL);
		
		static wstring ReplaceAmp(wstring url);
		
		static double CostCompareDirectories(const short &op, const unsigned int &dir1, const unsigned int &dir2);
};

#endif /*URL_H_*/
