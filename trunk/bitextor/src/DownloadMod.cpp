#include "DownloadMod.h"

DownloadMod::DownloadMod()
{
	this->max_downloaded_size=GlobalParams::GetMaxDownloadedSize();
}

DownloadMod::~DownloadMod()
{
}

void DownloadMod::SetMaxDownloadedSize(long mds)
{
	this->max_downloaded_size=mds;
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

	command="httrack --skeleton ";
	if(this->max_downloaded_size>-1){
		ss<<this->max_downloaded_size;
		command+=" -M"+ss.str();
	}
	command+=" -Q -q -%i0 -I0 -O "+dest_path+" "+website+" > /dev/null";
	exit=system(command.c_str());
	
	return exit;
}
