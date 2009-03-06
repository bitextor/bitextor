<<<<<<< .working
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
=======
#include "Heuristics.h"

bool Heuristics::HaveTheSameExtension(WebFile wf1, WebFile wf2)
{
	return (wf1.GetFileType()==wf2.GetFileType());
}

bool Heuristics::HaveAcceptableSizeDifference(WebFile wf1, WebFile wf2, float* result=NULL)
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
			f.open(wf1.GetPath().c_str());
			init=f.tellg();
			f.seekg(0, ios::end);
			end=f.tellg();
			f.close();
			wf1_size=end-init;
			
			f.open(wf2.GetPath().c_str());
			init=f.tellg();
			f.seekg(0, ios::end);
			end=f.tellg();
			f.close();
			wf2_size=end-init;

			exit=(((float)abs(wf1_size-wf2_size)/(float)(wf1_size+wf2_size))*100<=GlobalParams::GetFileSizeDifferencePercent());
			if(result!=NULL)
				*result=((float)abs(wf1_size-wf2_size)/(float)(wf1_size+wf2_size))*100;
		}
		catch(...){
			exit=false;
		}
	}
	return exit;
}

bool Heuristics::HaveAcceptableEditDistance(WebFile wf1, WebFile wf2, float* result=NULL)
{
	vector<int> tag_array1, tag_array2;
	int res;
	float beam;
	unsigned int vec1len, vec2len;
	
	tag_array1=wf1.GetTagArray();
	tag_array2=wf2.GetTagArray();
	vec1len=tag_array1.size();
	vec2len=tag_array2.size();
	//We calculate the maximal edit distance possible to accept the pair or not. 
	if(GlobalParams::GetMaxEditDistance()==-1)
		beam=0;
	else
		beam=GlobalParams::GetMaxEditDistance();

	res=EditDistance(tag_array1, tag_array2, true, beam);
	if(res>-1){
		if(result!=NULL)
			*result=res;
		return true;
	}
	else
		return false;
}


int Heuristics::EditDistance(vector<int>& tts1, vector<int>& tts2, const bool &percent_beam, const float &beam_value) {
	int result;
	unsigned int i, j, k;

	double **matrix;
	double subscost, inscost, delcost; //Temporary costs
	unsigned int startingcol, endingcol; //Indicate what columns of the row are processed
	unsigned int nextstartingcol, nextendingcol; //Indicate what columns of the row are processed in the next row
	unsigned short ttssize1=tts1.size();
	unsigned short ttssize2=tts2.size();
	double max_val;
	unsigned int x_limit, y_limit;
	double despl;
	unsigned int row_size, col_size;
	unsigned int dist_before;

	if (ttssize1 > 0 && ttssize2 > 0) {
		//We calculate the row maximum length
		if(beam_value>0){
			if(percent_beam){
				if(beam_value>100){
					row_size=ttssize2+1;
					col_size=ttssize1+1;
				}
				else{
					if(ttssize1>ttssize2){
						row_size=floor((beam_value/100)*(ttssize1));
						col_size=floor((beam_value/100)*(ttssize1));
					}
					else{
						row_size=floor((beam_value/100)*(ttssize2));
						col_size=floor((beam_value/100)*(ttssize2));
					}
					if(row_size>ttssize2+1 || col_size>ttssize1+1){
						row_size=ttssize2+1;
						col_size=ttssize1+1;
					}
				}
			}
			else{
				if(beam_value>ttssize2 || beam_value>ttssize1){
					row_size=ttssize2+1;
					col_size=ttssize1+1;
				}
				else{
					row_size=beam_value;
					col_size=beam_value;
				}
			}
		}
		else{
			row_size=ttssize2+1;
			col_size=ttssize1+1;
		}
		
		//We calculate the distance of the starting point of every row from the origin column
		if(col_size!=ttssize1+1)
			despl=((float)(ttssize2+1-row_size))/(ttssize1+1-col_size);
		else
			despl=0;

		//Initializing the algorithm
		matrix=new double*[2];//We keep only 2 rows of the matrix

		startingcol=0;
		endingcol=row_size-1;//Initially the whole row is processed
		nextstartingcol=floor(despl);
		nextendingcol=floor(row_size-1+despl);

		matrix[0]=NULL;
		matrix[1]=new double[nextendingcol-startingcol+1];

		//Inicialization of the first row: only insertion costs
		matrix[1][0]=0;

		for (j=0; j<endingcol; j++)
			matrix[1][j+1] = Cost(INSERT, tts2[j], 0) + matrix[1][j];
		for(;j<nextendingcol;j++)
			matrix[1][j+1] = numeric_limits<double>::max();
		
		for (i=0; i<ttssize1 && startingcol<endingcol; i++) {
			if(i>col_size){
				dist_before=nextstartingcol-startingcol;
				startingcol = nextstartingcol;
				endingcol = nextendingcol;
				nextstartingcol=floor(despl*(i+1-col_size));
			}
			else{
				dist_before=0;
				startingcol = nextstartingcol;
				endingcol = nextendingcol;
				nextstartingcol=0;
			}
			nextendingcol=ceil(row_size+(despl*(i+1)));
			if(nextendingcol>ttssize2)
				nextendingcol=ttssize2+1;

			if(matrix[0]!=NULL)
				delete[] matrix[0];
			matrix[0]=matrix[1];

			matrix[1]=new double[nextendingcol-startingcol+1];

			//Setting the first column position
			matrix[1][0] = Cost(DELETE, 0, tts1[i]) + matrix[0][dist_before];

			//Setting the rest of the row
			for (j=0; j<endingcol-startingcol; j++) {

				//Substitution cost
				subscost = Cost(SUBST,tts2[j+startingcol], tts1[i]) + matrix[0][j+dist_before];

				//Insertion cost
				inscost = Cost(INSERT,tts2[j+startingcol], 0) + matrix[1][j];

				//Deletion cost
				delcost = Cost(DELETE,0, tts1[i]) + matrix[0][j+1+dist_before];

				//Choosing the minimal cost
				if (subscost < inscost) {
					if (subscost < delcost)
						matrix[1][j+1] = subscost;
					else
						matrix[1][j+1] = delcost;
				} else {
					if (inscost < delcost)
						matrix[1][j+1] = inscost;
					else
						matrix[1][j+1] = delcost;
				}
			}
			for(;j<nextendingcol-startingcol; j++){
				matrix[1][j+1]=numeric_limits<double>::max();
			}
		}
		if (startingcol==endingcol){
			result=-1;
		}

		result=matrix[1][endingcol-startingcol];

		delete[] matrix[0];
		delete[] matrix[1];
		delete[] matrix;
		matrix = NULL;
	} // Big if
	else{
		result=-1;
	}
	if(row_size<result)
		result=-1;
	return result;
}


double Heuristics::Cost(const short &op, const int &tag1, const int &tag2){
	double result;
	unsigned int text_distance;
	switch(op){
		case SUBST:
			if(tag1>0 && tag2>0){
				if(GlobalParams::GetTextDistancePercentDifferenciator()<0)
					result=0;
				else{
					text_distance=abs(tag1-tag2);
					if(text_distance>(((float)(tag1+tag2)/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100)))
						result=1;
					else
						result=0;
				}
			}
			else{
				if(tag1!=tag2)
					result=1;
				else
					result=0;
			}
		break;
		default:
			result=1;
		break;
	}
	return result;
}
>>>>>>> .merge-right.r58
