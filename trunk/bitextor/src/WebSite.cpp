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
									if(wf->IsInitialized() && (GlobalParams::GetMinArraySize()==-1 || (GlobalParams::GetMinArraySize()!=-1 && GlobalParams::GetMinArraySize()<=(wf->GetTagArrayReference()->size())))){
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
		delete subdirs;
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
	unsigned int i,j,k,l,m;
	Bitext* bitext;
	string file_name;
	ostringstream *aux_sstream;
	struct stat my_stat;
	FILE* fout;
	unsigned int starting_tuid=0;
	unsigned int last_tuid=0;
	map<wstring, Bitext*> best_bitexts;
	map<wstring, Bitext*>::iterator bb_it, it;
	wostringstream oss;

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
					bb_it=best_bitexts.find(file_list[j]->at(k)->GetLang());
					if(bb_it==best_bitexts.end()){
						best_bitexts[file_list[j]->at(k)->GetLang()]=bitext;
					}
					else{
						if(bitext->isBestThan((*bb_it->second))){
							delete bb_it->second;
							bb_it->second=bitext;
						}
						else
							delete bitext;
					}
				}
				else
					delete bitext;
				k++;
			}
			if(best_bitexts.size()>0)
				exit=true;
			it=best_bitexts.begin();
			while(it!=best_bitexts.end()){
				if(GlobalParams::AllBitextInAFile()){
					it->second->GenerateBitext(main_fout, starting_tuid, &last_tuid);
					wcout<<L"The bitext between "<<Config::toWstring(it->second->GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(it->second->GetSecondWebFile()->GetPath())<<L" has been created"<<endl;
					oss<<it->second->GetEditDistance();
					GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(it->second->GetFirstWebFile()->GetPath())+L" and "+Config::toWstring(it->second->GetSecondWebFile()->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
					oss.seekp(ios_base::beg);
					starting_tuid=last_tuid;
				}
				else{
					file_name=dest_dir+GetFileName(it->second->GetFirstWebFile()->GetPath())+"_"+GetFileName(it->second->GetSecondWebFile()->GetPath())+".tmx";
					for(l=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;l++){
						delete aux_sstream;
						aux_sstream=new ostringstream(ios_base::out);
						*aux_sstream<<l;
						file_name=dest_dir+GetFileName(it->second->GetFirstWebFile()->GetPath())+"_"+GetFileName(it->second->GetSecondWebFile()->GetPath())+aux_sstream->str()+".tmx";
					}
					fout=fopen(file_name.c_str(),"w");
					if(fout){
						it->second->GenerateBitext(fout);
						wcout<<L"The bitext between "<<Config::toWstring(it->second->GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(it->second->GetSecondWebFile()->GetPath())<<L" has been created: "<<Config::toWstring(file_name)<<endl;
						oss<<it->second->GetEditDistance();
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(it->second->GetFirstWebFile()->GetPath())+L" and "+Config::toWstring(it->second->GetSecondWebFile()->GetPath())+L" has been created: "+Config::toWstring(file_name)+L">> Edit distance: "+oss.str()+L"%.");
						oss.seekp(ios_base::beg);
						fclose(fout);
					}
				}
				delete it->second;
				it++;
			}
			best_bitexts.clear();
		}
	}
	return exit;
}
