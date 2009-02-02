#include "WebSite.h"

WebSite::WebSite()
{
	this->base_path=L"";
	this->initialized=false;
}

WebSite::~WebSite(){}

wstring WebSite::GetBasePath()
{
	if(this->initialized)
		return this->base_path;
	else
		throw "Object not initialized.";
}

WebFile* WebSite::GetWebFile(const unsigned int &pos, const unsigned int &level)
{
	if(this->initialized){
		if(0<=level && this->file_list.size()>level){
			if(this-file_list[level].empty())
				throw "The position is out of the list range.";
			else{
				if(this->file_list[level].size()>=pos && pos>=0)
					return this->file_list[level][pos];
				else
					throw "The position is out of the list range.";
			}
		}
		else
			throw "The level is out of the list range.";
	}
	else
		throw "Object not initialized.";
}

bool WebSite::Initialize(const wstring &base_path)
{
	DIR *directorio;
	struct dirent *fichero;
	struct stat fich;
	wstring dir, file_name;
	vector<wstring> pila;
	vector<WebFile*> vec_aux;
	map<wstring,int> dir_level;
	WebFile *wf;
	unsigned int level;
	bool exit=true;

	try{
		//Firstly, we prove that base_path is a valid directory.
		if ( stat(Config::toString(base_path).c_str(), &fich)>=0 ){
			if(!S_ISDIR(fich.st_mode))
				return false;
		}
		
		this->file_list.push_back(vec_aux);
		pila.push_back(base_path);
		dir_level[base_path]=0;
	
		while(!pila.empty())
		{
			dir=pila.back();
			pila.pop_back();
			directorio = opendir(Config::toString(dir).c_str());
			level=dir_level[dir];
			if(directorio!=NULL){
				while ( (fichero=readdir(directorio)) != NULL ){
					if ( strcmp(fichero->d_name, "..") != 0 && strcmp(fichero->d_name, ".") != 0 ){
						file_name=dir+Config::toWstring(fichero->d_name);
						if ( stat(Config::toString(file_name).c_str(), &fich)>=0 ){
							if(S_ISDIR(fich.st_mode)){
								file_name=file_name+L"/";
								pila.push_back(file_name);
								dir_level[file_name]=level+1;
							}
							else
							{
								wf=new WebFile();
								wf->Initialize(file_name);
								if(wf->IsInitialized())
								{
									while(level>=file_list.size())
										this->file_list.push_back(vec_aux);
									this->file_list[level].push_back(wf);
								}
								else
									delete wf;
							}
						}
					}
				}
			}
			closedir(directorio);
		}
		directorio=NULL;
		this->initialized=true;
	}
	catch(...){
		exit=false;
		if(directorio!=NULL)
			closedir(directorio);
	}
	return exit;
}

unsigned int WebSite::LevelCount()
{
	if(this->initialized)
		return this->file_list.size();
	else
		throw "Object not initialized.";
}

unsigned int WebSite::WebFileCount(const unsigned int &level)
{
	int exit;
	if(this->initialized){
		if(level>=0 && level<file_list.size()){
			exit=this->file_list[level].size();
		}
		else
		{
			throw "The level is out of the list range.";
		}
	}
	else
		throw "Object not initialized.";
	return exit;
}

vector<Bitext> WebSite::GetMatchedFiles()
{
	vector<Bitext> exit;
	unsigned int orig_pos, dest_pos;
	unsigned int orig_level, dest_level;
	bool possible_matching;
	Bitext* bitext;
	
	if(this->initialized){
		for(orig_level=0;orig_level<this->file_list.size();orig_level++)
		{
			for(orig_pos=0;orig_pos<this->file_list[orig_level].size();orig_pos++)
			{
				for(dest_level=orig_level;dest_level<this->file_list.size() && dest_level<=orig_level+GlobalParams::GetDirectoryDepthDistance();dest_level++)
				{
					if(orig_level==dest_level)
						dest_pos=orig_pos+1;
					else
						dest_pos=0;
					for(;dest_pos<file_list[dest_level].size();dest_pos++)
					{
						if(this->file_list[orig_level][orig_pos]->GetLang()!=this->file_list[dest_level][dest_pos]->GetLang()){
							bitext=new Bitext();
							possible_matching=bitext->Initialize(this->file_list[orig_level][orig_pos],this->file_list[dest_level][dest_pos]);
							if(possible_matching)
							{
								exit.push_back(*bitext);
							}
							delete bitext;
						}
					}
				}
			}
		}
	}
	else
		throw "Object not initialized.";
	return exit;
}
