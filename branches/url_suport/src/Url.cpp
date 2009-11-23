/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "Url.h"
#include <cstdlib>
#include <climits>
//#include <cmath>

Url::Url(wstring url){
	unsigned int i;
	bool reading_variables=false, reading_var_name=true;
	wstring str_tmp=L"", var_name=L"", var_val=L"";
	
	this->url=url;
	#ifdef DEBUG_URL_INIT
		wcout<<L"URL:"<<url<<endl;
	#endif
	for(i=0;i<url.length();i++){
		if(reading_variables){
			if(reading_var_name){
				if(url[i]==L'=')
					reading_var_name=false;
				else
					var_name+=url[i];
			}
			else{
				if(url[i]==L'&'){
					this->variables[var_name]=var_val;
					#ifdef DEBUG_URL_INIT
						wcout<<L"\tVARIABLE: "<<var_name<<L"="<<var_val<<endl;
					#endif
					var_name=L"";
					var_val=L"";
					reading_var_name=true;
				}
				else
					var_val+=url[i];
			}
		}
		else{
			if(url[i]==L'/'){
				#ifdef DEBUG_URL_INIT
					wcout<<L"\tDIRECTORY: "<<str_tmp<<endl;
				#endif
				this->directories.push_back(str_tmp);
				str_tmp=L"";
			}
			else{
				if(url[i]==L'?'){
					reading_variables=true;
					this->filename=str_tmp;
					#ifdef DEBUG_URL_INIT
						wcout<<L"\tFILENAME: "<<str_tmp<<endl;
					#endif
					str_tmp=L"";
				}
				else{
					str_tmp+=url[i];
				}
			}
		}
	}
	if(reading_variables){
		this->variables[var_name]=var_val;
		#ifdef DEBUG_URL_INIT
			wcout<<L"\tVARIABLE: "<<var_name<<L"="<<var_val<<endl;
		#endif
	}
	else{
		this->filename=str_tmp;
		#ifdef DEBUG_URL_INIT
			wcout<<L"\tFILENAME: "<<str_tmp<<endl;
		#endif
	}
	#ifdef DEBUG_URL_INIT
		wcout<<L"\t(NUM. VARIABLES: "<<variables.size()<<L")"<<endl;
	#endif
}

wstring Url::GetCompleteURL(){
	return url;
}
	
wstring Url::GetDirectoriy(unsigned int &index){
	if(index>directories.size())
		return L"";
	else
		return directories[index];
}

wstring Url::GetVariableValue(wstring &var_name){
	map<wstring,wstring>::iterator it;
	
	it=variables.find(var_name);
	if(it==variables.end())
		return L"";
	else
		return it->second;
}

wstring Url::GetFilename(){
	return filename;
}

unsigned int Url::GetNumberDirectories(){
	return directories.size();
}

unsigned int Url::GetNumberVariables(){
	return variables.size();
}

bool Url::VariableExists(wstring &var_name){
	return(variables.find(var_name)!=variables.end());
}

unsigned int Url::Differences(Url* url){
	unsigned int i, max_len;
	unsigned int not_coincident_dirs=0, not_coincident_vars=0, not_coincident_names=0;
	map<wstring,wstring>::iterator it, it_fin;
	
	if(url!=NULL){
	
		//Calculating differences between directories
		if(directories.size()>url->directories.size())
			max_len=url->directories.size();
		else
			max_len=directories.size();

		for(i=0;i<max_len;i++){
			if(directories[i]!=url->directories[i])
				not_coincident_dirs++;
			#ifdef DEBUG_URL_CMP
				//wcout<<L"\tDIRECTORIES: "<<directories[i]<<L" -- "<<url->directories[i]<<endl;
			#endif
		}
		not_coincident_dirs+=abs(((int)directories.size())-((int)url->directories.size()));
		
		//Calculating differences between variables
		if(variables.size()<url->variables.size()){
			it=url->variables.begin();
			it_fin=url->variables.end();
			while(it!=it_fin){
				if(variables.find(it->first)!=variables.end()){
					if(it->second!=variables[it->first]){
						not_coincident_vars++;
						#ifdef DEBUG_URL_CMP
							wcout<<L"\tVARIABLE "<<it->first<<": "<<it->second<<L" -- "<<variables[it->first]<<endl;
						#endif
					}
						#ifdef DEBUG_URL_CMP
							else wcout<<L"\tVARIABLES IGUALS "<<it->first<<": "<<it->second<<L" -- "<<variables[it->first]<<endl;
						#endif
				}
				else
					not_coincident_vars++;
				
				it++;
			}
		}
		else{
			it=variables.begin();
			it_fin=variables.end();
			while(it!=it_fin){
				if(url->variables.find(it->first)!=url->variables.end()){
					if(it->second!=url->variables[it->first]){
						not_coincident_vars++;
						#ifdef DEBUG_URL_CMP
							wcout<<L"\tVARIABLE "<<it->first<<": "<<it->second<<L" -- "<<url->variables[it->first]<<endl;
						#endif
					}
						#ifdef DEBUG_URL_CMP
							else wcout<<L"\tVARIABLES IGUALS "<<it->first<<": "<<it->second<<L" -- "<<url->variables[it->first]<<endl;
						#endif
				}
				else
					not_coincident_vars++;
				
				it++;
			}
		}
		//wcout<<L"MIDA VARIABLES: "<<(int)variables.size()<<L"-"<<(int)url->variables.size()<<L"="<<(((int)variables.size())-((int)(url->variables.size())))<<endl;
		//not_coincident_vars+=abs(((int)variables.size())-((int)(url->variables.size())));
		//
		//Calculating if filename is different
		if(filename!=url->filename)
			not_coincident_names++;	
		#ifdef DEBUG_URL_CMP
			wcout<<L"\tFILENAME: "<<filename<<L" -- "<<url->filename<<endl;
		#endif

		
		#ifdef DEBUG_URL_CMP
			wcout<<L"DIFFERENCES IN "<<GetCompleteURL()<<L" "<<url->GetCompleteURL()<<L": "<<not_coincident_vars<<L","<<not_coincident_dirs<<L","<<not_coincident_names<<endl;
		#endif

		return (not_coincident_vars+not_coincident_dirs+not_coincident_names);
	}
	else
		return UINT_MAX;
}
