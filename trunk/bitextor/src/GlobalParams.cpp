<<<<<<< .working
#include "GlobalParams.h"

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

bool GlobalParams::all_bitexts_in_one=true;

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
=======
#include "GlobalParams.h"

float GlobalParams::max_edit_distance_length=-1;

map< string,int > GlobalParams::tag_map;

int GlobalParams::tag_map_counter=-1;

vector< string > GlobalParams::irrelevant_tags;
	
int GlobalParams::directory_depth_distance=0;

float GlobalParams::text_distance_percent_differenciator=-1;

float GlobalParams::file_size_difference_percent=-1;

string GlobalParams::tagaligner_config_file="";

string GlobalParams::textcat_config_file="";

int GlobalParams::tagaligner_mode=1;

string GlobalParams::config_file;

int GlobalParams::downloaded_size=-1;

vector<string> GlobalParams::accepted_extenssions;

string GlobalParams::download_path="~/";

bool GlobalParams::guess_language=true;

void GlobalParams::SetMaxEditDistance(float value)
{
	if(value>0)
		max_edit_distance_length=value;
	else
		throw "The assigned value for the max. edit distance parameter is not valid.";
}

float GlobalParams::GetMaxEditDistance()
{
	return max_edit_distance_length;
}

int GlobalParams::GetHTMLTagValue(string tag)
{
	if(Config::getTagType(tag)==IRRELEVANT)
		return IRRELEVANT;
	else{
		if(tag_map.find(tag)==tag_map.end()){
			tag_map[tag]=tag_map_counter;
			tag_map_counter--;
		}
		return tag_map[tag];
	}	
}
	
void GlobalParams::AddIrrelevantTag(string tag)
{
	if(tag_map.find(tag)==tag_map.end()){
		irrelevant_tags.push_back(tag);
		tag_map[tag]=(-1)*tag_map_counter;
	}
}
	
void GlobalParams::SetDirectoryDepthDistance(int value)
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

void GlobalParams::SetTextDistancePercentDifferenciator(float value)
{
	if(value>0)
		text_distance_percent_differenciator=value;
	else
		text_distance_percent_differenciator=value*(-1);
}

float GlobalParams::GetTextDistancePercentDifferenciator()
{
	return text_distance_percent_differenciator;
}

float GlobalParams::GetFileSizeDifferencePercent()
{
	return file_size_difference_percent;
}	

void GlobalParams::SetFileSizeDifferencePercent(float value)
{
	if(value>=0)
		file_size_difference_percent=value;
	else
		file_size_difference_percent=value*(-1);
}

void GlobalParams::SetTextCatConfigFile(string path)
{
	textcat_config_file=path;
}
	
string GlobalParams::GetTextCatConfigFile()
{
	return textcat_config_file;
}
	
void GlobalParams::SetTagAlignerConfigFile(string path)
{
	tagaligner_config_file=path;
}
	
string GlobalParams::GetTagAlignerConfigFile()
{
	return tagaligner_config_file;
}

void GlobalParams::SetTagAlignerMode(int mode)
{
	if(mode<=3 && mode>=1)
		tagaligner_mode=mode;
	else
		throw "The assigned value for the TagAligner mode parameter is not valid.";
}
	
int GlobalParams::GetTagAlignerMode()
{
	return tagaligner_mode;
}

void GlobalParams::ProcessNode(const xmlpp::Node* node, string tagname="", unsigned int indentation = 0)
{
	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
	const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
	const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

	string op;
	string in;
	string out;
	string before;
	string after;
	string accepted_ext;

	//if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
	//	return;
    
	string nodename = node->get_name();

	if(!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
		tagname = node->get_name();
  
	//Treat the various node types differently: 
	if(nodeText){

		if (tagname == "textCatConfigFile")
			textcat_config_file = nodeText->get_content();
	
		else if (tagname == "tagAlignerConfigFile")
			tagaligner_config_file = nodeText->get_content();
	
		else if (tagname == "tagAlignerMode"){
			if(nodeText->get_content()=="ad")
				tagaligner_mode=1;
			else if(nodeText->get_content()=="l")
				tagaligner_mode=2;
			else if(nodeText->get_content()=="direct")
				tagaligner_mode=3;
		}
	
		else if (tagname == "fileSizePercent")
			file_size_difference_percent = atof(nodeText->get_content().c_str());

		else if (tagname == "textLengthPercent")
			text_distance_percent_differenciator = atof(nodeText->get_content().c_str());

		else if (tagname == "directoryPathDistance")
			directory_depth_distance = atoi(nodeText->get_content().c_str());

		else if (tagname == "editDistancePercent")
			max_edit_distance_length = atof(nodeText->get_content().c_str());

		else if (tagname == "maxDownloadedSize")
			downloaded_size = atoi(nodeText->get_content().c_str());

		else if (tagname == "downloadPath")
			download_path=nodeText->get_content();
	}
	if(!nodeContent)
	{
		//Recurse through child nodes:
		xmlpp::Node::NodeList list = node->get_children();
		for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			ProcessNode(*iter, tagname, indentation + 2); //recursive
		}
	}
}

bool GlobalParams::LoadGlobalParams(string path)
{
	config_file=path;
	ifstream test_file;
	bool exit;
	
	try{
		xmlpp::DomParser parser;
		parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
	    	parser.parse_file(config_file);
	    	
	    	if ( parser ) {
	    		const xmlpp::Node* pNode = parser.get_document()->get_root_node();
	      		GlobalParams::ProcessNode(pNode);
	    	}	
	    exit=true;
	}
	catch ( const std::exception& ex ) {
		exit=false;
	}
	test_file.open(GlobalParams::GetTextCatConfigFile().c_str(),ios::in);
	if(!test_file.is_open())
		throw "The TextCat configuration file is not specified correctly.";
	try{
		Config::setXmlFile(GlobalParams::GetTagAlignerConfigFile());
		Config::buildTagSet();
	}
	catch(...){
		throw "The TagAligner configuration file can't been opened.";
	}

	test_file.open(GlobalParams::GetTagAlignerConfigFile().c_str(),ios::in);
	if(!test_file.is_open())
		throw "The TagAligner configuration file is not specified correctly.";
	
    return true;
}

vector<string> GlobalParams::GetAcceptedExtenssions()
{
	return accepted_extenssions;
}
	
int GlobalParams::GetMaxDownloadedSize()
{
	return downloaded_size;
}

string GlobalParams::GetDownloadPath()
{
	return download_path;
}

void GlobalParams::SetGuessLanguage(bool value)
{
	guess_language=value;
}

bool GlobalParams::GetGuessLanguage()
{
	return guess_language;
}
>>>>>>> .merge-right.r58
