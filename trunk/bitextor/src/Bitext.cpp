<<<<<<< .working
#include "Bitext.h"

Bitext::Bitext()
{
	this->is_initialized=false;
}

Bitext::~Bitext(){}

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
					if(exit){
						this->byte_size_distance=aux_result;
						exit=Heuristics::NearTotalTextSize(*wf1,*wf2);
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
							if(exit){
								exit=Heuristics::DistanceInNumericFingerprint(*wf1, *wf2);
							}
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
	
bool Bitext::GenerateBitext(FILE *main_fout, unsigned int starting_tuid, unsigned int *last_tuid)
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
				if(GlobalParams::AllBitextInAFile()){
					tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), false, false, true, starting_tuid, last_tuid);
				}
				else
					tagaligneroutput=aligner->GenerateTMX(wf1->GetLang(), wf2->GetLang(), true, true, false);

				if (tagaligneroutput!=L"")
					fputws(tagaligneroutput.c_str(),main_fout);
			}
			catch(char const* e){
				cout<<e<<endl;
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
=======
#include "Bitext.h"

Bitext::Bitext()
{
	this->is_initialized=false;
}

Bitext::~Bitext()
{
}

bool Bitext::Initialize(WebFile wf1, WebFile wf2)
{
	this->wf1=wf1;
	this->wf2=wf2;
	bool exit;

	float aux_result;
	
	if(wf1.IsInitialized() && wf2.IsInitialized()){
		if(wf1.GetLang()!=wf2.GetLang()){
			try{
				exit=Heuristics::HaveTheSameExtension(wf1,wf2);
				this->same_extension=exit;
				if(exit){
					exit=Heuristics::HaveAcceptableSizeDifference(wf1,wf2,&aux_result);
					this->byte_size_distance=aux_result;
					if(exit){
						exit=Heuristics::HaveAcceptableEditDistance(wf1,wf2,&aux_result);
						if(wf1.GetTagArray().size()>wf2.GetTagArray().size())
							aux_result=aux_result*100/((float)wf1.GetTagArray().size());
						else
							aux_result=aux_result*100/((float)wf2.GetTagArray().size());
						this->edit_distance=aux_result;
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
	
bool Bitext::GenerateBitext(string path)
{
	TagAligner2step_ad *tagalignerA;
	TagAligner2step_l *tagalignerB;
	TagAligner2_1 *tagalignerC;
	bool exit=true;
	string lwebpage, rwebpage, aux;
	ifstream fin1, fin2;
	int bytesreaded;
	int length;
	string tagaligneroutput;
	ofstream ofile;

	if(this->is_initialized){
		fin1.open(this->wf1.GetPath().c_str());
		if (!fin1.is_open()) {//There were errors opening the first input file
			fprintf(stderr,"File \"%s\" could not be opened\n",this->wf1.GetPath().c_str());
		} else {
			fin2.open(this->wf2.GetPath().c_str());
			if (!fin2.is_open()) {//There were errors opening the first input file
				fprintf(stderr,"File \"%s\" could not be opened\n",this->wf2.GetPath().c_str());
				fin1.close();
			} else {
				
				fin1>>lwebpage;
				fin1>>aux;
				while(!fin1.eof()){
					lwebpage+=" "+aux;
					fin1>>aux;
				}

				fin2>>rwebpage;
				fin2>>aux;
				while(!fin2.eof()){
					rwebpage+=" "+aux;
					fin2>>aux;
				}

				fin1.close();
				fin2.close();

				ofile.open(path.c_str(), ios::out);
				if(!ofile.is_open())
					exit=false;
				else{
					if(exit){
						switch(GlobalParams::GetTagAlignerMode()){
							case 1:
								tagalignerA=new TagAligner2step_ad();
								tagalignerA->getConfig(GlobalParams::GetTagAlignerConfigFile());
								if (tagalignerA->Align(lwebpage,rwebpage)) {
									tagaligneroutput=tagalignerA->GenerateTMX(this->wf1.GetLang().c_str(), this->wf2.GetLang().c_str());
								} else
									exit=false;
								delete tagalignerA;
							break;
							case 2:
								tagalignerB=new TagAligner2step_l();
								tagalignerB->getConfig(GlobalParams::GetTagAlignerConfigFile());
								if (tagalignerB->Align(lwebpage,rwebpage)) {
									tagaligneroutput=tagalignerB->GenerateTMX(this->wf1.GetLang().c_str(), this->wf2.GetLang().c_str());
								} else
									exit=false;
								delete tagalignerB;
							break;
							case 3:
								tagalignerC=new TagAligner2_1();
								tagalignerC->getConfig(GlobalParams::GetTagAlignerConfigFile());
								if (tagalignerC->Align(lwebpage,rwebpage)) {
									tagaligneroutput=tagalignerC->GenerateTMX(this->wf1.GetLang().c_str(), this->wf2.GetLang().c_str());
								} else
									exit=false;
								delete tagalignerC;
							break;
						}
						ofile<<tagaligneroutput<<endl;
					}
				}
			}
		}
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
	
float Bitext::GetSizeDistance()
{
	if(this->is_initialized)
		return this->byte_size_distance;
	else
		throw "Bitext not initialized.";
}
	
float Bitext::GetEditDistance()
{
	if(this->is_initialized)
		return this->edit_distance;
	else
		throw "Bitext not initialized.";
}

WebFile Bitext::GetFirstWebFile()
{
	if(this->is_initialized)
		return this->wf1;
	else
		throw "Bitext not initialized.";
}

WebFile Bitext::GetSecondWebFile()
{
	if(this->is_initialized)
		return this->wf2;
	else
		throw "Bitext not initialized.";
}
>>>>>>> .merge-right.r58
