/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "GlobalParams.h"
#include <libxml/xinclude.h>
#include <libtagaligner/ConfigReader.h>

double GlobalParams::max_edit_distance_length_absolute=-1;

double GlobalParams::max_edit_distance_length_percentual=-1;

int GlobalParams::directory_depth_distance=0;

double GlobalParams::text_distance_percent_differenciator=-1;

map<wstring,double> GlobalParams::file_size_diference_percents;

double GlobalParams::file_size_difference_percent=-1;

wstring GlobalParams::textcat_config_file=L"/tmp/textcat_conf.txt";

string GlobalParams::config_file;

int GlobalParams::downloaded_size=-1;

wstring GlobalParams::download_path=L"~/";

bool GlobalParams::guess_language=true;

wstring GlobalParams::fingerprints_dir=L"";

map<wstring,wstring> GlobalParams::fingerprints;

double GlobalParams::max_total_text_lenght_diff=-1;

int GlobalParams::max_nfingerprint_distance=-1;

bool GlobalParams::all_bitexts_in_one=false;

int GlobalParams::min_array_size=-1;

wofstream GlobalParams::log_file;

bool GlobalParams::verbose=false;

bool GlobalParams::create_all_candidates=false;

double GlobalParams::generate_ambiguous_bitexts=-1;

bool GlobalParams::generate_tmx=true;

wofstream GlobalParams::results_file;

map<UrlLangRule, pair<unsigned int, unsigned int> > GlobalParams::url_lang_rules;

void GlobalParams::GenerateTMX(bool generate){
	generate_tmx=generate;
}

bool GlobalParams::GetGenerateTMX(){
	return generate_tmx;
}

bool GlobalParams::AllBitextInAFile()
{
	return all_bitexts_in_one;
}

int GlobalParams::GetMaxNumericFingerprintDistance()
{
	return max_nfingerprint_distance;
}

double GlobalParams::GetMaxTotalTextLengthDiff()
{
	return max_total_text_lenght_diff;
}

/*void GlobalParams::SetMaxEditDistance(const double &value)
{
	if(value>0)
		max_edit_distance_length=value;
	else
		throw "The assigned value for the max. edit distance parameter is not valid.";
}*/

void GlobalParams::Clear()
{
	config_file="";
	max_edit_distance_length_absolute=-1;
	max_edit_distance_length_percentual=-1;
	directory_depth_distance=0;
	text_distance_percent_differenciator=-1;
	file_size_diference_percents.clear();
	file_size_difference_percent=-1;
	textcat_config_file=L"/tmp/textcat_conf.txt";
	downloaded_size=-1;
	download_path=L"~/";
	guess_language=true;
	fingerprints_dir=L"";
	max_total_text_lenght_diff=-1;
	max_nfingerprint_distance=-1;
	all_bitexts_in_one=true;
	min_array_size=-1;
	verbose=true;
	create_all_candidates=true;
	generate_ambiguous_bitexts=true;
}

double GlobalParams::GetMaxEditDistanceAbsolute()
{
	return max_edit_distance_length_absolute;
}

double GlobalParams::GetMaxEditDistancePercentual()
{
	return max_edit_distance_length_percentual;
}
	
void GlobalParams::SetDirectoryDepthDistance(const int &value)
{
	if(value>=0)
		directory_depth_distance=value;
	else
		directory_depth_distance=value*(-1); 
}
	
int GlobalParams::GetDirectoryDepthDistance()
{
	return directory_depth_distance;
}

void GlobalParams::SetTextDistancePercentDifferenciator(const double &value)
{
	if(value>0)
		text_distance_percent_differenciator=value;
	else
		text_distance_percent_differenciator=value*(-1);
}

double GlobalParams::GetTextDistancePercentDifferenciator()
{
	return text_distance_percent_differenciator;
}

double GlobalParams::GetFileSizeDifferencePercent()
{
	return file_size_difference_percent;
}	

void GlobalParams::SetFileSizeDifferencePercent(const double &value)
{
	if(value>=0)
		file_size_difference_percent=value;
	else
		file_size_difference_percent=value*(-1);
}

void GlobalParams::SetTextCatConfigFile(const wstring &path)
{
	textcat_config_file=path;
}
	
wstring GlobalParams::GetTextCatConfigFile()
{
	return textcat_config_file;
}

void GlobalParams::ProcessNode(xmlNode* node, wstring tagname){
	xmlNode *cur_node = NULL;
	xmlChar *node_prop;
	wstring key, value;
	xmlAttrPtr propPtr;
	wstring before;
	wstring after;
	map<wstring,short>::iterator iterator;
	wstring tmp;
	wstring lang1=L"", lang2=L"";
	double percent;
	bool continue_loop=true;
	wstring fingerprint=L"", lang_code=L"";

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!(cur_node->type==XML_TEXT_NODE && xmlIsBlankNode(cur_node))){
			if(cur_node->type==XML_ELEMENT_NODE && node->name!=NULL){
				if(cur_node->ns!=NULL && cur_node->ns->prefix!=NULL && Config::xmlToWstring((xmlChar*)cur_node->ns->prefix)==L"bi")
					tagname=Config::xmlToWstring((xmlChar*)cur_node->name);
				else
					continue_loop=false;
			}
			if(cur_node->type==XML_TEXT_NODE){
				if (tagname == L"downloadModDest")
					download_path=Config::xmlToWstring(cur_node->content);
			}
			else if(cur_node->type==XML_ELEMENT_NODE){
				propPtr = cur_node->properties;
				while(propPtr) {
					key = Config::xmlToWstring(propPtr->name);
					node_prop=xmlGetProp( cur_node, propPtr->name);
					value = Config::xmlToWstring(node_prop);
					if(tagname==L"downloadModDest" && key==L"max_bytes"){
						downloaded_size = atoi(Config::toString(value).c_str());
					}
					else if (tagname == L"maxEditDistance"){
						if(key==L"absolute"){
							max_edit_distance_length_absolute=atof(Config::toString(value).c_str());
						}
						else if(key==L"percentual"){
							max_edit_distance_length_percentual=atof(Config::toString(value).c_str());
						}
					}
					else if (tagname == L"textLengthDifferencePercents" && key==L"default"){
						text_distance_percent_differenciator = atof(Config::toString(value).c_str());
					}
					else if (tagname == L"directoryPathDistance" && key==L"value"){
						directory_depth_distance=atoi(Config::toString(value).c_str());
					}
					else if (tagname == L"textLengthDifferencePercent"){
						if(key==L"lang1")
							lang1 = value;
						else if(key==L"lang2")
							lang2 = value;
						else if(key==L"difference"){
							percent=atof(Config::toString(value).c_str());
							if(lang1!=L"" && lang2!=L""){
								AddFileSizeDiferencePercent(lang1,lang2,percent);
								lang1=L"";
								lang2=L"";
							}
						}
					}
					else if(tagname == L"languages" && key==L"dir"){
						if(value[value.length()-1]!=L'/')
							fingerprints_dir=value+L"/";
						else
							fingerprints_dir=value;
					}
					else if(tagname == L"language"){
						if(key==L"fingerprint")
							fingerprint=value;
						else if(key==L"code")
							lang_code=value;
						if(lang_code!=L"" && fingerprint!=L""){
							fingerprints[lang_code]=fingerprints_dir+fingerprint;
							lang_code=L"";
							fingerprint=L"";
						}
					}
					else if (tagname == L"fileSizePercent" && key==L"value"){
						file_size_difference_percent = atof(Config::toString(value).c_str());
					}
					else if (tagname == L"maxTotalTextLengthPercent" && key==L"value"){
						max_total_text_lenght_diff=atof(Config::toString(value).c_str())/100;
					}
					else if (tagname == L"numericFingerprintDistance" && key==L"value"){
						max_nfingerprint_distance=atoi(Config::toString(value).c_str());
					}
					else if (tagname == L"minArraySize" && key==L"value"){
						min_array_size=atoi(Config::toString(value).c_str());
					}
					else if (tagname == L"onlyBitextFile" && key==L"value"){
						if(value==L"true")
							all_bitexts_in_one=true;
						else
							all_bitexts_in_one=false;
					}
					else if (tagname == L"generateAllCandidates" && key==L"value"){
						if(value==L"true")
							create_all_candidates=true;
						else
							create_all_candidates=false;
					}
					else if (tagname == L"verbose" && key==L"value"){
						if(value==L"true")
							verbose=true;
						else
							verbose=false;
					}
					else if (tagname==L"generateAmbiguousBitexts" && key==L"max_text_percent"){
						generate_ambiguous_bitexts=atoi(Config::toString(value).c_str());
					}
					free(node_prop);
					propPtr = propPtr->next;
				}
			}
			if(!xmlIsBlankNode(cur_node) && continue_loop)
			{
				//Recurse through child nodes:
				ProcessNode(cur_node->children, tagname);
			}
			continue_loop=true;
		}
	}
}

void GlobalParams::GenerateTextCatConfigFile()
{
	wofstream os;
	map<wstring,wstring>::iterator it;

	os.open("/tmp/textcat_conf.txt",ios::out);
	for(it=fingerprints.begin();it!=fingerprints.end();it++)
		os<<it->second<<L" "<<it->first<<endl;
	os.close();
}

bool GlobalParams::LoadGlobalParams(const string &path)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	config_file=path;
	bool exit;
	
	try{
		doc = xmlReadFile(path.c_str(), NULL, 0);
		if(doc==NULL)
			throw "The specified configuration file does'nt exists.";
		else{
			xmlXIncludeProcess(doc);
			Config::buildTagSet(doc);
			root_element = xmlDocGetRootElement(doc);
			ProcessNode(root_element, L"");
			xmlFreeDoc(doc);
			xmlCleanupParser();
			GenerateTextCatConfigFile();
		}	
	    exit=true;
	}
	catch ( const std::exception& ex ) {
		exit=false;
	}
    return exit;
}

void GlobalParams::AddFileSizeDiferencePercent(const wstring &lang1, const wstring &lang2, const double &percent)
{
	file_size_diference_percents[lang1+L"_"+lang2]=percent;
	file_size_diference_percents[lang2+L"_"+lang1]=percent;
}

double GlobalParams::GetFileSizeDiferencePercent(const wstring &lang1, const wstring &lang2)
{
	map<wstring, double>::iterator iter = file_size_diference_percents.find(lang1+L"_"+lang2);
	
	if (iter != file_size_diference_percents.end())
		return iter->second;
	else
		return text_distance_percent_differenciator;
}

int GlobalParams::GetMaxDownloadedSize()
{
	return downloaded_size;
}

wstring GlobalParams::GetDownloadPath()
{
	return download_path;
}

void GlobalParams::SetGuessLanguage(const bool &value)
{
	guess_language=value;
}

bool GlobalParams::GetGuessLanguage()
{
	return GlobalParams::guess_language;
}

int GlobalParams::GetMinArraySize()
{
	return min_array_size;
}

void GlobalParams::WriteLog(const wstring &log_text)
{
	time_t rtime;
	struct tm* rawtime;

	if(log_file.is_open()){
		time(&rtime);
		rawtime=localtime( &rtime );
		log_file<<rawtime->tm_mday<<L"/"<<rawtime->tm_mon+1<<L"/"<<rawtime->tm_year<<L" "<<rawtime->tm_hour<<L":"<<rawtime->tm_min<<L":"<<rawtime->tm_sec<<L">> "<<log_text<<endl;
	}
}

void GlobalParams::WriteResults(const wstring &result_text)
{
	if(results_file.is_open()){
		results_file<<L"\t"<<result_text<<endl;
	}
}

bool GlobalParams::OpenLog(const string &log_path)
{
	if(log_file.is_open())
		log_file.close();
	log_file.open(log_path.c_str());
	return log_file.is_open();
}

bool GlobalParams::OpenResults(const string &results_path)
{
	if(results_file.is_open())
		results_file.close();
	results_file.open(results_path.c_str());
	if(results_file.is_open())
		results_file<<"<?xml version='1.0' encoding='UTF-8'?>"<<endl<<L"<bitextcandidates xmlns=\"http://bitextor.sf.net\">"<<endl;
	return results_file.is_open();
}

void GlobalParams::CloseLog()
{
	if(log_file.is_open())
		log_file.close();
}

bool GlobalParams::GetCreateAllCandidates(){
	return GlobalParams::create_all_candidates;
}

bool GlobalParams::IsVerbose()
{
	return GlobalParams::verbose;
}

void GlobalParams::SetVerbose()
{
	verbose=true;
}

double GlobalParams::GetGenerateAmbiguousBitexts()
{
	return generate_ambiguous_bitexts;
}

void GlobalParams::CloseResults()
{
	if(results_file.is_open()){
		results_file<<L"</bitextcandidates>";
		results_file.close();
	}
}

unsigned int GlobalParams::AddUrlLangRule(UrlLangRule *rule){
	if(url_lang_rules.find(*rule)!=url_lang_rules.end()){
		url_lang_rules[*rule].second+=1;
	}
	else{
		url_lang_rules[*rule].first=url_lang_rules.size();
		url_lang_rules[*rule].second=1;
	}
	return url_lang_rules[*rule].first;
}

vector<unsigned int> * GlobalParams::GetFreqRules(unsigned int min_count){
	map<UrlLangRule, pair<unsigned int, unsigned int> >::iterator it;
	vector<unsigned int>* eixida=new vector<unsigned int>();
	
	for(it=url_lang_rules.begin();it!=url_lang_rules.end();it++){
		if(it->second.second>=min_count)
			eixida->push_back(it->second.first);
	}
	return eixida;
}
