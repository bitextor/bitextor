#include "WebSite.h"
#include "FilePreprocess.h"
#include "Bitext.h"
#include "GlobalParams.h"
#include "DownloadMod.h"
#include <fstream>
#include <sstream>

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

int
main (int argc, char *const *argv)
{
	WebSite *ws;
	vector<Bitext> results;
	unsigned int n_results;
	Bitext bitext;
	ifstream file;
	unsigned int i;
	string file_name;
	bool show_howtouse=false;
	string dest_dir="";
	string config_file="/usr/local/etc/bitextor/conf/config.xml";
	bool download, any_bitext;
	struct stat my_stat;

	setlocale(LC_CTYPE, "");
	try{
		for(i=1;i<argc;i++){
			if(strcmp(argv[i],"-c")==0){
				config_file=argv[i+1];
				i++;
			}
			else{
				if(strcmp(argv[i],"-d")==0){
					dest_dir=argv[i+1];
					download=false;
					i++;
				}
				else{
					if(strcmp(argv[i],"-w")==0){
						dest_dir=argv[i+1];
						download=true;
						i++;
					}
					else{
						if(strcmp(argv[i],"--set_languages")==0)
							GlobalParams::SetGuessLanguage(false);
						else{
							if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0)
								show_howtouse=true;
						}
					}
				}
			}
		}
		if(dest_dir==""){
			cerr<<"You have to use one of those options: -d or -w."<<endl;
			show_howtouse=true;
		}
	}
	catch(...){
		cerr<<"Invalid calling to bitextor"<<endl;
		show_howtouse=true;
	}

	if(show_howtouse)
		wcout<<L"The correct way to call Bitextor is:"<<endl<<L"\tbitextor [options] -w url"<<endl<<L"\tbitextor [options] -d local_directory_with_html_files"<<endl<<endl<<L"OPTIONS"<<endl<<L"\tTo see this use instructions, use the option -h"<<endl<<L"\tTo especify the path of the configuration file use the option -c path_of_configuration_file"<<endl<<L"\tTo set manually the language of every file, use the option --set_languages"<<endl;
	else{
		try{
			if(!GlobalParams::LoadGlobalParams(config_file))
				wcerr<<L"Bitextor can't open the config file. Please, specifie it with the option -c or place it at /usr/local/etc/bitextor/conf/"<<endl;
			else{
				if(download){
					DownloadMod mod;
					mod.SetDestPath(GlobalParams::GetDownloadPath());
					mod.StartDownload(Config::toWstring(dest_dir));
				}
				wcout<<L"Initializing Bitextor's destination path..."<<endl;
				if(dest_dir[dest_dir.length()-1]!='/')
					dest_dir+="/";
				
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
			}
		}
		catch(char* e){
			cout<<e<<endl;
		}
	}
	return 0; 
}
