#include "GlobalParams.h"

double GlobalParams::max_edit_distance_length=-1;
	
int GlobalParams::directory_depth_distance=0;

double GlobalParams::text_distance_percent_differenciator=-1;

map<wstring,double> GlobalParams::file_size_diference_percents;

double GlobalParams::file_size_difference_percent=-1;

//wstring GlobalParams::tagaligner_config_file=L"";

wstring GlobalParams::textcat_config_file=L"/tmp/textcat_conf.txt";

//int GlobalParams::tagaligner_mode=1;

string GlobalParams::config_file;

int GlobalParams::downloaded_size=-1;

//vector<wstring> GlobalParams::accepted_extenssions;

wstring GlobalParams::download_path=L"~/";

bool GlobalParams::guess_language=true;

bool GlobalParams::is_percentual_edmax=true;

wstring GlobalParams::fingerprints_dir=L"";

map<wstring,wstring> GlobalParams::fingerprints;

void GlobalParams::SetMaxEditDistance(const double &value)
{
	if(value>0)
		max_edit_distance_length=value;
	else
		throw "The assigned value for the max. edit distance parameter is not valid.";
}

double GlobalParams::GetMaxEditDistance()
{
	return max_edit_distance_length;
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

bool GlobalParams::IsPercentMaxEditDistance()
{
	return is_percentual_edmax;
}

void GlobalParams::ProcessNode(xmlNode* node, wstring tagname){
	xmlNode *cur_node = NULL;
	xmlChar *node_prop;
	wstring key, value;
	xmlAttrPtr propPtr;
	short op=EMPTY;
	short in=EMPTY;
	short out=EMPTY;
	wstring before;
	wstring after;
	double dvalue;
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
				/*else if (tagname == L"textCatConfigFile"){
					textcat_config_file = Config::xmlToWstring(cur_node->content);
				}*/
				else if (tagname == L"fileSizePercent"){
					file_size_difference_percent = atof(Config::toString(Config::xmlToWstring(cur_node->content)).c_str());
				}
				else if (tagname == L"maxEditDistance"){
					max_edit_distance_length=atof(Config::toString(Config::xmlToWstring(cur_node->content)).c_str());
				}
				else if (tagname == L"directoryPathDistance"){
					directory_depth_distance=atoi(Config::toString(Config::xmlToWstring(cur_node->content)).c_str());
				}
			}
			else if(cur_node->type==XML_ELEMENT_NODE){
				propPtr = cur_node->properties;
				while(propPtr) {
					key = Config::xmlToWstring(propPtr->name);
					node_prop=xmlGetProp( cur_node, propPtr->name);
					value = Config::xmlToWstring(node_prop);
					if(tagname==L"downloadModDest" && key==L"max_size"){
						downloaded_size = atoi(Config::toString(value).c_str());
					}
					else if (tagname == L"maxEditDistance" && key==L"mode"){
						if( value==L"percent" )
							is_percentual_edmax=true;
						else if( value==L"absolute" )
							is_percentual_edmax=false;
					}
					else if (tagname == L"textLengthDifferencePercents"){
						if(key==L"default")
							text_distance_percent_differenciator = atof(Config::toString(value).c_str());
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
					else if(tagname == L"languages"){
						if(key==L"dir"){
							if(value[value.length()-1]!=L'/')
								fingerprints_dir=value+L"/";
							else
								fingerprints_dir=value;
						}
					}
					else if(tagname == L"language"){
						if(key==L"fingerprint")
							fingerprint=value;
						else if(key==L"lang_code")
							lang_code=value;
						if(lang_code!=L"" && fingerprint!=L""){
							fingerprints[lang_code]=fingerprints_dir+fingerprint;
							lang_code=L"";
							fingerprint=L"";
						}
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
	map<wstring, double>::iterator iter = file_size_diference_percents.find(lang1+L""+lang2);
	
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
	return guess_language;
}
