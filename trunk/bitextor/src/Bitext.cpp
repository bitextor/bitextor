#include "Bitext.h"

Bitext::Bitext()
{
	this->is_initialized=false;
}

Bitext::~Bitext(){}

bool Bitext::IsInitialized()
{
	return is_initialized;
}

bool Bitext::Initialize(WebFile *wf1, WebFile *wf2)
{
	this->wf1=wf1;
	this->wf2=wf2;
	bool exit;

	double aux_result;
	unsigned int diff_length;
	
	if(wf1->IsInitialized() && wf2->IsInitialized()){
		if(wf1->GetLang()!=wf2->GetLang()){
			try{
				exit=Heuristics::HaveTheSameExtension(wf1,wf2);
				this->same_extension=exit;
				if(exit){
					exit=Heuristics::HaveAcceptableSizeDifference(wf1,wf2,&aux_result);
					if(exit){
						this->byte_size_distance=aux_result;
						exit=Heuristics::NearTotalTextSize(*wf1,*wf2, &diff_length);
						this->text_difference=diff_length;
						if(exit){
							exit=Heuristics::HaveAcceptableEditDistance(wf1,wf2,&aux_result);
							if(exit){
								if(((*wf1->GetTagArray()).size()==0) || (*wf2->GetTagArray()).size()==0)
									aux_result=0;
								else{
									if((*wf1->GetTagArray()).size()>(*wf2->GetTagArray()).size())
										aux_result=aux_result*100/((double)(*wf1->GetTagArray()).size());
									else
										aux_result=aux_result*100/((double)(*wf2->GetTagArray()).size());
									this->edit_distance=aux_result;
								}
								
								exit=Heuristics::DistanceInNumericFingerprint(*wf1, *wf2, &aux_result);
								if(!exit)
									GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its \"numeic fingerprint\" is too different.");
								this->n_diff_numbers=aux_result;
							}
							else
								GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: they edit distance is excesive.");
						}
						else
							GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: the differente in the total text lenght is excesive");
					}
					else
						GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: its size is too different.");
				}
				else
					GlobalParams::WriteLog(L"The bitext between "+Config::toWstring(wf1->GetPath())+L" and "+Config::toWstring(wf2->GetPath())+L" will not be created: they have different file extensions.");
				this->is_initialized=true;
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
	else
		exit=false;
	is_initialized=exit;
	return exit;
}

bool Bitext::GenerateBitext(FILE *main_fout, unsigned int starting_tuid, unsigned int *last_tuid)
{
	Aligner *aligner;
	bool correct_alignment;
	bool exit=true;
	ifstream fin1, fin2;
	wstring tagaligneroutput;
	FILE* fout;
	FragmentedFile ff1, ff2;

	if(this->is_initialized){
		aligner=new Aligner();
		//if(ff1.fromXML(wf1->GetPath()+".xml") && ff2.fromXML(wf2->GetPath()+".xml")){
		if(ff1.LoadFile(wf1->GetPath()) && ff2.LoadFile(wf2->GetPath())){
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
					tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), false, false, true, starting_tuid, last_tuid);
				}
				else
					tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), true, true, false);

				if (tagaligneroutput!=L"")
					fputws(tagaligneroutput.c_str(),main_fout);
				else
					exit=false;
			}
			catch(char const* e){
				cout<<e<<endl;
				exit=false;
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

bool Bitext::isBetterThan(Bitext &bitext, bool *disabled)
{
	bool exit=true;
	
	//wcout<<bitext.edit_distance<<L" "<<edit_distance<<endl;
	if(this->is_initialized){
		if(bitext.IsInitialized()){
			if(bitext.edit_distance<edit_distance)
				exit=false;
			else if(bitext.edit_distance==edit_distance){
				if(bitext.n_diff_numbers<n_diff_numbers)
					exit= false;
				else if(bitext.n_diff_numbers==n_diff_numbers){
					if(disabled!=NULL){
						if(bitext.text_difference<text_difference)
							*disabled=((abs((int)bitext.text_difference-(int)text_difference)/text_difference)<GlobalParams::GetGenerateAmbiguousBitexts()/100);
						else{
							if(bitext.text_difference>text_difference)
								*disabled=((abs((int)bitext.text_difference-(int)text_difference)/bitext.text_difference)<GlobalParams::GetGenerateAmbiguousBitexts()/100);
							else
								*disabled=true;
						}
					}
					if(bitext.text_difference<text_difference)
						exit= false;
					else
						exit= true;
				}
			}
		}
		else
			exit=false;
	}
	else
		throw "Bitext not initialized.";

	return exit;
}
