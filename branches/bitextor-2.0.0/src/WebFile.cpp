#include "WebFile.h"
#include "LexAnalyzer.cpp"

WebFile::WebFile()
{
	this->initialized=false;
}

WebFile::~WebFile(){}

bool WebFile::Initialize(string path)
{
	string str_temp;
	filebuf *fb;
	istream *in;
	bool exit=true;
	
	if(GlobalParams::GetTextCatConfigFile()=="")
		throw "TextCat's configuration file has not been specified. Please, define it in the Bitextor's configuration file.";
	else{
		try{
			//We clean the format and convert to UTF8
			FilePreprocess::PreprocessFile(path);
			
			//We set the file path
			this->path=path;
			//We set the extension of the file
			try{
				this->file_type=path.substr(path.find_last_of('.'));
			}
			catch(std::out_of_range& e){
				this->file_type="";
			}
			
			//We open the file to process it whith the lexic analyzer to separate the tags and the text.
			fb=new filebuf();
			fb->open (path.c_str(),ios::in);
			if(fb->is_open()){
				in=new istream(fb);
				FlexLexer* lexer = new yyFlexLexer(in);
				lexer->yylex();
				
				//We set the tag list
				this->tag_list=my_lex_tag_list;
				if(GlobalParams::GetGuessLanguage()){
					//We gess the language and set it
					void *h = textcat_Init(GlobalParams::GetTextCatConfigFile().c_str());
					str_temp=textcat_Classify(h, my_lex_text.c_str(), my_lex_text.length());
					this->lang=str_temp.substr(1,str_temp.find_first_of("]")-1);
	
					if(str_temp[0]!='[')
						exit=false;
					textcat_Done(h);
					delete in;
					h=NULL;
					delete lexer;
					fb->close();
				}
				else{
					cout<<"Set the language for the file "<<this->path<<" : ";
					cin>>this->lang;
				}
			}
			else
				exit=false;
		}
		catch(...){
			exit=false;
			this->initialized=false;
		}
		if(exit)
			this->initialized=true;
		else
			this->initialized=false;
	}
	return exit;
}

string WebFile::GetLang()
{
	if(this->initialized==false)
		throw "Object not initialized";
	else
		return this->lang;
}

string WebFile::GetPath()
{
	if(this->initialized==false)
		throw "Object not initialized";
	else
		return this->path;
}

string WebFile::GetFileType()
{
	if(this->initialized==false)
		throw "Object not initialized";
	else
		return this->file_type;
}

vector<int> WebFile::GetTagArray()
{
	if(this->initialized==false)
		throw "Object not initialized.";
	else
		return tag_list;
}

bool WebFile::IsInitialized()
{
	return this->initialized;
}
