/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "collector/collector.h"

#define kNumPossibleOptions 5
#define kDefaultConfFile "bitextor.cfg"

#define kDefaultLangFile "lgtrained"

void OutOfMemory()
 {
  fprintf(stdout,"Memory exhausted. Program terminated.\n");
  exit(1);
 }

int main(int argc,char *argv[])
 {  
  Collector *collector;
  bool showuse;
  bool paramactivator[kNumPossibleOptions];
  unsigned short urlpatternsfile=0; //Fichero con los patrones de generacion de urls
  unsigned short configurationfile=0; //Fichero con la configuracion del parallelcollector
  unsigned short resultsfile=0; //Fichero de resultados de una ejecucion previa del parallelcollector
  char configfile[kSizeAuxStrings]; //Path del fichero de configuracion
  int i;
  
  std::set_new_handler(OutOfMemory);
  
  //Obtenemos la informacion de los parametros
  if(argc>1 && argc<10)
   {
    showuse=false;
    for(i=0;i<kNumPossibleOptions;i++)
      paramactivator[i]=false;
    
    for(i=1;i<argc;i++)
     {
      if(strcmp(argv[i],"-q")==0)
       {
        if(!paramactivator[0] && i<argc-1)
          paramactivator[0]=true;
         else
           showuse=true;
       }
       else
        {
         if(strcmp(argv[i],"-g1")==0)
          {
           if(!paramactivator[1] && !paramactivator[2] && i<argc-2)
            {
             paramactivator[1]=true;
             urlpatternsfile=i+1;
             i++;
            }
            else
              showuse=true;
          }
          else
           {
            if(strcmp(argv[i],"-g2")==0)
             {
              if(!paramactivator[2] && !paramactivator[1] && i<argc-2)
               {
                paramactivator[2]=true;
                urlpatternsfile=i+1;
                i++;
               }
               else
                {
                 showuse=true;
                }
             }
             else
              {
               if(strcmp(argv[i],"-c")==0)
                {
                 if(!paramactivator[3] && i<argc-2)
                  {
                   paramactivator[3]=true;
                   configurationfile=i+1;
                   i++;
                  }
                  else
                    showuse=true;
                }
                else
                 {
                  if(strcmp(argv[i],"-r")==0)
                   {
                    if(!paramactivator[4] && i<argc-1)
                     {
                      paramactivator[4]=true;
                      resultsfile=i+1;
                      i++;
                     }
                     else
                       showuse=true;
                   }
                 }
              }
           }
        }
     }
   }
   else
     showuse=true;
  
  if(!showuse)
   {
    collector=new Collector();
    if(paramactivator[3])
      sprintf(configfile,"%s",argv[configurationfile]);
     else
       sprintf(configfile,"%s/%s",CONF_FILE,kDefaultConfFile);

    if(collector->LoadConfiguration(configfile))
     {
      if(!paramactivator[0])
        collector->QuickMode(true); //Turn on quick mode (only stores similars in one of the files)
        
      if(paramactivator[1] || paramactivator[2])
       {
        if(collector->CompareOnlyURLPatterns(argv[urlpatternsfile]))
         {
          if(paramactivator[1])
            collector->GenerationType1();
           else
             collector->GenerationType2();
         }
         else
          {
           fprintf(stderr,"Error: URL patterns file \'%s\' could not be loaded\n",argv[urlpatternsfile]);
           showuse=true;
          }
       }
      if(!showuse)
       {
        if(paramactivator[4])
         {
          if(!collector->RefreshWebsite(argv[resultsfile]))
            fprintf(stderr,"Error: Website was not updated\n");
         }
         else
          {
           if(collector->Load(argv[argc-1]))
             collector->Start();
            else
             {
              if(collector->LoadURL(argv[argc-1]))
                collector->Start();
               else
                {
                 fprintf(stderr,"Error: URL file \'%s\' could not be loaded\n",argv[argc-1]);
                }
             }
          }
       }
     }
     else
       fprintf(stderr,"Error: configuration file \'%s\' could not be loaded\n",kDefaultConfFile);
    delete collector;
   }
  
  if(showuse)
   {
    fprintf(stdout,"Usage:\t%s [options] <url_file>|<website>\t\t(Generate collection)\n",argv[0]);
    fprintf(stdout,"\t\t%s [options] -r <resultsfile>\t\t(Update collection)\n",argv[0]);
    fprintf(stdout,"Options:\n");
    fprintf(stdout,"\t-dq\t\t\tDeactivate quick mode (that only generates one-way similars)\n");
    fprintf(stdout,"\t-c <configuration_file>\tLoading a different configuration file\n");
    fprintf(stdout,"\t-g1 <pattern_file>\tURL pattern generation type 1 (all changes each pattern)\n");
    fprintf(stdout,"\t-g2 <pattern_file>\tURL pattern generation type 2 (one change each pattern)\n");
    fprintf(stdout,"\t-r <website_directory>\tResume from a previous execution (to update a collection)\n");
   }
  return(0);
 }
