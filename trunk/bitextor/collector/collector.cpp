/**
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#include "collector.h"

Collector::Collector()
 {
  unsigned int i;  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Collector()\n");
#endif
  
  quickmode=false;
  urlhash=new URLHash();
  orderedlist=new OrderedList2Levels();
  trigramguesser=NULL;
  wordguesser=NULL;
  urlguesser=NULL;
  for(i=0;i<kNumberofGuessers;i++)
    applyguessers[i]=false;
  langid=new unsigned short[kNumLanguages];

  n_files=NULL;
  filelisturl=NULL;
  languagepages=NULL;  
  n_sizelanguagepages=NULL;
  n_languagepages=NULL;
  langpagesperlevel=NULL;
  n_sizelangpagesperlevel=NULL;
  n_langpagesperlevel=NULL;

  urllist=NULL;
  internalurllist=NULL;
  langurl=NULL;
  n_urllist=0;
  n_urlactual=0;
  possiblelanguages=NULL;
  urlguesserstart=0;
  
  //Se inician las estadisticas del proceso
  downloaded=0;
  discardedlang=0;
  discardedsize=0;
  diffbysize=0;
  diffbytags=0;
  diffbytagslevel=0;
  diffbytreelevel=0;
  diffbylinks=0;
  diffbyimages=0;
  diffbyext=0;
  n_similars=0;
  n_repeated=0;
  totalurlgenerated=0;
  totalgoodgenerations=0;
  
  //Se inicializan los valores de las heuristicas
  applysize=true;
  applyntags=true;
  applynlinks=true;
  applydepth=true;
  applynimages=true;
  applyntagslevel=true;
  applyextension=true;
  nappliedheuristics=7;
  weightsize=1;
  weightntags=1;
  weightnlinks=1;
  weightdepth=1;
  weightnimages=1;
  weightntagslevel=1;
  
  minimalcontentsize=kMinimalContentLength;
  rangesize=kRangeSize;
  absrangesize=kAbsRangeSize;
  normalizedsize=NULL;
  absrangetreelevel=kAbsRangeTreeLevel;
  rangenlinks=kRangeLinks;
  absrangenlinks=kAbsRangeLinks;
  rangenimages=kRangeImages;
  absrangenimages=kAbsRangeImages;
  rangentags=kRangeNTags;
  absrangentags=kAbsRangeNtags;
  
  for(i=0;i<kMaxLevelTags;i++)
   {
    rangentagslevel[i]=kRangeNTagsLevel;
    absrangentagslevel[i]=kAbsRangeNTagsLevel;
   }
  
  urlgenerator=NULL;
  generationtype=1;
  
  //Variables de la actualizacion progresiva
  updatingcollection=false;
  oldn_languagepages=NULL;
  statsfilename=NULL;
  statsfilename2=NULL;
  processedpreviously=0;
  olddiffbysize=0;
  olddiffbytags=0;
  olddiffbytagslevel=0;
  olddiffbytreelevel=0;
  olddiffbylinks=0;
  olddiffbyimages=0;
  olddiffbyext=0;
  oldn_similars=0;
  previousdate[0]='\0';
  newdate[0]='\0';
  oldurlhash=NULL;
  oldorderedlist=NULL;
  oldn_langpagesperlevel=NULL;
  
  extrastat=0;
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Collector()\n");
#endif
 }

Collector::~Collector()
 {
  unsigned int i,j;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::~Collector()\n");
#endif

  if(langid!=NULL)
   {
    delete langid;
    langid=NULL;
   }
  if(filelisturl!=NULL)
   {
    delete filelisturl;
    filelisturl=NULL;
   }
  if(urlhash!=NULL)
   {
    delete urlhash;
    urlhash=NULL;
   }
  if(orderedlist!=NULL)
   {
    delete orderedlist;
    orderedlist=NULL;
   }
  if(trigramguesser!=NULL)
   {
    delete trigramguesser;
    trigramguesser=NULL;
   }
  if(wordguesser!=NULL)
   {
    delete wordguesser;
    wordguesser=NULL;
   }
  if(urlguesser!=NULL)
   {
    delete urlguesser;
    urlguesser=NULL;
   }
  if(n_files!=NULL)
   {
    delete n_files;
    n_files=NULL;
   }
  if(urllist!=NULL)
   {
    for(i=0;i<n_urllist;i++)
     {
      if(urllist[i]!=NULL)
        delete urllist[i];
     }
    delete urllist;
    urllist=NULL;
   }
  if(internalurllist!=NULL)
   {
    for(i=0;i<n_urllist;i++)
     {
      if(internalurllist[i]!=NULL)
        delete internalurllist[i];
     }
    delete internalurllist;
    internalurllist=NULL;
   }
  if(langurl!=NULL)
   {
    for(i=0;i<n_urllist;i++)
     {
      if(langurl[i]!=NULL)
        delete langurl[i];
     }
    delete langurl;
    langurl=NULL;
   }
  if(languagepages!=NULL)
   {
    for(i=0;i<n_languages;i++)
     {
      if(languagepages[i]!=NULL)
        delete languagepages[i];
     }
    delete languagepages;
    languagepages=NULL;
   }
  if(n_sizelanguagepages!=NULL)
   {
    delete n_sizelanguagepages;
    n_sizelanguagepages=NULL;
   }
  if(n_languagepages!=NULL)
   {
    delete n_languagepages;
    n_languagepages=NULL;
   }
  if(langpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
     {
      for(j=0;j<kMaxDeepTree;j++)
       {
        if(langpagesperlevel[i][j]!=NULL)
          delete langpagesperlevel[i][j];
       }       
      delete langpagesperlevel[i];
     }
    delete langpagesperlevel;
    langpagesperlevel=NULL;
   }
  if(normalizedsize!=NULL)
   {
    delete normalizedsize;
    normalizedsize=NULL;
   }  
  if(n_sizelangpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
      delete n_sizelangpagesperlevel[i];
    delete n_sizelangpagesperlevel;
    n_sizelangpagesperlevel=NULL;
   }
  if(n_langpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
      delete n_langpagesperlevel[i];
    delete n_langpagesperlevel;
    n_langpagesperlevel=NULL;
   }  
  if(possiblelanguages!=NULL)
   {
    delete possiblelanguages;
    possiblelanguages=NULL;
   }
  if(urlgenerator!=NULL)
   {
    delete urlgenerator;
    urlgenerator=NULL;
   }
  
  if(oldn_languagepages!=NULL)
   {
    delete oldn_languagepages;
    oldn_languagepages=NULL;
   }
  if(oldn_langpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
      delete oldn_langpagesperlevel[i];
    delete oldn_langpagesperlevel;
    oldn_langpagesperlevel=NULL;
   }  
  if(statsfilename!=NULL)
   {
    delete statsfilename;
    statsfilename=NULL;
   }
  if(statsfilename2!=NULL)
   {
    delete statsfilename2;
    statsfilename2=NULL;
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::~Collector()\n");
#endif   
 }

bool Collector::Load(const char *file)
 {
  bool result;
  int i,j,k;
  FILE *fent;
  char buffer[kSizeAuxStrings]; //Buffer para lectura de ficheros
  char buflang[kSizeAuxStrings]; //Buffer para el reconocimiento de idiomas
  char **auxlist;
  bool found; //Indica si se han encontrado idiomas en la cadena
  bool langfound; //Indica si se ha encontrado un idioma o si hay que seguir buscando

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Load(%s)\n",file);
#endif

  result=true;
  if(!(fent=fopen(file,"r")))
    result=false;
   else
    {
     if(urllist!=NULL)
      {//Borramos la anterior lista de urls
       for(i=0;i<(signed)n_urllist;i++)
        {
         if(urllist[i]!=NULL)
           delete urllist[i];
        }
       delete urllist;
       urllist=NULL;
      }
     if(langurl!=NULL)
      {//Borramos la anterior lista de idiomas
       for(i=0;i<(signed)n_languages;i++)
        {
         if(langurl[i]!=NULL)
           delete langurl[i];
        }
       delete langurl;
       langurl=NULL;
      }
     n_languages=0;
     

     //Leemos la primera linea
     for(i=0,buffer[i]=fgetc(fent);i<kSizeAuxStrings && buffer[i]!='\n' && !feof(fent);i++,buffer[i]=fgetc(fent));
     
     if(feof(fent) || i==kSizeAuxStrings)
      {
       fprintf(stderr,"Error: The url file is not valid\n");
       result=false;
      }
      else
       {//Procesamos la cadena de idiomas
        buffer[i]='\0';
        i=0;
        while(i<(signed)strlen(buffer))
         {
          //Quitamos los espacios
          for(;i<(signed)strlen(buffer) && buffer[i]==' ' && buffer[i]=='\t';i++);
        
          //Cogemos el texto
          for(j=0;i<(signed)strlen(buffer) && buffer[i]!=' ' &&
                  buffer[i]!='\t' && buffer[i]!='\n';i++,j++)
          buflang[j]=buffer[i];
          buflang[j]='\0';
          i++;
          
          for(j=0,langfound=false;j<kNumLanguages && !langfound;j++)
           {
            if(strcmp(buflang,LanguageName(j))==0)
             {//Hemos encontrado el idioma
              langfound=true;
              langid[j]=n_languages;
              auxlist=new char*[n_languages+1];
              for(k=0;k<(signed)n_languages;k++)
                auxlist[k]=langurl[k];
              auxlist[k]=new char[1+strlen(buflang)];
              strcpy(auxlist[k],buflang);
              delete langurl;
              langurl=auxlist;
              n_languages++;
             }
           }
         }
        if(n_languages<2)//Al menos debe haber 2 idiomas distintos
         {
          fprintf(stderr,"Error: The url file is not valid (less than 2 languages specified)\n");
          result=false;
         }
         else
          {
           //Leemos la lista de urls           
           while(!feof(fent))
            {
             buffer[0]=fgetc(fent);
             for(i=1;i<kSizeAuxStrings && (i==0 || buffer[i-1]!='\n') && !feof(fent);i++)
              {
               buffer[i]=fgetc(fent);
               if(buffer[i]=='\n')
                {
                 found=true;
                 buffer[i]='\0';
              
                 //Insertamos la cadena
                 auxlist=new char*[n_urllist+1];
                 for(i=0;i<(signed)n_urllist;i++)
                   auxlist[i]=urllist[i];
                 auxlist[i]=new char[1+strlen(buffer)];
                 if(strlen(buffer)>7 && buffer[0]=='h' && buffer[1]=='t' &&
                    buffer[2]=='t' && buffer[3]=='p' && buffer[4]==':' &&
                    buffer[5]=='/' && buffer[6]=='/')
                  {//Quitamos el http:// inicial si existe
                   strcpy(auxlist[i],buffer+7);
                  }
                  else
                    strcpy(auxlist[i],buffer);
                 delete urllist;
                 urllist=auxlist;
                 n_urllist++;

                 i=-1;//Seguimos leyendo la entrada en busca de idiomas
                }
              }
            }
            
           if(feof(fent) && i>1)//Comprobamos la ultima linea
            {
             buffer[i-1]='\0';
             auxlist=new char*[n_urllist+1];
             for(i=0;i<(signed)n_urllist;i++)
               auxlist[i]=urllist[i];
             auxlist[i]=new char[1+strlen(buffer)];
             if(strlen(buffer)>7 && buffer[0]=='h' && buffer[1]=='t' &&
                buffer[2]=='t' && buffer[3]=='p' && buffer[4]==':' &&
                buffer[5]=='/' && buffer[6]=='/')
              {//Quitamos el http:// inicial si existe
               strcpy(auxlist[i],buffer+7);
              }
              else
                strcpy(auxlist[i],buffer);
             delete urllist;
             urllist=auxlist;
                      
             n_urllist++;
            }
          }
       }
    }
  if(result)
   {//Liberamos la memoria del objeto en caso de que se hubiera cargado previamente
    if(n_files!=NULL)
     {
      delete n_files;
      n_files=NULL;
     }
    if(filelisturl!=NULL)
     {
      delete filelisturl;
      filelisturl=NULL;
     }
    if(languagepages!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
       {
        if(languagepages[i]!=NULL)
          delete languagepages[i];
       }
      delete languagepages;
      languagepages=NULL;
     }
    if(n_sizelanguagepages!=NULL)
     {
      delete n_sizelanguagepages;
      n_sizelanguagepages=NULL;
     }
    if(n_languagepages!=NULL)
     {
      delete n_languagepages;
      n_languagepages=NULL;
     }
    if(langpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
       {
        for(j=0;j<kMaxDeepTree;j++)
         {
          if(langpagesperlevel[i][j]!=NULL)
            delete langpagesperlevel[i][j];
         }       
        delete langpagesperlevel[i];
       }
      delete langpagesperlevel;
      langpagesperlevel=NULL;
     }
    if(n_sizelangpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
        delete n_sizelangpagesperlevel[i];
      delete n_sizelangpagesperlevel;
      n_sizelangpagesperlevel=NULL;
     }
    if(n_langpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
        delete n_langpagesperlevel[i];
     delete n_langpagesperlevel;
      n_langpagesperlevel=NULL;
     }  
    //Reservamos la memoria para la recoleccion
    n_files=new unsigned int[n_languages];
    filelisturl=new FILE*[n_languages];
    languagepages=new int*[n_languages];  
    n_sizelanguagepages=new int[n_languages];
    n_languagepages=new int[n_languages];
    langpagesperlevel=new int**[n_languages];
    n_sizelangpagesperlevel=new int*[n_languages];
    n_langpagesperlevel=new int*[n_languages];
    for(i=0;i<(signed)n_languages;i++)
     {
      n_files[i]=0;
      languagepages[i]=new int[kNumPagesperLanguage];
      n_sizelanguagepages[i]=kNumPagesperLanguage;
      n_languagepages[i]=0;
      langpagesperlevel[i]=new int*[kMaxDeepTree];
      n_langpagesperlevel[i]=new int[kMaxDeepTree];
      n_sizelangpagesperlevel[i]=new int[kMaxDeepTree];
      for(j=0;j<kMaxDeepTree;j++)
       {
        langpagesperlevel[i][j]=NULL;
        n_langpagesperlevel[i][j]=0;
        n_sizelangpagesperlevel[i][j]=0;
       }
     }
   
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Load(%s) -> %d\n",file,result);
#endif
  
  return(result);
 }

bool Collector::LoadURL(const char *url)
 {
  bool result;
  unsigned int i,j;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::LoadURL(%s)\n",url);
#endif
    
  result=true;
  if(urllist!=NULL)
   {//Borramos la anterior lista de urls
    for(i=0;i<n_urllist;i++)
     {
      if(urllist[i]!=NULL)
        delete urllist[i];
     }
    delete urllist;
   }
  if(langurl!=NULL)
   {//Borramos la anterior lista de urls
    for(i=0;i<n_urllist;i++)
     {
      if(langurl[i]!=NULL)
        delete langurl[i];
     }
    delete urllist;
   }

  n_languages=kNumLanguages;
  langurl=new char*[n_languages];
  for(i=0;i<n_languages;i++)
   {
    langurl[i]=new char[1+strlen(LanguageName(i))];
    strcpy(langurl[i],LanguageName(i));
    
   }
  n_urllist=1;
  urllist=new char*[n_urllist];
  urllist[0]=new char[2+strlen(url)];
  
  if(strlen(url)>7 && url[0]=='h' && url[1]=='t' &&
     url[2]=='t' && url[3]=='p' && url[4]==':' &&
     url[5]=='/' && url[6]=='/')
   {//Quitamos el http:// inicial si existe
    strcpy(urllist[0],url+7);
   }
   else
     strcpy(urllist[0],url);
  
  if(urllist[0][strlen(urllist[0]-1)]!='/')
    strcat(urllist[0],"/");
  //Liberamos la memoria del objeto en caso de que se hubiera cargado previamente
  if(n_files!=NULL)
   {
    delete n_files;
    n_files=NULL;
   }
  if(filelisturl!=NULL)
   {
    delete filelisturl;
    filelisturl=NULL;
   }
  if(languagepages!=NULL)
   {
    for(i=0;i<n_languages;i++)
     {
      if(languagepages[i]!=NULL)
        delete languagepages[i];
     }
    delete languagepages;
    languagepages=NULL;
   }
  if(n_sizelanguagepages!=NULL)
   {
    delete n_sizelanguagepages;
    n_sizelanguagepages=NULL;
   }
  if(n_languagepages!=NULL)
   {
    delete n_languagepages;
    n_languagepages=NULL;
   }
  if(langpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
     {
      for(j=0;j<kMaxDeepTree;j++)
       {
        if(langpagesperlevel[i][j]!=NULL)
          delete langpagesperlevel[i][j];
       }       
      delete langpagesperlevel[i];
     }
    delete langpagesperlevel;
    langpagesperlevel=NULL;
   }
  if(n_sizelangpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
      delete n_sizelangpagesperlevel[i];
    delete n_sizelangpagesperlevel;
    n_sizelangpagesperlevel=NULL;
   }
  if(n_langpagesperlevel!=NULL)
   {
    for(i=0;i<n_languages;i++)
      delete n_langpagesperlevel[i];
   delete n_langpagesperlevel;
    n_langpagesperlevel=NULL;
   }  
  //Reservamos la memoria para la recoleccion
  n_files=new unsigned int[n_languages];
  filelisturl=new FILE*[n_languages];
  languagepages=new int*[n_languages];  
  n_sizelanguagepages=new int[n_languages];
  n_languagepages=new int[n_languages];
  langpagesperlevel=new int**[n_languages];
  n_sizelangpagesperlevel=new int*[n_languages];
  n_langpagesperlevel=new int*[n_languages];
  for(i=0;i<n_languages;i++)
   {
    n_files[i]=0;
    languagepages[i]=new int[kNumPagesperLanguage];
    n_sizelanguagepages[i]=kNumPagesperLanguage;
    n_languagepages[i]=0;
    langpagesperlevel[i]=new int*[kMaxDeepTree];
    n_langpagesperlevel[i]=new int[kMaxDeepTree];
    n_sizelangpagesperlevel[i]=new int[kMaxDeepTree];
    for(j=0;j<kMaxDeepTree;j++)
     {
      langpagesperlevel[i][j]=NULL;
      n_langpagesperlevel[i][j]=0;
      n_sizelangpagesperlevel[i][j]=0;
     }
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::LoadURL(%s) -> %d\n",url,result);
#endif
  
  return(result);
 }
 

short Collector::GuessLanguage(WebPage *webpage)
 {
  short result;
  const float *guesserpoints; //Puntos que ha asignado un guesser a cada idioma
  float *totalpoints;//Total de puntos que obtiene cada idioma
  float abstotalpoints; //Total de puntos obtenidos por todos los idiomas
  float bestresult,secondbest; //Resultado del idioma mas probable y del segundo mas probable
  unsigned int i,j;
  char *textofwebpage;//Contenido en texto de la pagina web

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::GuessLanguage(%p)\n",webpage->Content());//El texto sera muy largo para mostrarlo
#endif

  if(trigramguesser!=NULL || wordguesser!=NULL || urlguesser!=NULL)
   {
    totalpoints=new float[n_languages];
    for(j=0;j<n_languages;j++)
      totalpoints[j]=0;
      
    textofwebpage=webpage->FilterContent();
    for(i=0;i<kNumberofGuessers;i++)
     {
      if(applyguessers[i])
       {
        switch(i)
         {
          case kIdTrigramGuesser: result=trigramguesser->GuessText(textofwebpage);
                                  guesserpoints=trigramguesser->PointsPerLanguage();
                                  if(guesserpoints!=NULL)
                                   {
                                    webpage->AddNewGuesser("TrigramGuesser",n_possiblelanguages,possiblelanguages,guesserpoints);
                                    for(j=0;j<n_languages;j++)
                                      totalpoints[j]=totalpoints[j]+guesserpoints[possiblelanguages[j]]*guesserweight[i];
                                    guesserpoints=NULL;
                                   }                                
                                  break;
          
          case kIdWordGuesser: result=wordguesser->GuessText(textofwebpage);
                               guesserpoints=wordguesser->PointsPerLanguage();
                               if(guesserpoints!=NULL)
                                {
                                 webpage->AddNewGuesser("WordGuesser",n_possiblelanguages,possiblelanguages,guesserpoints);
                                 for(j=0;j<n_languages;j++)
                                   totalpoints[j]=totalpoints[j]+guesserpoints[possiblelanguages[j]]*guesserweight[i];
                                 guesserpoints=NULL;
                                }                                
                               break;
                                  
          case kIdURLGuesser: if(urlguesserstart<downloaded)
                               {
                                result=urlguesser->GuessText(textofwebpage);
                                guesserpoints=urlguesser->PointsPerLanguage();
                                if(guesserpoints!=NULL)
                                 {
                                  webpage->AddNewGuesser("URLGuesser",n_possiblelanguages,possiblelanguages,guesserpoints);
                                  for(j=0;j<n_languages;j++)
                                    totalpoints[j]=totalpoints[j]+guesserpoints[possiblelanguages[j]]*guesserweight[i];
                                  guesserpoints=NULL;
                                 }
                               }
                              break;
         }
       }
     }
    
    //Ahora escogemos el idioma mas probable
    bestresult=totalpoints[0];
    secondbest=totalpoints[1];
    result=possiblelanguages[0];
    abstotalpoints=totalpoints[0];
    for(i=1;i<n_languages;i++)
     {
      abstotalpoints+=totalpoints[i];
      if(totalpoints[i]>bestresult)
       {
        secondbest=bestresult;
        bestresult=totalpoints[i];
        result=possiblelanguages[i];
       }
       else
        {
         if(totalpoints[i]>secondbest)
           secondbest=totalpoints[i];
        }
     }
    
    //Normalizamos
    bestresult=bestresult/abstotalpoints;
    secondbest=secondbest/abstotalpoints;
    if((bestresult-secondbest)<kRangeAcceptingLanguage)
     {//No se puede decidir
      result=-1;
     }
    delete totalpoints;
    delete textofwebpage;
   }
   else
    {
     fprintf(stdout,"Warning: the language guesser was not properly loaded");
     result=0;
    }
     
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::GuessLanguage(%p) -> %d\n",webpage->Content(),result);//El texto sera muy largo para mostrarlo
#endif
  
  return(result);
 }

void Collector::InsertLanguagePage(int page,int language,unsigned int treelevel)
 {
  int *aux;
  int languageposition;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::InsertLanguagePage(%d,%d,%d)\n",page,language,treelevel);
#endif
  languageposition=langid[language];
  if(n_languagepages[languageposition]==n_sizelanguagepages[languageposition])
   {//El vector debe crecer ya que no cabe la nueva pagina
    aux=new int[kNumPagesperLanguage+n_sizelanguagepages[languageposition]];
    memcpy(aux,languagepages[languageposition],sizeof(int)*n_sizelanguagepages[languageposition]);
    delete languagepages[languageposition];
    languagepages[languageposition]=aux;
    n_sizelanguagepages[languageposition]+=kNumPagesperLanguage;
    languagepages[languageposition][n_languagepages[languageposition]]=page;
    n_languagepages[languageposition]++;
   }
   else
    {
     languagepages[languageposition][n_languagepages[languageposition]]=page;
     n_languagepages[languageposition]++;
    }
  
  if(n_langpagesperlevel[languageposition][treelevel]==n_sizelangpagesperlevel[languageposition][treelevel])
   {
    aux=new int[kNumPagesperLanguageLevel+n_sizelangpagesperlevel[languageposition][treelevel]];
    memcpy(aux,langpagesperlevel[languageposition][treelevel],sizeof(int)*n_sizelangpagesperlevel[languageposition][treelevel]);
    delete langpagesperlevel[languageposition][treelevel];
    langpagesperlevel[languageposition][treelevel]=aux;
    n_sizelangpagesperlevel[languageposition][treelevel]+=kNumPagesperLanguageLevel;
    langpagesperlevel[languageposition][treelevel][n_langpagesperlevel[languageposition][treelevel]]=page;
    n_langpagesperlevel[languageposition][treelevel]++;
    if(oldn_langpagesperlevel!=NULL)
      oldn_langpagesperlevel[languageposition][treelevel]++;     
   }
   else
    {
     langpagesperlevel[languageposition][treelevel][n_langpagesperlevel[languageposition][treelevel]]=page;
     n_langpagesperlevel[languageposition][treelevel]++;
     if(oldn_langpagesperlevel!=NULL)
       oldn_langpagesperlevel[languageposition][treelevel]++;     
    }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::InsertLanguagePage(%d,%d,%d)\n",page,language,treelevel);
#endif
 } 

float Collector::Similars(WebPage *a,WebPage *b,unsigned int b_place)
 {
  float distance;
  bool stillsimilars;
  double aux;
  char *auxurl;
  const char *exta,*extb;
  unsigned int i;
  const unsigned int *tagslevela,*tagslevelb;
  float diferences[nappliedheuristics];
  unsigned int appliedheuristics;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Similars(%p,%p,%d)\n",a,b,b_place);
#endif
    
  extrastat++;
  stillsimilars=true;
  appliedheuristics=0;
  if(b!=NULL)
   {
    if(applyntagslevel)
     {//Comprobamos el numero de etiquetas por nivel    
      tagslevela=a->N_LevelTags();
      tagslevelb=b->N_LevelTags();
      diferences[appliedheuristics]=0;
      for(i=0;i<kMaxLevelTags && stillsimilars;i++)
       {
        if(tagslevela[i]>=tagslevelb[i])
         {
          aux=tagslevela[i]-tagslevelb[i];
          if(aux>(rangentagslevel[i]*tagslevela[i]) || aux>absrangentagslevel[i])
           {
            stillsimilars=false;
            diffbytagslevel++;
           }
           else
            {
             if(rangentagslevel[i]==1)
               diferences[appliedheuristics]+=aux/absrangentagslevel[i];
              else
                diferences[appliedheuristics]+=aux/(rangentagslevel[i]*tagslevela[i]);
            }
         }
         else
          {
           aux=tagslevelb[i]-tagslevela[i];
           if(aux>(rangentagslevel[i]*tagslevelb[i]) || aux>absrangentagslevel[i])
            {
             stillsimilars=false;
             diffbytagslevel++;       
            }
            else
             {
              if(rangentagslevel[i]==1)
                diferences[appliedheuristics]+=aux/absrangentagslevel[i];
               else
                 diferences[appliedheuristics]+=aux/(rangentagslevel[i]*tagslevelb[i]);
             }
          }      
       }
      if(stillsimilars)
        diferences[appliedheuristics]=diferences[appliedheuristics]*weightntagslevel;
      appliedheuristics++;
     }
   
    if(stillsimilars && applynlinks)
     {//Comprobamos el numero de enlaces
      if(a->N_Links()>=b->N_Links())
       {
        aux=a->N_Links()-b->N_Links();
        if(aux>(rangenlinks*a->N_Links()) || aux>absrangenlinks)
         {
          stillsimilars=false;
          diffbylinks++;       
         }
         else
          {
           if(rangenlinks==1)
             diferences[appliedheuristics]=aux/absrangenlinks;
            else
              diferences[appliedheuristics]=aux/(rangenlinks*a->N_Links());
           diferences[appliedheuristics]=diferences[appliedheuristics]*weightnlinks;
          }
       }
       else
        {
         aux=b->N_Links()-a->N_Links();
         if(aux>(rangenlinks*b->N_Links()) || aux>absrangenlinks)
          {
           stillsimilars=false;
           diffbylinks++;       
          }
          else
           {
            if(rangenlinks==1)
              diferences[appliedheuristics]=aux/absrangenlinks;
             else
               diferences[appliedheuristics]=aux/(rangenlinks*b->N_Links());
            diferences[appliedheuristics]=diferences[appliedheuristics]*weightnlinks;
           }
        }
      appliedheuristics++;
     }

    if(stillsimilars && applysize)
     {//Comprobacion de tamaños
      if(a->Size()>=b->Size())
       {
        aux=a->Size()-b->Size();
        if(aux>(rangesize*a->Size()) || aux>absrangesize)
         {
          stillsimilars=false;
          diffbysize++;       
         }
         else
          {
           if(rangesize==1)
             diferences[appliedheuristics]=aux/absrangesize;
            else
              diferences[appliedheuristics]=aux/(rangesize*a->Size());
           diferences[appliedheuristics]=diferences[appliedheuristics]*weightsize;
          }
       }
       else
        {
         aux=b->Size()-a->Size();
         if(aux>(rangesize*b->Size()) || aux>absrangesize)
          {
           stillsimilars=false;
           diffbysize++;       
          }
          else
           {
            if(rangesize==1)
              diferences[appliedheuristics]=aux/absrangesize;
             else
               diferences[appliedheuristics]=aux/(rangesize*b->Size());
            diferences[appliedheuristics]=diferences[appliedheuristics]*weightsize;
           }
        }
      appliedheuristics++;
     } 
    
    if(stillsimilars && applyntags)
     {//Comprobacion de numero de etiquetas
      if(a->N_Tags()>=b->N_Tags())
       {
        aux=a->N_Tags()-b->N_Tags();
        if(aux>(rangentags*a->N_Tags()) || aux>absrangentags)
         {
          stillsimilars=false;
          diffbytags++;       
         }
         else
          {
           if(rangentags==1)
             diferences[appliedheuristics]=aux/absrangentags;
            else
              diferences[appliedheuristics]=aux/(rangentags*a->N_Tags());
           diferences[appliedheuristics]=diferences[appliedheuristics]*weightntags;
          }
       }
       else
        {
         aux=b->N_Tags()-a->N_Tags();
         if(aux>(rangentags*b->N_Tags()) || aux>absrangentags)
          {
           stillsimilars=false;
           diffbytags++;       
          }
          else
           {
            if(rangentags==1)
              diferences[appliedheuristics]=aux/absrangentags;
             else
               diferences[appliedheuristics]=aux/(rangentags*b->N_Tags());
            diferences[appliedheuristics]=diferences[appliedheuristics]*weightntags;
           }
        }
      appliedheuristics++;
     }
           
    if(stillsimilars && applydepth)
     {//Comprobamos la profundidad en el arbol de directorios del servidor
      auxurl=a->PageFullURL();
      if(auxurl!=NULL)
       {
        aux=urlhash->Depth(auxurl)-urlhash->Depth(b_place);
        delete auxurl;
        auxurl=NULL;
        if(aux<0)
          aux=-1*aux;
        if(aux>absrangetreelevel)
         {
          stillsimilars=false;
          diffbytreelevel++;
         }
         else
           diferences[appliedheuristics]=aux/absrangetreelevel;
        diferences[appliedheuristics]=diferences[appliedheuristics]*weightdepth;
       }
       else
        {
         stillsimilars=false;
         diffbytreelevel++;
        }
      appliedheuristics++;
     }
           
    if(stillsimilars && applyextension)
     {//Comprobacion de extensiones
      exta=a->Extension();
      extb=b->Extension();
      if(exta==extb || (exta!=NULL && extb!=NULL && strcmp(exta,extb)!=0))
       {
        diffbyext++;
        stillsimilars=false;
       }
     }
     
    if(stillsimilars && applynimages)
     {//Comprobamos el numero de imagenes
      if(a->N_Images()>=b->N_Images())
       {
        aux=a->N_Images()-b->N_Images();
        if(aux>(rangenimages*a->N_Images()) || aux>absrangenimages)
         {
          stillsimilars=false;
          diffbyimages++;       
         }
         else
          {
           if(rangenimages==1)
             diferences[appliedheuristics]=aux/absrangenimages;
            else
              diferences[appliedheuristics]=aux/(rangenimages*a->N_Images());
           diferences[appliedheuristics]=diferences[appliedheuristics]*weightnimages;
          }
       }
       else
        {
         aux=b->N_Images()-a->N_Images();
         if(aux>(rangenimages*b->N_Images()) || aux>absrangenimages)
          {
           stillsimilars=false;
           diffbyimages++;       
          }
          else
           {
            if(rangenimages==1)
              diferences[appliedheuristics]=aux/absrangenimages;
             else
               diferences[appliedheuristics]=aux/(rangenimages*b->N_Images());
            diferences[appliedheuristics]=diferences[appliedheuristics]*weightnimages;
           }
        }
      appliedheuristics++;
     }
   }

  if(!stillsimilars)
    distance=-1;
   else
    {//Calculamos la distancia de aplicar las heuristicas
     distance=0;
     for(i=0;i<appliedheuristics;i++)
       distance+=pow(diferences[i],2);
     distance=sqrt(distance);
    }
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Similars(%p,%p,%d) -> %f\n",a,b,b_place,distance);
#endif
  
  return(distance);
 }

bool Collector::Equals(WebPage *a,WebPage *b)
 {
  bool result;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Equals(%p,%p)\n",a,b);
#endif    
  result=true;
  if(b!=NULL)
   {//Comprobamos el tamaño
    if(a->Size()-b->Size()!=0)
      result=false;
     else
      {//Comprobamos el numero de enlaces
       if(a->N_Links()-b->N_Links()!=0)
         result=false;
        else
         {//Comprobamos el numero de etiquetas
          if(a->N_Tags()-b->N_Tags()!=0)
            result=false;
           else
            {//Comprobamos el numero de imagenes
             if(a->N_Images()-b->N_Images()!=0)
               result=false;
            }
         }
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Equals(%p,%p) -> %d\n",a,b,result);
#endif
  return(result);
 }

bool Collector::IsRepeatedWebpage(WebPage *a)
 {
  bool result;
  unsigned int size,repeated,i;
  const unsigned int *searchresults;
  
  size=a->Size();
  repeated=orderedlist->Search(size);
  result=false;
  if(repeated>0)
   {
    searchresults=orderedlist->SearchResults();
    for(i=0;i<repeated && !result;i++)
     {
      if(Equals(a,(WebPage*)urlhash->Content(searchresults[i])))
        result=true;
     }
   }
  
  return(result);
 }

bool Collector::Collect()
 {
  bool result,urldownloaded;
  char filetemp[30],filetemp2[100];
  int pid;
#ifndef HIDEMESSAGES
  struct tm *tmact; //Estructura para el manejo del tiempo en los logs
  time_t timeact;
  char timestring[kSizeAuxStrings]; //Cadena con la hora para los log
#endif  
  unsigned int i,hashsize;
  int j,k;
  int language; //Idioma de la pagina
  WebPage *webpage; //Pagina que se esta procesando actualmente
  WebPage *webpage2; //Pagina para comprobar las heuristicas
  WebPage website; //Pagina web asociada al sitio para comprobar enlaces externos
  WebPage auxwebpage; //Pagina auxiliar
  int whereintree; //Posicion que ocupa la pagina dentro del arbol
  const char **pagelinks; //Auxiliar para acceder a los links de una pagina
  const char *content; //Auxiliar para acceder al contenido de la pagina web
  char *auxurl,*auxurl2; //Auxiliar para almacenar url
  unsigned int nextnode; //Identificador del nodo que se esta recorriendo
  unsigned int treelevel; //Nivel en el arbol de la pagina que se esta insertando
  unsigned int actualnsimilars; //Numero de similares que tiene la pagina en proceso
  float similaritydistance; //Distancia entre dos ficheros que podrian ser similares
  
  //Variables para la generacion de urls
  char **generatedvariants; //URLs generadas en base a los patrones
  unsigned int ngeneratedvariants; //Numero de URLs generadas
  unsigned int variantposition; //Posicion de la variante dentro de la hash de urls
   
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Collect()\n");
#endif
  
  result=true;
  pid=getpid();
  sprintf(filetemp,"collectortmp%d",pid);
  
  
  urldownloaded=dmanager.DownloadURL(urllist[0],filetemp);
  website.FijarURL(urllist[0]);
  
  if(urldownloaded)
   {
    downloaded++;
    if(urlhash!=NULL)
      delete urlhash;
    urlhash=new URLHash();
    
    if(!Stats(statsfilename))
      fprintf(stderr,"Error: The stats file \"%s\"could not be created (updating collection disabled)\n",statsfilename);
      
#ifndef HIDEMESSAGES
     fprintf(stdout,"Downloading from: %s\n",urllist[0]);
      timeact=time(0);
      tmact=localtime(&timeact);
      sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
      if(strlen(urllist[0])>50)
        fprintf(stderr,"(%s):Processing  ...%s\n",timestring,urllist[0]+strlen(urllist[0])-50);
       else
         fprintf(stderr,"(%s):Processing %s\n",timestring,urllist[0]);
#endif    
    webpage=new WebPage();
    if(webpage->ImportConvertedPage(filetemp,urllist[0]))
     {
      //Ahora empezamos el recorrido de los enlaces de la pagina leida
      pagelinks=webpage->Links();
      for(i=0;i<webpage->N_Links();i++)
       {//Solo insertamos si no estaba ya
        auxurl2=CheckURL(pagelinks[i],webpage);
        if(auxurl2!=NULL)
         {
          if(!IsExternalLink(auxurl2) && urlhash->Search(auxurl2)==0)
           {          
            auxwebpage.FijarURL(auxurl2);
            if(dmanager.IsGoodExtension(auxwebpage.Extension()))
             {                       
              if(strlen(auxurl2)>7 && auxurl2[0]=='h' && auxurl2[1]=='t' &&
                 auxurl2[2]=='t' && auxurl2[3]=='p' && auxurl2[4]==':' &&
                 auxurl2[5]=='/' && auxurl2[6]=='/')
               {//Quitamos el http:// inicial si existe
                urlhash->InsertEnd(auxurl2+7,NULL);
               }
               else 
                 urlhash->InsertEnd(auxurl2,NULL);
             }
           }
          delete auxurl2;
          auxurl2=NULL;
         }
       }

#ifndef HIDEMESSAGES
      timeact=time(0);
      tmact=localtime(&timeact);
      sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
      if(strlen(urllist[0])>50)
        fprintf(stderr,"(%s):Guessing language ...%s...",timestring,urllist[0]+strlen(urllist[0])-50);
       else
         fprintf(stderr,"(%s):Guessing language %s...",timestring,urllist[0]);
      fflush(stderr);
#endif

      language=GuessLanguage(webpage);
#ifndef HIDEMESSAGES
      fprintf(stderr,"%d\n",language);
#endif
      webpage->SetLanguage(language);
      if(language>=0) //Si no se identifica el idioma no se procesaria
       {
        whereintree=urlhash->InsertEnd(urllist[0],(void*)webpage);
        orderedlist->Insert(webpage->Size(),whereintree);

        treelevel=urlhash->Depth(whereintree);
        InsertLanguagePage(whereintree,language,treelevel);
        sprintf(filetemp2,"%s/%s/%s/%06d.xml",website.Domain(),newdate,LanguageName(language),n_languagepages[langid[language]]);
        webpage->Save(filetemp2);
        webpage->CleanContent();//Liberamos el contenido de la memoria
        webpage->FreeUselessFields();//Liberamos los enlaces que ya no se usaran
        
        //Guardamos la pagina en el fichero con la lista de urls
        fprintf(filelisturl[langid[language]],"%s\n",urllist[0]);
        fflush(filelisturl[langid[language]]);
       }
       else
        {
         discardedlang++;
         delete webpage;
         webpage=NULL;
        }
     }
     else
      {
       delete webpage;
       webpage=NULL;
      }    
    
    nextnode=urlhash->NextNode(); //Saltamos el nodo que acabamos de recorrer
    for(nextnode=urlhash->NextNode();nextnode!=0;nextnode=urlhash->NextNode())    
     {//Recorremos una a una el resto de paginas hasta haberlas recorrido todas
      auxurl=urlhash->Search(nextnode);
      auxwebpage.FijarURL(auxurl);
      if(dmanager.IsGoodExtension(auxwebpage.Extension()) && dmanager.DownloadURL(auxurl,filetemp))
       {
        downloaded++;
        if(downloaded%(unsigned int)(1/kSecondsBetweenRequest)==0)
         {
          Stats();
          sleep(1);
         }
#ifndef HIDEMESSAGES
        timeact=time(0);
        tmact=localtime(&timeact);
        sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
        if(strlen(auxurl)>50)
          fprintf(stderr,"(%s):Processing  ...%s\n",timestring,auxurl+strlen(auxurl)-50);
         else
           fprintf(stderr,"(%s):Processing %s\n",timestring,auxurl);
#endif    
        webpage=new WebPage();
        if(webpage->ImportConvertedPage(filetemp,auxurl))
         {
          //webpage->Save(kTestLoadFile);
          content=webpage->Content();
          if(content!=NULL/* && auxwebpage.Load(kTestLoadFile)*/)
           {//Ahora empezamos el recorrido de los enlaces de la pagina leida
            auxwebpage.Reset();
            pagelinks=webpage->Links();
            for(i=0;i<webpage->N_Links();i++)
             {
              auxurl2=CheckURL(pagelinks[i],webpage);//Limpiamos la URL
              if(auxurl2!=NULL)
               {
                if(!IsExternalLink(auxurl2) && urlhash->Search(auxurl2)==0)
                 {//Solo insertamos si no estaba ya y si la web es interna
                  auxwebpage.FijarURL(auxurl2);
                  if(dmanager.IsGoodExtension(auxwebpage.Extension()))
                    urlhash->InsertEnd(auxurl2,NULL);
                 }
                delete auxurl2;
                auxurl2=NULL;
               }
             }
            if(!IsRepeatedWebpage(webpage) && webpage->TextSize()>=minimalcontentsize)
             {
#ifndef HIDEMESSAGES      
              timeact=time(0);
              tmact=localtime(&timeact);
              sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
              if(strlen(auxurl)>50)
                fprintf(stderr,"(%s):Guessing language ...%s...",timestring,auxurl+strlen(auxurl)-50);
               else
                 fprintf(stderr,"(%s):Guessing language %s...",timestring,auxurl);      
              fflush(stderr);
#endif
              language=GuessLanguage(webpage);
#ifndef HIDEMESSAGES
              fprintf(stderr,"%d\n",language);
#endif          
              webpage->SetLanguage(language);
              if(language>=0)
               {
                webpage2=(WebPage*)urlhash->Content(urlhash->Search(auxurl));
                if(webpage2!=NULL)
                 {//Borramos el contenido del arbol si lo hubiera puesto que se va a sobreescribir
                  delete webpage2;
                  webpage2=NULL;
                 }
                whereintree=urlhash->InsertEnd(auxurl,(void*)webpage);
                orderedlist->Insert(webpage->Size(),whereintree);
                                  
                treelevel=urlhash->Depth(whereintree);
                InsertLanguagePage(whereintree,language,treelevel);

#ifndef HIDEMESSAGES
                timeact=time(0);
                tmact=localtime(&timeact);
                sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
                if(strlen(auxurl)>50)
                  fprintf(stderr,"(%s):Looking for similars ...%s...",timestring,auxurl+strlen(auxurl)-50);
                 else
                   fprintf(stderr,"(%s):Looking for similars %s...",timestring,auxurl);      
                fflush(stderr);
#endif

                //Comprobamos las heuristicas con los ficheros descargados hasta ahora                
                if(urlgenerator==NULL)
                 {//Estrategia todos con todos
                  for(i=0,actualnsimilars=0;i<n_languages;i++)
                   {
                    if(i!=langid[language])
                     {//Solo comprobamos con paginas en distinto idioma
                      for(j=0;j<kMaxDeepTree;j++)
                       {
                        if((signed)treelevel>(signed)(j-absrangetreelevel) && (signed)treelevel<(signed)(j+absrangetreelevel))
                         {//Solo se comprueban con paginas en una profundidad similar
                          for(k=0;k<n_langpagesperlevel[i][j];k++)
                           {
                            webpage2=(WebPage*)urlhash->Content(langpagesperlevel[i][j][k]);
                            if(webpage2!=NULL)
                             {
                              similaritydistance=Similars(webpage,webpage2,langpagesperlevel[i][j][k]);
                              if(similaritydistance!=-1)
                               {//Se añade a la lista de similares
                                n_similars++;
                                webpage->NewSimilar(webpage2->WhatLanguage(),webpage2->LocalFileId(),similaritydistance);
                                if(!quickmode)
                                 {
                                  webpage2->NewSimilar(language,n_languagepages[langid[language]],similaritydistance);
                                  webpage2->Refresh();
                                  webpage2->CleanContent();
                                 }
                                actualnsimilars++;
                               }
                             }
                           }
                         }
                       }
                     }
                   }
                 }
                 else
                  {//Estrategia en base a patrones
                   if(generationtype==1)
                     generatedvariants=urlgenerator->GenerateVariants1(language,auxurl,&ngeneratedvariants);
                    else
                      generatedvariants=urlgenerator->GenerateVariants2(language,auxurl,&ngeneratedvariants);
                   
                   totalurlgenerated+=ngeneratedvariants;
                   //Buscamos las variantes
                   for(i=0,actualnsimilars=0;i<ngeneratedvariants;i++)
                    {
                     variantposition=urlhash->Search(generatedvariants[i]);
                     if(variantposition!=0)
                      {//La variante existia en la hash
                       webpage2=(WebPage*)urlhash->Content(variantposition);
                       if(webpage2!=NULL)//Nos aseguramos de que ya estuviera descargada
                        {
                         totalgoodgenerations++;                   
                         if(webpage->WhatLanguage()!=webpage2->WhatLanguage())
                          {
                           similaritydistance=Similars(webpage,webpage2,webpage2->WhatLanguage());
                           if(similaritydistance!=-1)
                            {//Se añade a la lista de similares
                             n_similars++;
                             webpage->NewSimilar(webpage2->WhatLanguage(),webpage2->LocalFileId(),similaritydistance);
                             if(!quickmode)
                              {
                               webpage2->NewSimilar(language,n_languagepages[langid[language]],similaritydistance);
                               webpage2->Refresh();
                               webpage2->CleanContent();
                              }
                             actualnsimilars++;
                            }
                          }
                        }
                      }                            
                    }
                   delete generatedvariants;
                   generatedvariants=NULL;
                  }
                 
#ifndef HIDEMESSAGES
                fprintf(stderr,"%d similars found\n",actualnsimilars);
                fflush(stderr);
#endif                 
                sprintf(filetemp2,"%s/%s/%s/%06d.xml",website.Domain(),newdate,LanguageName(language),n_languagepages[langid[language]]);
                if(!webpage->Save(filetemp2))
                 {
                  if(!webpage->Save(filetemp2))
                   {
#ifndef HIDEMESSAGES
                    fprintf(stderr,"Error: Can't write file \"%s\"\n",filetemp2);
#endif               
                   }
                 }
                webpage->CleanContent();//No nos quedamos con el contenido
                webpage->FreeUselessFields();//Liberamos los enlaces que ya no se usaran
                
                //Guardamos la pagina en el fichero con la lista de urls
                fprintf(filelisturl[langid[language]],"%s\n",auxurl);
                fflush(filelisturl[langid[language]]);                
               }
               else
                {
                 discardedlang++;
#ifndef HIDEMESSAGES
                 if(strlen(auxurl)>45)
                   fprintf(stderr,"Discarding page (language)...%s...\n",auxurl+strlen(auxurl)-45);
                  else
                    fprintf(stderr,"Discarding page (language) %s...\n",auxurl);                    
#endif
                 
                 delete webpage;
                 webpage=NULL;
                }
             }
             else
              {
               if(webpage->TextSize()<minimalcontentsize)
                {
                 discardedsize++;
#ifndef HIDEMESSAGES
                 if(strlen(auxurl)>45)
                   fprintf(stderr,"Discarding page (textsize)...%s...\n",auxurl+strlen(auxurl)-45);
                  else
                    fprintf(stderr,"Discarding page (textsize) %s...\n",auxurl);                    
#endif
                }
                else
                 {
                  n_repeated++;
#ifndef HIDEMESSAGES      
                  if(strlen(auxurl)>45)
                    fprintf(stderr,"Discarding page (repeated)...%s...\n",auxurl+strlen(auxurl)-45);
                   else
                     fprintf(stderr,"Discarding page (repeated) %s...\n",auxurl);                    
#endif                  
                 }
               delete webpage;
               webpage=NULL;
              }                             
           }
           else
            {
             delete webpage;
             webpage=NULL;
#ifndef HIDEMESSAGES
               if(strlen(auxurl)>45)
                 fprintf(stderr,"Discarding page (size)...%s...",auxurl+strlen(auxurl)-45);
                else
                  fprintf(stderr,"Discarding page (size) %s...",auxurl);      
#endif
            }           
         }
         else
          {
#ifndef HIDEMESSAGES       
           fprintf(stderr,"Error: Can't import %s\n",auxurl);
#endif     
           delete webpage;
           webpage=NULL;
          }
       }
       else
        {
#ifndef HIDEMESSAGES
         if(dmanager.IsGoodExtension(auxwebpage.Extension()))
           fprintf(stderr,"Error: Can't download %s\n",auxurl);
          else
            fprintf(stderr,"Error: Wrong extension %s\n",auxurl);
#endif         
        }
      delete auxurl;
      auxurl=NULL;
     }
    
    for(i=0,hashsize=urlhash->Size();i<hashsize;i++)
     {
      webpage=(WebPage*)urlhash->Content(i);
      if(webpage!=NULL)
       {
        urlhash->SetContentNULL(i);
        if(webpage!=NULL)
         {
          delete webpage;
          webpage=NULL;
         }
       }
     }
    fprintf(stdout,"Finished download from %s\n",urllist[0]);
    Stats();
    
    if(!Stats(statsfilename))
      fprintf(stderr,"Error: The stats file \"%s\"could not be updated (updating collection disabled)\n",statsfilename);

    //A modo de backup se almacena también el fichero de estadisticas en el directorio de la descarga
    Stats(statsfilename2);
          
    delete urlhash;
    urlhash=NULL;
    ResetStats();
   }
   else
    {
     result=false;
#ifndef HIDEMESSAGES     
     fprintf(stderr,"Error: The url %s was not found\n",urllist[0]);
#endif     
    }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Collect() -> %d\n",result);
#endif
      
  return(result);
 }

bool Collector::Start()
 {
  bool result;
  bool resultcollect;
  char filename[1000];
  char directoryname[1000];
  char auxdirectoryname[1000];
  char languageurl[kNumLanguages];
  unsigned int i;
  int j;
  WebPage auxwebpage; //Pagina auxiliar
  
  struct tm *tmact; //Estructura para el manejo del tiempo
  time_t timeact;

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Start()\n");
#endif
  
  updatingcollection=false;
  if(urllist!=NULL)
   {
    auxwebpage.FijarURL(urllist[0]);    
    //Creamos el directorio que se usara de cache para esta ejecucion (por si no existiera)
    strcpy(directoryname,kLocalDirectory);
    if(directoryname[strlen(directoryname)-1]=='/')
      directoryname[strlen(directoryname)-1]='\0';//Quitamos el '/' del final
    strcat(directoryname,"/");
    strcat(directoryname,auxwebpage.Domain());

    //Cogemos la fecha actual
    timeact=time(0);
    tmact=localtime(&timeact);
    sprintf(newdate,"%4d-%02d-%02d",1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
    
    //Creamos el directorio que se usara para almacenar las paginas descargadas
    strcpy(directoryname,auxwebpage.Domain());
    mkdir(directoryname,0777);
    
    sprintf(auxdirectoryname,"%s/%s",directoryname,newdate);
    mkdir(auxdirectoryname,0777);

    //Creamos el vector con la utilizacion o no de los diferentes idiomas
    //que soporta el identificador de idiomas
    for(i=0;i<kNumLanguages;i++)
      languageurl[i]='0';
    languageurl[i]='\0';
    for(i=0;i<n_languages;i++)
      languageurl[LanguageCode(langurl[i])]='1';
     
    //Se crea la estructura de directorios adecuada para el procesamiento
    for(i=0,result=true;i<n_languages && result;i++)
     {
      //Creamos un directorio para cada idioma
      strcpy(filename,auxdirectoryname);
      strcat(filename,"/");
      strcat(filename,langurl[i]);      
      mkdir(filename,0777);
      
      strcat(filename,"/");
      strcat(filename,kNameURLList);
      strcat(filename,"_");
      strcat(filename,langurl[i]);
      
      if(!(filelisturl[i]=fopen(filename,"w")))
       {
        result=false;
        //Cerramos los ficheros previamente abiertos
        for(j=i-1;j>=0;j--)
          fclose(filelisturl[j]);
       }
     }

    if(result)
     {         
      if(trigramguesser!=NULL)
       {
        trigramguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=trigramguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      if(wordguesser!=NULL)
       {
        wordguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=wordguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      if(urlguesser!=NULL)
       {
        urlguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=urlguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      if(statsfilename!=NULL)
        delete statsfilename;
      statsfilename=new char[2+strlen(directoryname)+strlen(kNameStatsFile)];
      sprintf(statsfilename,"%s/%s",directoryname,kNameStatsFile);
      
      if(statsfilename2!=NULL)
        delete statsfilename2;
      statsfilename2=new char[2+strlen(auxdirectoryname)+strlen(kNameStatsFile)];
      sprintf(statsfilename2,"%s/%s",auxdirectoryname,kNameStatsFile);
      
      resultcollect=Collect();//Ahora ya empezamos la recopilacion
      if(!resultcollect)
        result=false;
      for(i=0;i<n_languages;i++)
        fclose(filelisturl[i]);
     }
    if(!result)
     {//Removing created directories
      for(i--;i<n_languages;i--) //(i is unsigned)
       {//Removing files inside directories before erasing them
        strcpy(filename,auxdirectoryname);
        strcat(filename,"/");
        strcat(filename,langurl[i]); 
        strcat(filename,"/");
        strcat(filename,kNameURLList);
        strcat(filename,"_");
        strcat(filename,langurl[i]);
        unlink(filename);
        
        strcpy(filename,auxdirectoryname);
        strcat(filename,"/");
        strcat(filename,langurl[i]);     
        rmdir(filename);
       }
      rmdir(auxdirectoryname);
      rmdir(directoryname);
     }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: A file with urls should have been loaded before starting the process\n");
    }
    
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Start() -> %d\n",result);
#endif
     
  return(result);
 }

char* Collector::DownloadingDirectory()
 {
  char *result;
  
  if(urllist!=NULL)
   {
    result=new char[1+strlen(urllist[0])];
    strcpy(result,urllist[0]);
   }
   else
     result=NULL;
  return(result);
 }
 
char* Collector::CheckURL(const char *url,WebPage *webpage)
 {
  char *result;
  const char *domain;//Variable para acceder a la webpage
  const char *path;//Variable para acceder a la webpage
 
#ifdef TRAZANDO_Collector
  if(url!=NULL)
    fprintf(stderr,"BeginCollector::CheckURL(%s,%p)\n",url,webpage);
   else
     fprintf(stderr,"BeginCollector::CheckURL(NULL,%p)\n",webpage);
#endif
  
  if(url!=NULL)
   {
    if((strlen(url)>4 && (url[0]=='j' || url[0]=='J') && //No se aceptan si empiezan por 'Java' 
      (url[1]=='a' || url[1]=='A') &&
      (url[2]=='v' || url[2]=='V') &&
      (url[3]=='a' || url[3]=='A')) ||
     (strlen(url)>7 && (url[0]=='m' || url[0]=='M') && //No se aceptan si empiezan por 'Mailto:'
      (url[1]=='a' || url[1]=='A') &&
      (url[2]=='i' || url[2]=='I') &&
      (url[3]=='l' || url[3]=='L') &&
      (url[4]=='t' || url[4]=='T') &&
      (url[5]=='o' || url[5]=='O') &&
       url[6]==':') ||
     (strlen(url)>5 && (url[0]=='h' || url[0]=='H') && //No se aceptan si empiezan por 'https'
      (url[1]=='t' || url[1]=='T') &&
      (url[2]=='t' || url[2]=='T') &&
      (url[3]=='p' || url[3]=='P') &&
      (url[4]=='s' || url[4]=='S')))
      result=NULL;
     else
      {
       if(url[0]=='/')
        {//Se refiere a la raiz del domain actual
         domain=webpage->Domain();
         result=new char[1+strlen(url)+strlen(domain)+7];
         sprintf(result,"%s%s",domain,url);
        }
        else
         {
          if(strlen(url)>7 && (url[0]=='h' || url[0]=='H') &&
             (url[1]=='t' || url[1]=='T') &&
             (url[2]=='t' || url[2]=='T') &&
             (url[3]=='p' || url[3]=='P') &&
             url[4]==':' && url[5]=='/' && url[6]=='/')
           {//el enlace ya era absoluto
            result=new char[1+strlen(url)-7];
            strcpy(result,url+7);
           }
           else
            {
             domain=webpage->Domain();
             path=webpage->Path();
             result=new char[1+strlen(url)+strlen(domain)+strlen(path)+7];
             if(strlen(path)>0)
               sprintf(result,"%s/%s/%s",domain,path,url);
              else
                sprintf(result,"%s/%s",domain,url);
            }
         }
      }
    }
    else
      result=NULL;

#ifdef TRAZANDO_Collector
  if(url!=NULL)
   {
    if(result!=NULL)
      fprintf(stderr,"BeginCollector::CheckURL(%s,%p) -> %s\n",url,webpage,result);
     else
       fprintf(stderr,"BeginCollector::CheckURL(%s,%p) -> NULL\n",url,webpage);
   }
   else
    {
     if(result!=NULL)
       fprintf(stderr,"BeginCollector::CheckURL(NULL,%p) -> %s\n",webpage,result);
      else
        fprintf(stderr,"BeginCollector::CheckURL(NULL,%p) -> NULL\n",webpage);
    }
#endif
       
  return(result);
 }

void Collector::Stats()
 {
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Stats()\n");
#endif

  fprintf(stdout,"----------------------------------------------------------\n");
  if(updatingcollection)
   {
    fprintf(stdout,"Retrieved (new ones): %s->%d (%d) ",langurl[0],n_languagepages[0],n_languagepages[0]-oldn_languagepages[0]);
     for(i=1;i<n_languages;i++)
       fprintf(stdout,", %s->%d (%d)",langurl[i],n_languagepages[i],n_languagepages[i]-oldn_languagepages[i]);
     fprintf(stdout,"\nDownloaded: %d/%d, Previously processed: %d\n",downloaded,downloaded+dmanager.NotFoundPages(),processedpreviously);
     fprintf(stdout,"Discarded: language->%d, textsize->%d, repeated->%d\n",discardedlang,discardedsize,n_repeated);
     if(urlgenerator!=NULL)
      {//Mostramos las estadisticas de generacion
       fprintf(stdout,"URLs found (generated): %d(%d)\n",totalgoodgenerations,totalurlgenerated);
      }
     fprintf(stdout,"Not similars (new ones): tagslevel->%d (%d), links->%d (%d), size->%d (%d),\n\t      tags->%d (%d), depth->%d (%d), ext->%d (%d)\n",
                    diffbytagslevel,diffbytagslevel-olddiffbytagslevel,diffbylinks,diffbylinks-olddiffbylinks,diffbysize,diffbysize-olddiffbysize,
                    diffbytags,diffbytags-olddiffbytags,diffbytreelevel,diffbytreelevel-olddiffbytreelevel,diffbyext,diffbyext-olddiffbyext);
     fprintf(stdout,"Num links left (total): %d(%d)\n",urlhash->QueueSize(),urlhash->NURLs());
     fprintf(stdout,"Num similars (new ones): %d(%d)\n",n_similars,n_similars-oldn_similars);
     fflush(stdout);
   }
   else
    {
     fprintf(stdout,"Retrieved: (%s->%d ",langurl[0],n_languagepages[0]);
     for(i=1;i<n_languages;i++)
       fprintf(stdout,", %s->%d",langurl[i],n_languagepages[i]);
     fprintf(stdout,")\nDownloaded: %d/%d, Cache hits: %d (%.2f%%)\n",downloaded,downloaded+dmanager.NotFoundPages(),dmanager.CacheHits(),100*dmanager.CacheAccuracy());
     fprintf(stdout,"Discarded: language->%d, textsize->%d, repeated->%d\n",discardedlang,discardedsize,n_repeated);
     if(urlgenerator!=NULL)
      {//Mostramos las estadisticas de generacion
       fprintf(stdout,"URLs found (generated): %d(%d)\n",totalgoodgenerations,totalurlgenerated);
      }
     fprintf(stdout,"Not similars: tagslevel->%d, links->%d, size->%d,\n\t      tags->%d, depth->%d, ext->%d\n",diffbytagslevel,diffbylinks,diffbysize,diffbytags,diffbytreelevel,diffbyext);
     fprintf(stdout,"Num links left (total): %d(%d)\n",urlhash->QueueSize(),urlhash->NURLs());
     fprintf(stdout,"Num similars: %d\n",n_similars);
//     fprintf(stdout,"Extra: %d\n",extrastat); //Estadistica extra para la depuracion
     fflush(stdout);
    }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Stats()\n");
#endif
 }

bool Collector::Stats(const char *filename)
 {
  unsigned int i;
  FILE *fsal;
  bool result;
  
#ifdef TRAZANDO_Collector
  if(filename!=NULL)
    fprintf(stderr,"BeginCollector::Stats(%s)\n",filename);
   else
     fprintf(stderr,"BeginCollector::Stats(filename)\n");
#endif
  
  if(filename!=NULL && (fsal=fopen(filename,"w"))!=NULL)
   {
    result=true;
    fprintf(fsal,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(fsal,"<!DOCTYPE pacostats SYSTEM \"file://pacostats.dtd\">\n");

    fprintf(fsal,"<pacostats href=\"%s\" date=\"%s\">\n",urllist[0],newdate);

    fprintf(fsal,"  <nonexternalwebsites>\n");
    for(i=1;i<n_urllist;i++)
      fprintf(fsal,"    <website href=\"%s\">",urllist[i]);
    fprintf(fsal,"  </nonexternalwebsites>\n");

    fprintf(fsal,"  <targetlanguages>\n");
    for(i=0;i<n_languages;i++)
      fprintf(fsal,"    <tlanguage code=\"%s\" webpages=\"%d\"/>\n",langurl[i],n_languagepages[i]);
    fprintf(fsal,"  </targetlanguages>\n");
    
    fprintf(fsal,"  <downloading success=\"%d\" failure=\"%d\"/>\n",downloaded,dmanager.NotFoundPages());
    fprintf(fsal,"  <discarded unknownlang=\"%d\" insufficientsize=\"%d\" repeated=\"%d\"/>\n",discardedlang,discardedsize,n_repeated);

    if(urlgenerator!=NULL)
      fprintf(fsal,"  <urlgenerationapplied type=\"%d\" coincidences=\"%d\" generated=\"%d\"/>\n",generationtype,totalgoodgenerations,totalurlgenerated);
    
    fprintf(fsal,"  <notsimilars tagslevel=\"%d\" links=\"%d\" size=\"%d\" tags=\"%d\" depth=\"%d\" extension=\"%d\"/>\n",diffbytagslevel,diffbylinks,diffbysize,diffbytags,diffbytreelevel,diffbyext);
    fprintf(fsal,"  <similars value=\"%d\"/>\n",n_similars);
    
    fprintf(fsal,"</pacostats>\n");
    fclose(fsal);
   }  
   else
     result=false;
#ifdef TRAZANDO_Collector
  if(filename!=NULL)
    fprintf(stderr,"EndCollector::Stats(\"%s\") -> %d\n",filename,result);
   else
     fprintf(stderr,"EndCollector::Stats(NULL) -> %d\n",result);
#endif
  return(result);
 }

void Collector::ResetStats()
 {
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ResetStats()\n");
#endif
  
  downloaded=0;
  for(i=0;i<n_languages;i++)
    n_languagepages[i]=0;
  discardedlang=0;
  discardedsize=0;
  diffbylinks=0;
  diffbysize=0;
  diffbytags=0;
  diffbytagslevel=0;
  diffbytreelevel=0;
  diffbyext=0;
  n_similars=0;
  n_repeated=0;
  totalurlgenerated=0;
  totalgoodgenerations=0;
  dmanager.Reset();
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ResetStats()\n");
#endif    
 }

bool Collector::RefreshWebsite(const char *website)
 {
  bool result;
  bool resultcollect;
  char filename[kSizeAuxStrings],filename2[kSizeAuxStrings];
  char directoryname[kSizeAuxStrings],auxdirectoryname[kSizeAuxStrings];
  char languageurl[kNumLanguages];
  unsigned int i;
  int j;
  WebPage *webpage; //Pagina auxiliar
  char *auxurl; //Auxiliar para recuperar las urls
  unsigned int totalfilesdownloaded;
  float totalfilesprocessed;
  unsigned int whereintree;

  struct tm *tmact; //Estructura para el manejo del tiempo
  time_t timeact;

  char linkbuffer[kSizeAuxStrings];
  int sizebuffer;  
    
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::RefreshWebsite()\n");
#endif
  
  strcpy(directoryname,website);
  if(directoryname[strlen(directoryname)-1]=='/')
    directoryname[strlen(directoryname)-1]='\0';

  //Cogemos la fecha actual
  timeact=time(0);
  tmact=localtime(&timeact);
  sprintf(newdate,"%4d-%02d-%02d",1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);

      
  if(statsfilename!=NULL)
    delete statsfilename;
  statsfilename=new char[2+strlen(directoryname)+strlen(kNameStatsFile)];
  sprintf(statsfilename,"%s/%s",directoryname,kNameStatsFile);

  if(statsfilename2!=NULL)
    delete statsfilename2;
  statsfilename2=new char[3+strlen(directoryname)+strlen(newdate)+strlen(kNameStatsFile)];
  sprintf(statsfilename2,"%s/%s/%s",directoryname,newdate,kNameStatsFile);
      
  updatingcollection=true;
  if(LoadResultsFile(statsfilename))
   {//Creamos el vector con la utilizacion o no de los diferentes idiomas
    //que soporta el identificador de idiomas
    for(i=0;i<kNumLanguages;i++)
      languageurl[i]='0';
    languageurl[i]='\0';
    for(i=0;i<n_languages;i++)
      languageurl[LanguageCode(langurl[i])]='1';
     
    if(oldurlhash!=NULL)
      delete oldurlhash;
    oldurlhash=new URLHash();
    if(oldorderedlist!=NULL)
      delete oldorderedlist;
    oldorderedlist=new OrderedList2Levels();
        
    //Reservamos la memoria para la relectura de las paginas y la recoleccion
    n_files=new unsigned int[n_languages];
    filelisturl=new FILE*[n_languages];
    languagepages=new int*[n_languages];  
    n_sizelanguagepages=new int[n_languages];
    n_languagepages=new int[n_languages];
    langpagesperlevel=new int**[n_languages];
    n_sizelangpagesperlevel=new int*[n_languages];
    n_langpagesperlevel=new int*[n_languages];
    oldn_langpagesperlevel=new int*[n_languages];
    for(i=0;i<n_languages;i++)
     {
      n_files[i]=0;
      languagepages[i]=new int[kNumPagesperLanguage];
      n_sizelanguagepages[i]=kNumPagesperLanguage;
      n_languagepages[i]=0;
      langpagesperlevel[i]=new int*[kMaxDeepTree];
      n_langpagesperlevel[i]=new int[kMaxDeepTree];
      oldn_langpagesperlevel[i]=new int[kMaxDeepTree];
      n_sizelangpagesperlevel[i]=new int[kMaxDeepTree];
      for(j=0;j<kMaxDeepTree;j++)
       {
        langpagesperlevel[i][j]=NULL;
        n_langpagesperlevel[i][j]=0;
        oldn_langpagesperlevel[i][j]=0;
        n_sizelangpagesperlevel[i][j]=0;
       }
     }
    
    for(totalfilesdownloaded=0,i=0;i<n_languages;i++)
      totalfilesdownloaded+=oldn_languagepages[i];
    totalfilesprocessed=0;
    
    sprintf(auxdirectoryname,"%s/%s",directoryname,newdate);
    mkdir(auxdirectoryname,0777);
    
    //Se abren los ficheros de URLs descargadas en cada idioma
    for(i=0,result=true;i<n_languages && result;i++)
     {
      sprintf(filename,"%s/%s/%s/%s_%s",directoryname,previousdate,langurl[i],kNameURLList,langurl[i]);
      
      if(!(filelisturl[i]=fopen(filename,"a")))
       {
        result=false;
        //Cerramos los ficheros previamente abiertos
        for(j=i-1;j>=0;j--)
          fclose(filelisturl[j]);
       }
       else
        {//Se crea el directorio asociado al idioma para crear los enlaces simbolicos
         sprintf(auxdirectoryname,"%s/%s/%s",directoryname,newdate,langurl[i]);
         mkdir(auxdirectoryname,0777);
        
         //Cargamos todos los ficheros recolectados previamente         
         for(j=0;j<oldn_languagepages[i];j++)
          {
#ifndef HIDEMESSAGES
           fprintf(stdout,"\b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b: \b\bs\b\bu\b\bp\b\br\b\bo\b\bc\b\b \b\bo\b\bd\b\bn\b\ba\b\bg\b\br\b\ba\b\bC\b\b");
           fprintf(stdout,"Cargando corpus: %.2f%%",100*(totalfilesprocessed+j)/totalfilesdownloaded);
           fflush(stdout);
#endif
           sprintf(filename,"%s/%s/%s/%06d.xml",directoryname,previousdate,langurl[i],j+1);
           webpage=new WebPage();
           
           //Seguimos los enlaces simbolicos (en su caso) hacia el fichero
           bzero(linkbuffer,kSizeAuxStrings);
           do
            {
             sizebuffer=readlink(filename,linkbuffer,kSizeAuxStrings);
             if(sizebuffer>=0)
              {//Era un enlace simbolico
               strcpy(filename,linkbuffer);
              }
            } while(sizebuffer!=-1);
           
           if(webpage->LoadHeader(filename))
            {//Se almacena la cabecera en las estructuras de datos correspondientes para compararla con las nuevas paginas web
             auxurl=webpage->PageFullURL();
             whereintree=oldurlhash->InsertEnd(auxurl,(void*)webpage);
             delete auxurl;
             oldorderedlist->Insert(webpage->Size(),whereintree);
             InsertLanguagePage(whereintree,webpage->WhatLanguage(),oldurlhash->Depth(whereintree));
             
             //Se genera un enlace simbolico en el nuevo directorio
             sprintf(filename2,"%s/%06d.xml",auxdirectoryname,j+1);
             
             if(symlink(filename,filename2)!=0)
              {
               if(symlink(filename,filename2)!=0)
                {
                 fprintf(stderr,"Error: Generating symbolic link ...%s\n",filename2);
                 result=false;
                }
              }
            }
            else
             {
              delete webpage;
              webpage=NULL;
             }
          }
         totalfilesprocessed+=oldn_languagepages[i];
        }
     }

    if(result)
     {
#ifndef HIDEMESSAGES
      fprintf(stdout,"\b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b: \b\bs\b\bu\b\bp\b\br\b\bo\b\bc\b\b \b\bo\b\bd\b\bn\b\ba\b\bg\b\br\b\ba\b\bC\b\b");
      fprintf(stdout,"Cargando corpus: 100.00%%\n");
      fflush(stdout);
#endif

      if(trigramguesser!=NULL)
       {
        trigramguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=trigramguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      if(wordguesser!=NULL)
       {
        wordguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=wordguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      if(urlguesser!=NULL)
       {
        urlguesser->SetPossibleLanguages(languageurl);//Fijamos los idiomas de trabajo
        if(possiblelanguages!=NULL)
          delete possiblelanguages;
        possiblelanguages=urlguesser->GivePossibleLanguages(&n_possiblelanguages);
       }
      
      diffbytagslevel=olddiffbytagslevel;
      diffbylinks=olddiffbylinks;
      diffbysize=olddiffbysize;
      diffbytags=olddiffbytags;
      diffbytreelevel=olddiffbytreelevel;
      diffbyext=olddiffbyext;
      n_similars=oldn_similars;
           
      resultcollect=true;
      dmanager.CacheRead(false); //La cache no se utilizará
      resultcollect=RECollect();//Ahora ya empezamos el nuevo recorrido de la web para actualizar la coleccion
      if(!resultcollect)
        result=false;
      for(i=0;i<n_languages;i++)
        fclose(filelisturl[i]);
     }
    if(!result)
     {//Removing new downloaded files
      fprintf(stderr,"Error: Collector aborted restoring the collection ");
     }
   }
   else
     result=false;
    
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::RefreshWebsite() -> %d\n",result);
#endif
     
  return(result); 
 }

bool Collector::PreviouslyDownloaded(WebPage *webpage)
 {
  bool result;
  unsigned int size,repeated,i;
  const unsigned int *searchresults;
  
  result=false;
  if(webpage!=NULL)
   {
    size=webpage->Size();
    repeated=oldorderedlist->Search(size);
    if(repeated>0)
     {
      searchresults=oldorderedlist->SearchResults();
      for(i=0;i<repeated && !result;i++)
       {
        if(Equals(webpage,(WebPage*)oldurlhash->Content(searchresults[i])))
          result=true;
       }
     }
   }
  
  return(result);
 }
 
bool Collector::RECollect()
 {
  bool result;
  char filetemp[30],filetemp2[100];
  int pid;
#ifndef HIDEMESSAGES
  struct tm *tmact; //Estructura para el manejo del tiempo en los logs
  time_t timeact;
  char timestring[kSizeAuxStrings]; //Cadena con la hora para los log
#endif  
  unsigned int i,hashsize;
  int j,k;
  int language; //Idioma de la pagina
  WebPage *webpage; //Pagina que se esta procesando actualmente
  WebPage *webpage2; //Pagina para comprobar las heuristicas
  WebPage website; //Pagina web asociada al sitio para comprobar enlaces externos
  WebPage auxwebpage; //Pagina auxiliar
  int whereintree; //Posicion que ocupa la pagina dentro del arbol
  const char **pagelinks; //Auxiliar para acceder a los links de una pagina
  const char *content; //Auxiliar para acceder al contenido de la pagina web
  char *auxurl,*auxurl2; //Auxiliar para almacenar url
  unsigned int nextnode; //Identificador del nodo que se esta recorriendo
  unsigned int treelevel; //Nivel en el arbol de la pagina que se esta insertando
  unsigned int actualnsimilars; //Numero de similares que tiene la pagina en proceso
  float similaritydistance; //Distancia entre dos ficheros que podrian ser similares
  bool waspreviouslydownloaded; //Indica si la pagina habia sido descargada en la ejecucion previa o no
  
  //Variables para la generacion de urls
  char **generatedvariants; //URLs generadas en base a los patrones
  unsigned int ngeneratedvariants; //Numero de URLs generadas
  unsigned int variantposition; //Posicion de la variante dentro de la hash de urls
   
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::Collect()\n");
#endif
  
  result=true;
  pid=getpid();
  sprintf(filetemp,"collectortmp%d",pid);
  website.FijarURL(urllist[0]);
  
  urlhash->InsertEnd(urllist[0],NULL);
  nextnode=urlhash->NextNode(); //Saltamos el nodo que acabamos de recorrer
  for(;nextnode!=0;nextnode=urlhash->NextNode())    
   {//Recorremos una a una el resto de paginas hasta haberlas recorrido todas
    auxurl=urlhash->Search(nextnode);
    auxwebpage.FijarURL(auxurl);
    if(dmanager.IsGoodExtension(auxwebpage.Extension()) && dmanager.DownloadURL(auxurl,filetemp))
     {
      downloaded++;
      if(downloaded%(unsigned int)(1/kSecondsBetweenRequest)==0)
       {
        Stats();
        sleep(1);
       }
      webpage=new WebPage();
      if(webpage->ImportConvertedPage(filetemp,auxurl))
       {
        //webpage->Save(kTestLoadFile);
        content=webpage->Content();
        if(content!=NULL/* && auxwebpage.Load(kTestLoadFile)*/)
         {//Ahora empezamos el recorrido de los enlaces de la pagina leida
          auxwebpage.Reset();
          pagelinks=webpage->Links();
          for(i=0;i<webpage->N_Links();i++)
           {
            auxurl2=CheckURL(pagelinks[i],webpage);//Limpiamos la URL
            if(auxurl2!=NULL)
             {
              if(!IsExternalLink(auxurl2) && urlhash->Search(auxurl2)==0)
               {//Solo insertamos si no estaba ya y si la web es interna
                auxwebpage.FijarURL(auxurl2);
                if(dmanager.IsGoodExtension(auxwebpage.Extension()))
                  urlhash->InsertEnd(auxurl2,NULL);
               }
              delete auxurl2;
              auxurl2=NULL;
             }
           }
          waspreviouslydownloaded=PreviouslyDownloaded(webpage);
          if(!waspreviouslydownloaded && webpage->TextSize()>=minimalcontentsize && !IsRepeatedWebpage(webpage))
           {
#ifndef HIDEMESSAGES      
            timeact=time(0);
            tmact=localtime(&timeact);
            sprintf(timestring,"%d:%d:%d (%d/%d/%d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,tmact->tm_mday,1+tmact->tm_mon,1900+tmact->tm_year);
            if(strlen(auxurl)>50)
              fprintf(stderr,"(%s):Guessing language ...%s...",timestring,auxurl+strlen(auxurl)-50);
             else
               fprintf(stderr,"(%s):Guessing language %s...",timestring,auxurl);      
            fflush(stderr);
#endif
            language=GuessLanguage(webpage);
#ifndef HIDEMESSAGES
            fprintf(stderr,"%d\n",language);
#endif          
            webpage->SetLanguage(language);
            if(language>=0)
             {
              webpage2=(WebPage*)urlhash->Content(urlhash->Search(auxurl));
              if(webpage2!=NULL)
               {//Borramos el contenido del arbol si lo hubiera puesto que se va a sobreescribir
                delete webpage2;
                webpage2=NULL;
               }
              whereintree=urlhash->InsertEnd(auxurl,(void*)webpage);
              orderedlist->Insert(webpage->Size(),whereintree);
                                 
              treelevel=urlhash->Depth(whereintree);
              InsertLanguagePage(whereintree,language,treelevel);

#ifndef HIDEMESSAGES
              timeact=time(0);
              tmact=localtime(&timeact);
              sprintf(timestring,"%02d:%02d:%02d (%4d/%02d/%02d)",tmact->tm_hour,tmact->tm_min,tmact->tm_sec,1900+tmact->tm_year,1+tmact->tm_mon,tmact->tm_mday);
              if(strlen(auxurl)>50)
                fprintf(stderr,"(%s):Looking for similars ...%s...",timestring,auxurl+strlen(auxurl)-50);
               else
                 fprintf(stderr,"(%s):Looking for similars %s...",timestring,auxurl);      
              fflush(stderr);
#endif
              //Comprobamos las heuristicas con los ficheros descargados hasta ahora                
              if(urlgenerator==NULL)
               {//Estrategia todos con todos
                for(i=0,actualnsimilars=0;i<n_languages;i++)
                 {
                  if(i!=langid[language])
                   {//Solo comprobamos con paginas en distinto idioma
                    for(j=0;j<kMaxDeepTree;j++)
                     {
                      if((signed)treelevel>(signed)(j-absrangetreelevel) && (signed)treelevel<(signed)(j+absrangetreelevel))
                       {//Solo se comprueban con paginas en una profundidad similar
                        for(k=0;k<n_langpagesperlevel[i][j];k++)
                         {
                          if(k<oldn_langpagesperlevel[i][j])
                           {
                            webpage2=(WebPage*)oldurlhash->Content(langpagesperlevel[i][j][k]);
                            if(webpage2!=NULL)
                             {
                              similaritydistance=Similars(webpage,webpage2,langpagesperlevel[i][j][k]);
                              if(similaritydistance!=-1)
                               {//Se añade a la lista de similares
                                n_similars++;
                                webpage->NewSimilar(webpage2->WhatLanguage(),webpage2->LocalFileId(),similaritydistance);
                                if(!quickmode)
                                 {
                                  webpage2->NewSimilar(language,n_languagepages[langid[language]],similaritydistance);
                                  webpage2->Refresh();
                                  webpage2->CleanContent();
                                 }
                                actualnsimilars++;
                               }
                             }
                           }
                           else
                            {
                             webpage2=(WebPage*)urlhash->Content(langpagesperlevel[i][j][k]);
                             if(webpage2!=NULL)
                              {
                               similaritydistance=Similars(webpage,webpage2,langpagesperlevel[i][j][k]);
                               if(similaritydistance!=-1)
                                {//Se añade a la lista de similares
                                 n_similars++;
                                 webpage->NewSimilar(webpage2->WhatLanguage(),webpage2->LocalFileId(),similaritydistance);
                                 if(!quickmode)
                                  {
                                   webpage2->NewSimilar(language,n_languagepages[langid[language]],similaritydistance);
                                   webpage2->Refresh();
                                   webpage2->CleanContent();
                                  }
                                 actualnsimilars++;
                                }
                              }
                            }
                         }
                       }
                     }
                   }
                 }
               }
               else
                {//Estrategia en base a patrones
                 if(generationtype==1)
                   generatedvariants=urlgenerator->GenerateVariants1(language,auxurl,&ngeneratedvariants);
                  else
                    generatedvariants=urlgenerator->GenerateVariants2(language,auxurl,&ngeneratedvariants);
                   
                 totalurlgenerated+=ngeneratedvariants;
                 //Buscamos las variantes
                 for(i=0,actualnsimilars=0;i<ngeneratedvariants;i++)
                  {
                   variantposition=urlhash->Search(generatedvariants[i]);
                   if(variantposition!=0)
                     variantposition=oldurlhash->Search(generatedvariants[i]);
                   if(variantposition!=0)
                    {//La variante existia en alguna de las hash
                     webpage2=(WebPage*)urlhash->Content(variantposition);
                     if(webpage2!=NULL)//Nos aseguramos de que ya estuviera descargada
                      {
                       totalgoodgenerations++;                   
                       if(webpage->WhatLanguage()!=webpage2->WhatLanguage())
                        {
                         similaritydistance=Similars(webpage,webpage2,webpage2->WhatLanguage());
                         if(similaritydistance!=-1)
                          {//Se añade a la lista de similares
                           n_similars++;
                           webpage->NewSimilar(webpage2->WhatLanguage(),webpage2->LocalFileId(),similaritydistance);
                           if(!quickmode)
                            {
                             webpage2->NewSimilar(language,n_languagepages[langid[language]],similaritydistance);
                             webpage2->Refresh();
                             webpage2->CleanContent();
                            }
                           actualnsimilars++;
                          }
                        }
                      }
                    }
                  }
                 delete generatedvariants;
                 generatedvariants=NULL;
                }
               
#ifndef HIDEMESSAGES
              fprintf(stderr,"%d similars found\n",actualnsimilars);
              fflush(stderr);
#endif                 
              sprintf(filetemp2,"%s/%s/%s/%06d.xml",website.Domain(),newdate,LanguageName(language),n_languagepages[langid[language]]);
              if(!webpage->Save(filetemp2))
               {
                if(!webpage->Save(filetemp2))
                 {
#ifndef HIDEMESSAGES
                  fprintf(stderr,"Error: Can't write file \"%s\"\n",filetemp2);
#endif               
                 }
               }
              webpage->CleanContent();//No nos quedamos con el contenido
              webpage->FreeUselessFields();//Liberamos los enlaces que ya no se usaran
              
              //Guardamos la pagina en el fichero con la lista de urls
              fprintf(filelisturl[langid[language]],"%s\n",auxurl);
              fflush(filelisturl[langid[language]]);                
             }
             else
              {
               discardedlang++;
#ifndef HIDEMESSAGES
               if(strlen(auxurl)>45)
                 fprintf(stderr,"Discarding page (language)...%s...\n",auxurl+strlen(auxurl)-45);
                else
                  fprintf(stderr,"Discarding page (language) %s...\n",auxurl);                    
#endif
                
               delete webpage;
               webpage=NULL;
              }
           }
           else
            {
             if(webpage->TextSize()<minimalcontentsize)
              {
               discardedsize++;
#ifndef HIDEMESSAGES
               if(strlen(auxurl)>45)
                 fprintf(stderr,"Discarding page (textsize)...%s...\n",auxurl+strlen(auxurl)-45);
                else
                  fprintf(stderr,"Discarding page (textsize) %s...\n",auxurl);                    
#endif
              }
              else
               {
                if(waspreviouslydownloaded)
                 {
                  alreadyprocessed++;
#ifndef HIDEMESSAGES
                  if(strlen(auxurl)>45)
                    fprintf(stderr,"Discarding page (not new)...%s...\n",auxurl+strlen(auxurl)-45);
                   else
                     fprintf(stderr,"Discarding page (not new) %s...\n",auxurl);                    
#endif
                    delete webpage;
                    webpage=NULL;
                 }
                 else
                  {
                   n_repeated++;
#ifndef HIDEMESSAGES      
                   if(strlen(auxurl)>45)
                     fprintf(stderr,"Discarding page (repeated)...%s...\n",auxurl+strlen(auxurl)-45);
                    else
                      fprintf(stderr,"Discarding page (repeated) %s...\n",auxurl);                    
#endif                  
                  }
               }
             delete webpage;
             webpage=NULL;
            }                             
         }
         else
          {
           delete webpage;
           webpage=NULL;
#ifndef HIDEMESSAGES
           if(strlen(auxurl)>45)
             fprintf(stderr,"Discarding page (size)...%s...",auxurl+strlen(auxurl)-45);
            else
              fprintf(stderr,"Discarding page (size) %s...",auxurl);      
#endif
          }           
       }
       else
        {
#ifndef HIDEMESSAGES       
         fprintf(stderr,"Error: Can't import %s\n",auxurl);
#endif     
         delete webpage;
         webpage=NULL;
        }
     }
     else
      {
#ifndef HIDEMESSAGES
       if(dmanager.IsGoodExtension(auxwebpage.Extension()))
         fprintf(stderr,"Error: Can't download %s\n",auxurl);
        else
          fprintf(stderr,"Error: Wrong extension %s\n",auxurl);
#endif         
      }
    delete auxurl;
    auxurl=NULL;
   }
    
  for(i=0,hashsize=urlhash->Size();i<hashsize;i++)
   {
    webpage=(WebPage*)urlhash->Content(i);
    if(webpage!=NULL)
     {
      urlhash->SetContentNULL(i);
      if(webpage!=NULL)
       {
        delete webpage;
        webpage=NULL;
       }
     }
   }
  fprintf(stdout,"Finished download from %s\n",urllist[0]);
  Stats();
  
  
  if(!Stats(statsfilename))
    fprintf(stderr,"Error: The stats file \"%s\" could not be updated (updating website was disabled)\n",statsfilename);
  
  //A modo de backup se almacena también el fichero de estadisticas en el directorio de la descarga
  Stats(statsfilename2);
  
  delete urlhash;
  urlhash=NULL;
  ResetStats();

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::Collect() -> %d\n",result);
#endif
      
  return(result);
 }

bool Collector::GenerateCorpus(const char *website,const char *directory)
 {
  bool result;
  char filename[kSizeAuxStrings],filename2[kSizeAuxStrings];
  char filecopy[kSizeAuxStrings];
  char languageurl[kNumLanguages];
  unsigned int i;
  int j;
  unsigned int totalfilesdownloaded;
  float totalfilesprocessed;
  WebPage *webpage; //Pagina auxiliar

  char linkbuffer[kSizeAuxStrings];
  int sizebuffer;  

#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::GenerateCorpus(%s)\n",directory);
#endif

  //Creamos el vector con la utilizacion o no de los diferentes idiomas
  //que soporta el identificador de idiomas
  for(i=0;i<kNumLanguages;i++)
    languageurl[i]='0';
  languageurl[i]='\0';
  for(i=0;i<n_languages;i++)
    languageurl[LanguageCode(langurl[i])]='1';
     
  sprintf(filename,"%s/%s",website,directory);
  mkdir(filename,0777);
  
  for(totalfilesdownloaded=0,i=0;i<n_languages;i++)
    totalfilesdownloaded+=oldn_languagepages[i];
  totalfilesprocessed=0;
    
  //Se crea la estructura de directorios adecuada para la recopilacion
  for(i=0,result=true;i<n_languages && result;i++)
   {
    //Creamos un directorio para cada idioma
    sprintf(filename,"%s/%s/%s",website,directory,langurl[i]);
    mkdir(filename,0777);
    
    //Cargamos todos los ficheros recolectados previamente         
    for(j=0;j<oldn_languagepages[i];j++)
     {
#ifndef HIDEMESSAGES
      fprintf(stdout,"\b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b: \b\bs\b\bu\b\bp\b\br\b\bo\b\bc\b\b \b\bo\b\bd\b\bn\b\ba\b\bg\b\br\b\ba\b\bC\b\b");
      fprintf(stdout,"Building temporary corpus: %.2f%%",100*(totalfilesprocessed+j)/totalfilesdownloaded);
      fflush(stdout);
#endif
      sprintf(filename,"%s/%s/%s/%06d.xml",website,previousdate,langurl[i],j+1);
      webpage=new WebPage();
           
      //Seguimos los enlaces simbolicos (en su caso) hacia el fichero
      bzero(linkbuffer,kSizeAuxStrings);
      do
       {
        sizebuffer=readlink(filename,linkbuffer,kSizeAuxStrings);
        if(sizebuffer>=0)
         {//Era un enlace simbolico
          strcpy(filename,linkbuffer);
         }
       } while(sizebuffer!=-1);
           
      if(webpage->LoadHeader(filename))
       {//El fichero existia y era correcto
        //Se copia en el nuevo directorio
        sprintf(filename2,"%s/%s/%s/%06d.xml",website,directory,langurl[i],j+1);
        sprintf(filecopy,"cp \"%s\" \"%s\"",filename,filename2);
        system(filecopy);
       }
      delete webpage;
      webpage=NULL;
     }
    totalfilesprocessed+=oldn_languagepages[i];
   }
#ifndef HIDEMESSAGES
  fprintf(stdout,"\n");
  fflush(stdout);
#endif

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::GenerateCorpus() -> %d\n",result);
#endif 
  return(result);
 }
 
bool Collector::CompareOnlyURLPatterns(char *patternsfile)
 {
  bool result;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::CompareOnlyURLPatterns(%s)\n",patternsfile);
#endif
  
  if(urlgenerator!=NULL)
    delete urlgenerator;
  urlgenerator=new URLGenerator();
  if(!(urlgenerator->Load(patternsfile)))
   {
    delete urlgenerator;
    urlgenerator=NULL;
    result=false;
    fprintf(stderr,"Error: Patterns file \'%s\' could not be loaded\n",patternsfile);
   }
   else
     result=true;
  
#ifdef TRAZANDO_Collector
  if(result)
    fprintf(stderr,"EndCollector::CompareOnlyURLPatterns(%s) -> true\n",patternsfile);
   else
     fprintf(stderr,"EndCollector::CompareOnlyURLPatterns(%s) -> false\n",patternsfile);
#endif
  return(result); 
 }

bool Collector::IsExternalLink(const char *url)
 {
  bool result;
  unsigned int i;
  
  if(url!=NULL)
   {
    if(internalurllist==NULL)
     {//Inicializamos las paginas para comparar
      internalurllist=new WebPage*[n_urllist];
      for(i=0;i<n_urllist;i++)
       {
        internalurllist[i]=new WebPage;
        internalurllist[i]->FijarURL(urllist[i]);
       }
     }
    result=true;
    for(i=0;i<n_urllist && result;i++)
     {
      if(!internalurllist[i]->ExternalLink(url))
        result=false;
     }
   }
   else
     result=true;
  return(result);
 }

/////////////////////////////////////////////////////////////// 
//Lista de funciones para la carga del fichero de configuracion
///////////////////////////////////////////////////////////////
bool Collector::LoadConfiguration(const char* file)
 {
  bool result;
  bool finished;
  int ret;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::LoadConfiguration(%s)\n",file);
#endif
  
  reader=xmlReaderForFile(file,NULL,0);
  if(reader==NULL)
   {
    result=false;
   }
   else
    {
     result=true;
     finished=false;
     n_languages=0;     
     ret=xmlTextReaderRead(reader);
     while(ret==1 && result && !finished)
      {
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(tag!=NULL)
        {
         if(strcmp(tag,"COLLECTOR")==0) 
          {//Es el DOCTYPE
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"collector")==0)
             {
              delete tag;
              tag=NULL;
              result=ProcessCOLLECTOR();
              finished=true;
             }
             else
              {
               if(strcmp(tag,"#text")==0)
                {
                 delete tag;
                 tag=NULL;
                }
                else
                 {
                  delete tag;
                  tag=NULL;
                  result=false;
                  fprintf(stderr,"File \"%s\" does not contain <collector> tag.\n",file);
                 }
              }
           }
        }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }

     if(ret!=1)
      {
       fprintf(stderr,"Error: Parse error at the end of input.\n");
       result=false;
      }
     xmlFreeTextReader(reader);
     //xmlCleanupParser();
    }
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::LoadConfiguration(%s) -> %d\n",file,result);
#endif
  return(result);
 } 

bool Collector::ProcessCOLLECTOR()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessCOLLECTOR()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"languages")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessLANGUAGES();
           }
           else
            {
             if(strcmp(tag,"heuristics")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessHEURISTICS();
              }
              else
               {
                if(strcmp(tag,"urlgeneration")==0)
                 {
                  delete tag;
                  tag=NULL;
                  result=ProcessURLGENERATION();
                 }
                 else
                  {
                   if(strcmp(tag,"languageguesser")==0)
                    {
                     delete tag;
                     tag=NULL;
                     result=ProcessLANGUAGEGUESSER();
                    }
                    else
                     {
                      if(strcmp(tag,"mode")==0)
                       {
                        delete tag;
                        tag=NULL;
                        result=ProcessMODE();
                        finished=true;//Se ha procesado la ultima etiqueta que se esperaba
                       }
                       else
                        {
                         if(strcmp(tag,"collector")==0)
                          {//Ignoramos el doctype
                           delete tag;
                           tag=NULL;
                          }
                          else
                           {
                            delete tag;
                            tag=NULL;
                            result=false;
                           }
                        }
                     }
                  }
               }
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessCOLLECTOR() -> %d\n",result);
#endif     
  return(result);
 }


bool Collector::ProcessLANGUAGES()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLANGUAGES()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      if(n_languages<1)//Debe haber uno o mas idiomas
        result=false;
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"language")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessLANGUAGE();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessLANGUAGES() -> %d\n",result);
#endif     
  return(result);
 }

bool Collector::ProcessLANGUAGE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char **aux; //Auxiliar para el crecimiento del vector de idiomas
  double *aux2; //Auxiliar para el crecimiento del vector de tamaños normalizados
  char *attribcode; //Valor del atributo code una vez convertido
  char *attribsize; //Valor del atributo normalizedsize una vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  unsigned int i,j;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLANGUAGES()\n");
#endif
    
  nameattrib=xmlCharStrdup("code");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribcode=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("normalizedsize");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribsize=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attribcode!=NULL)
   {
    result=true;
    aux=new char*[n_languages+1];
    aux2=new double[n_languages+1];
    for(j=0;j<n_languages;j++)
     {
      aux[j]=langurl[j];
      aux2[j]=normalizedsize[j];
     }
    aux[j]=attribcode;
    if(attribsize==NULL)
     {
      aux2[j]=0;
     }
     else
      {//Cogemos el valor normalizado
       for(i=0,aux2[j]=0,dotplace=1000;result && i<strlen(attribsize);i++)
        {                           
         if(!isdigit(attribsize[i]) && attribsize[i]!='.')
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (normalizedsize)\n");
          }
          else
           {
            if(attribsize[i]!='.')
              aux2[j]=aux2[j]*10+(attribsize[i]-48);
             else
               dotplace=i;
           }
        }
       if(result)
        {
         for(i--;(signed)i>dotplace;i--)
           aux2[j]=aux2[j]/10;
        }
       delete attribsize;
       attribsize=NULL;
      }
    
    delete langurl;
    langurl=aux;
    delete normalizedsize;;
    normalizedsize=aux2;
    n_languages++;
   }
   else
    {
     result=false;
    }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessLANGUAGES() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool Collector::ProcessHEURISTICS()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLANGUAGES()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  nappliedheuristics=0;
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
      finished=true;
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"size")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessSIZE();
           }
           else
            {
             if(strcmp(tag,"tags")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessTAGS();
              }
              else
               {
                if(strcmp(tag,"links")==0)
                 {
                  delete tag;
                  tag=NULL;
                  result=ProcessLINKS();
                 }
                 else
                  {
                   if(strcmp(tag,"images")==0)
                    {
                     delete tag;
                     tag=NULL;
                     result=ProcessIMAGES();
                    }
                    else
                     {
                      if(strcmp(tag,"tagslevel")==0)
                       {
                        delete tag;
                        tag=NULL;
                        result=ProcessTAGSLEVEL();
                       }
                       else
                        {
                         if(strcmp(tag,"depth")==0)
                          {
                           delete tag;
                           tag=NULL;
                           result=ProcessDEPTH();
                          }
                          else
                           {
                            if(strcmp(tag,"extension")==0)
                             {
                              delete tag;
                              tag=NULL;
                              result=ProcessEXTENSION();
                             }
                             else
                              {
                               delete tag;
                               tag=NULL;
                               result=false;
                              }
                           }
                        }
                     }                     
                  }
               }
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessHEURISTICS() -> %d\n",result);
#endif     
  return(result);
 }

bool Collector::ProcessSIZE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  double auxd; //Auxiliar para la lectura de numeros enteros
  char *attrib; //Valor del atributo codeuna vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessSIZE()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (size -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applysize=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applysize=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (size -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("generalsize");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,auxd=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (size -> generalsize)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxd=auxd*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxd=auxd/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     else
       auxd=1;
    for(i=0;i<n_languages;i++)//Si hay algun <language> sin normalizedsize se pone el por defecto
     {
      if(normalizedsize[i]==0)
        normalizedsize[i]=auxd;
     }
   }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("minimal");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    if(attrib!=NULL)
     {//Cogemos el valor del atributo
      for(i=0,minimalcontentsize=0;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (size -> minimal)\n");
         }
         else
           minimalcontentsize=minimalcontentsize*10+(attrib[i]-48);
       }
      delete attrib;
      attrib=NULL; 
     }
     else
      {//No se descarta ninguna por tamaño minimo
       minimalcontentsize=0;
      }
   }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("range");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (size -> range)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (size -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
    
    if(result)
     {
      if(auxabsrange==0 && auxrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute range or absrange required (size)\n");
       }
       else
        {
         if(auxrange!=0)
           rangesize=auxrange;
          else
            rangesize=1;//Le damos el margen máximo
         if(auxabsrange!=0)
           absrangesize=auxabsrange;
          else
            absrangesize=kVeryBigValue;//Le damos un valor enorme
        }
     }
     
    if(result)
     {
      nameattrib=xmlCharStrdup("weight");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxrange=0;
      if(attrib!=NULL)
       {//Cogemos el valor normalizado
        for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (size -> weight)\n");
           }
           else
            {
             if(attrib[i]!='.')
               auxrange=auxrange*10+(attrib[i]-48);
              else
                dotplace=i;
            }      
         }
        if(result)
         {
          for(i--;(signed)i>dotplace;i--)
            auxrange=auxrange/10;
          weightsize=auxrange;
         }
        delete attrib;
        attrib=NULL;     
       }
     }   
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessSIZE() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool Collector::ProcessTAGS()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessTAGS()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (tags -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applyntags=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applyntags=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (tags -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("range");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (tags -> range)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (tags -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
    
    if(result)
     {
      if(auxabsrange==0 && auxrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute range or absrange required (tags)\n");
       }
       else
        {
         if(auxrange!=0)
           rangentags=auxrange;
          else
            rangentags=1;//Le damos el margen máximo
         if(auxabsrange!=0)
           absrangentags=auxabsrange;
          else
            absrangentags=kVeryBigValue;//Le damos un valor enorme
        }
     }
     
    if(result)
     {
      nameattrib=xmlCharStrdup("weight");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxrange=0;
      if(attrib!=NULL)
       {//Cogemos el valor normalizado
        for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (tags -> weight)\n");
           }
           else
            {
             if(attrib[i]!='.')
               auxrange=auxrange*10+(attrib[i]-48);
              else
                dotplace=i;
            }      
         }
        if(result)
         {
          for(i--;(signed)i>dotplace;i--)
            auxrange=auxrange/10;
          weightntags=auxrange;
         }
        delete attrib;
        attrib=NULL;     
       }     
     }
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessTAGS() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessLINKS()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLINKS()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (links -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applynlinks=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applynlinks=false;
          nappliedheuristics++;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (links -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("range");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (links -> range)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (links -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
    
    if(result)
     {
      if(auxabsrange==0 && auxrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute range or absrange required (links)\n");
       }
       else
        {
         if(auxrange!=0)
           rangenlinks=auxrange;
          else
            rangenlinks=1;//Le damos el margen máximo
         if(auxabsrange!=0)
           absrangenlinks=auxabsrange;
          else
            absrangenlinks=kVeryBigValue;//Le damos un valor enorme
        }
     }
     
    if(result)
     {
      nameattrib=xmlCharStrdup("weight");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxrange=0;
      if(attrib!=NULL)
       {//Cogemos el valor normalizado
        for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (links -> weight)\n");
           }
           else
            {
             if(attrib[i]!='.')
               auxrange=auxrange*10+(attrib[i]-48);
              else
                dotplace=i;
            }      
         }
        if(result)
         {
          for(i--;(signed)i>dotplace;i--)
            auxrange=auxrange/10;
          weightnlinks=auxrange;
         }
        delete attrib;
        attrib=NULL;     
       }     
     }
     
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessLINKS() -> %d\n",result);
#endif
  
  return(result);
 } 

bool Collector::ProcessDEPTH()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxabsrange,auxrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessDEPTH()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (depth -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applydepth=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applydepth=false;
          nappliedheuristics++;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (depth -> apply)\n");
           delete attrib;
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (depth -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;
     }
    
    if(result)
     {
      if(auxabsrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute absrange required (depth)\n");
       }
       else
         absrangetreelevel=(int)auxabsrange;
     }
     
    if(result)
     {
      nameattrib=xmlCharStrdup("weight");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxrange=0;
      if(attrib!=NULL)
       {//Cogemos el valor normalizado
        for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (depth -> weight)\n");
           }
           else
            {
             if(attrib[i]!='.')
               auxrange=auxrange*10+(attrib[i]-48);
              else
                dotplace=i;
            }      
         }
        if(result)
         {
          for(i--;(signed)i>dotplace;i--)
            auxrange=auxrange/10;
          weightdepth=auxrange;
         }
        delete attrib;
        attrib=NULL;     
       }     
     }     
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessDEPTH() -> %d\n",result);
#endif
  
  return(result);
 } 
 
bool Collector::ProcessIMAGES()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessIMAGES()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (images -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applynimages=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applynimages=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (images -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("range");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (images -> range)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (images -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
    
    if(result)
     {
      if(auxabsrange==0 && auxrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute range or absrange required (images)\n");
       }
       else
        {
         if(auxrange!=0)
           rangenimages=auxrange;
          else
            rangenimages=1;//Le damos el margen máximo
         if(auxabsrange!=0)
           absrangenimages=auxabsrange;
          else
            absrangenimages=kVeryBigValue;//Le damos un valor enorme
        }
     }
    
    if(result)
     {
      nameattrib=xmlCharStrdup("weight");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxrange=0;
      if(attrib!=NULL)
       {//Cogemos el valor normalizado
        for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (images -> weight)\n");
           }
           else
            {
             if(attrib[i]!='.')
               auxrange=auxrange*10+(attrib[i]-48);
              else
                dotplace=i;
            }      
         }
        if(result)
         {
          for(i--;(signed)i>dotplace;i--)
            auxrange=auxrange/10;
          weightnimages=auxrange;
         }
        delete attrib;
        attrib=NULL;     
       }     
     }
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessIMAGES() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessEXTENSION()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessEXTENSION()\n");
#endif
  
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (extension -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applyextension=true;
       nappliedheuristics++;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applyextension=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (extension -> apply)\n");
           delete attrib;    
          }
       }
    }  

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessEXTENSION() -> %d\n",result);
#endif
  return(result);
 }

bool Collector::ProcessTAGSLEVEL()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria  
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo code una vez convertido
  char *tag; //Tag una vez convertido
  int ret,type;
  int n_levels; //Numero de etiquetas level que se han leido
  int dotplace; //Indica la posicion del punto en un numero decimal
  double auxrange; //Auxiliares para la lectura de rangos
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessTAGSLEVEL()\n");
#endif
  
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (tagslevel -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applyntagslevel=true;
       nappliedheuristics++;
       delete attrib;
       
       //Inicializamos los rangos y valores absolutos de los niveles
       for(n_levels=0;n_levels<kMaxLevelTags;n_levels++)
        {
         rangentagslevel[n_levels]=1;
         absrangentagslevel[n_levels]=kVeryBigValue;
        }
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applyntagslevel=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (tagslevel -> apply)\n");
           delete attrib;    
          }
       }
    }  
  
  if(result)
   {
    nameattrib=xmlCharStrdup("weight");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (tagslevel -> weight)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
           else
             dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
        weightntagslevel=auxrange;
       }
      delete attrib;
      attrib=NULL;     
     }     
   }
    
  if(result)
   {
    n_levels=0;
    finished=false;
    ret=xmlTextReaderRead(reader);
    while(result && ret==1 && !finished)
     {
      type=xmlTextReaderNodeType(reader);
      if(type==XML_READER_TYPE_END_ELEMENT)
       {
        if(n_levels<1)//Debe haber uno o mas niveles
          result=false;
        finished=true;
       }
       else
        {    
         xtag=xmlTextReaderConstName(reader);
         tag=FixLibXML2Encoding(xtag);
         if(strcmp(tag,"#text")==0)
          {
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"level")==0)
             {
              delete tag;
              tag=NULL;
              result=ProcessLEVEL();
              n_levels++;
             }
             else
              {
               delete tag;
               tag=NULL;
               result=false;
              }
           }
         if(result && !finished)
           ret=xmlTextReaderRead(reader);
        }
     }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessTAGSLEVEL() -> %d\n",result);
#endif     
  return(result);
 }
 
bool Collector::ProcessLEVEL()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  unsigned int auxvalue; //Indica la posicion del punto en un numero decimal
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  int dotplace; //Indica la posicion del punto en un numero decimal
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLEVEL()\n");
#endif
    
  result=true;
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  auxvalue=0;
  if(attrib!=NULL)
   {//Cogemos el valor del atributo
    for(i=0,auxvalue=0;result && i<strlen(attrib);i++)
     {                           
      if(!isdigit(attrib[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file (level -> value)\n");
       }
       else
         auxvalue=auxvalue*10+(attrib[i]-48);
     }
    delete attrib;
    attrib=NULL;
    if(auxvalue>=kMaxLevelTags)
     {
      result=false;
      fprintf(stderr,"Error: Max value = %d (level -> value)\n",kMaxLevelTags);
     }
   }
   else
    {//No se descarta ninguna por tamaño minimo
     result=false;
     fprintf(stderr,"Error: Attribute value required (level)\n");
    }
    
  if(result)
   {
    nameattrib=xmlCharStrdup("range");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (level -> range)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxrange=auxrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxrange=auxrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     
    nameattrib=xmlCharStrdup("absrange");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxabsrange=0;
    if(attrib!=NULL)
     {//Cogemos el valor normalizado
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (level -> absrange)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxabsrange=auxabsrange*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxabsrange=auxabsrange/10;
       }
      delete attrib;
      attrib=NULL;     
     }
    
    if(result)
     {
      if(auxabsrange==0 && auxrange==0)
       {
        result=false;
        fprintf(stderr,"Error: Attribute range or absrange required (level)\n");
       }
       else
        {
         if(auxrange!=0)
           rangentagslevel[auxvalue]=auxrange;
          else
            rangentagslevel[auxvalue]=1;//Le damos el margen máximo
         if(auxabsrange!=0)
           absrangentagslevel[auxvalue]=auxabsrange;
          else
            absrangentagslevel[auxvalue]=kVeryBigValue;//Le damos un valor enorme
        }
     }
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessLEVEL() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool Collector::ProcessURLGENERATION()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  int n_generation; //Numero de etiquetas level que se han leido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessURLGENERATION()\n");
#endif
  
  result=true;
  finished=false;
  n_generation=0;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      if(n_generation<1)//Debe haber uno o mas niveles
        result=false;
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"generation")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessGENERATION();
            n_generation++;
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessURLGENERATION() -> %d\n",result);
#endif     
  return(result);
 }
 
bool Collector::ProcessGENERATION()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  unsigned int gentype; //Indica el tipo de generacion que se realizara
  double auxrange,auxabsrange; //Auxiliares para la lectura de rangos
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessGENERATION()\n");
#endif
    
  result=true;
  gentype=0;
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (generation -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       applyurlgenerator=true;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          applyurlgenerator=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (generation -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("type");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxrange=0;
    if(attrib!=NULL)
     {
      if(strcmp(attrib,"1")==0)
        gentype=1;
       else
        {
         if(strcmp(attrib,"2")==0)
           gentype=2;
          else
           {
            result=false;
            fprintf(stderr,"Error: Unknown generation type (generation -> type)\n");           
           }
        }
      delete attrib;
      attrib=NULL;     
     }
     else
      {
       result=false;
       fprintf(stderr,"Error: Invalid format while loading file (generation -> type)\n");
      }
     
    if(result)
     {
      nameattrib=xmlCharStrdup("filename");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=FixLibXML2Encoding(xattrib);
      xmlFree(xattrib);
      auxabsrange=0;
      if(attrib!=NULL)
       {
        if(applyurlgenerator)
         {
          result=CompareOnlyURLPatterns(attrib);
          if(result)
           {
            if(gentype==2)
              generationtype=2;
             else
               generationtype=1;
           }
           else
            {
             result=false;
             fprintf(stderr,"Error: File not found (generation -> filename)\n");
            }
         }
        delete attrib;
        attrib=NULL;
       }
     }    
   }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessGENERATION() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool Collector::ProcessLANGUAGEGUESSER()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  int n_guessers; //Numero de etiquetas guesser que se han leido
  float auxweight;
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessLANGUAGEGUESSER()\n");
#endif
  
  result=true;
  finished=false;
  n_guessers=0;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      if(n_guessers<1)//Debe haber uno o mas guessers
        result=false;
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"guesser")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessGUESSER();
            n_guessers++;
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

  if(result)
   {
    //Ahora normalizamos el peso de los guessers leidos
    for(auxweight=0,i=0,result=false;i<kNumberofGuessers;i++)
     {
      if(applyguessers[i])
       {
        result=true;//Hay al menos uno activo
        auxweight+=guesserweight[i];
       }
     }
    if(result)
     {
      for(i=0;i<kNumberofGuessers;i++)
       {
        if(applyguessers[i])
          guesserweight[i]=guesserweight[i]/auxweight;
       }
     }
     else
       fprintf(stderr,"Error: All guessers deactivated (languagegueser)\n");
   } 
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessLANGUAGEGUESSER() -> %d\n",result);
#endif
  return(result);
 }

bool Collector::ProcessGUESSER()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  unsigned int guessertype; //Indica el tipo de guesser que se esta leyendo
  double auxweight; //Auxiliares para la lectura del peso
  bool isapplied; //Indica si se va a usar o no el guesser actual
  int dotplace; //Indica la posicion del punto en un numero decimal
  unsigned int i,auxvalue;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessGUESSER()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  guessertype=0;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (guesser -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       isapplied=true;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          isapplied=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (guesser -> apply)\n");
           delete attrib;    
          }
       }
    }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("type");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    if(attrib!=NULL)
     {
      if(strcmp(attrib,"trigram")==0)
        guessertype=kIdTrigramGuesser;
       else
        {
         if(strcmp(attrib,"word")==0)
           guessertype=kIdWordGuesser;
          else
           {
            if(strcmp(attrib,"url")==0)
              guessertype=kIdURLGuesser;
             else
              {
               result=false;
               fprintf(stderr,"Error: Unknown generation type (generation -> type)\n");           
              }
           }
        }
      delete attrib;
      attrib=NULL;     
     }
   }
  
  if(result)
   {
    nameattrib=xmlCharStrdup("weight");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    auxweight=0;
    if(attrib!=NULL)
     {
      for(i=0,dotplace=1000;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]) && attrib[i]!='.')
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (guesser -> weight)\n");
         }
         else
          {
           if(attrib[i]!='.')
             auxweight=auxweight*10+(attrib[i]-48);
            else
              dotplace=i;
          }      
       }
      if(result)
       {
        for(i--;(signed)i>dotplace;i--)
          auxweight=auxweight/10;
       }
      delete attrib;
      attrib=NULL;     
     }
     else
      {
       result=false;
       fprintf(stderr,"Error: Attribute weight required (guesser)\n");      
      }
   }
   
  if(result)
   {
    nameattrib=xmlCharStrdup("filename");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    if(attrib!=NULL)
     {
#ifdef CONF_FILE
      //Guesser file is obtained from a different path
      char *auxattrib; //Auxiliar to build the new path
      
      auxattrib=new char[2+strlen(attrib)+strlen(CONF_FILE)];
      sprintf(auxattrib,"%s/%s",CONF_FILE,attrib);
      delete attrib;
      attrib=auxattrib;
      auxattrib=NULL;
#endif

      
      if(isapplied)
       {
        applyguessers[guessertype]=true;
        guesserweight[guessertype]=auxweight;
        switch(guessertype)       
         {
          case kIdTrigramGuesser: trigramguesser=new LangGuesser<TrigramLangGuesser>(kNumLanguages);
                                  if(!trigramguesser->Load(attrib))
                                   {
                                    applyguessers[guessertype]=false;
                                    fprintf(stderr,"Error: TrigramGuesser file could not be loaded -> TrigramGuesser deactivated\n");
                                   }
                                  break;
                                  
          case kIdWordGuesser: wordguesser=new LangGuesser<WordLangGuesser>(kNumLanguages);
                               if(!wordguesser->Load(attrib))
                                {
                                 applyguessers[guessertype]=false;
                                 fprintf(stderr,"Error: WordGuesser file could not be loaded -> WordGuesser deactivated\n");
                                }
                               break;
                               
          case kIdURLGuesser: urlguesser=new LangGuesser<URLLangGuesser>(kNumLanguages);
                              if(!urlguesser->Load(attrib))
                               {
                                applyguessers[guessertype]=false;
                                fprintf(stderr,"Error: URLGuesser file could not be loaded -> URLGuesser deactivated\n");
                               }
                              break;
         }
       }
       else
        {//No se usa el guesser de este tipo
         applyguessers[guessertype]=false;
        }
      delete attrib;
      attrib=NULL;     
     }    
   }
   
  if(result)
   {
    nameattrib=xmlCharStrdup("start");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    if(attrib!=NULL)
     {
      for(i=0,auxvalue=0;result && i<strlen(attrib);i++)
       {                           
        if(!isdigit(attrib[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (level -> value)\n");
         }
         else
           auxvalue=auxvalue*10+(attrib[i]-48);
       }
      delete attrib;
      attrib=NULL;
      if(guessertype==kIdURLGuesser)
        urlguesserstart=auxvalue;
     }
   }
   
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessGUESSER() -> %d\n",result);
#endif  
  return(result);
 }
 
bool Collector::ProcessMODE()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessMODE()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"quickmode")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessQUICKMODE();
           }
           else
            {
             if(strcmp(tag,"cache")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessCACHE();
              }
              else
               {
                delete tag;
                tag=NULL;
                result=false;
               }
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessMODE() -> %d\n",result);
#endif     
  return(result);
 }

bool Collector::ProcessQUICKMODE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessQUICKMODE()\n");
#endif
    
  nameattrib=xmlCharStrdup("apply");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (generation -> apply)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       quickmode=true;
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          quickmode=false;
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (quickmode -> apply)\n");
           delete attrib;    
          }
       }
    }  
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessQUICKMODE() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessCACHE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo codeuna vez convertido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessQUICKMODE()\n");
#endif
    
  nameattrib=xmlCharStrdup("build");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  result=true;
  if(attrib==NULL || (attrib[0]!='o' && attrib[0]!='O'))
   {
    result=false;
    fprintf(stderr,"Error: Invalid format while loading file (cache -> build)\n");
    if(attrib!=NULL)
      delete attrib;
   }
   else
    {//Miramos si el valor es on o off
     if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
      {
       dmanager.CacheGeneration(true);
       delete attrib;
      }
      else
       {
        if((attrib[1]=='f' || attrib[1]=='F') &&
           (attrib[2]=='f' || attrib[2]=='F') &&
           attrib[3]=='\0')
         {
          dmanager.CacheGeneration(false);
          delete attrib;
         }
         else
          {
           result=false;
           fprintf(stderr,"Error: Invalid format while loading file (cache -> build)\n");
           delete attrib;    
          }
       }
    }  
  
  if(result)
   {
    nameattrib=xmlCharStrdup("use");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    
    if(attrib==NULL && (attrib[0]=='o' || attrib[0]=='O'))
     {
      result=false;
      fprintf(stderr,"Error: Invalid format while loading file (cache -> use)\n");
      if(attrib!=NULL)
        delete attrib;
     }
     else
      {//Miramos si el valor es on o off
       if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
        {
         dmanager.CacheRead(true);
         delete attrib;
        }
        else
         {
          if((attrib[1]=='f' || attrib[1]=='F') &&
             (attrib[2]=='f' || attrib[2]=='F') &&
             attrib[3]=='\0')
           {
            dmanager.CacheRead(false);
            delete attrib;
           }
           else
            {
             result=false;
             fprintf(stderr,"Error: Invalid format while loading file (cache -> use)\n");
             delete attrib;    
            }
         }
      } 
   }

  if(result)
   {
    nameattrib=xmlCharStrdup("exclusivity");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=FixLibXML2Encoding(xattrib);
    xmlFree(xattrib);
    
    if(attrib==NULL && (attrib[0]=='o' || attrib[0]=='O'))
     {
      result=false;
      fprintf(stderr,"Error: Invalid format while loading file (cache -> exclusivity)\n");
      if(attrib!=NULL)
        delete attrib;
     }
     else
      {//Miramos si el valor es on o off
       if((attrib[1]=='n' || attrib[1]=='N') && attrib[2]=='\0')
        {
         dmanager.OnlyCache(true);
         delete attrib;
        }
        else
         {
          if((attrib[1]=='f' || attrib[1]=='F') &&
             (attrib[2]=='f' || attrib[2]=='F') &&
             attrib[3]=='\0')
           {
            dmanager.OnlyCache(false);
            delete attrib;
           }
           else
            {
             result=false;
             fprintf(stderr,"Error: Invalid format while loading file (cache -> use)\n");
             delete attrib;    
            }
         }
      } 
   }   
      
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessQUICKMODE() -> %d\n",result);
#endif
  return(result);
 }

/////////////////////////////////////////////////////////////// 
//Lista de funciones para la carga del fichero de estadisticas
///////////////////////////////////////////////////////////////
bool Collector::LoadResultsFile(const char* file)
 {
  bool result;
  bool finished;
  int ret;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int i,j;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::LoadResultsFile(%s)\n",file);
#endif
  
  reader=xmlReaderForFile(file,NULL,0);
  if(reader==NULL)
    result=false;
   else
    {
     result=true;
     finished=false;
     n_languages=0;     
     ret=xmlTextReaderRead(reader);
     while(ret==1 && result && !finished)
      {
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(tag!=NULL)
        {
         if(strcmp(tag,"PACOSTATS")==0) 
          {//Es el DOCTYPE
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"pacostats")==0)
             {
              if(xmlTextReaderNodeType(reader)==XML_READER_TYPE_ELEMENT)
               {
                result=ProcessPACOSTATS();
                finished=true;
               }              
              delete tag;
              tag=NULL;
             }
             else
              {
               if(strcmp(tag,"#text")==0)
                {
                 delete tag;
                 tag=NULL;
                }
                else
                 {
                  delete tag;
                  tag=NULL;
                  result=false;
                  fprintf(stderr,"File \"%s\" does not contain <pacostats> tag.\n",file);
                 }
              }
           }
        }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }

     if(ret!=1)
      {
       fprintf(stderr,"Error: Parse error at the end of input.\n");
       result=false;
      }
     xmlFreeTextReader(reader);
     //xmlCleanupParser();
    }
    
  if(result)
   {//Liberamos la memoria del objeto en caso de que se hubiera cargado previamente
    if(n_files!=NULL)
     {
      delete n_files;
      n_files=NULL;
     }
    if(filelisturl!=NULL)
     {
      delete filelisturl;
      filelisturl=NULL;
     }
    if(languagepages!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
       {
        if(languagepages[i]!=NULL)
          delete languagepages[i];
       }
      delete languagepages;
      languagepages=NULL;
     }
    if(n_sizelanguagepages!=NULL)
     {
      delete n_sizelanguagepages;
      n_sizelanguagepages=NULL;
     }
    if(langpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
       {
        for(j=0;j<kMaxDeepTree;j++)
         {
          if(langpagesperlevel[i][j]!=NULL)
            delete langpagesperlevel[i][j];
         }       
        delete langpagesperlevel[i];
       }
      delete langpagesperlevel;
      langpagesperlevel=NULL;
     }
    if(n_sizelangpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
        delete n_sizelangpagesperlevel[i];
      delete n_sizelangpagesperlevel;
      n_sizelangpagesperlevel=NULL;
     }
    if(n_langpagesperlevel!=NULL)
     {
      for(i=0;i<(signed)n_languages;i++)
        delete n_langpagesperlevel[i];
     delete n_langpagesperlevel;
      n_langpagesperlevel=NULL;
     }  
    //Reservamos la memoria para la recoleccion
    n_files=new unsigned int[n_languages];
    filelisturl=new FILE*[n_languages];
    languagepages=new int*[n_languages];  
    n_sizelanguagepages=new int[n_languages];
    langpagesperlevel=new int**[n_languages];
    n_sizelangpagesperlevel=new int*[n_languages];
    n_langpagesperlevel=new int*[n_languages];
    for(i=0;i<(signed)n_languages;i++)
     {
      n_files[i]=0;
      languagepages[i]=new int[kNumPagesperLanguage];
      n_sizelanguagepages[i]=kNumPagesperLanguage;
      langpagesperlevel[i]=new int*[kMaxDeepTree];
      n_langpagesperlevel[i]=new int[kMaxDeepTree];
      n_sizelangpagesperlevel[i]=new int[kMaxDeepTree];
      for(j=0;j<kMaxDeepTree;j++)
       {
        langpagesperlevel[i][j]=NULL;
        n_langpagesperlevel[i][j]=0;
        n_sizelangpagesperlevel[i][j]=0;
       }
     }
   }
   
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::LoadResultsFile(%s) -> %d\n",file,result);
#endif
  return(result);
 }
 
bool Collector::ProcessPACOSTATS()
 {
  bool result;
  bool finished;
  bool subtagsfound[7]; //Indica cuales de las posibles etiquetas se han encontrado
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  unsigned int i;
  
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib,*attrib2; //Valores de los atributos hred y date una vez convertidos
    
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessPACOSTATS()\n");
#endif
  
  for(i=0;i<7;i++)
    subtagsfound[i]=false;
    
  nameattrib=xmlCharStrdup("href");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("date");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib2=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attrib!=NULL && attrib2!=NULL)
   {
    if(urllist!=NULL)
     {//Borramos la anterior lista de urls
      for(i=0;i<n_urllist;i++)
       {
        if(urllist[i]!=NULL)
          delete urllist[i];
       }
       delete urllist;
     }
    
    n_urllist=1;
    urllist=new char*[n_urllist];
    urllist[0]=attrib;
    processedpreviously=0;

    strcpy(previousdate,attrib2);
    delete attrib2;
    attrib2=NULL;
        
    result=true;
    finished=false;
    ret=xmlTextReaderRead(reader);
    while(result && ret==1 && !finished)
     {
      type=xmlTextReaderNodeType(reader);
      if(type==XML_READER_TYPE_END_ELEMENT)
       {
        finished=true;
       }
       else
        {
         xtag=xmlTextReaderConstName(reader);
         tag=FixLibXML2Encoding(xtag);
         if(strcmp(tag,"#text")==0)
          {
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"nonexternalwebsites")==0)
             {
              delete tag;
              tag=NULL;
              result=ProcessNONEXTERNALWEBSITES();
              subtagsfound[0]=true;
             }
             else
              {
               if(strcmp(tag,"targetlanguages")==0)
                {
                 delete tag;
                 tag=NULL;
                 result=ProcessTARGETLANGUAGES();
                 subtagsfound[1]=true;
                }
                else
                 {
                  if(strcmp(tag,"downloading")==0)
                   {
                    delete tag;
                    tag=NULL;
                    result=ProcessDOWNLOADING();
                    subtagsfound[2]=true;
                   }
                   else
                    {
                     if(strcmp(tag,"discarded")==0)
                      {
                       delete tag;
                       tag=NULL;
                       result=ProcessDISCARDED();
                       subtagsfound[3]=true;
                      }
                      else
                       {
                        if(strcmp(tag,"urlgenerationapplied")==0)
                         {
                          delete tag;
                          tag=NULL;
                          result=ProcessURLGENERATIONAPPLIED();
                          subtagsfound[4]=true;
                         }
                         else
                          {
                        
                           if(strcmp(tag,"notsimilars")==0)
                            {
                             delete tag;
                             tag=NULL;
                             result=ProcessNOTSIMILARS();
                             subtagsfound[5]=true;
                            }
                            else
                             {
                              if(strcmp(tag,"similars")==0)
                               {
                                delete tag;
                                tag=NULL;
                                result=ProcessSIMILARS();
                                subtagsfound[6]=true;
                               }
                               else
                                {
                                 if(strcmp(tag,"pacostats")==0)
                                  {//Ignoramos el doctype
                                   delete tag;
                                   tag=NULL;
                                  }
                                  else
                                   {
                                    delete tag;
                                    tag=NULL;
                                    result=false;
                                   }
                                }
                             }
                          }
                       }
                    }
                 }
              }
           }
         if(result && !finished)
           ret=xmlTextReaderRead(reader);
        }
     }
   }
   else
    {
     result=false;
     if(attrib!=NULL)
       fprintf(stderr,"Error: Tag <pacostats> does not contain required attribute \"href\".\n");
     if(attrib2!=NULL)
       fprintf(stderr,"Error: Tag <pacostats> does not contain required attribute \"date\".\n");
    }

  if(!subtagsfound[1])
   {
    result=false;
    fprintf(stderr,"Error: Tag <pacostats> does not contain required tag <targetlanguages>.\n");
   }
  if(!subtagsfound[2])
   {
    result=false;
    fprintf(stderr,"Error: Tag <pacostats> does not contain required tag <downloading>.\n");
   }
  if(!subtagsfound[3])
   {
    result=false;
    fprintf(stderr,"Error: Tag <pacostats> does not contain required tag <discarded>.\n");
   }
  if(!subtagsfound[4])
   {
    generationtype=0;
    urlgenerator=NULL;
   }

  if(!subtagsfound[5])
   {
    result=false;
    fprintf(stderr,"Error: Tag <pacostats> does not contain required tag <notsimilars>.\n");
   }
  if(!subtagsfound[6])
   {
    result=false;
    fprintf(stderr,"Error: Tag <pacostats> does not contain required tag <similars>.\n");
   }
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessPACOSTATS() -> %d\n",result);
#endif     
  return(result);
 }
 
bool Collector::ProcessNONEXTERNALWEBSITES()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessNONEXTERNALWEBSITES()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"website")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessWEBSITE();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessWEBSITES -> %d\n",result);
#endif
  return(result);
 }

bool Collector::ProcessWEBSITE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char **aux; //Auxiliar para el crecimiento del vector de urls
  char *attrib; //Valor del atributo una vez convertido
  unsigned int i;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessWEBSITE()\n");
#endif
    
  nameattrib=xmlCharStrdup("href");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
    
  if(attrib!=NULL)
   {
    result=true;
    aux=new char*[n_urllist+1];
    for(i=0;i<n_urllist;i++)
     {
      aux[i]=urllist[i];
     }
    aux[i]=attrib;
    delete urllist;
    urllist=aux;
    n_urllist++;
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Tag <website> does not contain required attribute \"href\".\n");
    }
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessWEBSITE() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessTARGETLANGUAGES()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessTARGETLANGUAGES()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  n_languages=0;
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=FixLibXML2Encoding(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"tlanguage")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessTLANGUAGE();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }
  
  if(result && n_languages<2)
   {
    result=false;
    fprintf(stderr,"Error: Tag <targetlanguages> should contain two tags <tlanguage> at least.\n");
   }
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessTARGETLANGUAGES -> %d\n",result);
#endif
  return(result);
 }
 
bool Collector::ProcessTLANGUAGE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char **aux; //Auxiliar para el crecimiento del vector de idiomas
  int *aux2; //Auxiliar para el crecimiento del vector de ficheros en cada idioma
  char *attribcode; //Valor del atributo code una vez convertido
  char *attribnumber; //Valor del atributo webpages una vez convertido
  unsigned int i,j,k;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessTLANGUAGE()\n");
#endif
  
  result=true; 
  nameattrib=xmlCharStrdup("code");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribcode=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("webpages");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attribcode!=NULL && attribnumber!=NULL)
   {
    for(i=0,j=0;result && i<strlen(attribnumber);i++)
     {                           
      if(!isdigit(attribnumber[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading attribute \"webpage\" in tag <tlanguage>\n");
       }
       else
         j=j*10+(attribnumber[i]-48);
     }

    if(result)
     {
      //Procesamos el idioma
      for(i=0,result=false;i<kNumLanguages && !result;i++)
       {
        if(strcmp(attribcode,LanguageName(i))==0)
         {//Hemos encontrado el idioma
          result=true;
          langid[i]=n_languages;
          aux=new char*[n_languages+1];
          aux2=new int[n_languages+1];
          for(k=0;k<n_languages;k++)
           {
            aux[k]=langurl[k];
            aux2[k]=oldn_languagepages[k];
           }
          aux[k]=new char[1+strlen(attribcode)];
          strcpy(aux[k],attribcode);
          delete langurl;
          langurl=aux;
          aux2[k]=j;
          delete oldn_languagepages;
          oldn_languagepages=aux2;
          n_languages++;
         }
       } 
     }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Invalid format while loading tag <tlanguage>\n");
    }
    
  delete attribnumber;
  attribnumber=NULL;
  delete attribcode;
  attribcode=NULL;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessTLANGUAGE() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessDOWNLOADING()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attribnumber; //Valor del atributo success una vez convertido
  char *attribnumber2; //Valor del atributo failure una vez convertido
  unsigned int i,j,k;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessDOWNLOADING()\n");
#endif
    
  result=true;
  nameattrib=xmlCharStrdup("success");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("failure");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber2=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attribnumber!=NULL && attribnumber2!=NULL)
   {
    for(i=0,j=0;result && i<strlen(attribnumber);i++)
     {                           
      if(!isdigit(attribnumber[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading attribute \"success\" in tag <downloading>\n");
       }
       else
         j=j*10+(attribnumber[i]-48);
     }

    if(result)
     {//El numero de descargas falladas se descarta
      for(i=0,k=0;result && i<strlen(attribnumber2);i++)
       {                           
        if(!isdigit(attribnumber2[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading attribute \"failure\" in tag <downloading>\n");
         }
         else
          k=k*10+(attribnumber2[i]-48);
       }
     }    
    processedpreviously+=j; //Se añade el numero de paginas descargadas a las que fueron procesadas anteriormente
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Invalid format while loading tag <downloading>\n");
    }

  delete attribnumber;
  attribnumber=NULL;
  delete attribnumber2;
  attribnumber2=NULL;
      
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessDOWNLOADING() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessDISCARDED()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attribnumber; //Valor del atributo unknownlang una vez convertido
  char *attribnumber2; //Valor del atributo insufficientsize una vez convertido
  char *attribnumber3; //Valor del atributo repeated una vez convertido
  unsigned int i,j;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessDISCARDED()\n");
#endif
    
  result=true;
  nameattrib=xmlCharStrdup("unknownlang");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("insufficientsize");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber2=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("repeated");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber3=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
    
  if(attribnumber!=NULL && attribnumber2!=NULL && attribnumber3!=NULL)
   {
    for(i=0,j=0;result && i<strlen(attribnumber);i++)
     {                           
      if(!isdigit(attribnumber[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading attribute \"unknownlang\" in tag <discarded>\n");
       }
       else
         j=j*10+(attribnumber[i]-48);
     }
    
    if(result)
     {
      processedpreviously-=j; //Se sustrae el numero de paginas descartadas a las que fueron procesadas anteriormente

      for(i=0,j=0;result && i<strlen(attribnumber2);i++)
       {                           
        if(!isdigit(attribnumber2[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading attribute \"insufficientsize\" in tag <discarded>\n");
         }
         else
          j=j*10+(attribnumber2[i]-48);
       }
      
      if(result)
       {
        processedpreviously-=j; //Se sustrae el numero de paginas descartadas a las que fueron procesadas anteriormente
        
        for(i=0,j=0;result && i<strlen(attribnumber3);i++)
         {                           
          if(!isdigit(attribnumber3[i]))
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading attribute \"repeated\" in tag <discarded>\n");
           }
           else
            j=j*10+(attribnumber3[i]-48);
         }
        
        if(result)
          processedpreviously-=j; //Se sustrae el numero de paginas descartadas a las que fueron procesadas anteriormente
       }
     }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Invalid format while loading tag <discarded>\n");
    }

  delete attribnumber;
  attribnumber=NULL;
  delete attribnumber2;
  attribnumber2=NULL;
  delete attribnumber3;
  attribnumber3=NULL;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessDISCARDED() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessURLGENERATIONAPPLIED()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attribtype; //Valor del atributo type una vez convertido
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessURLGENERATIONAPPLIED()\n");
#endif
 
  result=true;
  nameattrib=xmlCharStrdup("type");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribtype=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attribtype!=NULL)
   {
    if(strcmp(attribtype,"1")==0)
     {
      generationtype=1;
      applyurlgenerator=true;
     }
     else
      {
       if(strcmp(attribtype,"2")==0)
        {
         generationtype=2;
         applyurlgenerator=true;
        }
        else
         {
          if(strcmp(attribtype,"0")==0)
           {
            generationtype=0;
            applyurlgenerator=false;
           }
           else
            {
             fprintf(stderr,"Error: Invalid format while loading attribute \"type\" in tag <tlanguage>\n");
             result=false;
            }
         }
      }
   }
   else
     result=false;

  delete attribtype;
  attribtype=NULL;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessURLGENERATIONAPPLIED() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessNOTSIMILARS()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attribnumber; //Valor del atributo tagslevel una vez convertido
  char *attribnumber2; //Valor del atributo links una vez convertido
  char *attribnumber3; //Valor del atributo size una vez convertido
  char *attribnumber4; //Valor del atributo tags una vez convertido
  char *attribnumber5; //Valor del atributo extension una vez convertido
  char *attribnumber6; //Valor del atributo depth una vez convertido  
  unsigned int i,j;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessNOTSIMILARS()\n");
#endif
    
  result=true;
  nameattrib=xmlCharStrdup("tagslevel");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("links");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber2=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("size");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber3=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("tags");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber4=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("extension");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber5=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);

  nameattrib=xmlCharStrdup("depth");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber6=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
        
  if(attribnumber!=NULL && attribnumber2!=NULL && attribnumber3!=NULL && attribnumber4!=NULL && attribnumber5!=NULL && attribnumber6!=NULL)
   {
    for(i=0,j=0;result && i<strlen(attribnumber);i++)
     {                           
      if(!isdigit(attribnumber[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading attribute \"tagslevel\" in tag <notsimilars>\n");
       }
       else
         j=j*10+(attribnumber[i]-48);
     }
    
    if(result)
     {
      olddiffbytagslevel=j;

      for(i=0,j=0;result && i<strlen(attribnumber2);i++)
       {                           
        if(!isdigit(attribnumber2[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading attribute \"links\" in tag <notsimilars>\n");
         }
         else
          j=j*10+(attribnumber2[i]-48);
       }
      
      if(result)
       {
        olddiffbylinks=j;
        
        for(i=0,j=0;result && i<strlen(attribnumber3);i++)
         {                           
          if(!isdigit(attribnumber3[i]))
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading attribute \"size\" in tag <notsimilars>\n");
           }
           else
            j=j*10+(attribnumber3[i]-48);
         }
        
        if(result)
         {
          olddiffbysize=j;

          for(i=0,j=0;result && i<strlen(attribnumber4);i++)
           {                           
            if(!isdigit(attribnumber4[i]))
             {
              result=false;
              fprintf(stderr,"Error: Invalid format while loading attribute \"tags\" in tag <notsimilars>\n");
             }
             else
              j=j*10+(attribnumber4[i]-48);
           }
        
          if(result)
           {
            olddiffbytags=j;
            
            for(i=0,j=0;result && i<strlen(attribnumber5);i++)
             {                           
              if(!isdigit(attribnumber5[i]))
               {
                result=false;
                fprintf(stderr,"Error: Invalid format while loading attribute \"extension\" in tag <notsimilars>\n");
               }
               else
                j=j*10+(attribnumber5[i]-48);
             }
          
            if(result)
             {
              olddiffbyext=j;
              
              for(i=0,j=0;result && i<strlen(attribnumber6);i++)
               {                           
                if(!isdigit(attribnumber6[i]))
                 {
                  result=false;
                  fprintf(stderr,"Error: Invalid format while loading attribute \"depth\" in tag <notsimilars>\n");
                 }
                 else
                   j=j*10+(attribnumber6[i]-48);
               }
              if(result)
                olddiffbytreelevel=j;
             }
           }
         }
       }
     }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Invalid format while loading tag <notsimilars>\n");
    }

  delete attribnumber;
  attribnumber=NULL;
  delete attribnumber2;
  attribnumber2=NULL;
  delete attribnumber3;
  attribnumber3=NULL;
  delete attribnumber4;
  attribnumber4=NULL;
  delete attribnumber5;
  attribnumber5=NULL;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessNOTSIMILARS() -> %d\n",result);
#endif
  
  return(result);
 }

bool Collector::ProcessSIMILARS()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attribnumber; //Valor del atributo tagslevel una vez convertido
  unsigned int i,j;
  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"BeginCollector::ProcessSIMILARS()\n");
#endif
  
  result=true;
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attribnumber=FixLibXML2Encoding(xattrib);
  xmlFree(xattrib);
  
  if(attribnumber!=NULL)
   {
    for(i=0,j=0;result && i<strlen(attribnumber);i++)
     {                           
      if(!isdigit(attribnumber[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading attribute \"value\" in tag <similars>\n");
       }
       else
         j=j*10+(attribnumber[i]-48);
     }
    
    if(result)
      oldn_similars=j;
    delete attribnumber;
    attribnumber=NULL;
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Tag <similars> does not contain required attribute \"value\".\n");
    }

  
#ifdef TRAZANDO_Collector
  fprintf(stderr,"EndCollector::ProcessSIMILARS() -> %d\n",result);
#endif
  
  return(result);
 }
