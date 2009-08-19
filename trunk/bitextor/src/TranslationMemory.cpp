#include "TranslationMemory.h"
#include "GlobalParams.h"
#include "WebSite.h"
#include <sstream>
#include <libtagaligner/ConfigReader.h>
#include <libtagaligner/FragmentedFile.h>
#include <libtagaligner/Aligner.h>
#include <sys/stat.h>

string TranslationMemory::dest_path="~";
		
map< wstring, pair< FILE*,int >* > TranslationMemory::uniq_files;

void TranslationMemory::SetDestPath(const string &path){
	dest_path=path;
}

bool TranslationMemory::WriteInSameFile(WebFile* wf1, WebFile* wf2, BitextData* data){
	wstring lang_code1, lang_code2;
	map< wstring, pair< FILE*,int >* >::iterator it_files;
	FragmentedFile ff1, ff2;
	bool exit;
	wstring tagaligneroutput;
	unsigned int last_tuid;
	Aligner *aligner;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream oss;
	
	try{
		aligner=new Aligner();
		if(ff1.LoadFile(wf1->GetPath()) && ff2.LoadFile(wf2->GetPath())){
			lang_code1=wf1->GetLang()+L"_"+wf2->GetLang();
			lang_code2=wf2->GetLang()+L"_"+wf1->GetLang();

			it_files=uniq_files.find(lang_code1);
			if(it_files!=uniq_files.end()){
				try{
					switch (Config::getMode()) {
						case 1:exit=aligner->Align2StepsTED(ff1,ff2); break;
						case 2: exit=aligner->Align2StepsL(ff1,ff2); break;
						case 3: exit=aligner->Align1Step(ff1,ff2); break;
					}
					tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), false, false, true, it_files->second->second, &last_tuid);
					it_files->second->second=last_tuid+1;
					if (tagaligneroutput!=L""){
						fputws(tagaligneroutput.c_str(),it_files->second->first);
						if(GlobalParams::IsVerbose())
							wcout<<L"The bitext between "<<Config::toWstring(wf1->GetPath())<<L" and "<<Config::toWstring(wf2->GetPath())<<L" has been created"<<endl;
						oss<<data->edit_distance;
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
						oss.seekp(ios_base::beg);
						uniq_files[lang_code1]->second=last_tuid;
					}
					else
						exit=false;
				}
				catch(char const* e){
					cout<<e<<endl;
					exit=false;
				}
			} else{
				it_files=uniq_files.find(lang_code2);
				if(it_files!=uniq_files.end()){
					try{
						switch (Config::getMode()) {
							case 1:exit=aligner->Align2StepsTED(ff2,ff1); break;
							case 2: exit=aligner->Align2StepsL(ff2,ff1); break;
							case 3: exit=aligner->Align1Step(ff2,ff1); break;
						}
						tagaligneroutput=aligner->GenerateTMX(wf2->GetLang(), wf1->GetLang(), false, false, true, it_files->second->second, &last_tuid);
						it_files->second->second=last_tuid+1;
						if (tagaligneroutput!=L""){
							fputws(tagaligneroutput.c_str(),it_files->second->first);
							if(GlobalParams::IsVerbose())
								wcout<<L"The bitext between "<<Config::toWstring(wf2->GetPath())<<L" and "<<Config::toWstring(wf1->GetPath())<<L" has been created"<<endl;
							oss<<data->edit_distance;
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf2->GetPath())+L" and "+Config::toWstring(wf1->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
							oss.seekp(ios_base::beg);
							uniq_files[lang_code2]->second=last_tuid;
						}
						else
							exit=false;
					}
					catch(char const* e){
						cout<<e<<endl;
						exit=false;
					}
				}
				else{
					uniq_files[lang_code2]=new pair<FILE*,int>(fopen((dest_path+"/"+Config::toString(lang_code2+L"_")+"bitext.tmx").c_str(),"w"),1);
					fputws(Aligner::GetHeading().c_str(),uniq_files[lang_code2]->first);

					try{
						switch (Config::getMode()) {
							case 1:exit=aligner->Align2StepsTED(ff2,ff1); break;
							case 2: exit=aligner->Align2StepsL(ff2,ff1); break;
							case 3: exit=aligner->Align1Step(ff2,ff1); break;
						}
						tagaligneroutput=aligner->GenerateTMX(wf2->GetLang(), wf1->GetLang(), false, false, true, uniq_files[lang_code2]->second, &last_tuid);
						uniq_files[lang_code2]->second=last_tuid+1;
						if (tagaligneroutput!=L""){
							fputws(tagaligneroutput.c_str(),uniq_files[lang_code2]->first);
							if(GlobalParams::IsVerbose())
								wcout<<L"The bitext between "<<Config::toWstring(wf2->GetPath())<<L" and "<<Config::toWstring(wf1->GetPath())<<L" has been created"<<endl;
							oss<<data->edit_distance;
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf2->GetPath())+L" and "+Config::toWstring(wf1->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
							oss.seekp(ios_base::beg);
							uniq_files[lang_code2]->second=last_tuid;
						}
						else
							exit=false;
					}
					catch(char const* e){
						cout<<e<<endl;
						exit=false;
					}
				}
			}
		}
		delete aligner;
	}
	catch(char const* e){
		cout<<e<<endl;
		exit=false;
	}
	return true;
}

bool TranslationMemory::WriteInDifferentFile(WebFile* wf1, WebFile* wf2, BitextData* data){
	wstring lang_code1, lang_code2;
	map< wstring, pair< FILE*,int >* >::iterator it_files;
	FragmentedFile ff1, ff2;
	bool exit;
	wstring tagaligneroutput;
	Aligner *aligner;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream oss;
	FILE *fout;
	string file_name;
	ostringstream *aux_sstream;
	unsigned int l;
	struct stat my_stat;
	
	try{
		aligner=new Aligner();
		if(ff1.LoadFile(wf1->GetPath()) && ff2.LoadFile(wf2->GetPath())){
			try{
				switch (Config::getMode()) {
					case 1:exit=aligner->Align2StepsTED(ff1,ff2); break;
					case 2: exit=aligner->Align2StepsL(ff1,ff2); break;
					case 3: exit=aligner->Align1Step(ff1,ff2); break;
				}
				
				file_name=dest_path+WebSite::GetFileName(wf1->GetPath())+"_"+WebSite::GetFileName(wf2->GetPath())+".tmx";
				for(l=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;l++){
					delete aux_sstream;
					aux_sstream=new ostringstream(ios_base::out);
					*aux_sstream<<l;
					file_name=dest_path+WebSite::GetFileName(wf1->GetPath())+"_"+WebSite::GetFileName(wf2->GetPath())+aux_sstream->str()+".tmx";
				}
				delete aux_sstream;
				fout=fopen(file_name.c_str(),"w");

				//fout=fopen((dest_path+"/"+WebSite::GetFileName(wf1->GetPath())+"_"+WebSite::GetFileName(wf2->GetPath())+".tmx").c_str(), "w");
				tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), true, true, false);
				if (tagaligneroutput!=L""){
					fputws(tagaligneroutput.c_str(),fout);
					if(GlobalParams::IsVerbose())
						wcout<<L"The bitext between "<<Config::toWstring(wf1->GetPath())<<L" and "<<Config::toWstring(wf2->GetPath())<<L" has been created"<<endl;
					oss<<data->edit_distance;
					GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
					oss.seekp(ios_base::beg);
				}
				else
					exit=false;

				fclose(fout);
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
		delete aligner;
	}
	catch(char const* e){
		cout<<e<<endl;
		exit=false;
	}
	return true;
}
	
bool TranslationMemory::WriteTM(WebFile* wf1, WebFile* wf2, BitextData* data){
	bool exit;
	
	if(GlobalParams::AllBitextInAFile())
		exit=WriteInSameFile(wf1, wf2, data);
	else{
		wcout<<L"ACI SI"<<endl;
		exit=WriteInDifferentFile(wf1, wf2, data);
	}
	return exit;
}

void TranslationMemory::Reset(){
	map< wstring, pair< FILE*,int >* >::iterator f_it;
	
	for(f_it=uniq_files.begin();f_it!=uniq_files.end();f_it++){
		fclose(f_it->second->first);
		delete f_it->second;
	}
	uniq_files.clear();
}
