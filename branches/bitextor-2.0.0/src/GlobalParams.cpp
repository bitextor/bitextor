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

int GlobalParams::bytes_to_download=-1;

vector<string> GlobalParams::accepted_extenssions;

string GlobalParams::download_path="~/";

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
	if(tag_map.find(tag)==tag_map.end()){
		tag_map[tag]=tag_map_counter;
		tag_map_counter--;
	}
	return tag_map[tag];	
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

		else if (tagname == "maxBytesToDownload")
			bytes_to_download = atoi(nodeText->get_content().c_str());

		else if (tagname == "acceptedExtenssion")
			accepted_extenssions.push_back(nodeText->get_content());

		else if (tagname == "downloadPath")
			download_path=nodeText->get_content();

		else if (tagname == "tag"){
				irrelevant_tags.push_back(nodeText->get_content());
				tag_map[nodeText->get_content()]=1;
		}
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

	test_file.open(GlobalParams::GetTagAlignerConfigFile().c_str(),ios::in);
	if(!test_file.is_open())
		throw "The TagAligner configuration file is not specified correctly.";
	
    return true;
}

vector<string> GlobalParams::GetAcceptedExtenssions()
{
	return accepted_extenssions;
}
	
int GlobalParams::GetMaxBytesToDownload()
{
	return bytes_to_download;
}

string GlobalParams::GetDownloadPath()
{
	return download_path;
}
