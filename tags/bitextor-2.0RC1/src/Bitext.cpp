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
						aux_result=aux_result*100/(float)wf1.GetTagArray().size();
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
	char *lwebpage, *rwebpage;
	FILE *fin1, *fin2;
	int bytesreaded;
	int length;
	string tagaligneroutput;
	ofstream ofile;

	if(this->is_initialized){
		if (!(fin1=fopen(this->wf1.GetPath().c_str(), "r"))) {//There were errors opening the first input file
			exit=false;
		} else {
			if (!(fin2=fopen(this->wf2.GetPath().c_str(), "r"))) {//There were errors opening the first input file
				fclose(fin1);
				exit=false;
			} else {
				fseek(fin1, 0, SEEK_END);
				length=ftell(fin1);
				lwebpage=new char[length+1];
				rewind(fin1);
				bytesreaded=fread(lwebpage, 1, length, fin1);
				lwebpage[bytesreaded]='\0';
				fclose(fin1);
		
				fseek(fin2, 0, SEEK_END);
				length=ftell(fin2);
				rwebpage=new char[length+1];
				rewind(fin2);
				bytesreaded=fread(rwebpage, 1, length, fin2);
				rwebpage[bytesreaded]='\0';
				fclose(fin2);
				
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
				delete[] lwebpage;
				delete[] rwebpage;
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
