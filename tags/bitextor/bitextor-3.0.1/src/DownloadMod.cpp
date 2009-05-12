/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "DownloadMod.h"
#include "GlobalParams.h"
#include <sstream>
#include <string>
#include <libtagaligner/ConfigReader.h>

DownloadMod::DownloadMod()
{
	this->max_downloaded_size=GlobalParams::GetMaxDownloadedSize();
}

DownloadMod::~DownloadMod()
{
}

void DownloadMod::SetMaxDownloadedSize(const long &mds)
{
	this->max_downloaded_size=mds;
}

void DownloadMod::SetDestPath(const wstring &path)
{
	this->dest_path=path;
}

bool DownloadMod::StartDownload(const wstring &website)
{
	string command;
	unsigned int i;
	ostringstream ss;
	bool exit;

	command="httrack --skeleton ";
	if(this->max_downloaded_size>-1){
		ss<<this->max_downloaded_size;
		command+=" -M"+ss.str();
	}
	command+=" -Q -q -%i0 -I0 -O "+Config::toString(dest_path)+" "+Config::toString(website)+" > /dev/null";
	exit=system(command.c_str());
	
	return exit;
}
