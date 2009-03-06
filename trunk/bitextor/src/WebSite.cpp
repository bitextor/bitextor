<<<<<<< .working
#include "WebSite.h"

WebSite::WebSite(const string &path)
{
	this->base_path=path;
}

WebSite::~WebSite(){}

string WebSite::GetFileName(string path)
{
	string exit;
	size_t found;

	found=path.find_last_of("/\\");
	exit=path.substr(found+1);

	return exit;
}

string WebSite::GetBasePath()
{
		return this->base_path;
}

bool WebSite::GenerateBitexts(const string &dest_path)
{
	DIR *directorio;
	struct dirent *fichero;
	struct stat fich;
	string dir, file_name;
	vector<string> pila;
	vector<WebFile*> vec_aux;
	
	
	map<string,int> dir_level;
	WebFile *wf;
	unsigned int level;
	bool exit=false;
	vector< WebFile* > **file_list;
	stack<string> *dirs, *subdirs;
	unsigned int i;
	FILE * fout;

	try{
		if(GlobalParams::AllBitextInAFile()){
			fout=fopen((dest_path+"/bitext.tmx").c_str(),"a");
			fputws(Aligner::GetHeading().c_str(),fout);
		}
		else
			fout=NULL;

		file_list=new vector< WebFile* >*[GlobalParams::GetDirectoryDepthDistance()+1];
		//Firstly, we prove that base_path is a valid directory.
		if ( stat(this->base_path.c_str(), &fich)>=0 ){
			if(!S_ISDIR(fich.st_mode))
				throw "The specified directory doesn't exist.";
		}
		
		level=0;
		dirs=new stack<string>();
		subdirs=new stack<string>();
		dirs->push(this->base_path);
		for(i=0;i<GlobalParams::GetDirectoryDepthDistance()+1;i++)
			file_list[i]=new vector< WebFile* >();
		while(dirs->size()+subdirs->size()>0){
			while(dirs->size()!=0){
				directorio = opendir((dirs->top()).c_str());
				if(directorio!=NULL){
					while ( (fichero=readdir(directorio)) != NULL ){
						if ( strcmp(fichero->d_name, "..") != 0 && strcmp(fichero->d_name, ".") != 0 && strcmp(fichero->d_name, "bitexts") != 0 ){
							file_name=dirs->top()+fichero->d_name;
							if ( stat(file_name.c_str(), &fich)>=0 ){
								if(S_ISDIR(fich.st_mode)){
									file_name=file_name+"/";
									subdirs->push(file_name);
								}
								else
								{
									wf=new WebFile();
									wf->Initialize(file_name);
									if(wf->IsInitialized()){
										file_list[level]->push_back(wf);
									}
									else
										delete wf;
								}
							}
						}
					}
					closedir(directorio);
				}
				dirs->pop();
			}
			delete dirs;
			dirs=subdirs;
			subdirs=new stack<string>();
			if(level<GlobalParams::GetDirectoryDepthDistance())
				level++;
			else{
				if(!exit)
					exit=GetMatchedFiles(dest_path, file_list, level+1, fout);
				else
					GetMatchedFiles(dest_path, file_list, level+1, fout);
				for(i=0; i<file_list[0]->size();i++)
					delete file_list[0]->at(i);
				delete file_list[0];
				for(i=1;i<GlobalParams::GetDirectoryDepthDistance()+1;i++)
					file_list[i-1]=file_list[i];
				file_list[i-1]=new vector< WebFile* >();
			}
		}
		delete dirs;
		for(level=GlobalParams::GetDirectoryDepthDistance()+1;level>=1;level--){
			if(!exit)
				exit=GetMatchedFiles(dest_path, file_list, level, fout);
			else
				GetMatchedFiles(dest_path, file_list, level, fout);
			for(i=0; i<file_list[0]->size();i++)
				delete file_list[0]->at(i);
			delete file_list[0];
			for(i=1;i<level;i++)
				file_list[i-1]=file_list[i];
		}
		delete[] file_list;

		if(GlobalParams::AllBitextInAFile()){
			fputws(Aligner::GetFoot().c_str(),fout);
			fclose(fout);
		}
	}
	catch(...){
		exit=false;
		if(directorio!=NULL)
			closedir(directorio);
	}
	return exit;
}

bool WebSite::GetMatchedFiles(const string &dest_dir, vector< WebFile* > **file_list, const unsigned int &size, FILE * main_fout)
{
	bool exit=false;
	unsigned int i,j,k,l;
	Bitext* bitext;
	string file_name;
	ostringstream *aux_sstream;
	struct stat my_stat;
	FILE* fout;
	unsigned int starting_tuid=0;
	unsigned int last_tuid=0;

	for(i=0;i<file_list[0]->size();i++){
		for(j=0;j<size;j++){
			if(j==0)
				k=i+1;
			else
				k=0;
			while(k<file_list[j]->size()){
				bitext=new Bitext();
				if(bitext->Initialize(file_list[0]->at(i),file_list[j]->at(k)))
				{
					if(GlobalParams::AllBitextInAFile()){
						bitext->GenerateBitext(main_fout, starting_tuid, &last_tuid);
						wcout<<L"\tThe bitext between "<<Config::toWstring(bitext->GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(bitext->GetSecondWebFile()->GetPath())<<L" has been created"<<endl;
						wcout<<L"\tEdit distance: "<<bitext->GetEditDistance()<<L"%  Size difference:"<<bitext->GetSizeDistance()<<L"%"<<endl<<endl;
						starting_tuid=last_tuid;
					}
					else{
						file_name=dest_dir+GetFileName(bitext->GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext->GetSecondWebFile()->GetPath())+".tmx";
						for(l=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;l++){
							delete aux_sstream;
							aux_sstream=new ostringstream(ios_base::out);
							*aux_sstream<<l;
							file_name=dest_dir+GetFileName(bitext->GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext->GetSecondWebFile()->GetPath())+aux_sstream->str()+".tmx";
						}
						fout=fopen(file_name.c_str(),"w");
						if(fout){
							bitext->GenerateBitext(fout);
							wcout<<L"\tThe bitext between "<<Config::toWstring(bitext->GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(bitext->GetSecondWebFile()->GetPath())<<L" has been created: "<<Config::toWstring(file_name)<<endl;
							wcout<<L"\tEdit distance: "<<bitext->GetEditDistance()<<L"%  Size difference:"<<bitext->GetSizeDistance()<<L"%"<<endl<<endl;
							fclose(fout);
						}
					}
					exit=true;
				}
				delete bitext;
				k++;
			}
		}
	}
	return exit;
}
=======
#include "WebSite.h"

WebSite::WebSite()
{
	this->base_path="";
	this->initialized=false;
}

WebSite::~WebSite(){}

string WebSite::GetBasePath()
{
	if(this->initialized)
		return this->base_path;
	else
		throw "Object not initialized.";
}

WebFile WebSite::GetWebFile(unsigned int pos, unsigned int level)
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

bool WebSite::Initialize(string base_path)
{
	DIR *directorio;
	struct dirent *fichero;
	struct stat fich;
	string dir, file_name;
	vector<string> pila;
	vector<WebFile> vec_aux;
	map<string,int> dir_level;
	WebFile wf;
	unsigned int level;
	bool exit=true;

	try{
		//Firstly, we prove that base_path is a valid directory.
		if ( stat(base_path.c_str(), &fich)>=0 ){
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
			directorio = opendir(dir.c_str());
			level=dir_level[dir];
			if(directorio!=NULL){
				while ( (fichero=readdir(directorio)) != NULL ){
					if ( strcmp(fichero->d_name, "..") != 0 && strcmp(fichero->d_name, ".") != 0 ){
						file_name=dir+(fichero->d_name);
						if ( stat(file_name.c_str(), &fich)>=0 ){
							if(S_ISDIR(fich.st_mode)){
								file_name=file_name+"/";
								pila.push_back(file_name);
								dir_level[file_name]=level+1;
							}
							else
							{
								wf.Initialize(file_name);
								if(wf.IsInitialized())
								{
									while(level>=file_list.size())
										this->file_list.push_back(vec_aux);
									this->file_list[level].push_back(wf);
								}
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

unsigned int WebSite::WebFileCount(unsigned int level)
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
						if(this->file_list[orig_level][orig_pos].GetLang()!=this->file_list[dest_level][dest_pos].GetLang()){
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
>>>>>>> .merge-right.r58
