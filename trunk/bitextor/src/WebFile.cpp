<<<<<<< .working
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

/*typedef struct {

	void **fprint;
	uint4 size;
	uint4 maxsize;

	char output[MAXOUTPUTSIZE];

} textcat_t;

void *textcatInit(){
	textcat_t *h;
	char line[1024];
	FILE *fp;
	map<wstring,wstring>::iterator it;
	unsigned int i;

	h = (textcat_t *)wg_malloc(sizeof(textcat_t));
	h->size = GlobalParams::fingerprints.size();
	h->maxsize = GlobalParams::fingerprints.size();
	h->fprint = (void **)wg_malloc( sizeof(void*) * h->maxsize );

	for(i=0,it=GlobalParams::fingerprints.begin();it!=fingerprints.end();i++,it++){
		if ((h->fprint[i] = fp_Init( it->first ))!=NULL) {
			if ( fp_Read( h->fprint[i], it->second, 400 ) == 0 ) {
				textcat_Done(h);
				return NULL;
			}
		}
		else{
			free (h->fprint);
			free (h);
			return NULL;
		}
	}
	fclose(fp);
	return h;
}*/

bool WebFile::IsAlphabetic(const wchar_t& car){
	int status;

	regex_t re;
	wstring pattern = L"[0-9]";
	wchar_t text[2];
	text[0] = car;
	text[1] = L'\0';

	if (regcomp(&re, Config::toString(pattern).c_str(), REG_EXTENDED|REG_NOSUB) != 0)
		return false;

	status = regexec(&re, Config::toString(text).c_str(), (size_t) 0, NULL, 0);
    regfree(&re);

    if (status != 0) 
       	return false;
    else
		return true;
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
					if(str_temp[0]!='[')
						exit=false;
					textcat_Done(h);
					//delete in;
					h=NULL;
				}
				else{
					cout<<"Set the language for the file "<<this->path<<" : ";
					wcin>>this->lang;
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
=======
#include "WebFile.h"
#include <tagaligner/tagaligner-generic.h>

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
	bool exit=true, found_ext;
	int i;
	ifstream fin;
	string text, content, aux;
	vector<string> elem_list;
	vector<int> tags;
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
				this->file_type=path.substr(path.find_last_of('.')+1);
			}
			catch(std::out_of_range& e){
				this->file_type="";
			}

			//We convert the extenssion of the file to lower characters to make it compatible whith the accepted extenssions in configuration file.
			/*transform(this->file_type.begin(), this->file_type.end(), this->file_type.begin(), ::tolower);
			//We open the file to process it whith the lexic analyzer to separate the tags and the text.
			found_ext=false;
			for(i=0;i<GlobalParams::GetAcceptedExtenssions().size() && found_ext==false;i++){
				if(this->file_type==GlobalParams::GetAcceptedExtenssions()[i])
					found_ext=true;
			}
			if(!found_ext){
				exit=false;
			}
			else{*/
				fin.open(path.c_str());
				if(fin.is_open()){
					fin>>content;
					while(!fin.eof()){
						fin>>aux;
						content+=" "+aux;
					}
					fin.close();
					elem_list=TagAligner_generic::SplitFilterText(content);
					text="";
					for(i=0;i<elem_list.size();i++){
						if(TagAligner_generic::isTag(elem_list[i]))
							tags.push_back(GlobalParams::GetHTMLTagValue(elem_list[i]));
						else{
							tags.push_back(elem_list[i].length());
							text+=elem_list[i];
						}
					}
					//We set the tag list
					this->tag_list=tags;
					if(GlobalParams::GetGuessLanguage()){
						//We gess the language and set it
						void *h = textcat_Init(GlobalParams::GetTextCatConfigFile().c_str());
						str_temp=textcat_Classify(h, text.c_str(), text.length());
						this->lang=str_temp.substr(1,str_temp.find_first_of("]")-1);
						if(str_temp[0]!='[')
							exit=false;
						textcat_Done(h);
						delete in;
						h=NULL;
					}
					else{
						cout<<"Set the language for the file "<<this->path<<" : ";
						cin>>this->lang;
					}
				}
				else
					exit=false;
			//}
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
>>>>>>> .merge-right.r58
