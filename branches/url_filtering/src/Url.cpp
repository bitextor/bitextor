/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include <libtagaligner/ConfigReader.h>
#include "Url.h"
#include "GlobalParams.h"
#include "BitextCandidates.h"
#include "TranslationMemory.h"
#include <cmath>
#include <tre/regex.h>
#include <fstream>

Url::Url(wstring url){
	unsigned int i;
	bool variables=false, reading_var_name=true;
	wstring str_tmp=L"", var_name=L"", var_val=L"";
	
	this->url=url;
	#ifdef DEBUG_URL_INIT
		wcout<<L"URL:"<<url<<endl;
	#endif
	for(i=0;i<url.length();i++){
		if(variables){
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
					variables=true;
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
	if(variables){
		this->variables[var_name]=var_val;
		#ifdef DEBUG_URL_INIT
			wcout<<L"\tVARIABLE: "<<var_name<<L"="<<var_val<<endl;
		#endif
	}
	else
		this->filename=str_tmp;
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
	
	//Calculating differences between directories
	if(directories.size()>url->directories.size())
		max_len=url->directories.size();
	else
		max_len=directories.size();

	for(i=0;i<directories.size();i++){
		if(directories[i]!=url->directories[i])
			not_coincident_dirs++;
	}
	not_coincident_dirs+=abs((double)(directories.size()-url->directories.size()));
	
	//Calculating differences between variables
	if(variables.size()>url->variables.size()){
		it=url->variables.begin();
		it_fin=url->variables.end();
	}
	else{
		it=variables.begin();
		it_fin=variables.end();
	}
		
	while(it!=it_fin){
		if(it->second!=url->variables[it->first])
			not_coincident_vars++;
		it++;
	}
	not_coincident_vars+=abs((double)(variables.size()-url->variables.size()));
	
	//Calculating if filename is different
	if(filename!=url->filename)
		not_coincident_names++;
	
	#ifdef DEBUG_URL_CMP
		wcout<<L"DIFFERENCES IN "<<url<<L": "<<not_coincident_vars+not_coincident_dirs<<endl;
	#endif

	return (not_coincident_vars+not_coincident_dirs+not_coincident_names);
}

bool Url::ProcessHTtrackLogFile(string &file_path, string &dest_dir){
	bool exit;
	int status;
	FILE* fin;
	string aux="";
	char aux_car;
	unsigned int i;
	regex_t re1, re2, re3, re4;
	regmatch_t matches1[1], matches2[1];
	string tmp_str, url_and_filename;
	wofstream fout;
	string aux_conv_char;
	WebFile wf;
	string url, filename;
	bool reading_url;

	if (regcomp(&re1, "[^\t\n ]+\t+[^\t\n ]+\t+[^\t\n ]+\t+[ae]", REG_EXTENDED) != 0)
		return false;
	if (regcomp(&re2, "^[a-z]+", REG_EXTENDED) != 0){
    	regfree(&re1);
		return false;
	}
	if (regcomp(&re3, " [^\t\n ]+\t+[^\t\n ]+\t+[^\t\n ]+\t+[^\t\n ]", REG_EXTENDED) != 0){
		regfree(&re1);
		regfree(&re2);
		return false;
	}
	if (regcomp(&re4, "[^\t\n ]+\t+[^\t\n ]+", REG_EXTENDED) != 0){
		regfree(&re1);
		regfree(&re2);
		regfree(&re3);
		return false;
	}

	fin=fopen(file_path.c_str(),"r");
	fout.open((dest_dir + "/URLList.xml").c_str());
	if(fin){
		if(fout.is_open()){
			fout<<L"<?xml version=\"1.0\" encoding='UTF-8'?>"<<endl<<L"<urlsAndFiles>"<<endl;

			aux_car=getc(fin);
			while(aux_car!=EOF){
				if(aux_car=='\n'){
					/*found=aux.find(L"<!-- Mirrored from ");
					if (found<aux.length()){
						found=aux.find_first_of(L' ',20);
						url=new Url(aux.substr(19,found-19));
						aux_car=WEOF;
					}
					else
						aux_car=getwc(fin);*/
					status = regexec(&re1, aux.c_str(), (size_t) 1, matches1, 0);
					if(status==0){
						status = regexec(&re2, aux.substr(matches1[0].rm_eo-1).c_str(), (size_t) 1, matches2, 0);
						if(status==0 && aux.substr(matches1[0].rm_eo-1,matches2[0].rm_eo)=="added"){
							tmp_str=aux.substr(matches1[0].rm_eo-1+matches2[0].rm_eo);
							status = regexec(&re3, tmp_str.c_str(), (size_t) 1, matches1, 0);
							if(status==0){
								status = regexec(&re4, tmp_str.substr(matches1[0].rm_eo).c_str(), (size_t) 1, matches2, 0);
								if(status==0){
									url_and_filename=tmp_str.substr(matches1[0].rm_eo-1,matches2[0].rm_eo+1);
									//fout<<L"\t<file><url>";
									filename=url="";
									reading_url=true;
									for(i=0;i<url_and_filename.length();i++){
										if(url_and_filename[i]!='\t'){
											if(url_and_filename[i]!='&'){
												//aux_conv_char=url_and_filename[i];
												//fout<<Config::toWstring(aux_conv_char);
												if(reading_url)
													url+=url_and_filename[i];
												else
													filename+=url_and_filename[i];
											}
											//else
											//	fout<<L"&amp;";
										}
										else
											//fout<<L"</url><filename>";
											reading_url=false;
									}
									wf=new WebFile();
									wf->Initialize(filename, new Url(Config::toWstring(url)));
									fout<<wf->toXML()<<endl;
									//fout<<L"</filename></file>"<<endl;
								}
							}
						}
					}
					aux="";
				}
				else
					aux+=(char)aux_car;
				aux_car=getc(fin);
			}
			fout<<"</urlsAndFiles>"<<endl;
			fout.close();
			exit=true;
		}
		else{
			exit=false;
			throw "The url list file couldn't be created.";

		}
		fclose(fin);
	}
	else{
		exit=false;
		throw "The Htrack log file couldn't be oppened.";
	}

    regfree(&re1);
    regfree(&re2);
    regfree(&re3);
    regfree(&re4);
	return exit;
}

bool Url::FilterWebFilesFromUrls(string &dest_path){
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL, *cur_node = NULL, *cur_comparing_node;
	string path=dest_path+"URLList.xml";
	bool exit;
	BitextCandidates *bitext;
	wstring url1, filename1, url2, filename2;
	Url *original_url, *comparing_url;
	WebFile *wf;

	TranslationMemory::SetDestPath(dest_path);
	
	try{
		doc = xmlReadFile(path.c_str(), NULL, 0);
		if(doc==NULL)
			throw "The URL list file doesn't exists.";
		else{
			root_element = xmlDocGetRootElement(doc);
			for (cur_node = root_element->xmlChildrenNode; cur_node/* && aux_eixida<2*/; cur_node = cur_node->next) {
				if(cur_node->type==XML_ELEMENT_NODE && cur_node->name!=NULL && Config::xmlToWstring((xmlChar*)cur_node->name)==L"file"){
					if(GetUrlAndFilename(cur_node,filename1,url1)){
						wf=new WebFile();
						original_url=new Url(url1);

						if(wf->Initialize(Config::toString(filename1), original_url)){

							bitext=new BitextCandidates(wf);
							
							for(cur_comparing_node = cur_node->next; cur_comparing_node; cur_comparing_node = cur_comparing_node->next) {
								if(cur_comparing_node->type==XML_ELEMENT_NODE && cur_comparing_node->name!=NULL && Config::xmlToWstring((xmlChar*)cur_comparing_node->name)==L"file"){
									if(GetUrlAndFilename(cur_comparing_node,filename2,url2)){
										comparing_url=new Url(url2);
										if(original_url->Differences(comparing_url)==1){
											wf=new WebFile();
											if(wf->Initialize(Config::toString(filename2), comparing_url)){
												if(bitext->Add(new BitextCandidates(wf))){
													if(GlobalParams::GetCreateAllCandidates())
														bitext->GenerateLastAddedBitext();
												}
											}
										}
									}
								}
							}
							if(!GlobalParams::GetCreateAllCandidates()){
								bitext->GenerateBitexts();
							}
							delete bitext;
						}
						//ws->GetMatchedFiles(dest_path, file_list, 1);
					}
				}
			}

			xmlFreeDoc(doc);
			xmlCleanupParser();
		}	
	    exit=true;
	}
	catch ( const std::exception& ex ) {
		wcout<<ex.what()<<endl;
		exit=false;
	}
	TranslationMemory::Reset();
	return exit;
}

bool Url::GetUrlAndFilename(xmlNode *cur_node, wstring &filename, wstring &url){
	xmlNode *node, *child_node;
	wstring tagname;
	filename=url=L"";

	for (node = cur_node->xmlChildrenNode; node; node = node->next) {
		if(node->type==XML_ELEMENT_NODE && node->name!=NULL && Config::xmlToWstring((xmlChar*)node->name)==L"filename"){
			for (child_node=node->xmlChildrenNode; child_node; child_node = child_node->next) {
				if(child_node->type==XML_TEXT_NODE)
					filename=Config::xmlToWstring(child_node->content);
			}
		} else if(node->type==XML_ELEMENT_NODE && node->name!=NULL && Config::xmlToWstring((xmlChar*)node->name)==L"url"){
			for (child_node=node->xmlChildrenNode; child_node; child_node = child_node->next) {
				if(child_node->type==XML_TEXT_NODE)
					url=Config::xmlToWstring(child_node->content);
			}
		}
	}
	if(url!=L"" && filename!=L"")
		return true;
	else
		return false;
}
