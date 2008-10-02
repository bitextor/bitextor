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
	ostringstream aux_sstream;
	string file_name;
	bool show_howtouse=false;
	string dest_dir;

	if(argc==2){
		show_howtouse=true;
		if(strcmp(argv[1],"-h")!=0 && strcmp(argv[1],"--help")!=0)
			cout<<"The selected option is not correct."<<endl;
	}
	else{
		if(argc<4)
			show_howtouse=true;
		else{
			try{
				if(!GlobalParams::LoadGlobalParams(argv[1]))
					cerr<<"Bitextor can't open the config file. Please, specifie it in the bitextor's configuration file in the section <textCatConfigFile>XXXXX</textCatConfigFile>"<<endl;
				else{
					if(strcmp(argv[2],"-d")==0){
						cout<<"Inicialising Bitextor's destination path..."<<endl;
						dest_dir=argv[3];
						if(dest_dir[dest_dir.length()-1]!='/')
							dest_dir+="/";
						if(!ws.Initialize(dest_dir))
							cerr<<"There were an error while trying to load the files in the selected directory."<<endl;
						else{
							mkdir(((string)argv[3]+"bitexts/").c_str(),0777);
							cout<<"Comparing the files..."<<endl;
							results=ws.GetMatchedFiles();
							for(n_results=0;n_results<results.size();n_results++){
								bitext=results[n_results];
								
								file.open(((string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+".tmx").c_str(),ios::in);
								if(file.is_open()){
									file.close();
									i=1;
									do{
										file.close();
										aux_sstream<<i;
										file.open(((string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+aux_sstream.str()+".tmx").c_str(),ios::in);
										aux_sstream.clear();
										i++;
									}while(file.is_open());
									file_name=(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+aux_sstream.str()+".tmx";
								}
								else
									file_name=(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+".tmx";
								bitext.GenerateBitext(file_name);
								cout<<"\tThe bitext between "<<GetFileName(bitext.GetFirstWebFile().GetPath())<<" and "<<GetFileName(bitext.GetSecondWebFile().GetPath())<<" has been created."<<endl;
								cout<<"\tEdit distance: "<<bitext.GetEditDistance()<<"%  Size difference:"<<bitext.GetSizeDistance()<<"%"<<endl<<endl;
							}
							if(results.size()==0)
								cout<<"No correspondences were found between the files in the specified directory."<<endl;
						}
					}
					else if(strcmp(argv[2],"-w")==0){
						DownloadMod mod;
						cout<<"Downloading from "<<argv[3]<<" (this will take some time)..."<<endl;
						mod.SetDestPath(GlobalParams::GetDownloadPath());
						mod.StartDownload(argv[3]);
		
						cout<<"Inicialising Bitextor's destination path..."<<endl; 
						if(!ws.Initialize(GlobalParams::GetDownloadPath()+(string)argv[3]+"/"))
							cerr<<"There were an error while trying to load the files in the selected directory."<<endl;
						else{
							mkdir((GlobalParams::GetDownloadPath()+(string)argv[3]+"/bitexts/").c_str(),0777);
							cout<<"Comparing the files..."<<endl;
							results=ws.GetMatchedFiles();
							for(n_results=0;n_results<results.size();n_results++){
								bitext=results[n_results];
								file.open((GlobalParams::GetDownloadPath()+(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+".tmx").c_str(),ios::in);
								if(file.is_open()){
									file.close();
									i=1;
									do{
										file.close();
										aux_sstream<<i;
										file.open((GlobalParams::GetDownloadPath()+(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+aux_sstream.str()+".tmx").c_str(),ios::in);
										aux_sstream.clear();
										i++;
									}while(file.is_open());
									file_name=GlobalParams::GetDownloadPath()+(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+aux_sstream.str()+".tmx";
								}
								else
									file_name=GlobalParams::GetDownloadPath()+(string)argv[3]+"/bitexts/"+GetFileName(bitext.GetFirstWebFile().GetPath())+"_"+GetFileName(bitext.GetSecondWebFile().GetPath())+".tmx";
								bitext.GenerateBitext(file_name);
								cout<<"\tThe bitext between "<<GetFileName(bitext.GetFirstWebFile().GetPath())<<" and "<<GetFileName(bitext.GetSecondWebFile().GetPath())<<" has been created."<<endl;
								cout<<"\tEdit distance: "<<bitext.GetEditDistance()<<"%  Size difference:"<<bitext.GetSizeDistance()<<"%"<<endl<<endl;
							}
							if(results.size()==0)
								cout<<"No correspondences were found between the files in the specified directory."<<endl;
						}
					}
				}
			}
			catch(char* e){
				cout<<e<<endl;
			}
		}
	}
	if(show_howtouse)
		cout<<"The correct way to call Bitextor is:"<<endl<<"\tbitextor config_file_path -w url"<<endl<<"\tbitextor config_file_path -d local_directory_with_html_files"<<endl<<endl<<"To see this use instructions, call: bitextor -h"<<endl;
	return 0; 
}
