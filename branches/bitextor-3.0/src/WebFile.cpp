#include "WebFile.h"

WebFile::WebFile()
{
	this->initialized=false;
}

WebFile::~WebFile(){}

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
				//We set the tag list
				if(GlobalParams::GetGuessLanguage()){
					//We gess the language and set it
					void *h = textcat_Init(Config::toString(GlobalParams::GetTextCatConfigFile()).c_str());
					str_temp=Config::toWstring(textcat_Classify(h, Config::toString(file.getFullText(true)).c_str(), file.getFullText(true).length()));
					this->lang=str_temp.substr(1,str_temp.find_first_of(L"]")-1);
					if(str_temp[0]!='[')
						exit=false;
					textcat_Done(h);
					delete in;
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





















