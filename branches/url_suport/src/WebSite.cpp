/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "WebSite.h"
#include "TranslationMemory.h"
#include <cstring>
#include <stack>
#include <map>
#include <dirent.h> 
#include <sstream>
#include <sys/stat.h>
#include <libtagaligner/ConfigReader.h>
#include <libtagaligner/Aligner.h>

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
	DIR *directorio=NULL;
	struct dirent *fichero;
	struct stat fich;
	string dir, file_name;
	vector<string> pila;
	vector<WebFile*> vec_aux;
	map<string,int> dir_level;
	WebFile *wf;
	unsigned int level;
	bool exit=false;
	vector< BitextCandidates* > **file_list;
	stack<string> *dirs, *subdirs;
	unsigned int i;
	map<wstring,FILE *>::iterator it_files;
	

	try{
		TranslationMemory::SetDestPath(dest_path);
		if(GlobalParams::GetDirectoryDepthDistance()!=-1){
			file_list=new vector< BitextCandidates* >*[GlobalParams::GetDirectoryDepthDistance()+1];
			for(i=0;i<(unsigned int)GlobalParams::GetDirectoryDepthDistance()+1;i++)
				file_list[i]=new vector< BitextCandidates* >();
		}
		else{
			file_list=new vector< BitextCandidates* >*[1];
			file_list[0]=new vector< BitextCandidates* >();
		}
		//Firstly, we prove that base_path is a valid directory.
		if ( stat(this->base_path.c_str(), &fich)>=0 ){
			if(!S_ISDIR(fich.st_mode))
				throw "The specified directory doesn't exist.";
		}
		//Initializing structure of levels of files
		level=0;
		dirs=new stack<string>();
		subdirs=new stack<string>();
		dirs->push(this->base_path);
		
		//Starting reading of directory by levels
		while(dirs->size()+subdirs->size()>0){
			while(dirs->size()!=0){
				if(dirs->top()!=dest_path){
					directorio = opendir((dirs->top()).c_str());
					if(directorio!=NULL){
						while ( (fichero=readdir(directorio)) != NULL ){
							if ( strcmp(fichero->d_name, "..") != 0 && strcmp(fichero->d_name, ".") != 0){
								file_name=dirs->top()+fichero->d_name;
								if ( stat(file_name.c_str(), &fich)>=0 ){
									//Subfiles are saved in the subdirs stack
									if(S_ISDIR(fich.st_mode)){
										file_name=file_name+"/";
										subdirs->push(file_name);
									}
									//Files are processed and saved in the corresponding level in the levels structure of the directory
									else
									{
										if(file_name.length()<4 || file_name[file_name.length()-4]!=L'.' || file_name[file_name.length()-3]!=L'x' || file_name[file_name.length()-2]!=L'm' || file_name[file_name.length()-1]!=L'l'){
											wf=new WebFile();
											wf->Initialize(file_name);
											if(wf->IsInitialized() && (GlobalParams::GetMinArraySize()==-1 || (unsigned int)GlobalParams::GetMinArraySize()<=(wf->GetTagArray()->size()))){
												file_list[level]->push_back(new BitextCandidates(wf));
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
				}
				dirs->pop();
			}
			//We delete the empty "actual directorys" list and make the subdirs list the actual dirs list
			delete dirs;
			dirs=subdirs;
			subdirs=new stack<string>();
			//If we have not full the directory level structure, we follow loading files
			if(GlobalParams::GetDirectoryDepthDistance()!=-1){
				if((int)level<GlobalParams::GetDirectoryDepthDistance())
					level++;
				//If the structure is full, we calculate the candidates for higher level files, delete this level and load a new lower level
				else{
					if(!exit)
						exit=GetMatchedFiles(dest_path, file_list, level+1);
					else
						GetMatchedFiles(dest_path, file_list, level+1);
					for(i=0; i<file_list[0]->size();i++){
						//remove((file_list[0]->at(i)->GetPath()+".xml").c_str());
						delete file_list[0]->at(i);
					}
					delete file_list[0];
					for(i=1;i<(unsigned int)GlobalParams::GetDirectoryDepthDistance()+1;i++)
						file_list[i-1]=file_list[i];
					file_list[i-1]=new vector< BitextCandidates* >();
				}
			}
		}
		//We delete the dirs and subdirs list and process all the resting files in the level files structure
		delete dirs;
		delete subdirs;
		if(GlobalParams::GetDirectoryDepthDistance()!=-1){
			for(level=GlobalParams::GetDirectoryDepthDistance()+1;level>=1;level--){
				if(!exit)
					exit=GetMatchedFiles(dest_path, file_list, level);
				else
					GetMatchedFiles(dest_path, file_list, level);
				for(i=0; i<file_list[0]->size();i++){
					delete file_list[0]->at(i);
				}
				delete file_list[0];
				for(i=1;i<level;i++)
					file_list[i-1]=file_list[i];
			}
		}
		else{
			exit=GetMatchedFiles(dest_path, file_list, 1);
			for(i=0; i<file_list[0]->size();i++){
				delete file_list[0]->at(i);
			}
			delete file_list[0];
		}
		delete[] file_list;
	}
	catch(...){
		exit=false;
		if(directorio!=NULL)
			closedir(directorio);
	}
	TranslationMemory::Reset();
	return exit;
}

bool WebSite::GetMatchedFiles(const string &dest_dir, vector< BitextCandidates* > **file_list, unsigned int size)
{
	bool exit=false;
	unsigned int i,j,k;
	string file_name;
	map <wstring, BitextCandidates* > best_bitexts;
	map <wstring, BitextCandidates* >::iterator bb_it, it;
	vector<bool> enabled_bitexts;
	wostringstream oss;
	BitextCandidates* main_bitext;

	//We get the files in the top level one by one
	for(i=0;i<file_list[0]->size();i++){
		main_bitext=file_list[0]->at(i);
		for(j=0;j<size;j++){
			//If we are comparing in the same level, we get the next file in the level
			if(j==0)
				k=i+1;
			//If we are comparing in another level, we get the first file in the level
			else
				k=0;
			//Now we start to compare every file with the first of the top level
			
			if(GlobalParams::GetCreateAllCandidates()){
				while(k<file_list[j]->size()){
					if(main_bitext->Add(file_list[j]->at(k)))
					{
						if(main_bitext->GenerateLastAddedBitext()){
							exit=true;
						}
						main_bitext->EraseLastAdded();
					}
					k++;
				}
			}
			else{
				for(;k<file_list[j]->size();k++){
					//wcout<<main_bitext->GetWebFile()->GetURL()->GetCompleteURL()<<L"  "<<file_list[j]->at(k)->GetWebFile()->GetURL()->GetCompleteURL()<<endl;
					main_bitext->Add(file_list[j]->at(k));
				}
			}
		}
		if(!GlobalParams::GetCreateAllCandidates()){
			if(!exit)
				exit=main_bitext->GenerateBitexts();
			else
				main_bitext->GenerateBitexts();
		}
	}
	return exit;
}
