#ifndef URL_H_
#define URL_H_

#include <iostream>
#include <map>
#include <vector>

using namespace std; 

#define DEBUG_URL_INIT
#define DEBUG_URL_CMP

class UrlLangRule
{
	public: enum LangRuleType{DIRECTORY, FILENAME, VARIABLE};
	
	private:
		
		LangRuleType rule_type;
		
		wstring value1;

		wstring value2;
	
	public:
	
		UrlLangRule(const LangRuleType &type, const wstring &val1, const wstring &val2);
		
		UrlLangRule(const LangRuleType &type, const unsigned int &val1, const unsigned int &val2);
		
		LangRuleType GetRuleType();
		
		wstring GetValue1();
		
		wstring GetValue2();

		bool operator==(const UrlLangRule &rule);
		
		bool operator<(const UrlLangRule &rule) const;
};

/**
 * @class URL
 * @brief Classe que representa una URL i es capaç d'operar sobre els elements que la componen
 * 
 * Aquesta classe emmagatzema tota la informació relacionada amb una URL: els directoris, el nom de la pàgina
 * i les variables. A partir d'ací, pot realitzar diferents càlculs per tal de comparar-les i utilitzar-les com
 * a paràmetre de valoració de la similitud entre dos fitxers web.
 * 
 * @author Miquel Esplà i Gomis
 */
class Url
{
	private:
		wstring url;
	
		vector<unsigned int> directories;
		
		wstring filename;
		
		map<wstring,wstring> variables;
	
	public:
	
		Url(wstring url);
		
		wstring GetCompleteURL();
		
		unsigned int GetDirectoriy(unsigned int &index);
		
		wstring GetVariableValue(wstring &var_name);
		
		wstring GetFilename();
		
		unsigned int GetNumberDirectories();
		
		unsigned int GetNumberVariables();
		
		bool VariableExists(wstring &var_name);
		
		unsigned int Differences(Url *url, vector<UrlLangRule*> *rules=NULL);
		
		static wstring ReplaceAmp(wstring url);
		
		static double CostCompareDirectories(const short &op, const unsigned int &dir1, const unsigned int &dir2);
};

#endif /*URL_H_*/
