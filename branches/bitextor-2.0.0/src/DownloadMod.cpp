#include "DownloadMod.h"

DownloadMod::DownloadMod()
{
	this->max_downloaded_size=GlobalParams::GetMaxBytesToDownload();
	this->accepted_extenssions=GlobalParams::GetAcceptedExtenssions();
}

DownloadMod::~DownloadMod()
{
}

void DownloadMod::SetMaxDownloadedSize(long mds)
{
	this->max_downloaded_size=mds;
}

void DownloadMod::AddAcceptedExtenssions(string extenssion)
{
	this->accepted_extenssions.push_back(extenssion);
}

void DownloadMod::AddAcceptedExtenssions(vector<string> extenssion_list)
{
	this->accepted_extenssions.insert(this->accepted_extenssions.begin(), extenssion_list.begin(), extenssion_list.end());
}

void DownloadMod::RemoveAcceptedExtenssions(vector<string> extenssion_list)
{
	vector<string>::iterator iter_princ, iter_aux;
	for(iter_princ = this->accepted_extenssions.begin(); iter_princ!=this->accepted_extenssions.end(); iter_princ++)
	{
		for(iter_aux = extenssion_list.begin(); iter_aux!=extenssion_list.end(); iter_aux++)
		{
			if(*iter_princ==*iter_aux)
			{
				this->accepted_extenssions.erase(iter_princ);
				iter_princ--;
			}
		}
	}
}

const char* DownloadMod::FormatAcceptedExtenssions()
{
	string exit;
	vector<string>::iterator iter;
	
	exit="";
	for(iter = this->accepted_extenssions.begin(); iter!=this->accepted_extenssions.end(); iter++)
	{
		if(iter!=this->accepted_extenssions.begin())
			exit+=",";
		exit+=*iter;
	}
	return exit.c_str();
}

void DownloadMod::SetDestPath(string path)
{
	this->dest_path=path;
}

bool DownloadMod::StartDownload(string website)
{
	string command;
	unsigned int i;
	ostringstream ss;
	bool exit;

	command="wget -rq ";
	if(this->max_downloaded_size>-1){
		ss<<this->max_downloaded_size;
		command+=" -Q "+ss.str();
	}
	if(this->accepted_extenssions.size()>0){
		command+=" -A "+this->accepted_extenssions[0];
		for(i=1;i<this->accepted_extenssions.size();i++)
			command+=","+this->accepted_extenssions[i];
	}
	command+=" -P "+dest_path+" "+website;
	exit=system(command.c_str());
	
	return exit;
}
