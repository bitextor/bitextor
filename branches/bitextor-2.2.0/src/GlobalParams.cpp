#include "GlobalParams.h"

float GlobalParams::max_edit_distance_length=-1;

//map< wstring,int > GlobalParams::tag_map;

//int GlobalParams::tag_map_counter=-1;

//vector< wstring > GlobalParams::irrelevant_tags;
	
int GlobalParams::directory_depth_distance=0;

float GlobalParams::text_distance_percent_differenciator=-1;

float GlobalParams::file_size_difference_percent=-1;

wstring GlobalParams::tagaligner_config_file=L"";

wstring GlobalParams::textcat_config_file=L"";

int GlobalParams::tagaligner_mode=1;

wstring GlobalParams::config_file;

int GlobalParams::downloaded_size=-1;

vector<wstring> GlobalParams::accepted_extenssions;

wstring GlobalParams::download_path=L"~/";

bool GlobalParams::guess_language=true;

bool GlobalParams::is_percentual_edmax=true;

void GlobalParams::SetMaxEditDistance(const float &value)
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

/*int GlobalParams::GetHTMLTagValue(wstring tag)
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
}*/
	
/**void GlobalParams::AddIrrelevantTag(wstring tag)
{
	if(tag_map.find(tag)==tag_map.end()){
		irrelevant_tags.push_back(tag);
		tag_map[tag]=(-1)*tag_map_counter;
	}
}*/
	
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

void GlobalParams::SetTextDistancePercentDifferenciator(const float &value)
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

void GlobalParams::SetFileSizeDifferencePercent(const float &value)
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
	
void GlobalParams::SetTagAlignerConfigFile(const wstring &path)
{
	tagaligner_config_file=path;
}
	
wstring GlobalParams::GetTagAlignerConfigFile()
{
	return tagaligner_config_file;
}

void GlobalParams::SetTagAlignerMode(const int &mode)
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

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!(cur_node->type==XML_TEXT_NODE && xmlIsBlankNode(cur_node))){
			if(cur_node->type!=XML_TEXT_NODE && cur_node->type!=XML_COMMENT_NODE && node->name!=NULL)
				tagname=Config::xmlToWstring((xmlChar*)cur_node->name);
			if(cur_node->type==XML_TEXT_NODE){
				if (tagname == L"downloadModDest")
					download_path=Config::xmlToWstring(cur_node->content);
				else if (tagname == L"tagAlignerConfigFile"){
					tagaligner_config_file = Config::xmlToWstring(cur_node->content);
				}
				else if (tagname == L"textCatConfigFile"){
					textcat_config_file = Config::xmlToWstring(cur_node->content);
				}
				else if (tagname == L"tagAlignerMode"){
					if(Config::xmlToWstring(cur_node->content)==L"ad")
						tagaligner_mode=1;
					else if(Config::xmlToWstring(cur_node->content)==L"l")
						tagaligner_mode=2;
					else if(Config::xmlToWstring(cur_node->content)==L"direct")
						tagaligner_mode=3;
				}
				else if (tagname == L"fileSizePercent"){
					file_size_difference_percent = atof(Config::toString(Config::xmlToWstring(cur_node->content)).c_str());
				}
				else if (tagname == L"textLengthPercent"){
					text_distance_percent_differenciator = atof(Config::toString(Config::xmlToWstring(cur_node->content)).c_str());
				}
				else if (tagname == L"editDistancePercent"){
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
					else if ( tagname==L"editDistancePercent" && key==L"mode" ){
						if( value==L"percent" )
							is_percentual_edmax=true;
						else if( value==L"absolute" )
							is_percentual_edmax=false;
					}
					free(node_prop);
					propPtr = propPtr->next;
				}
			}
			if(!xmlIsBlankNode(cur_node))
			{
				//Recurse through child nodes:
				ProcessNode(cur_node->children, tagname);
			}
		}
	}
}

bool GlobalParams::LoadGlobalParams(const wstring &path)
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	config_file=path;
	bool exit;
	
	try{
		doc = xmlReadFile(Config::toString(path).c_str(), NULL, 0);
		if(doc==NULL)
			throw "The specified configuration file does'nt exists.";
		else{
			root_element = xmlDocGetRootElement(doc);
			ProcessNode(root_element, L"");
			xmlFreeDoc(doc);
			xmlCleanupParser();
		}	
	    exit=true;
	}
	catch ( const std::exception& ex ) {
		exit=false;
	}
	
    return exit;
}

vector<wstring> GlobalParams::GetAcceptedExtenssions()
{
	return accepted_extenssions;
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
