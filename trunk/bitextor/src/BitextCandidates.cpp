#include "BitextCandidates.h"
#include "Heuristics.h"
#include "WebSite.h"
#include <libtagaligner/Aligner.h>
#include <sstream>
#include <sys/stat.h>
#include <math.h>

BitextData::BitextData(WebFile* wf1, WebFile* wf2){
	bool exit=true;
	wostringstream *oss;
	wstring pathdistance;
	
	passes=false;

	double aux_result;

	if(wf1->IsInitialized() && wf2->IsInitialized()){
		if(wf1->GetLang()!=wf2->GetLang()){
			try{
				exit=Heuristics::HaveTheSameExtension(wf1,wf2);
				this->same_extension=exit;
				if(exit){
					exit=Heuristics::HaveAcceptableSizeDifference(wf1,wf2);
					if(exit){
						this->byte_size_distance=aux_result;
						exit=Heuristics::NearTotalTextSize(*wf1,*wf2, &aux_result);
						this->text_difference=aux_result;
						if(exit){
							exit=Heuristics::HaveAcceptableEditDistance(wf1,wf2,NULL,&aux_result);
							if(exit){
								if(((*wf1->GetTagArray()).size()==0) || (*wf2->GetTagArray()).size()==0)
									aux_result=0;
								else{
									if(Config::getDiagonalSize()==-1)
										pathdistance=EditDistanceTools::EditDistanceBeam(*wf1->GetNumbersVector(), *wf2->GetNumbersVector(), &Heuristics::CostTextAlignment, Config::diagonalSizeIsPercent(), 0, &aux_result);
									else
										pathdistance=EditDistanceTools::EditDistanceBeam(*wf1->GetNumbersVector(), *wf2->GetNumbersVector(), &Heuristics::CostTextAlignment, Config::diagonalSizeIsPercent(), Config::getDiagonalSize(), &aux_result);
									
									
									if((*wf1->GetTagArray()).size()>(*wf2->GetTagArray()).size())
										aux_result=aux_result*100/((double)(*wf1->GetTagArray()).size());
									else
										aux_result=aux_result*100/((double)(*wf2->GetTagArray()).size());
									this->edit_distance=aux_result;
									this->percent_text_distance=Heuristics::GetPhraseVariance(*wf1,*wf2,pathdistance);
									this->percent_text_distance_variation=Heuristics::GetPhraseVarianceDesviation(*wf1,*wf2,pathdistance,this->percent_text_distance);
									this->text_difference=this->percent_text_distance;
									
								}
								exit=Heuristics::DistanceInNumericFingerprint(*wf1, *wf2, &aux_result);
								
								if(!exit){
									GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its \"numeic fingerprint\" is too different.");
								}
								this->n_diff_numbers=aux_result;
							}
							else{
								oss=new wostringstream();
								*oss<<aux_result;
								GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: they edit distance is excesive ("+oss->str()+L").");
								delete oss;
							}
						}
						else{
							oss=new wostringstream();
							*oss<<aux_result;
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: the differente in the total text lenght is excesive ("+oss->str()+L").");
							delete oss;
						}
					}
					else{
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its size is too different.");
					}
				}
				else{
					GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: they have different file extensions.");
				}
			}
			catch(...){
				exit=false;
			}
		}
		else{
			exit=false;
			GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: the both files have the same language ("+wf2->GetLang()+L").");
		}
	}
	else{
		exit=false;
	}
	passes=exit;
	if(exit)
		files_related=2;
	else
		files_related=0;
}

bool BitextData::Passes(){
	return passes;
}

int BitextData::UnRelate(){
	files_related--;
	if(files_related<0)
		files_related=0;
	return files_related;
}

int BitextData::RelatedFiles(){
	return files_related;
}

bool BitextData::isBetterThan(BitextData* bitext_data, bool *disabled){
	bool exit=true;

	if(this->files_related>0){
		if(bitext_data->Passes() && bitext_data->RelatedFiles()>0){
			if(bitext_data->edit_distance<edit_distance)
				exit=false;
			else if(bitext_data->edit_distance==edit_distance){
				if(bitext_data->n_diff_numbers<n_diff_numbers)
					exit= false;
				else if(bitext_data->n_diff_numbers==n_diff_numbers){
					if(disabled!=NULL && GlobalParams::GetGenerateAmbiguousBitexts()!=-1){
						if(bitext_data->text_difference<text_difference){
							*disabled=((abs((int)bitext_data->text_difference-(int)text_difference)/text_difference)<GlobalParams::GetGenerateAmbiguousBitexts()/100);
							exit=false;
						}
						else{
							if(bitext_data->text_difference>text_difference)
								*disabled=((abs((int)bitext_data->text_difference-(int)text_difference)/bitext_data->text_difference)<GlobalParams::GetGenerateAmbiguousBitexts()/100);
							else
								*disabled=true;
						}
					}
					if(bitext_data->text_difference<text_difference)
						exit= false;
					else
						exit= true;
				}
			}
		}
		else
			exit=true;
	}
	else
		throw "BitextData object is not related with any WebFile.";

	return exit;
}

bool BitextCandidates::Add(BitextCandidates* c){
	bool exit=false;
	BitextData* tmp;
	map <wstring, pair<WebFile*,BitextData*>* >::iterator it;
	
	it=candidates.find(c->GetWebFile()->GetLang());
	if(it==candidates.end()){
		tmp=new BitextData(wf, c->GetWebFile());
		if(tmp->Passes()){
			candidates[c->GetWebFile()->GetLang()]=new pair<WebFile*,BitextData*>(c->GetWebFile(),tmp);
			if(!c->Add(wf->GetLang(),tmp))
				tmp->UnRelate();
			last_insertion=candidates.find(c->GetWebFile()->GetLang());
			exit=true;
		}
		else
			delete tmp;
	}
	else if(candidates[c->GetWebFile()->GetLang()]!=NULL){
		tmp=new BitextData(wf, c->GetWebFile());
		if(tmp->Passes()){
			if(tmp->isBetterThan(candidates[c->GetWebFile()->GetLang()]->second)){
				candidates[c->GetWebFile()->GetLang()]->second->UnRelate();
				if(candidates[c->GetWebFile()->GetLang()]->second->RelatedFiles()==0)
					delete candidates[c->GetWebFile()->GetLang()]->second;
				delete candidates[c->GetWebFile()->GetLang()];
				candidates[c->GetWebFile()->GetLang()]=new pair<WebFile*,BitextData*>(c->GetWebFile(),tmp);
				
				if(!c->Add(wf->GetLang(),tmp))
					tmp->UnRelate();
				last_insertion=candidates.find(c->GetWebFile()->GetLang());
				exit=true;
			}
			else
				delete tmp;
		}
		else
			delete tmp;
	}
	return exit;
}

bool BitextCandidates::Add(const wstring &lang, BitextData* d){
	bool exit=false;
	map <wstring, pair<WebFile*,BitextData*>* >::iterator it;

	it=candidates.find(lang);
	if(it==candidates.end()){
		candidates[lang]=new pair<WebFile*,BitextData*>(NULL,d);
		last_insertion=candidates.find(lang);
		exit=true;
	}
	else if(candidates[lang]!=NULL){
		if(d->isBetterThan(candidates[lang]->second)){
			candidates[lang]->second->UnRelate();
			if (candidates[lang]->second->RelatedFiles()==0)
				delete candidates[lang]->second;
			delete candidates[lang];
			candidates[lang]=new pair<WebFile*,BitextData*>(NULL,d);
			last_insertion=candidates.find(lang);
			exit=true;
		}
	}
	return exit;
}

BitextCandidates::BitextCandidates(WebFile* wf){
	this->wf=wf;
}

BitextCandidates::~BitextCandidates(){
	 map <wstring, pair<WebFile*,BitextData*>* >::iterator it;

	delete wf;
	for(it=candidates.begin();it!=candidates.end();it++){
		if(it->second->second!=NULL && it->second->second->UnRelate()==0)
			delete it->second->second;
		delete it->second;
	}
}

WebFile* BitextCandidates::GetWebFile(){
	return wf;
}

BitextData* BitextCandidates::GetBitextData(const wstring &lang){
	map <wstring, pair<WebFile*,BitextData*>* >::iterator it;

	it=candidates.find(lang);
	if(it!=candidates.end())
		return it->second->second;
	else
		return NULL;
}

bool BitextCandidates::GenerateBitexts(const string &dest_dir){
	Aligner *aligner;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FragmentedFile ff1, ff2;
	int l;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream oss;
	string file_name;
	ostringstream *aux_sstream;
	struct stat my_stat;
	FILE* fout;

	aligner=new Aligner();
	for(it=candidates.begin();it!=candidates.end();it++){

		aligner->Reset();
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){
		if(it->second!=NULL
				&& it->second->first!=NULL
				&& ff1.LoadFile(wf->GetPath())
				&& ff2.LoadFile(it->second->first->GetPath())){
			try{

				switch (Config::getMode()) {
					case 1: exit=aligner->Align2StepsTED(ff1,ff2); break;
					case 2: exit=aligner->Align2StepsL(ff1,ff2); break;
					case 3: exit=aligner->Align1Step(ff1,ff2); break;
				}
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
		else
			exit=false;
		if (exit) {
			file_name=dest_dir+WebSite::GetFileName(wf->GetPath())+
				"_"+
				WebSite::GetFileName(it->second->first->GetPath())+
				".tmx";
			for(l=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;l++){
				delete aux_sstream;
				aux_sstream=new ostringstream(ios_base::out);
				*aux_sstream<<l;
				file_name=dest_dir+WebSite::GetFileName(wf->GetPath())+"_"+WebSite::GetFileName(it->second->first->GetPath())+aux_sstream->str()+".tmx";
			}
			fout=fopen(file_name.c_str(),"w");
			if(fout){
				try{
					tagaligneroutput=aligner->GenerateTMX(wf->GetLang(), it->second->first->GetLang(), true, true, false);

					if (tagaligneroutput!=L""){
						fputws(tagaligneroutput.c_str(),fout);
						if(GlobalParams::IsVerbose())
							wcout<<L"The bitext between "<<Config::toWstring(wf->GetPath())<<L" and "<<Config::toWstring(it->second->first->GetPath())<<L" has been created: "<<Config::toWstring(file_name)<<L"("<<it->second->second->percent_text_distance<<L" | "<<it->second->second->percent_text_distance_variation<<L")"<<endl;
						oss<<L"Edit distance: "<<it->second->second->edit_distance<<L"% Size (bytes): "<<it->second->second->byte_size_distance<<L"% Size (characters): "<<it->second->second->text_difference;
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf->GetPath())+L" and "+Config::toWstring(it->second->first->GetPath())+L" has been created: "+Config::toWstring(file_name)+L">> "+oss.str());
						oss.seekp(ios_base::beg);
						exit=true;
					}
					else{
						fclose(fout);
						remove(file_name.c_str());
						exit=false;
					}
				}
				catch(...){
					fclose(fout);
					remove(file_name.c_str());
				}
				candidates.erase(it);
			}
		}
	}
	delete aligner;
	return exit;
}

bool BitextCandidates::GenerateBitexts(FILE * main_fout, unsigned int starting_tuid, unsigned int *last_tuid){
	Aligner *aligner;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FragmentedFile ff1, ff2;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream oss;

	aligner=new Aligner();
	for(it=candidates.begin();it!=candidates.end();it++){
		aligner->Reset();
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){
		if(it->second!=NULL && it->second->first!=NULL && ff1.LoadFile(wf->GetPath()) && ff2.LoadFile(it->second->first->GetPath())){
			try{
				switch (Config::getMode()) {
					case 1:exit=aligner->Align2StepsTED(ff1,ff2); break;
					case 2: exit=aligner->Align2StepsL(ff1,ff2); break;
					case 3: exit=aligner->Align1Step(ff1,ff2); break;
				}
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
		else
			exit=false;
		if (exit) {
			try{
				tagaligneroutput=aligner->GenerateTMX(wf->GetLang(), it->second->first->GetLang(), false, false, true, starting_tuid, last_tuid);

				if (tagaligneroutput!=L"")
					fputws(tagaligneroutput.c_str(),main_fout);
				else
					exit=false;
					
				//if(GlobalParams::IsVerbose())
				//	wcout<<L"The bitext between "<<Config::toWstring(wf->GetPath())<<L" and "<<Config::toWstring(it->second->first->GetPath())<<L" has been created"<<endl;
				oss<<it->second->second->edit_distance;
				GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf->GetPath())+L" and "+Config::toWstring(it->second->first->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
				oss.seekp(ios_base::beg);
				starting_tuid=*last_tuid;
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
	}
	delete aligner;

	return exit;					
}

bool BitextCandidates::GenerateLastAddedBitext(FILE * main_fout, unsigned int starting_tuid, unsigned int *last_tuid){
	Aligner *aligner;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FragmentedFile ff1, ff2;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream oss;

	aligner=new Aligner();
	if(last_insertion!=candidates.end()){
		aligner->Reset();
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){
		if(last_insertion->second->first!=NULL && ff1.LoadFile(wf->GetPath()) && ff2.LoadFile(last_insertion->second->first->GetPath())){
			try{
				switch (Config::getMode()) {
					case 1:exit=aligner->Align2StepsTED(ff1,ff2); break;
					case 2: exit=aligner->Align2StepsL(ff1,ff2); break;
					case 3: exit=aligner->Align1Step(ff1,ff2); break;
				}
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
		if (exit) {
			try{
				if(GlobalParams::AllBitextInAFile()){
					tagaligneroutput=aligner->GenerateTMX(wf->GetLang(), last_insertion->second->first->GetLang(), false, false, true, starting_tuid, last_tuid);
					
					if (tagaligneroutput!=L""){
						fputws(tagaligneroutput.c_str(),main_fout);
						if(GlobalParams::IsVerbose())
							wcout<<L"The bitext between "<<Config::toWstring(wf->GetPath())<<L" and "<<Config::toWstring(last_insertion->second->first->GetPath())<<L" has been created"<<endl;
						oss<<last_insertion->second->second->edit_distance;
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf->GetPath())+L" and "+Config::toWstring(last_insertion->second->first->GetPath())+L" has been created>> Edit distance: "+oss.str()+L"%.");
						oss.seekp(ios_base::beg);
					}
					else
						exit=false;
				}
				else{
					tagaligneroutput=aligner->GenerateTMX(wf->GetLang(), last_insertion->second->first->GetLang(), true, true, false);

					if(main_fout){
						if (tagaligneroutput!=L""){
							fputws(tagaligneroutput.c_str(),main_fout);
							//if(GlobalParams::IsVerbose())
							//		wcout<<L"The bitext between "<<Config::toWstring(wf->GetPath())<<L" and "<<Config::toWstring(last_insertion->second->first->GetPath())<<L" has been created: "<<endl;
							oss<<last_insertion->second->second->edit_distance;
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf->GetPath())+L" and "+Config::toWstring(last_insertion->second->first->GetPath())+L" has been created."+L">> Edit distance: "+oss.str()+L"%.");
							oss.seekp(ios_base::beg);
							exit=true;
						}
					}
					else
						exit=false;
				}
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
			}
		}
	}
	delete aligner;
	return exit;
}

void BitextCandidates::EraseLastAdded(){
	if(last_insertion!=candidates.end()){
		candidates.erase(last_insertion);
		last_insertion=candidates.end();
	}
}

WebFile* BitextCandidates::GetWebFile(const wstring &lang){
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;

	it=candidates.find(lang);
	if(it==candidates.end())
		return NULL;
	else
		return it->second->first;
}

WebFile* BitextCandidates::GetLastAddedWebFile(){
	if(last_insertion==candidates.end())
		return NULL;
	else
		return last_insertion->second->first;
}
