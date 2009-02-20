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
	double aux_result;
	
	tag_array1=wf1->GetTagArrayReference();
	tag_array2=wf2->GetTagArrayReference();
	vec1len=wf1->GetFragmentedFileReference()->getSize();
	vec2len=wf2->GetFragmentedFileReference()->getSize();
	//We calculate the maximal edit distance possible to accept the pair or not. 
	if(Config::getDiagonalSize()==-1)
		beam=0;
	else
		beam=Config::getDiagonalSize();

	/*switch (Config::getMode()){
		case 2: TagAligner_generic::EditDistanceBeam(*tag_array1, *tag_array2, &TagAligner2step_l::Cost_text_and_tags_l, Config::diagonalSizeIsPercent(), beam, &res);
		default: TagAligner_generic::EditDistanceBeam(*tag_array1, *tag_array2, &TagAligner_dt::Cost_text_and_tags_dt, Config::diagonalSizeIsPercent(), beam, &res);
	}*/
	TagAligner_generic::EditDistanceBeam(*tag_array1, *tag_array2, &Cost, GlobalParams::IsPercentMaxEditDistance(), beam, &res);
	if(result!=NULL)
		*result=res;

	if(GlobalParams::IsPercentMaxEditDistance()){
		if(vec1len>vec2len)
			aux_result=res*100/((double)(vec1len));
		else
			aux_result=res*100/((double)(vec2len));
	}
	else
		aux_result=res;
	
	if(aux_result<=GlobalParams::GetMaxEditDistance()){
		return true;
	}
	else
		return false;
}

double Heuristics::Cost(const short &op, const FragmentRef &ctag1, const FragmentRef &ctag2){
	unsigned int text_distance;
	Fragment* tag1=const_cast<Fragment*>(ctag1);
	Fragment* tag2=const_cast<Fragment*>(ctag2);
	wstring current_tag1, current_tag2;
	double result=0;
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
					if(text_distance>(((double)(aux_text->getLength()+aux_text2->getLength())/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(double)100)))
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
