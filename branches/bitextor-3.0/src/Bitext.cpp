#include "Bitext.h"

Bitext::Bitext()
{
	this->is_initialized=false;
}

Bitext::~Bitext()
{
}

bool Bitext::Initialize(WebFile *wf1, WebFile *wf2)
{
	this->wf1=wf1;
	this->wf2=wf2;
	bool exit;

	double aux_result;
	
	if(wf1->IsInitialized() && wf2->IsInitialized()){
		if(wf1->GetLang()!=wf2->GetLang()){
			try{
				exit=Heuristics::HaveTheSameExtension(wf1,wf2);
				this->same_extension=exit;
				if(exit){
					exit=Heuristics::HaveAcceptableSizeDifference(wf1,wf2,&aux_result);
					this->byte_size_distance=aux_result;
					if(exit){
						exit=Heuristics::HaveAcceptableEditDistance(wf1,wf2,&aux_result);
						if(((*wf1->GetTagArrayReference()).size()==0) || (*wf2->GetTagArrayReference()).size()==0)
							aux_result=0;
						else{
							if((*wf1->GetTagArrayReference()).size()>(*wf2->GetTagArrayReference()).size())
								aux_result=aux_result*100/((double)(*wf1->GetTagArrayReference()).size());
							else
								aux_result=aux_result*100/((double)(*wf2->GetTagArrayReference()).size());
							this->edit_distance=aux_result;
						}
					}
				}
				this->is_initialized=true;
			}
			catch(...){
				exit=false;
			}
		}
		else
			exit=false;
	}
	else
		exit=false;
	return exit;
}
	
bool Bitext::GenerateBitext(const string &path)
{
	Aligner *aligner;
	bool correct_alignment;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FILE* fout;
	if(this->is_initialized){
		aligner=new Aligner();
		try{
			switch (Config::getMode()) {
				case 1:exit=aligner->Align2StepsTED(*wf1->GetFragmentedFileReference(),*wf2->GetFragmentedFileReference()); break;
				case 2: exit=aligner->Align2StepsL(*wf1->GetFragmentedFileReference(),*wf2->GetFragmentedFileReference()); break;
				case 3: exit=aligner->Align1Step(*wf1->GetFragmentedFileReference(),*wf2->GetFragmentedFileReference()); break;
			}
		}
		catch(char const* e){
			cout<<e<<endl;
		}
		if (exit) {
			try{
				tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang());
			}
			catch(char const* e){
				cout<<e<<endl;
			}
			fout=fopen(path.c_str(),"w");
			if (!fout) {//Hubo errores en la apertura del fichero de salida
					wcout<<tagaligneroutput;
			} else {
				if (tagaligneroutput!=L"")
					fputws(tagaligneroutput.c_str(),fout);
				fclose(fout);
			}
		}
		delete aligner;
	}
	else
		throw "Bitext not initialized.";
	return exit;
}
	
bool Bitext::GetSameExtension()
{
	if(this->is_initialized)
		return this->same_extension;
	else
		throw "Bitext not initialized.";
}
	
double Bitext::GetSizeDistance()
{
	if(this->is_initialized)
		return this->byte_size_distance;
	else
		throw "Bitext not initialized.";
}
	
double Bitext::GetEditDistance()
{
	if(this->is_initialized)
		return this->edit_distance;
	else
		throw "Bitext not initialized.";
}

WebFile* Bitext::GetFirstWebFile()
{
	if(this->is_initialized)
		return this->wf1;
	else
		throw "Bitext not initialized.";
}

WebFile* Bitext::GetSecondWebFile()
{
	if(this->is_initialized)
		return this->wf2;
	else
		throw "Bitext not initialized.";
}
