#include "WebFile.h"

WebFile::WebFile()
{
	this->file_type="";
	this->lang="";
	this->path="";
}

WebFile::~WebFile(){}


WebFile::WebFile(string path)
{
	this->path=path;
	this->file_type=path.substr(path.find_last_of('.'));
}

void WebFile::setLang(string language)
{
	this->lang=language;
}

string WebFile::getLang()
{
	//If path is defined, but language is unknown:
	if(this->lang=="" && this->path!=""){
		string str_temp;
		ostringstream oss;
		filebuf fb;
		fb.open ("/home/miquel/Desktop/SEPC.html",ios::in);
		istream *in;
		in=new istream(&fb);
		
		//We creatre a TagClean FlexLexer to get the clear text from the file.
		FlexLexer* lexer = new yyFlexLexer(in,&oss);
		lexer->yylex();
		void *h = textcat_Init("/home/miquel/workspace/bitextor/conf.txt");
		string sample=oss.str();
		str_temp=textcat_Classify(h, sample.c_str(), sample.length()+1);
		//If language is detected, we get the first possibility given by libtextcat
		if(str_temp[0]=='[')
			this->lang=str_temp.substr(1,str_temp.find_first_of("]")-1);
		else
			this->lang=str_temp;
		textcat_Done(h);
	}
	return this->lang;
}

void WebFile::setPath(string path)
{
	this->path=path;
}

string WebFile::getPath()
{
	return this->path;
}

string WebFile::getFileType()
{
	return this->file_type;
}

string WebFile::getXMLString()
{
	std::string whole;
	try
	{
		xmlpp::Document parser;
		
		xmlpp::Element* nodeRoot = parser.create_root_node("WebFile");
     
	    nodeRoot->set_attribute("lang", this->lang);
	    nodeRoot->set_attribute("file_type", this->file_type);
	    nodeRoot->set_child_text(this->path);
	
	    whole = parser.write_to_string();
	}
	catch(const std::exception& ex)
	{
		std::cout << "Exception caught: " << ex.what() << std::endl;
	}

    return whole;
}
