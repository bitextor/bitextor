/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "Heuristics.h"
#include <limits>
#include <math.h>
#include <libtagaligner/EditDistanceTools.h>

wstring Heuristics::lang1=L"";
wstring Heuristics::lang2=L"";

bool Heuristics::HaveTheSameExtension(WebFile *wf1, WebFile *wf2)
{
	return (wf1->GetFileType()==wf2->GetFileType());
}

bool Heuristics::HaveAcceptableSizeDifference(WebFile *wf1, WebFile *wf2, double *result)
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

			if(wf1_size>wf2_size){
				exit=(((double)abs(wf1_size-wf2_size)/(double)(wf1_size))*100<=GlobalParams::GetFileSizeDifferencePercent());
				if(result!=NULL)
					*result=((double)abs(wf1_size-wf2_size)/(double)(wf1_size))*100;
			}
			else{
				exit=(((double)abs(wf1_size-wf2_size)/(double)(wf2_size))*100<=GlobalParams::GetFileSizeDifferencePercent());
				if(result!=NULL)
					*result=((double)abs(wf1_size-wf2_size)/(double)(wf2_size))*100;
			}
		}
		catch(...){
			exit=false;
		}
	}
	return exit;
}

bool Heuristics::HaveAcceptableEditDistance(WebFile *wf1, WebFile *wf2, wstring* pathdistance, double* result)
{
	vector<int> *tag_array1, *tag_array2;
	double res;
	double beam;
	unsigned int vec1len, vec2len;
	unsigned int max_diff_abs, max_diff_percent;
	wstring pdistance;

	tag_array1=wf1->GetTagArray();
	tag_array2=wf2->GetTagArray();
	vec1len=wf1->GetTagArray()->size();
	vec2len=wf2->GetTagArray()->size();
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

	if((unsigned int)abs((int)vec1len-(int)vec2len)>max_diff_abs || (unsigned int)abs((int)vec1len-(int)vec2len)>max_diff_percent){
		if(result!=NULL)
			*result=abs((int)vec1len-(int)vec2len);
		return false;
	}
	else{
		if(Config::getDiagonalSize()==-1)
			beam=0;
		else
			beam=Config::getDiagonalSize();
		lang1=wf1->GetLang();
		lang2=wf2->GetLang();
		pdistance=EditDistanceTools::EditDistanceBeam(*tag_array1, *tag_array2, &Cost, Config::diagonalSizeIsPercent(), beam, &res);

		if(result!=NULL)
			*result=res;
		if(pathdistance!=NULL)
			*pathdistance=pdistance;

		if(max_diff_abs>=res && max_diff_percent>=res){
			return true;
		}
		else
			return false;
	}
}

double Heuristics::CostTextAlignment(const short &op, const int &ctag1, const int &ctag2){
	unsigned int text_distance;
	double result=0, tmp, difference;

	switch(op){
		case SUBST:
			if(ctag1>=0 && ctag2>=0){
				text_distance=abs(ctag1-ctag2);
				if(ctag1>ctag2)
					result=(double)text_distance/(double)ctag1;
				else{
					if(ctag2!=0)
						result=(double)text_distance/(double)ctag2;
					else
						result=ctag1;
				}
			}
			else if(ctag1<0 && ctag2<0){
				if(ctag1!=ctag2)
					result=1;
			}
			else
				result = numeric_limits<double>::max();
		break;
		default: result=1; break;
	}
	return result;
}

double Heuristics::Cost(const short &op, const int &ctag1, const int &ctag2){
	unsigned int text_distance;
	double result=0, tmp, difference;

	switch(op){
		case SUBST:
			if(ctag1>=0 && ctag2>=0){
				text_distance=abs(ctag1-ctag2);
				if(ctag1>ctag2)
					tmp=(double)text_distance/(double)ctag1;
				else{
					if(ctag2!=0)
						tmp=(double)text_distance/(double)ctag2;
					else
						tmp=ctag1;
				}
				difference=GlobalParams::GetFileSizeDiferencePercent(lang1,lang2)/(double)100;
				if(difference>=0){
					if(tmp>difference)
						result=1;
				}
			}
			else if(ctag1<0 && ctag2<0){
				if(ctag1!=ctag2)
					result=1;
			}
			else
				result = numeric_limits<double>::max();
		break;
		default: result=1; break;
	}
	return result;
}

bool Heuristics::NearTotalTextSize(WebFile &wf1, WebFile &wf2, double *value){
	unsigned int maj, diff;

	if(GlobalParams::GetMaxTotalTextLengthDiff()==-1)
		return true;
	else{
		if(wf2.GetTextSize()>wf1.GetTextSize())
			maj=wf2.GetTextSize();
		else
			maj=wf1.GetTextSize();
		diff=abs((int)wf1.GetTextSize()-(int)wf2.GetTextSize());
		
		if(value!=NULL)
			*value=diff/(double)maj;
		
		if(((double)maj*GlobalParams::GetMaxTotalTextLengthDiff())>diff)
			return true;
		else
			return false;
	}
}

double Heuristics::GetPhraseVariance(WebFile &wf1, WebFile &wf2, const wstring &pathdistance){
	unsigned int i,j,w;
	unsigned int count;
	double value, percent;

	if(wf1.GetTagArray()->size()==0 || wf2.GetTagArray()->size()==0){
		value=numeric_limits<double>::max();
	}
	else{
		percent=0;
		count=0;
		value=0;

		for(i=0,j=0,w=0;w<pathdistance.length();w++){
			switch (pathdistance[w]){
				case 'd':
					if(wf1.GetTagArray()->at(i)>=0){
						if(wf1.GetTagArray()->at(i)>0)
							percent+=1;
						count++;
					}
					i++;
				break;
				case 'i':
					if(wf2.GetTagArray()->at(j)>=0){
						if(wf2.GetTagArray()->at(j)>0)
							percent+=1;
						count++;
					}
					j++;
				break;
				default:
					if(wf1.GetTagArray()->at(i)>=0 && wf2.GetTagArray()->at(j)>=0){
						if(wf1.GetTagArray()->at(i)>wf2.GetTagArray()->at(j))
							percent+=abs(wf1.GetTagArray()->at(i)-wf2.GetTagArray()->at(j))/(double)wf1.GetTagArray()->at(i);
						else if(wf2.GetTagArray()->at(j)>0)
							percent+=abs(wf1.GetTagArray()->at(i)-wf2.GetTagArray()->at(j))/(double)wf2.GetTagArray()->at(j);
						count++;
					}
					i++;
					j++;
				break;
			}
		}
		if(count==0)
			value=numeric_limits<double>::max();
		else
			value=percent/(double)count;
	}
	//wcout<<value<<endl;
	return value;
}

double Heuristics::GetPhraseVarianceDesviation(WebFile &wf1, WebFile &wf2, const wstring &pathdistance, const double &phrasevariance){
	unsigned int i,j,w;
	unsigned int count;
	double value, percent;

	if(wf1.GetTagArray()->size()==0 || wf2.GetTagArray()->size()==0){
		value=numeric_limits<double>::max();
	}
	else{
		percent=0;
		count=0;
		value=0;
		
		for(i=0,j=0,w=0;w<pathdistance.length();w++){
			switch (pathdistance[w]){
				case 'd':
					if(wf1.GetTagArray()->at(i)>=0){
						if(wf1.GetTagArray()->at(i)>0)
							percent+=fabs(phrasevariance-1);
						else
							percent+=phrasevariance;
						count++;
					}
					i++;
				break;
				case 'i':
					if(wf2.GetTagArray()->at(j)>=0){
						if(wf2.GetTagArray()->at(j)>0)
							percent+=fabs(phrasevariance-1);
						else
							percent+=phrasevariance;
						count++;
					}
					j++;
				break;
				default:
					if(wf1.GetTagArray()->at(i)>=0 && wf2.GetTagArray()->at(j)>=0){
						if(wf1.GetTagArray()->at(i)>wf2.GetTagArray()->at(j))
							percent+=fabs(phrasevariance-(abs(wf1.GetTagArray()->at(i)-wf2.GetTagArray()->at(j))/(double)wf1.GetTagArray()->at(i)));
						else if(wf2.GetTagArray()->at(j)>0)
							percent+=fabs(phrasevariance-(abs(wf1.GetTagArray()->at(i)-wf2.GetTagArray()->at(j))/(double)wf2.GetTagArray()->at(j)));
						else
							percent+=phrasevariance;
						count++;
					}
					i++;
					j++;
				break;
			}
		}
		if(count==0)
			value=numeric_limits<double>::max();
		else
			value=percent/(double)count;
	}
	return value;
}


bool Heuristics::DistanceInNumericFingerprint(WebFile &wf1, WebFile &wf2, double *result){
	double res;

	if(GlobalParams::GetMaxNumericFingerprintDistance()==-1 || (wf1.GetNumbersVector()->size()==0 && wf2.GetNumbersVector()->size()==0)){
		if(result!=NULL)
			*result=0;
		return true;
	}
	else{
		EditDistanceTools::EditDistanceBeam(*wf1.GetNumbersVector(), *wf2.GetNumbersVector(), &CostNumbers, false, 10, &res);

		if(result!=NULL)
			*result=res;
		if(res<=GlobalParams::GetMaxNumericFingerprintDistance())
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
