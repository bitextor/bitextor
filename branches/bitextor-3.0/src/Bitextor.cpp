#include "WebSite.h"
#include "FilePreprocess.h"
#include "Bitext.h"
#include "GlobalParams.h"
#include "DownloadMod.h"
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

string
GetFileName(string path)
{
	string exit;
	size_t found;

	found=path.find_last_of("/\\");
	exit=path.substr(found+1);

	return exit;
}

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
	WebSite ws;
	vector<Bitext> results;
	unsigned int n_results;
	Bitext bitext;
	ifstream file;
	unsigned int i;
	ostringstream *aux_sstream;
	string file_name;
	bool show_howtouse=false;
	string dest_dir="";
	string config_file="/usr/local/etc/bitextor/conf/bitextor_config.xml";
	bool download;
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
				if(!download){
					wcout<<L"Inicialising Bitextor's destination path..."<<endl;
					if(dest_dir[dest_dir.length()-1]!='/')
						dest_dir+="/";
					if(!ws.Initialize(dest_dir))
						wcerr<<L"There were an error while trying to load the files in the selected directory."<<endl;
					else{
						if(stat((dest_dir+"bitexts/").c_str(), &my_stat) != 0)
							mkdir((dest_dir+"bitexts/").c_str(),0777);
						wcout<<L"Comparing the files..."<<endl;
						results=ws.GetMatchedFiles();
						for(n_results=0;n_results<results.size();n_results++){
							bitext=results[n_results];
							file_name=dest_dir+"/bitexts/"+GetFileName(bitext.GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext.GetSecondWebFile()->GetPath())+".tmx";
							for(i=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;i++){
								delete aux_sstream;
								aux_sstream=new ostringstream(ios_base::out);
								*aux_sstream<<i;
								file_name=dest_dir+"/bitexts/"+GetFileName(bitext.GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext.GetSecondWebFile()->GetPath())+aux_sstream->str()+".tmx";
							}
							delete aux_sstream;
							bitext.GenerateBitext(file_name);
							wcout<<L"\tThe bitext between "<<Config::toWstring(bitext.GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(bitext.GetSecondWebFile()->GetPath())<<L" has been created."<<endl;
							wcout<<L"\tEdit distance: "<<bitext.GetEditDistance()<<L"%  Size difference:"<<bitext.GetSizeDistance()<<L"%"<<endl<<endl;
						}
						if(results.size()==0)
							wcout<<L"No correspondences were found between the files in the specified directory."<<endl;
					}
				}
				else{
					DownloadMod mod;
					wcout<<L"Downloading from "<<Config::toWstring(dest_dir)<<L" (this will take some time)..."<<endl;
					mod.SetDestPath(GlobalParams::GetDownloadPath());
					mod.StartDownload(Config::toWstring(dest_dir));
	
					wcout<<L"Inicialising Bitextor's destination path..."<<endl; 
					if(!ws.Initialize(Config::toString(GlobalParams::GetDownloadPath())+dest_dir+"/"))
						wcerr<<L"There were an error while trying to load the files in the selected directory."<<endl;
					else{
						if(stat((dest_dir+"bitexts/").c_str(), &my_stat) != 0)
							mkdir((Config::toString(GlobalParams::GetDownloadPath())+dest_dir+"/bitexts/").c_str(),0777);
						wcout<<L"Comparing the files..."<<endl;
						results=ws.GetMatchedFiles();
						for(n_results=0;n_results<results.size();n_results++){
							bitext=results[n_results];
							file_name=Config::toString(GlobalParams::GetDownloadPath())+dest_dir+"/bitexts/"+GetFileName(bitext.GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext.GetSecondWebFile()->GetPath())+".tmx";
							for(i=0, aux_sstream=new ostringstream(ios_base::out);stat(file_name.c_str(), &my_stat) == 0;i++){
								delete aux_sstream;
								aux_sstream=new ostringstream(ios_base::out);
								*aux_sstream<<i;
								file_name=Config::toString(GlobalParams::GetDownloadPath())+dest_dir+"/bitexts/"+GetFileName(bitext.GetFirstWebFile()->GetPath())+"_"+GetFileName(bitext.GetSecondWebFile()->GetPath())+aux_sstream->str()+".tmx";
							}
							bitext.GenerateBitext(file_name);
							wcout<<L"\tThe bitext between "<<Config::toWstring(bitext.GetFirstWebFile()->GetPath())<<L" and "<<Config::toWstring(bitext.GetSecondWebFile()->GetPath())<<L" has been created."<<endl;
							wcout<<L"\tEdit distance: "<<bitext.GetEditDistance()<<L"%  Size difference:"<<bitext.GetSizeDistance()<<L"%"<<endl<<endl;
						}
						if(results.size()==0)
							wcout<<L"No correspondences were found between the files in the specified directory."<<endl;
					}
				}
			}
		}
		catch(char* e){
			cout<<e<<endl;
		}
	}
	return 0; 
}
