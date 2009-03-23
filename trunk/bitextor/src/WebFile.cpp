#include "WebFile.h"

WebFile::WebFile()
{
	this->initialized=false;
	numbers_vec=NULL;
	text_size=0;
}

WebFile::~WebFile()
{
	if(numbers_vec!=NULL)
		delete numbers_vec;
}

bool WebFile::IsAlphabetic(const wchar_t& car){
	/*int status;

	regex_t re;
	wstring pattern = L"[0-9]";*/
	wchar_t text[2];
	text[0] = car;
	text[1] = L'\0';

	/*if (regcomp(&re, Config::toString(pattern).c_str(), REG_EXTENDED|REG_NOSUB) != 0)
		return false;

	status = regexec(&re, Config::toString(text).c_str(), (size_t) 0, NULL, 0);
    regfree(&re);

    if (status != 0) 
       	return false;
    else
		return true;*/


	const char *error;
	int erroroffset;
	int result[3];
	int workspace[4096];
	pcre *re = pcre_compile(Config::toString(L"[0-9]").c_str(), PCRE_DOTALL|PCRE_CASELESS|PCRE_EXTENDED|PCRE_UTF8, &error, &erroroffset, NULL);
	if(re!=NULL){
		int rc = pcre_dfa_exec(re, NULL, Config::toString(text).c_str(), Config::toString(text).length(), 0, PCRE_NO_UTF8_CHECK, result, 3, workspace, 4096);
		if(rc==1)
			return true;
		else
			return false;
	}
	else
		return false;
}

vector<int>* WebFile::GetNonAplha(wstring text){
	unsigned int i;
	vector<int> *vec=new vector<int>;
	wstring st=L"";

	for(i=0;i<text.length();i++){
		if(IsAlphabetic(text[i]))
			st+=text[i];
		else{
			if(st!=L""){
				vec->push_back(atoi(Config::toString(st).c_str()));
				wcout<<st<<endl;
				st=L"";
			}
		}
	}
	return vec;
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
			if(file.LoadFile(path)){
				file.Compact();
				text=file.getFullText(true);
				if(numbers_vec!=NULL)
					delete numbers_vec;
				numbers_vec=GetNonAplha(text);
				
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

vector<Fragment*> * WebFile::GetTagArrayReference()
{
	if(this->initialized==false)
		throw "Object not initialized.";
	else
		return file.getFragmentsVectorRefference();
}

bool WebFile::IsInitialized()
{
	return this->initialized;
}

FragmentedFile* WebFile::GetFragmentedFileReference()
{
	if(this->initialized==false)
		throw "Object not initialized.";
	else
		return &file;
}

vector<int>* WebFile::GetNumbersVector()
{
	return numbers_vec;
}
	
unsigned int WebFile::GetTextSize()
{
	return text_size;
}

wstring WebFile::toXML()
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
	exit+=L"\n\t<fragmentedfile>";
	for(i=0;i<file.getSize();i++){
		if(file.isTag(i))
			exit+=L"\n\t\t<tag>"+file.getTag(i)->getTagName()+L"</tag>";
		else
			exit+=L"\n\t\t<text>"+file.getText(i)->getString()+L"</text>";
	}
	exit+=L"\n\t</fragmentedfile>\n</webfile>";
	return exit;
}
