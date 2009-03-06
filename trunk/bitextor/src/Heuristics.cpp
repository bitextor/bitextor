#include "Heuristics.h"

bool Heuristics::HaveTheSameExtension(WebFile *wf1, WebFile *wf2)
{
	return (wf1->GetFileType()==wf2->GetFileType());
}

bool Heuristics::HaveAcceptableSizeDifference(WebFile *wf1, WebFile *wf2, double* result=NULL)
{
	bool exit;
	int init, end;
	int wf1_size, wf2_size;
	ifstream f;
	
	if(GlobalParams::GetFileSizeDifferencePercent()==-1){
		if(result!=NULL)
			*result=-1;
		exit=true;
	}
	else{
		try{
			f.open(wf1->GetPath().c_str());
			init=f.tellg();
			f.seekg(0, ios::end);
			end=f.tellg();
			f.close();
			wf1_size=end-init;
			
			f.open(wf2->GetPath().c_str());
			init=f.tellg();
			f.seekg(0, ios::end);
			end=f.tellg();
			f.close();
			wf2_size=end-init;

			exit=(((double)abs(wf1_size-wf2_size)/(double)(wf1_size+wf2_size))*100<=GlobalParams::GetFileSizeDifferencePercent());
			if(result!=NULL)
				*result=((double)abs(wf1_size-wf2_size)/(double)(wf1_size+wf2_size))*100;
		}
		catch(...){
			exit=false;
		}
	}
	return exit;
}

bool Heuristics::HaveAcceptableEditDistance(WebFile *wf1, WebFile *wf2, double* result=NULL)
{
	vector<Fragment*> *tag_array1, *tag_array2;
	double res;
	double beam;
	unsigned int vec1len, vec2len;
	unsigned int max_diff_abs, max_diff_percent;

	tag_array1=wf1->GetTagArrayReference();
	tag_array2=wf2->GetTagArrayReference();
	vec1len=wf1->GetFragmentedFileReference()->getSize();
	vec2len=wf2->GetFragmentedFileReference()->getSize();

	//We calculate the maximal edit distance possible to accept the pair or not. 
	if(GlobalParams::GetMaxEditDistancePercentual()==-1)
		max_diff_percent=numeric_limits<unsigned int>::max();
	else{
		if(vec1len>vec2len)
			max_diff_percent=floor(vec1len*(GlobalParams::GetMaxEditDistancePercentual()/(double)100));
		else
			max_diff_percent=floor(vec2len*(GlobalParams::GetMaxEditDistancePercentual()/(double)100));
	}
	if(GlobalParams::GetMaxEditDistanceAbsolute()==-1)
		max_diff_abs=numeric_limits<unsigned int>::max();
	else
		max_diff_abs=GlobalParams::GetMaxEditDistanceAbsolute();

	if(abs((int)vec1len-(int)vec2len)>max_diff_abs || abs((int)vec1len-(int)vec2len)>max_diff_percent){
		if(result!=NULL)
			*result=abs((int)vec1len-(int)vec2len);
		return false;
	}
	else{
		if(Config::getDiagonalSize()==-1)
			beam=0;
		else
			beam=Config::getDiagonalSize();
	
		/*switch (Config::getMode()){
			case 2: TagAligner_generic::EditDistanceBeam(*tag_array1, *tag_array2, &TagAligner2step_l::Cost_text_and_tags_l, Config::diagonalSizeIsPercent(), beam, &res);
			default: TagAligner_generic::EditDistanceBeam(*tag_array1, *tag_array2, &TagAligner_dt::Cost_text_and_tags_dt, Config::diagonalSizeIsPercent(), beam, &res);
		}*/
		EditDistanceTools::EditDistanceBeam(*tag_array1, *tag_array2, &Cost, true, 100, &res);
		if(result!=NULL)
			*result=res;

		if(max_diff_abs>res && max_diff_percent>res)
			return true;
		else
			return false;
	}
}

double Heuristics::Cost(const short &op, const FragmentRef &ctag1, const FragmentRef &ctag2){
	unsigned int text_distance;
	Fragment* tag1=const_cast<Fragment*>(ctag1);
	Fragment* tag2=const_cast<Fragment*>(ctag2);
	wstring current_tag1, current_tag2;
	double result=0, tmp;
	Text *aux_text, *aux_text2;
	Tag *aux_tag, *aux_tag2;

	switch(op){
		case SUBST:
			aux_tag=dynamic_cast<Tag*>(tag1);
			aux_tag2=dynamic_cast<Tag*>(tag2);
			if(aux_tag!=NULL){
				if(aux_tag2!=NULL){
					if(aux_tag->getCode()!=aux_tag2->getCode())
						result = 1;
				}
				else{
					result = numeric_limits<double>::max();
				}
			} else {
				if (aux_tag2!=NULL) {
					result = numeric_limits<double>::max();
				}
				else {
					aux_text=dynamic_cast<Text*>(tag1);
					aux_text2=dynamic_cast<Text*>(tag2);
					text_distance=abs(aux_text2->getLength()-aux_text->getLength());
					/*if(text_distance>(((double)(aux_text->getLength()+aux_text2->getLength())/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(double)100)))
						result=1;
					else
						result=0;*/
					if(aux_text->getLength()>aux_text2->getLength())
						tmp=abs(aux_text->getLength()-aux_text2->getLength())/aux_text->getLength();
					else{
						if(aux_text2->getLength()!=0)
							tmp=abs(aux_text->getLength()-aux_text2->getLength())/aux_text2->getLength();
						else
							tmp=aux_text->getLength();
					}
					if(tmp>(GlobalParams::GetTextDistancePercentDifferenciator()/(double)100))
						result=1;
					else
						result=0;
				}
			}
		break;
		default:
			result=1;
		break;
	}
	return result;
}

bool Heuristics::NearTotalTextSize(WebFile &wf1, WebFile &wf2){
	unsigned int maj;

	if(GlobalParams::GetMaxTotalTextLengthDiff()==-1)
		return true;
	else{
		if(wf2.GetTextSize()>wf1.GetTextSize())
			maj=wf2.GetTextSize();
		else
			maj=wf1.GetTextSize();
		if(((double)maj*GlobalParams::GetMaxTotalTextLengthDiff())>abs((int)wf1.GetTextSize()-(int)wf2.GetTextSize()))
			return true;
		else
			return false;
	}
}

bool Heuristics::DistanceInNumericFingerprint(WebFile &wf1, WebFile &wf2, double *result){
	double res;
	unsigned int maj;

	if(GlobalParams::GetMaxNumericFingerprintDistance() || (wf1.GetNumbersVector()->size()==0 && wf2.GetNumbersVector()->size()==0)){
		if(result!=NULL)
			*result=0;
		return true;
	}
	else{
		EditDistanceTools::EditDistanceBeam(*wf1.GetNumbersVector(), *wf2.GetNumbersVector(), &CostNumbers, false, 10, &res);

		if(result!=NULL)
			*result=res;
		if(res<=1)
			return true;
		else
			return false;
	}
}

double Heuristics::CostNumbers(const short &op, const int &c1, const int &c2){
	double result=0;

	if(op==SUBST){
		if(c1!=c2)
			result = 1;
	}
	else
		result=1;

	return result;
}
