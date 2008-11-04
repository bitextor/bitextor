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
			if(wf1_size>wf2_size){
				exit=((((float)wf1_size/wf2_size)-1)*100<=GlobalParams::GetFileSizeDifferencePercent());
				if(result!=NULL)
					*result=(((float)wf1_size/wf2_size)-1)*100;
			}
			else{
				exit=((((float)wf2_size/wf1_size)-1)*100<=GlobalParams::GetFileSizeDifferencePercent());
				if(result!=NULL)
					*result=(((float)wf2_size/wf1_size)-1)*100;
			}
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
	int i, j, val_aux, max_dist, max_j, text_distance;
	float diagonal_inc;
	bool jump;
	int **tab_aux;
	int vec1len, vec2len;
	float exit;
	
	//We get the tag and text lists from both the WebFiles 
	try{
		tag_array1=wf1.GetTagArray();
		tag_array2=wf2.GetTagArray();
	}
	catch(char* error){
		cout<<error;
	}
	
	try{
		vec1len=tag_array1.size();
		vec2len=tag_array2.size();
		
		//We calculate the maximal edit distance possible to accept the pair or not. 
		if(GlobalParams::GetMaxEditDistance()==-1)
			max_dist=vec1len+vec2len;
		else
		{
			max_dist=((float)GlobalParams::GetMaxEditDistance()/100)*((float)(vec1len+vec2len)/2);
		}
		//If the size diference between the both tag/text arrays is greather than the max. distance accepted, we will consider that the files are different.
		if(vec1len-vec2len<=max_dist && vec2len-vec1len<=max_dist){
			//If the max. distance acceptable is greather than the length of one of the tag/text arrays, we will calculate the wole table of values for the edit distance.
			if(max_dist>vec1len || max_dist>vec2len)
				max_dist=vec1len+vec2len;
			//We inicialize the table to calculate the edit distance
			tab_aux=new int*[vec1len+1];
			for(i=0;i<vec1len+1;i++)
				tab_aux[i]=new int[vec2len+1];
			
			for(i=0;i<vec1len+1;i++)
			{
				if(i>max_dist)
					tab_aux[i][0]=-1;
				else
					tab_aux[i][0]=i;
			}
			for(i=1;i<vec2len+1;i++)
			{
				if(i>max_dist)
					tab_aux[0][i]=-1;
				else
					tab_aux[0][i]=i;
			}
			
			//We calculate the value of the inclination in the diagonal search for the edit distance.
			if(vec1len>max_dist || vec2len>max_dist){
				diagonal_inc=((float)(vec2len-max_dist))/((float)(vec1len-max_dist));
				if(diagonal_inc<0)
					diagonal_inc=diagonal_inc*-1;
			}
			else
				diagonal_inc=0;
			//If the value of diagonal_inc is greather than 0, we search only in the diagonal determined
			//by this value.
			if(diagonal_inc>(float)0)
			{
				/*
				cout<<"  |";
				for(i=1; i<vec1len+1; i++)
					cout<<tag_array1[i-1]<<"|";
				cout<<endl;
				//*/
				for(i=1; i<vec1len+1; i++)
				{
					/*We calculate the starting point and the ending point in the row of the table
					  for the iteration*/
					//
					//cout<<tag_array2[i-1]<<"|";
					//
					if(i>max_dist+1){
						j=1+(diagonal_inc*((i-max_dist)-1));
						tab_aux[i][j-1]=-1;
						
						max_j=max_dist+1+(diagonal_inc*(i-1));
						if(max_j>vec2len+1)
							max_j=vec2len+1;
						/*
						for(int w=1;w<j;w++)
							cout<<"|  ";
						//*/
					}
					else{
						j=1;
						max_j=max_dist+1+(diagonal_inc*(i-1));
						if(max_j>vec2len+1)
							max_j=vec2len+1;
					}
					//Now we start the iteration, considering that -1 is the infinite value.
					for(; j<max_j ; j++)
					{
						val_aux=GetMinorValue(tab_aux[i-1][j],tab_aux[i-1][j-1],tab_aux[i][j-1], max_dist+1);
						if(tag_array1[i-1]==tag_array2[j-1] || val_aux==-1)
							tab_aux[i][j]=val_aux;
						else
						{
							if(tag_array1[i-1]>0 && tag_array2[j-1]>0){
								if(GlobalParams::GetTextDistancePercentDifferenciator()<0)
									tab_aux[i][j]=val_aux;
								else{
									text_distance=tag_array1[i-1]-tag_array2[j-1];
									if(text_distance<0)
										text_distance=text_distance*-1;
									if(text_distance>(((float)(tag_array1[i-1]+tag_array2[j-1])/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/100)))
										tab_aux[i][j]=val_aux+1;
									else
										tab_aux[i][j]=val_aux;
									/*if(text_distance>(((tag_array1[i-1]+tag_array2[j-1])/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/100)))
										cout<<"Tag1="<<tag_array1[i-1]<<" Tag2="<<tag_array2[j-1]<<" Text Distance="<<text_distance<<" Max distance="<<(((tag_array1[i-1]+tag_array2[j-1])/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100))<<" -> "<<"Sí"<<endl;
									else
										cout<<"Tag1="<<tag_array1[i-1]<<" Tag2="<<tag_array2[j-1]<<" Text Distance="<<text_distance<<" Max distance="<<(((tag_array1[i-1]+tag_array2[j-1])/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100))<<" -> "<<"No"<<endl;*/
								}
								
							}
							else
								tab_aux[i][j]=val_aux+1;
						}
						/*
						if(tab_aux[i][j]<10 && tab_aux[i][j]!=-1)
							cout<<tab_aux[i][j]<<" |";
						else
							cout<<tab_aux[i][j]<<"|";
						//*/
					}
					/*Finaly, continue writing -1 in the cells which will be visited by the algorithm in the next iteration.
					  In this way, we are telling the algorithm that those cells contents the value "infinite".*/
					if(max_dist+1+(diagonal_inc*(i))>vec2len+1)
						max_j=vec2len+1;
					else
						max_j=max_dist+1+(diagonal_inc*(i));
					for(; j<max_j ; j++){
						tab_aux[i][j]=-1;
						//
						//cout<<tab_aux[i][j]<<"|";
					}
					//
					//cout<<endl;
				}
			}
			//If diagonal_inc value is 0, we will calculate the whole table to obtain the edit distance.
			else
			{
				for(i=1; i<vec1len+1; i++)
				{
					jump=false;
					for(j=1; j<vec2len+1 && !jump; j++)
					{
						val_aux=GetMinorValue(tab_aux[i-1][j],tab_aux[i-1][j-1],tab_aux[i][j-1], max_dist+1);
						if(val_aux==-1){
							tab_aux[i][j]=val_aux;
							jump=true;
						}
						else{
							if(tag_array1[i-1]==tag_array2[j-1])
								tab_aux[i][j]=val_aux;
							else{
								if(tag_array1[i-1]>0 && tag_array2[j-1]>0){
									if(GlobalParams::GetTextDistancePercentDifferenciator()<0)
										tab_aux[i][j]=val_aux;
									else{
										text_distance=tag_array1[i-1]-tag_array2[j-1];
										if(text_distance<0)
											text_distance=text_distance*-1;
										if(text_distance>(((float)(tag_array1[i-1]+tag_array2[j-1])/2)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100)))
											tab_aux[i][j]=val_aux+1;
										else
											tab_aux[i][j]=val_aux;
										//cout<<"Tag1="<<tag_array1[i-1]<<" Tag2="<<tag_array2[j-1]<<" Text Distance="<<text_distance<<" Max distance="<<((((tag_array1[i-1]+tag_array2[j-1])/2)*-1)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100))<<" -> "<<(text_distance>((((tag_array1[i-1]+tag_array2[j-1])/2)*-1)*(GlobalParams::GetTextDistancePercentDifferenciator()/(float)100)))<<endl;
									}
								}
								else
									tab_aux[i][j]=val_aux+1;
							}
						}
					}
				}
			}
			val_aux=tab_aux[vec1len][vec2len];
			for(i=0; i<vec1len+1; i++){
				delete[] tab_aux[i];
			}
			delete[] tab_aux;
			exit= val_aux;
		}
		else
			exit= -1;
		if(result!=NULL)
			*result=exit;
		if(exit>-1 && (float)(exit*100)/vec1len<=GlobalParams::GetMaxEditDistance())
			return true;
		else
			return false;
	}
	catch(...){
		try{
			for(i=0; i<vec1len+1; i++){
				delete[] tab_aux[i];
			}
			delete[] tab_aux;
		}
		catch(...){}
		return false;
	}
}



int Heuristics::GetMinorValue(int vl1, int vl2, int vl3, int infinite)
{
	int exit;
	int v1,v2,v3;

	if(vl1==-1) v1=infinite; else v1=vl1;
	if(vl2==-1) v2=infinite; else v2=vl2;
	if(vl3==-1) v3=infinite; else v3=vl3;
	if(v1<v2){
		if(v1<v3)
			exit= v1;
		else
			exit= v3;
	}
	else{
		if(v2<v3)
			exit= v2;
		else
			exit= v3;
	}
	
	if(exit>=infinite)
		return -1;
	else
		return exit;
}
