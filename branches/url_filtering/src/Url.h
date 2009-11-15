#ifndef URL_H_
#define URL_H_

#include <iostream>
#include <map>
#include <vector>
#include "WebSite.h"

//using namespace std; 

//#define DEBUG_URL_INIT
//#define DEBUG_URL_CMP

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
	
		vector<wstring> directories;
		
		wstring filename;
		
		map<wstring,wstring> variables;
		
		static bool GetUrlAndFilename(xmlNode *cur_node, wstring &filename, wstring &url);
	
	public:
	
		Url(wstring url);
		
		wstring GetCompleteURL();
		
		wstring GetDirectoriy(unsigned int &index);
		
		wstring GetVariableValue(wstring &var_name);
		
		wstring GetFilename();
		
		unsigned int GetNumberDirectories();
		
		unsigned int GetNumberVariables();
		
		bool VariableExists(wstring &var_name);
		
		unsigned int Differences(Url *url);
		
		static bool ProcessHTtrackLogFile(string &file_path, string &dest_dir);
		
		static bool FilterWebFilesFromUrls(string &dest_path, WebSite &ws);
};

/*class UrlLanguageRule
{
	public enum LangRuleType{DIRECTORY, FILENAME, VARIABLE};
	
	private:
		
		LangRuleType rule_type;
		
		wstring value;
	
	public:
	
		Url(wstring url);
		
		wstring GetDirectoriy(unsigned int &index);
		
		wstring GetVariableValue(wstring &var_name);
		
		wstring GetFilename();
		
		unsigned int GetNumberDirectories();
		
		unsigned int GetNumberVariables();
		
		bool VariableExists(wstring &var_name);
};*/

#endif /*URL_H_*/
