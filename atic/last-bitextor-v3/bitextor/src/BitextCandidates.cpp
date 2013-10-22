/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "BitextCandidates.h"
#include "Heuristics.h"
#include "WebSite.h"
#include "TranslationMemory.h"
#include <libtagaligner/Aligner.h>
#include <sstream>
#include <sys/stat.h>
#include <math.h>
#include <algorithm>


BitextData::BitextData(WebFile* wf1, WebFile* wf2){
	bool exit=true;
	wostringstream *oss;
	wstring pathdistance;
	vector<UrlLangRule*> *rules=new vector<UrlLangRule*>();
	unsigned int i;
	
	passes=false;

	double aux_result;

	if(wf1->IsInitialized() && wf2->IsInitialized()){
		if(wf1->GetLang()!=wf2->GetLang()){
			try{
				exit=(wf1->GetURL()!=NULL && wf1->GetURL()->Differences(wf2->GetURL(),rules)==1);
				if(exit){
					exit=Heuristics::HaveAcceptableSizeDifference(wf1,wf2,&aux_result);
					if(exit){
						this->byte_size_distance=aux_result;
						exit=Heuristics::NearTotalTextSize(*wf1,*wf2, &aux_result);
						this->text_difference=aux_result;
						if(exit){
							exit=Heuristics::HaveAcceptableEditDistance(wf1,wf2,NULL,&aux_result);
							if(exit){
								if(rules->size()>0 && rules->at(0)!=NULL){
									this->url_lang_rule=GlobalParams::AddUrlLangRule(rules->at(0));
									for(i=0;i<rules->size();i++)
										delete rules->at(i);
								}
								if(((*wf1->GetTagArray()).size()==0) || (*wf2->GetTagArray()).size()==0)
									aux_result=0;
								else{
									this->edit_distance=aux_result;
									//this->edit_distance=aux_result+(5/this->url_lang_rule);
								}
								/*exit=Heuristics::DistanceInNumericFingerprint(*wf1, *wf2, &aux_result);

								if(!exit){
									GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its \"numeic fingerprint\" is too different.");
								}*/
								//this->n_diff_numbers=aux_result;
							}
							else{
								for(i=0;i<rules->size();i++)
									delete rules->at(i);
								oss=new wostringstream();
								*oss<<aux_result;
								GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: they edit distance is excesive ("+oss->str()+L").");
								delete oss;
							}
						}
						else{
							for(i=0;i<rules->size();i++)
								delete rules->at(i);
							oss=new wostringstream();
							*oss<<aux_result;
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: the differente in the total text lenght is excesive ("+oss->str()+L").");
							delete oss;
						}
					}
					else{
						for(i=0;i<rules->size();i++)
							delete rules->at(i);
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its size is too different.");
					}
				}
				else
				{
					for(i=0;i<rules->size();i++)
						delete rules->at(i);
					GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: their URLs are too different.");
				}
			}
			catch(...){
				for(i=0;i<rules->size();i++)
					delete rules->at(i);
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
	delete rules;
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

	if(this->files_related>0 && bitext_data->RelatedFiles()>0){
		if(bitext_data->Passes() && this->Passes()){
			if(bitext_data->edit_distance<edit_distance)
				exit=false;
			else if(bitext_data->edit_distance==edit_distance){
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
		else if(this->Passes())
			exit=true;
		else
			exit=false;
	}
	else if(this->files_related>0)
		exit=true;
	else
		exit=false;

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
				if(candidates[c->GetWebFile()->GetLang()]->second->UnRelate()==0)
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
			if (candidates[lang]->second->UnRelate()==0)
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

bool BitextCandidates::GenerateBitexts(){
	
	bool exit=false;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	wostringstream *s;

	for(it=candidates.begin();it!=candidates.end();it++){
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){

		if(it->second!=NULL && it->second->first!=NULL){
			if(GlobalParams::GetGenerateTMX())
				TranslationMemory::WriteTM(wf,it->second->first, it->second->second);
			else{
				s=new wostringstream();
				*s<<it->second->second->url_lang_rule;
				GlobalParams::WriteResults(L"<bitext urllangrule=\""+s->str()+L"\"><lwebpage url=\""+Url::ReplaceAmp(wf->GetURL()->GetCompleteURL())+L"\">"+Config::toWstring(wf->GetPath())+L"</lwebpage><rwebpage url=\""+Url::ReplaceAmp(it->second->first->GetURL()->GetCompleteURL())+L"\">"+Config::toWstring(it->second->first->GetPath())+L"</rwebpage></bitext>");
				delete s;
			}
		}
		exit=true;
	}

	return exit;					
}

bool BitextCandidates::GenerateLastAddedBitext(){
	Aligner *aligner;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FragmentedFile ff1, ff2;
	map< wstring,pair<WebFile*,BitextData*>* >::iterator it;
	map<wstring,FILE*>::iterator it_files;
	wostringstream oss;
	wstring lang_code;
	wostringstream s;

	aligner=new Aligner();
	if(last_insertion!=candidates.end()){
		aligner->Reset();
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){
		if(last_insertion->second->first!=NULL){
			if(GlobalParams::GetGenerateTMX())
				TranslationMemory::WriteTM(wf,last_insertion->second->first,last_insertion->second->second);
			else{
				s<<last_insertion->second->second->url_lang_rule;
				GlobalParams::WriteResults(L"<bitext urllangrule=\""+s.str()+L"\"><lwebpage url=\""+Url::ReplaceAmp(wf->GetURL()->GetCompleteURL())+L"\">"+Config::toWstring(wf->GetPath())+L"</lwebpage><rwebpage url=\""+Url::ReplaceAmp(last_insertion->second->first->GetURL()->GetCompleteURL())+L"\">"+Config::toWstring(last_insertion->second->first->GetPath())+L"</rwebpage></bitext>");
			}
		}
	}
	delete aligner;
	return exit;
}

void BitextCandidates::EraseLastAdded(){
	if(last_insertion!=candidates.end()){
		if(last_insertion->second->second->UnRelate()==0)
			delete last_insertion->second->second;
		delete last_insertion->second;
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

bool BitextCandidates::CleanUnfrequentCases(const string &filename){
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	bool exit;
	wofstream results_file;
	vector<unsigned int> *freq_rules=GlobalParams::GetFreqRules(3);
	unsigned int i;
	string new_file_name;

	try{
		doc = xmlReadFile(filename.c_str(), NULL, 0);
		if(doc==NULL){
			exit=false;
		}
		else{
			for(i=0;i<filename.length() && filename[i]!='.';i++)
				new_file_name+=filename[i];
			new_file_name+="_clear.xml";
			results_file.open(new_file_name.c_str());
			root_element = xmlDocGetRootElement(doc);
			CleanUnfrequentCasesProcessNode(root_element, results_file, *freq_rules, false);
			xmlFreeDoc(doc);
			xmlCleanupParser();
			results_file.close();
		}	
	    exit=true;
	}
	catch ( const std::exception& ex ) {
		exit=false;
	}
	
	delete freq_rules;
    return exit;
}


void BitextCandidates::CleanUnfrequentCasesProcessNode(xmlNode* node, wofstream &results_file, vector<unsigned int> &freq_rules, bool write){
	xmlNode *cur_node = NULL;
	xmlChar *node_prop;
	wstring key, value;
	xmlAttrPtr propPtr;
	wstring before;
	wstring after;
	map<wstring,short>::iterator iterator;
	wstring tmp;
	wstring lang1=L"", lang2=L"";
	wstring fingerprint=L"", lang_code=L"";
	wstring tagname;
	

	for (cur_node = node; cur_node; cur_node = cur_node->next) {
		if(!(cur_node->type==XML_TEXT_NODE && xmlIsBlankNode(cur_node))){
			if(cur_node->type==XML_TEXT_NODE){
				if (write)
					results_file<<Url::ReplaceAmp(Config::xmlToWstring(cur_node->content));
			}
			else if(cur_node->type==XML_ELEMENT_NODE){
				tagname=Config::xmlToWstring((xmlChar*)cur_node->name);
				if(tagname==L"bitextcandidates"){
					results_file<<"<?xml version='1.0' encoding='UTF-8'?>"<<endl<<L"<bitextcandidates>"<<endl;
					CleanUnfrequentCasesProcessNode(cur_node->children, results_file, freq_rules, write);
					results_file<<L"</bitextcandidates>";
				}
				else if(tagname==L"bitext"){
					propPtr = cur_node->properties;
					while(propPtr) {
						key = Config::xmlToWstring(propPtr->name);
						node_prop=xmlGetProp( cur_node, propPtr->name);
						value = Config::xmlToWstring(node_prop);
						if(key==L"urllangrule"){
							//wcout<<value<<endl;
							if(find(freq_rules.begin(),freq_rules.end(),atoi(Config::toString(value).c_str()))!=freq_rules.end()){
								write=true;
								results_file<<L"\t<bitext>";
								CleanUnfrequentCasesProcessNode(cur_node->children, results_file, freq_rules, write);
								results_file<<L"</bitext>"<<endl;
							}
							else
								write=false;
						}
						free(node_prop);
						propPtr = propPtr->next;
					}
				}
				else if(write){
					if(tagname==L"lwebpage"){
						propPtr = cur_node->properties;
						while(propPtr) {
							key = Config::xmlToWstring(propPtr->name);
							node_prop=xmlGetProp( cur_node, propPtr->name);
							value = Config::xmlToWstring(node_prop);
							if(key==L"url"){
								results_file<<L"<lwebpage url=\""+Url::ReplaceAmp(value)+L"\">";
								CleanUnfrequentCasesProcessNode(cur_node->children, results_file, freq_rules, write);
								results_file<<L"</lwebpage>";
							}
							free(node_prop);
							propPtr = propPtr->next;
						}
					}else if(tagname==L"rwebpage"){
						propPtr = cur_node->properties;
						while(propPtr) {
							key = Config::xmlToWstring(propPtr->name);
							node_prop=xmlGetProp( cur_node, propPtr->name);
							value = Config::xmlToWstring(node_prop);
							if(key==L"url"){
								results_file<<L"<rwebpage url=\""+Url::ReplaceAmp(value)+L"\">";
								CleanUnfrequentCasesProcessNode(cur_node->children, results_file, freq_rules, write);
								results_file<<L"</rwebpage>";
							}
							free(node_prop);
							propPtr = propPtr->next;
						}
					}
				}
			}
		}
	}
}
