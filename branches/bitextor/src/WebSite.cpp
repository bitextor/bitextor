#include "WebSite.h"

WebSite::WebSite()
{
	this->base_path="";
}

WebSite::~WebSite(){}

WebSite(string base_path)
{
	this->base_path=base_path;
}

void toXML(string XML_path)
{
	
}

void setBasePath(string base_path)
{
	this->base_path=base_path;
}

string getBasePath()
{
	return this->base_path;
}

bool addWebFile(WebFile wf)
{
	bool exit = true;

	vector<string>::iterator iter;
	for(iter = this->llista_fitxers.begin(); iter_!=this->llista_fitxers.end() && exit; iter++)
	{
		if(*iter.getPath()==wf.getPath())
			exit=false;
	}
	if(exit)
		this->wf.push_back(wf);
	return exit;
}

bool removeWebFile(string file_path)
{
	bool exit=false;

	vector<string>::iterator iter;
	for(iter = this->llista_fitxers.begin(); iter_!=this->llista_fitxers.end() && exit; iter++)
	{
		if(*iter.getPath()==wf.getPath()){
			exit=true;
			this->llista_fitxers.erase(iter);
		}
	}
	return exit;
}

int getWebFile(int pos)
{
	return this->llista_fitxers [pos]
}
