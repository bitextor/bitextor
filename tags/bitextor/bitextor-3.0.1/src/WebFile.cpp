/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "WebFile.h"
#include <fstream>
#include <stdexcept>
#include <tre/regex.h>
#include <libtagaligner/ConfigReader.h>
#include <libtagaligner/FragmentedFile.h>

WebFile::WebFile()
{
	this->initialized=false;
	text_size=0;
}

WebFile::~WebFile()
{
}

bool WebFile::IsAlphabetic(const wchar_t& car){
	int status;

	regex_t re;
	wstring pattern = L"[0-9]";
	wchar_t text[2];
	text[0] = car;
	text[1] = L'\0';

	if (regwcomp(&re, pattern.c_str(), REG_EXTENDED|REG_NOSUB) != 0)
		return false;

	status = regwexec(&re, text, (size_t) 0, NULL, 0);
    regfree(&re);

    return (status == 0);
}

void WebFile::GetNonAplha(wstring text){
	unsigned int i;
	wstring st=L"";

	for(i=0;i<text.length();i++){
		if(IsAlphabetic(text[i])){
			st+=text[i];
		}
		else{
			if(st!=L""){
				numbers_vec.push_back(atoi(Config::toString(st).c_str()));
				st=L"";
			}
		}
	}
	if(st!=L"")
		numbers_vec.push_back(atoi(Config::toString(st).c_str()));
}

bool WebFile::Initialize(const string &path)
{
	wstring str_temp;
	filebuf *fb;
	istream *in;
	bool exit=true, found_ext;
	int i;
	ifstream fin;
	wstring text, content;
	vector<int> tags;
	time_t rawtime;
	FragmentedFile ffile;
	FILE *f;

	if(GlobalParams::GetTextCatConfigFile()==L"")
		throw "TextCat's configuration file has not been specified. Please, define it in the Bitextor's configuration file.";
	else{
		try{
			//We clean the format and convert to UTF8
			FilePreprocess::PreprocessFile(path);
			//We set the file path
			this->path=path;
			//We set the extension of the file
			try{
				this->file_type=path.substr(path.find_last_of('.')+1);
			}
			catch(std::out_of_range& e){
				this->file_type="";
			}
			if(ffile.LoadFile(path)){
				ffile.Compact();
				/*f=fopen((path+".xml").c_str(), "w");
				if(f){
					fputws(ffile.toXML().c_str(),f);
					fclose(f);*/
					
					for(i=0;i<ffile.getSize();i++){
						if(ffile.isTag(i))
							file.push_back(ffile.getTag(i)->getCode()*(-1));
						else
							file.push_back(ffile.getText(i)->getLength());
					}

					text=ffile.getFullText(true);
					GetNonAplha(text);
					text_size=text.size();
					//We set the tag list
					if(GlobalParams::GetGuessLanguage()){
						//We gess the language and set it
						void *h = textcat_Init(Config::toString(GlobalParams::GetTextCatConfigFile()).c_str());
						str_temp=Config::toWstring(textcat_Classify(h, Config::toString(text).c_str(), text.length()));
						this->lang=str_temp.substr(1,str_temp.find_first_of(L"]")-1);
						if(str_temp[0]!='['){
							exit=false;
							GlobalParams::WriteLog(L"Language of "+Config::toWstring(path)+L" couldn't be guessed.");
						}
						else
							GlobalParams::WriteLog(L"File "+Config::toWstring(path)+L" loaded correctly (Language: "+this->lang+L").");
						textcat_Done(h);
						h=NULL;
					}
					else{
						cout<<"Set the language for the file "<<this->path<<" : ";
						wcin>>this->lang;
					}
				/*}
				else
					exit=false;*/
			}
			else{
				exit=false;
				GlobalParams::WriteLog(L"File "+Config::toWstring(path)+L" couldn't be loaded.");
			}
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

wstring WebFile::GetLang()
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

bool WebFile::IsInitialized()
{
	return this->initialized;
}

vector<int>* WebFile::GetNumbersVector()
{
	return &numbers_vec;
}
	
unsigned int WebFile::GetTextSize()
{
	return text_size;
}

/*wstring WebFile::toXML()
{
	unsigned int i;
	wostringstream ss;
	ss<<text_size;
	wstring exit= L"<webfile path=\""+Config::toWstring(path)+L"\" lang=\""+lang+L"\" file_type=\""+Config::toWstring(file_type)+L"\" text_size=\""+ss.str()+L"\">";

	ss.seekp(ios_base::beg);
	if(numbers_vec!=NULL && numbers_vec->size()>0){
		exit+=L"\n\t<numbervector>";
		for(i=0;i<numbers_vec->size();i++){
			ss<<numbers_vec->at(i);
			exit+=L"\n\t\t<number value=\""+ss.str()+L"\">";
			ss.seekp(ios_base::beg);
		}
		exit+=L"\n\t</numbervector>";
	}
	return exit;
}*/

vector<int>* WebFile::GetTagArray(){
	return &file;
}
