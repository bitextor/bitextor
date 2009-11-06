/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "WebSite.h"
#include "FilePreprocess.h"
#include "GlobalParams.h"
#include "DownloadMod.h"
#include "bitextor_config.h"
#include <getopt.h>
#include <sys/stat.h>
#include <tre/regex.h>

using namespace std;

string
GetFilePath(string path)
{
	string exit;
	size_t found;

	found=path.find_last_of("/\\");
	exit=path.substr(0,found+1);

	return exit;
}

string
CorrectURLPathName(string path)
{
	string exit="";
	
	if(path.length()>6 && path.substr(0,7)=="http://")
		exit=path.substr(7);
	else
		exit=path;
	return exit;
}

int
main (int argc, char *const *argv)
{
	WebSite *ws;
	//Bitext bitext;
	ifstream file;
	string file_name;
	bool show_howtouse=false, mode_set=false, verbose=false, download=false;
	string dest_dir="";
	string config_file=BASE_CONF;
	struct stat my_stat;
	int next_op;
	unsigned short option; //Indicates if the user has introduced an option
	extern char *optarg;

	setlocale(LC_CTYPE, "");

	//Short options
	const char* const short_op = "d:w:hc:sl:vp:";

	//Set of long options
	const struct option long_op[] =
	{
		{ "directory", 1, NULL, 'd'}, //Execution in direct mode
		{ "web-site", 1, NULL, 'w'}, //Execution in 2-steps mode with text length comparison
		{ "set-languages", 0, NULL, 's'}, //Execution in 2-setps mode with alignment distance text comparison
		{ "help", 0, NULL, 'h'},
		{ "config-file", 1, NULL, 'c'},
		{ "paired-files", 1, NULL, 'p'},
		{ "log-file", 1, NULL, 'l'},
		{ "verbose", 0, NULL, 'v'},
		{ NULL, 0, NULL, 0}
	};

	next_op = getopt_long (argc, argv, short_op, long_op, NULL);
	for(option=0; next_op!=-1 && !show_howtouse; option++){
		switch(next_op){
			case 'd':
				if(mode_set){
					wcout<<"You only can select one option [-d|-w] to set the target of bitextor."<<endl;
					show_howtouse=true;
				}
				else{
					dest_dir=optarg;
					download=false;
					mode_set=true;
					if(dest_dir==""){
						wcout<<"You must define the directory from wich bitext must be extracted."<<endl;
						show_howtouse=true;
					}
				}
			break;
			case 'w':
				if(mode_set){
					wcout<<"You only can select one option [-d|-w] to set the target of bitextor."<<endl;
					show_howtouse=true;
				}
				else{
					dest_dir=optarg;
					download=true;
					mode_set=true;
					if(dest_dir==""){
						wcout<<"You must define the URL from wich bitext must be extracted."<<endl;
						show_howtouse=true;
					}
				}
			break;
			case 's':
				GlobalParams::SetGuessLanguage(false);
			break;
			case 'l':
				GlobalParams::OpenLog(optarg);
			break;
			case 'p':
				GlobalParams::OpenResults(optarg);
				GlobalParams::GenerateTMX(false);
			break;
			case 'h': show_howtouse=true; break;
			case 'c': config_file=optarg; break;
			case 'v': verbose=true; break;
			default:
				wcout<<"Unknown option "<<next_op<<"."<<endl;
				show_howtouse=true;
			break;
		}
		next_op = getopt_long (argc, argv, short_op, long_op, NULL);
	}

	if(show_howtouse || !mode_set)
		wcout<<L"The correct way to call Bitextor is:"<<endl<<L"\tbitextor [options] -w url"<<endl<<L"\tbitextor [options] -d local_directory_with_html_files"<<endl<<endl<<L"OPTIONS"<<endl<<L"\tTo see this use instructions, use the option -h"<<endl<<L"\tTo especify the path of the configuration file use the option -c path_of_configuration_file"<<endl<<L"\tTo set manually the language of every file, use the option --set_languages"<<endl<<L"\tTo create a log file with all the information of the process, use the option -l path_of_log_file"<<endl<<L"To see all the bitexts generated, use the option -v"<<endl;
	else{
		try{
			if(!GlobalParams::LoadGlobalParams(config_file))
				wcerr<<L"Bitextor can not open the config file. Please, specifie it with the option -c or place it at /usr/local/etc/bitextor/conf/"<<endl;
			else{
				if(verbose)
					GlobalParams::SetVerbose();
				if(download){
					wcout<<L"Downloading the website..."<<endl;
					DownloadMod mod;
					mod.SetDestPath(GlobalParams::GetDownloadPath());
					mod.StartDownload(Config::toWstring(dest_dir));
					dest_dir=Config::toString(GlobalParams::GetDownloadPath())+CorrectURLPathName(dest_dir);
				}
				wcout<<L"Initializing Bitextor's destination path..."<<endl;
				if(dest_dir[dest_dir.length()-1]!='/')
					dest_dir+="/";
				if(stat(dest_dir.c_str(), &my_stat) != 0)
					wcerr<<L"The specified directory doesn't exits."<<endl;
				else{
					if(stat((dest_dir+"bitexts/").c_str(), &my_stat) != 0)
						mkdir((dest_dir+"/bitexts/").c_str(),0777);
					wcout<<L"Comparing files and generating bitexts..."<<endl;
					ws=new WebSite(dest_dir);
					try{
						if(!ws->GenerateBitexts(dest_dir+"bitexts/"))
							wcout<<L"No correspondences were found between the files in the specified directory."<<endl;
					}
					catch(char const*e){
						cout<<e<<endl;
					}
					delete ws;
					Config::CleanUpConfiguration();
				}
			}
		}
		catch(char* e){
			cout<<e<<endl;
		}
	}
	GlobalParams::CloseLog();
	GlobalParams::CloseResults();

	return 0; 
}
