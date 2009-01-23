/*
   Implemented by Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2005
*/

#ifndef CONF_FILE
  #error CONF_FILE was not defined
#endif

#define USINGBASH

#include "collector/collector.h"
#include "heavyheuristics/heavyheuristics.h"
#include "tagaligner/tagaligner2-1.h"
#define cout std::cout
#define cerr std::cerr
#define endl std::endl

#define kNumPossibleOptions 4
#define kDefaultConfFile "bitextor.cfg"

//Constantes para los programas BuildSimilars, SimilarsFilter, Cleaner y AlignWeb
#define kSizeAuxStrings 3000 /** Standard size of string variables */
#define kMaxFilesPerLanguage 1000000 /** Maximum number of files per language */
#define kNumErrorsOpeningAllowed 1000 /** Number of consecutive files not found that finish the search for new ones */
#define kIniNumFilesPerLanguage 20000 /** Numero inicial de ficheros por cada lengua para la tabla que relaciona numeros con cadenas */
#define kIncNumFilesPerLanguage 25000 /** Numero de ficheros que se incrementan cuando no caben mas en la tabla */

#define kMaxEditDistance 1000 /** Maximum value of the edit distance to consider two webpages parallel */
#define kMaxTagCountsDistance 1000 /** Maximum value of the tag count average to discard similarities */

#define kIniSizeList 5 /** Numero inicial de la lista de ficheros similares */
#define kIncSizeList 50 /** Incremento del tamaño de la lista cuando se llena */
#define kMaxElectedSimilars 1000 /** Numero maximo de similares que pueden mantenerse en las paginas */

#define kDirFilename "tmx" /** Directory in which the tmx files will be stored */
#define kWholeCorpusDir "whole" /** Directory in which the downloaded xml are placed to postprocess it */


typedef struct SimilarTo
 {
  unsigned int *list; //lista de ficheros que tienen a este como similar
  unsigned short *langlist; //lista de idiomas de los ficheros que tienen a este como similar
  float *distancelist; //lista de las distancias de los ficheros que tienen a este como similar
  unsigned int nlist; //Numero de elementos de la lista
  unsigned int sizelist; //Tamaño del vector que representa la lista
  unsigned int *nsimilarsperlanguage; //Numero de similares que tiene en cada idioma
 };

//Lista de similares
typedef struct Similarity
 {
  unsigned int left,right; //Elementos de la relacion de similaridad
  unsigned short lleft,lright; //Idiomas de dichos elementos
  float distance; //Distancia entre los similares
 };
  
void OutOfMemory()
 {
  cout<<"Memory exhausted. Program terminated."<<endl;
  exit(1);
 }

void BuildSimilars(const char *directory)
 {
  unsigned int i,j,k,l,m,n;
  char filename[kSizeAuxStrings];
  FILE *fent;
  unsigned int *numfilesperlang; //Numero de ficheros que se han encontrado para cada idioma
  WebPage *aux;
  SimilarTo *act; //Variable para ahorrar codigo al acceder a la lista de similares
  SimilarTo **similarlist; //Aqui construimos la matriz de similares para actualizar todos los ficheros
  SimilarTo *auxsimilarlist; //Variable para aumentar el tamaño de la lista de similares
  unsigned int *sizesimilarlist; //Numero de ficheros que caben en la lista para cada idioma
  
  const int *similarspage; //Auxiliar para el recorrido de los similares de una pagina
  const short *langsimilarspage; //Auxiliar para el recorrido de los similares de una pagina
  const float *distancesimilarspage; //Auxiliar para el recorrido de los similares de una pagina
  
  unsigned int *auxlist; //Auxiliar para el crecimiento de la lista de similares
  unsigned short *auxlanglist; //Auxiliar para el crecimiento de la lista de similares
  float *auxdistancelist; //Auxiliar para el crecimiento de la lista de similares
  
  unsigned int repeated; //Numero de ficheros que estan repetidos
  unsigned int corrected; //Numero de similares que se han añadido a los ficheros
  unsigned int n_similars; //Numero total de ficheros similares
  unsigned int loaded; //Numero de ficheros que se han podido cargar

    
  if(directory!=NULL)
   {
    similarlist=new SimilarTo*[kNumLanguages];
    sizesimilarlist=new unsigned int[kNumLanguages];
    numfilesperlang=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      similarlist[i]=new SimilarTo[kIniNumFilesPerLanguage];
      sizesimilarlist[i]=kIniNumFilesPerLanguage;
      for(j=0;j<kIniNumFilesPerLanguage;j++)
       {
        similarlist[i][j].list=new unsigned int[kIniSizeList];
        similarlist[i][j].langlist=new unsigned short[kIniSizeList];
        similarlist[i][j].distancelist=new float[kIniSizeList];
        similarlist[i][j].nlist=0;
        similarlist[i][j].sizelist=kIniSizeList;
       }
     }
    
    for(i=0,n_similars=0,loaded=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nRetrieving similars on \'%s\' pages: ",LanguageName(i));
      n=0;
      for(j=1,k=0;j<kMaxFilesPerLanguage && k<kNumErrorsOpeningAllowed;j++)
       {
        if((j-k)%100==0)
         {
          fprintf(stdout,". ");
          fflush(stdout);
         }
        
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(!(fent=fopen(filename,"r")))
         {//Permitimos errores hasta en kNumErrorsOpeningAllowed ficheros consecutivos
          k++;
         }
         else
          {
           n=j;
           fclose(fent);
           k=0;
           aux=new WebPage();
           if(aux->LoadHeader(filename))
            {
             loaded++;
             //Miramos a ver si tiene similares
             if(aux->N_Similars()>0)
              {//Introducimos los similares en la lista
               n_similars+=aux->N_Similars();
               similarspage=aux->Similars();
               langsimilarspage=aux->LangSimilars();
               distancesimilarspage=aux->DistanceSimilars();
               for(l=0;l<aux->N_Similars();l++)
                {
                 if(similarspage[l]<kMaxFilesPerLanguage)
                  {
                   while((unsigned)similarspage[l]>sizesimilarlist[langsimilarspage[l]])
                    {//Aumentamos el tamaño del vector de ficheros para el idioma en cuestion
                     auxsimilarlist=new SimilarTo[sizesimilarlist[langsimilarspage[l]]+kIncNumFilesPerLanguage];
                     for(m=0;m<sizesimilarlist[langsimilarspage[l]];m++)
                       auxsimilarlist[m]=similarlist[langsimilarspage[l]][m];
                     sizesimilarlist[langsimilarspage[l]]+=kIncNumFilesPerLanguage;
                     for(;m<sizesimilarlist[langsimilarspage[l]];m++)
                      {
                       auxsimilarlist[m].list=new unsigned int[kIniSizeList];
                       auxsimilarlist[m].langlist=new unsigned short[kIniSizeList];
                       auxsimilarlist[m].distancelist=new float[kIniSizeList];                       
                       auxsimilarlist[m].nlist=0;
                       auxsimilarlist[m].sizelist=kIniSizeList;
                      }
                     delete similarlist[langsimilarspage[l]];
                     similarlist[langsimilarspage[l]]=auxsimilarlist;
                    }
                   act=&(similarlist[langsimilarspage[l]][similarspage[l]]);
                   act->list[act->nlist]=j;
                   act->langlist[act->nlist]=i;
                   
                   act->distancelist[act->nlist]=distancesimilarspage[l];
                   
                   act->nlist++;
                   if(act->nlist==act->sizelist)
                    {//Aumentamos el tamaño del vector
                     auxlist=new unsigned int[act->sizelist+kIncSizeList];
                     auxlanglist=new unsigned short[act->sizelist+kIncSizeList];
                     auxdistancelist=new float[act->sizelist+kIncSizeList];
                     for(m=0;m<act->nlist;m++)
                      {
                       auxlist[m]=act->list[m];
                       auxlanglist[m]=act->langlist[m];
                       auxdistancelist[m]=act->distancelist[m];
                      }
                     delete act->list;
                     act->list=auxlist;
                     delete act->langlist;
                     act->langlist=auxlanglist;
                     delete act->distancelist;
                     act->distancelist=auxdistancelist;
                     act->sizelist+=kIncSizeList;
                    }
                  }
                }
              }
            }
            else
             {
              fprintf(stdout,"Error while reading file \"%s\"\n",filename);
             }
          delete aux;
         }
       }
      numfilesperlang[i]=n;
     }
    fprintf(stdout,"\nRetrieving finished: %d similars found in %d files processed\n",n_similars,loaded);
      
    //Recorremos de nuevo todos los ficheros para renumerar sus ficheros
    //similares de acuerdo a la renumeracion que han sufrido
    for(i=0,repeated=0,corrected=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nProcessing \'%s\' pages: ",LanguageName(i));
      fflush(stdout);
      for(j=1;j<numfilesperlang[i]+1;j++)
       {
        if(j%100==0)
         {
          if(j%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
        
        if(j<sizesimilarlist[i] && similarlist[i][j].nlist>0)
         {//Almacenamos los nuevos similares de este fichero
          sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
          if((fent=fopen(filename,"r"))!=NULL)
           {
            fclose(fent);
            aux=new WebPage();
            if(aux->Load(filename))
             {//Añadimos los similares por los renombrados
              for(k=0,l=corrected;k<similarlist[i][j].nlist;k++)
               {
                if(!aux->NewSimilar(similarlist[i][j].langlist[k],similarlist[i][j].list[k],similarlist[i][j].distancelist[k]))
                  repeated++;//Incrementamos el numero de similares que estaban en ambos ficheros
                 else
                   corrected++;
               }
              if(l<corrected) //Se inserto algun similar
               {
                aux->AddApplication("buildsimilars","1.0");
                aux->Save(filename);
               }
             }
            delete aux;
            aux=NULL;
           }
         }
       }
     }
   
    fprintf(stdout,"\nProcess finished: Inserted %d similars and the total is now %d\n",corrected,n_similars);
     
    for(i=0;i<kNumLanguages;i++)
     {
      for(j=0;j<sizesimilarlist[i];j++)
       {
        delete similarlist[i][j].list;
        delete similarlist[i][j].langlist;
        delete similarlist[i][j].distancelist;
       }
      delete similarlist[i];            
     }
    delete similarlist;
    delete sizesimilarlist;
    delete numfilesperlang;
   } 
 }

void SimilarsFilter(const char *directory)
 {
  unsigned int i,j,k,l,m,n;
  char filename[kSizeAuxStrings],filename2[kSizeAuxStrings]; //Names of the files that are compared
  FILE *fent; //Auxiliar to test if files exist
  
  WebPage *aux,*aux2; //Webpages that will be loaded to compare them
  float distancefilterED; //Webpages distance obtained by the edit distance filter
  float distancefilterTD; //Weboages distance obtained by comparing tag frecuency
  float distancefilter; //Webpages distance once that the values have been combined
  HeavyHeuristics hh; //Useful for applying the heavy heuristic filters
    
  short *sequence,*sequence2; //Tag sequences to count its tags and perform the edit distance
  unsigned int sizesequence,sizesequence2; //Sizes of the tag sequences
  short *filtsequence,*filtsequence2; //Filtered Tag sequences to perform the edit distance
  unsigned int sizefiltsequence,sizefiltsequence2; //Sizes of the filtered tag sequences
  unsigned short *tagcounter,*tagcounter2; //Tag counters to compare the number of times that tags appear in the webpages
  bool erasesimilar; //Indicates if the current similar has to be discarded or not
    
  const int *similarspage; //Auxiliar to process all similars of a webpage
  const short *langsimilarspage; //Auxiliar to process the language of all similars of a webpage
  
  unsigned int n_filtered; //Number of similars that have been filtered
  unsigned int loaded; //Number of files that have been successfully loaded
  bool modified; //Indicates if the current webpage has to be saved because it has been modified
  
  SimilarTo *act; //Var to save source code while acceding the similar list
  SimilarTo **similarlist; //Similar matrix to update distances of all files
  SimilarTo *auxsimilarlist; //Auxiliar to allow size increase of the similar list
  unsigned int *sizesimilarlist; //Number of files that has been allocated in memory
  unsigned int *auxlist; //Auxiliar to increase the size of the similar list
  unsigned short *auxlanglist; //Auxiliar to increase the size of the similar language list
  float *auxdistancelist; //Auxiliar to increase the size of the similar distance list

  
  if(directory!=NULL)
   {
    similarlist=new SimilarTo*[kNumLanguages];
    sizesimilarlist=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      similarlist[i]=new SimilarTo[kIniNumFilesPerLanguage];
      sizesimilarlist[i]=kIniNumFilesPerLanguage;
      for(j=0;j<kIniNumFilesPerLanguage;j++)
       {
        similarlist[i][j].list=new unsigned int[kIniSizeList];
        similarlist[i][j].langlist=new unsigned short[kIniSizeList];
        similarlist[i][j].distancelist=new float[kIniSizeList];
        similarlist[i][j].nlist=0;
        similarlist[i][j].sizelist=kIniSizeList;
       }
     } 
     
    distancefilter=0;
    tagcounter=0;
    for(i=0,n_filtered=0,loaded=0,modified=false;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nApplying heavy heuristics on \'%s\' pages: ",LanguageName(i));
      fflush(stdout);
      n=0;
      for(j=1,k=0;j<kMaxFilesPerLanguage && k<kNumErrorsOpeningAllowed;j++)
       {
        if((j-k)%100==0)
         {
          if((j-k)%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
     
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(!(fent=fopen(filename,"r")))
         {//Only kNumErrorsOpeningAllowed consecutive opening errors are allowed
          k++;
         }
         else
          {
           n=j;
           fclose(fent);
           k=0;
           aux=new WebPage();
           if(aux->Load(filename))
            {
             loaded++;
             //Looking for similars
             if(aux->N_Similars()>0)
              {//Processing the similars
               similarspage=aux->Similars();
               langsimilarspage=aux->LangSimilars();
               
               sequence=hh.ObtainNumericTagSequence(aux->Content(),&sizesequence);
               if(sequence!=NULL && sizesequence>0)
                 filtsequence=hh.FilterTagSequence(sequence,sizesequence,&sizefiltsequence);
                else
                  filtsequence=NULL;
               if(filtsequence!=NULL && sizefiltsequence>0)
                {
                 tagcounter=hh.CountTagsFromSequence(sequence,sizesequence);
                 for(l=0;l<aux->N_Similars();l++)
                  {
                   if(similarspage[l]<kMaxFilesPerLanguage && langsimilarspage[l]>(signed)i)
                    {//Checking of if this similar is still valid with this filter
                     modified=true;
                     sprintf(filename2,"%s/%s/%06d.xml",directory,LanguageName(langsimilarspage[l]),similarspage[l]);
                     aux2=new WebPage();
                     if(aux2->Load(filename2))
                      {
                       erasesimilar=false;
                       sequence2=hh.ObtainNumericTagSequence(aux2->Content(),&sizesequence2);
                       if(sequence2!=NULL && sizesequence2>0)
                        {
                         tagcounter2=hh.CountTagsFromSequence(sequence2,sizesequence2);
                         distancefilterTD=hh.CountsDistance(tagcounter,tagcounter2);
                         
                         if(distancefilterTD>kMaxTagCountsDistance)
                           erasesimilar=true;
                          else
                           {
                            filtsequence2=hh.FilterTagSequence(sequence2,sizesequence2,&sizefiltsequence2);
                            if(filtsequence2!=NULL && sizefiltsequence2>0)
                             {
                              distancefilterED=hh.EditDistanceNumTagSeq(filtsequence,sizefiltsequence,filtsequence2,sizefiltsequence2,kMaxTagCountsDistance);
                              if(distancefilterED<kMaxTagCountsDistance)
                               {//The edit distance will substitute the previous distance value
                                distancefilter=distancefilterED/kMaxEditDistance;
                               } 
                               else
                                erasesimilar=true; 
                                
                             }
                             else
                              {//There was a problem filtering the tag sequences (bad format of the webpages)
                               erasesimilar=true;
                              }
                            delete filtsequence2;
                            filtsequence2=NULL;
                           }
                         delete tagcounter2;
                         tagcounter2=NULL;
                        }
                        else
                         {//There was a problem obtaining the tag sequences (bad format of the webpages)
                          erasesimilar=true;
                         }
                       
                       while((unsigned)similarspage[l]>sizesimilarlist[langsimilarspage[l]])
                        {//Increase of the size of the vector of files per language
                         auxsimilarlist=new SimilarTo[sizesimilarlist[langsimilarspage[l]]+kIncNumFilesPerLanguage];
                         for(m=0;m<sizesimilarlist[langsimilarspage[l]];m++)
                           auxsimilarlist[m]=similarlist[langsimilarspage[l]][m];
                         sizesimilarlist[langsimilarspage[l]]+=kIncNumFilesPerLanguage;
                         for(;m<sizesimilarlist[langsimilarspage[l]];m++)
                          {
                           auxsimilarlist[m].list=new unsigned int[kIniSizeList];
                           auxsimilarlist[m].langlist=new unsigned short[kIniSizeList];
                           auxsimilarlist[m].distancelist=new float[kIniSizeList];                       
                           auxsimilarlist[m].nlist=0;
                           auxsimilarlist[m].sizelist=kIniSizeList;
                          }
                         delete similarlist[langsimilarspage[l]];
                         similarlist[langsimilarspage[l]]=auxsimilarlist;
                        }
                    
                       act=&(similarlist[langsimilarspage[l]][similarspage[l]]);
                       act->list[act->nlist]=j;
                       act->langlist[act->nlist]=i;
                   
                       if(erasesimilar)
                        {
                         aux->DeleteSimilar(langsimilarspage[l],similarspage[l]);
                         act->distancelist[act->nlist]=kMaxEditDistance;
                         l--;
                         n_filtered++;
                        }
                        else
                         {
                          aux->RefreshSimilar(langsimilarspage[l],similarspage[l],distancefilter);
                          act->distancelist[act->nlist]=distancefilter;
                         }
                         
                       act->nlist++;
                       if(act->nlist==act->sizelist)
                        {//Increase of the similars vector size
                         auxlist=new unsigned int[act->sizelist+kIncSizeList];
                         auxlanglist=new unsigned short[act->sizelist+kIncSizeList];
                         auxdistancelist=new float[act->sizelist+kIncSizeList];
                         for(m=0;m<act->nlist;m++)
                          {
                           auxlist[m]=act->list[m];
                           auxlanglist[m]=act->langlist[m];
                           auxdistancelist[m]=act->distancelist[m];
                          }
                         delete act->list;
                         act->list=auxlist;
                         delete act->langlist;
                         act->langlist=auxlanglist;
                         delete act->distancelist;
                         act->distancelist=auxdistancelist;
                         act->sizelist+=kIncSizeList;
                        }
                                                                      
                       delete sequence2;
                       sequence2=NULL;
                      }
                      else
                       {
                        fprintf(stderr,"Warning: Similar file (%s,%d) not found when looking for similars of ($s,%d)\n",LanguageName(langsimilarspage[l]),similarspage[l],LanguageName(i),j);
                        aux->DeleteSimilar(langsimilarspage[l],similarspage[i]);
                       }
                     delete aux2;
                     aux2=NULL;
                    }
                    else
                     {//The distance of this similar could have been modified
                      act=&(similarlist[i][j]);
                      for(m=0;m<act->nlist;m++)
                       {
                        if(act->list[m]==(unsigned)similarspage[l] && act->langlist[m]==(unsigned short)langsimilarspage[l])
                         {
                          aux->RefreshSimilar(langsimilarspage[l],similarspage[l],act->distancelist[m]);
                          modified=true;
                         }
                       }
                     }
                  }
                 delete tagcounter;
                 tagcounter=NULL;
                 delete filtsequence;
                 filtsequence=NULL;
                }
               delete sequence;
               sequence=NULL;
               if(modified)
                {
                 aux->AddApplication("similarsfilter","1.0");
                 aux->Save(filename);
                 modified=false;
                }
              }
            }
           delete aux;
          }
       }
     }
    fprintf(stdout,"\nProcess finished: %d similars filtered in %d files processed\n",n_filtered,loaded);
    for(i=0;i<kNumLanguages;i++)
     {
      for(j=0;j<sizesimilarlist[i];j++)
       {
        delete similarlist[i][j].list;
        delete similarlist[i][j].langlist;
        delete similarlist[i][j].distancelist;
       }
      delete similarlist[i];            
     }
    delete similarlist;
    delete sizesimilarlist;
   }
 }

void DiscardSimilars(unsigned int maxnsimilars,const char *directory)
 {
  unsigned int i,j,k,l,m,n;
  int o;
  char filename[kSizeAuxStrings];
  FILE *fent;
  
  unsigned int *numfilesperlang; //Numero de ficheros que se han encontrado para cada idioma
  WebPage *aux;
  SimilarTo *act; //Variable para ahorrar codigo al acceder a la lista de similares
  SimilarTo **similarlist; //Aqui construimos la matriz de similares para actualizar todos los ficheros
  SimilarTo *auxsimilarlist; //Variable para aumentar el tamaño de la lista de similares
  unsigned int *sizesimilarlist; //Numero de ficheros que caben en la lista para cada idioma
  
  const int *similarspage; //Auxiliar para el recorrido de los similares de una pagina
  const short *langsimilarspage; //Auxiliar para el recorrido de los similares de una pagina
  const float *distancesimilarspage; //Auxiliar para el recorrido de los similares de una pagina
  
  unsigned int *auxlist; //Auxiliar para el crecimiento de la lista de similares
  unsigned short *auxlanglist; //Auxiliar para el crecimiento de la lista de similares
  float *auxdistancelist; //Auxiliar para el crecimiento de la lista de similares
  
  unsigned int repeated; //Numero de ficheros que estan repetidos
  unsigned int corrected; //Numero de similares que se han añadido a los ficheros
  unsigned int n_similars; //Numero total de ficheros similares
  unsigned int loaded; //Numero de ficheros que se han podido cargar
  
  OrderedList2Levels *orderedlist; //Lista para la ordenacion por distancia de los similares
  Similarity *similarvector; //Vector con los similares
  unsigned int nsimilarvector; //Numero de elementos rellenos del vector
  
  unsigned int result_similars; //Numero de similares que se han borrado en el fichero actual
  unsigned int deleted_similars; //Indica cuantos similares se han borrado hasta el momento
  bool found;

  if(directory!=NULL)
   {
    similarlist=new SimilarTo*[kNumLanguages];
    sizesimilarlist=new unsigned int[kNumLanguages];
    numfilesperlang=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      similarlist[i]=new SimilarTo[kIniNumFilesPerLanguage];
      sizesimilarlist[i]=kIniNumFilesPerLanguage;
      for(j=0;j<kIniNumFilesPerLanguage;j++)
       {
        similarlist[i][j].list=new unsigned int[kIniSizeList];
        similarlist[i][j].langlist=new unsigned short[kIniSizeList];
        similarlist[i][j].distancelist=new float[kIniSizeList];
        similarlist[i][j].nsimilarsperlanguage=NULL;
        similarlist[i][j].nlist=0;
        similarlist[i][j].sizelist=kIniSizeList;
       }
     }
     
    for(i=0,n_similars=0,loaded=0,deleted_similars=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nRetrieving similars on \'%s\' pages: ",LanguageName(i));
      n=0;
      for(j=1,k=0;j<kMaxFilesPerLanguage && k<kNumErrorsOpeningAllowed;j++)
       {
        if((j-k)%100==0)
         {
          if((j-k)%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
     
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(!(fent=fopen(filename,"r")))
         {//Permitimos errores hasta en kNumErrorsOpeningAllowed ficheros consecutivos
          k++;
         }
         else
          {
           n=j;
           fclose(fent);
           k=0;
           aux=new WebPage();
           if(aux->LoadHeader(filename))
            {
             loaded++;
             //Miramos a ver si tiene similares
             if(aux->N_Similars()>0)
              {//Introducimos los similares en la lista
               n_similars+=aux->N_Similars();
               similarspage=aux->Similars();
               langsimilarspage=aux->LangSimilars();
               distancesimilarspage=aux->DistanceSimilars();
               for(l=0;l<aux->N_Similars();l++)
                {
                 if(similarspage[l]<kMaxFilesPerLanguage)
                  {
                   while((unsigned)similarspage[l]>sizesimilarlist[langsimilarspage[l]])
                    {//Aumentamos el tamaño del vector de ficheros para el idioma en cuestion
                     auxsimilarlist=new SimilarTo[sizesimilarlist[langsimilarspage[l]]+kIncNumFilesPerLanguage];
                     for(m=0;m<sizesimilarlist[langsimilarspage[l]];m++)
                       auxsimilarlist[m]=similarlist[langsimilarspage[l]][m];
                     sizesimilarlist[langsimilarspage[l]]+=kIncNumFilesPerLanguage;
                     for(;m<sizesimilarlist[langsimilarspage[l]];m++)
                      {
                       auxsimilarlist[m].list=new unsigned int[kIniSizeList];
                       auxsimilarlist[m].langlist=new unsigned short[kIniSizeList];
                       auxsimilarlist[m].distancelist=new float[kIniSizeList];                       
                       auxsimilarlist[m].nlist=0;
                       similarlist[i][j].nsimilarsperlanguage=NULL;
                       auxsimilarlist[m].sizelist=kIniSizeList;
                      }
                     delete similarlist[langsimilarspage[l]];
                     similarlist[langsimilarspage[l]]=auxsimilarlist;
                    }
                   act=&(similarlist[langsimilarspage[l]][similarspage[l]]);
                   act->list[act->nlist]=j;
                   act->langlist[act->nlist]=i;
                  
                   act->distancelist[act->nlist]=distancesimilarspage[l];
                 
                   act->nlist++;
                   if(act->nlist==act->sizelist)
                    {//Aumentamos el tamaño del vector
                     auxlist=new unsigned int[act->sizelist+kIncSizeList];
                     auxlanglist=new unsigned short[act->sizelist+kIncSizeList];
                     auxdistancelist=new float[act->sizelist+kIncSizeList];
                     for(m=0;m<act->nlist;m++)
                      {
                       auxlist[m]=act->list[m];
                       auxlanglist[m]=act->langlist[m];
                       auxdistancelist[m]=act->distancelist[m];
                      }
                     delete act->list;
                     act->list=auxlist;
                     delete act->langlist;
                     act->langlist=auxlanglist;
                     delete act->distancelist;
                     act->distancelist=auxdistancelist;
                     act->sizelist+=kIncSizeList;
                    }
                  }
                }
              }
            }
           delete aux;
         }
       }
      numfilesperlang[i]=n;
     }
    fprintf(stdout,"\nRetrieving finished: %d similars found in %d files processed\n",n_similars/2,loaded);
       
    fprintf(stdout,"Applying spanning tree algorithm");
    orderedlist=new OrderedList2Levels(25,100,10000000);
    similarvector=new Similarity[n_similars];
    nsimilarvector=0;
    //Recorremos de nuevo todos los ficheros para almacenar sus relaciones
    //de similaridad y ordenarlas
    for(i=0,repeated=0,corrected=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nOrdering distances of \'%s\' pages: ",LanguageName(i));
      fflush(stdout);
      for(j=1;j<numfilesperlang[i]+1;j++)
       {
        if(j%100==0)
         {
          if(j%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
         
        for(k=0;k<similarlist[i][j].nlist;k++)
         {//Insertamos los similares en la lista ordenada
          orderedlist->Insert((unsigned int)(10000000*similarlist[i][j].distancelist[k]),nsimilarvector);
          similarvector[nsimilarvector].left=j;
          similarvector[nsimilarvector].right=similarlist[i][j].list[k];
          similarvector[nsimilarvector].lleft=i;
          similarvector[nsimilarvector].lright=similarlist[i][j].langlist[k];
          similarvector[nsimilarvector].distance=similarlist[i][j].distancelist[k];
          nsimilarvector++;
         }
       }
     }
    
    fprintf(stdout,"\nReorganizing similars\n");
    result_similars=0;
       
    //Borramos la lista de similares por fichero, para reconstruirla
    for(i=0;i<kNumLanguages;i++)
     {
      for(j=1;j<sizesimilarlist[i] || j<kIniNumFilesPerLanguage;j++)
       {
        delete similarlist[i][j].list;
        delete similarlist[i][j].langlist;
        delete similarlist[i][j].distancelist;
       }
      delete similarlist[i];            
     }
    delete similarlist;
    delete sizesimilarlist;
      
    similarlist=new SimilarTo*[kNumLanguages];
    sizesimilarlist=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      similarlist[i]=new SimilarTo[numfilesperlang[i]+1];
      sizesimilarlist[i]=numfilesperlang[i]+1;
      for(j=1;j<numfilesperlang[i]+1;j++)
       {
        similarlist[i][j].list=new unsigned int[kNumLanguages*maxnsimilars];
        similarlist[i][j].langlist=new unsigned short[kNumLanguages*maxnsimilars];
        similarlist[i][j].distancelist=NULL;
        similarlist[i][j].nsimilarsperlanguage=new unsigned int[kNumLanguages];
        for(k=0;k<kNumLanguages;k++)
          similarlist[i][j].nsimilarsperlanguage[k]=0;
        similarlist[i][j].nlist=0;
        similarlist[i][j].sizelist=kNumLanguages*maxnsimilars;
       }
     }
    
    //Recorremos el vector de distancias ordenado para regenerar todos las
    //relaciones de similaridad que sean posibles (que no superen el maximo)
    //Intentamos insertar las relaciones por orden de menor distancia (Spanning tree)
    orderedlist->GoStart();
    for(o=orderedlist->GoNext();o!=-1;o=orderedlist->GoNext())
     {//Comprobamos que la relacion pueda realizarse en ambas paginas
      if(similarlist[similarvector[o].lleft][similarvector[o].left].nsimilarsperlanguage[similarvector[o].lright]<maxnsimilars &&
         similarlist[similarvector[o].lright][similarvector[o].right].nsimilarsperlanguage[similarvector[o].lleft]<maxnsimilars)
       {//Se inserta el similar en ambos sitios (salvo que este repetido)
        act=&(similarlist[similarvector[o].lleft][similarvector[o].left]);           
        for(i=0,found=false;i<act->nlist && !found;i++)
         {
          if(act->list[i]==similarvector[o].right &&
             act->langlist[i]==similarvector[o].lright)
           found=true; //Estaba repetido
         }
        if(!found)
         {
          act->list[act->nlist]=similarvector[o].right;
          act->langlist[act->nlist]=similarvector[o].lright;           
          act->nlist++;
          act->nsimilarsperlanguage[similarvector[o].lright]++;
         }
          
        act=&(similarlist[similarvector[o].lright][similarvector[o].right]);
           
        for(i=0,found=false;i<act->nlist && !found;i++)
         {
          if(act->list[i]==similarvector[o].left &&
             act->langlist[i]==similarvector[o].lleft)
           found=true; //Estaba repetido
         }
        if(!found)
         {
          act->list[act->nlist]=similarvector[o].left;
          act->langlist[act->nlist]=similarvector[o].lleft;
          act->nlist++;
          act->nsimilarsperlanguage[similarvector[o].lleft]++;
         }
       }
     }
                     
    //Ya no hacen falta las estructuras de distancia ordenadas
    //delete orderedlist;
    //delete similarvector;

    //Borramos todos los similares sobrantes
    fprintf(stdout,"Deletion of extra similars",n_similars,loaded);

    for(i=0,result_similars=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nDeleting similars of \'%s\' pages: ",LanguageName(i));
      fflush(stdout);
      for(j=1;j<numfilesperlang[i]+1;j++)
       {
        if(j%100==0)
         {
          if(j%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
                       
        if(j<sizesimilarlist[i])
         {//Borramos los similares sobrantes de este fichero
          sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
          if((fent=fopen(filename,"r"))!=NULL)
           {
            fclose(fent);
            aux=new WebPage();
            if(aux->Load(filename))
             {//Añadimos los similares por los renombrados
              result_similars=aux->ResetSimilarListTo(similarlist[i][j].nlist,similarlist[i][j].langlist,similarlist[i][j].list);
              if(result_similars>0) //Se borro algun similar
               {
                aux->AddApplication("discardsimilars","1.0");
                aux->Save(filename);
                deleted_similars+=result_similars;
               }
             }
            delete aux;
            aux=NULL;
           }
         }
       }
     }
       
    fprintf(stdout,"\nProcess finished: Deleted %d similars and the total is now %d\n",(deleted_similars)/2,(n_similars-deleted_similars)/2);
       
    for(i=0;i<kNumLanguages;i++)
     {
      for(j=1;j<sizesimilarlist[i];j++)
       {
        delete similarlist[i][j].list;
        delete similarlist[i][j].langlist;
        delete similarlist[i][j].nsimilarsperlanguage;
       }
      delete similarlist[i];            
     }
    delete similarlist;
    delete sizesimilarlist;
    delete numfilesperlang;
   }
 }

void Cleaner(const char *directory)
 {
  unsigned int i,j,k,l,m,n;
  char filename[kSizeAuxStrings],filename2[kSizeAuxStrings];
  FILE *fent,*fsal;
  WebPage *aux;
  unsigned int *n_erased; //Variable para la estadistica de numero de borrados
  unsigned int *n_similars; //Variable para la estadistica de similares
  unsigned int *n_nonerased; //Ficheros que tenian similares
  char ***urls; //Vector con las urls asociadas a cada pagina en cada idioma
  unsigned int *sizeurl; //Tamaño del vector de urls en el idioma que estamos procesando
  char **auxurls; //Variable para el crecimiento del vector
  const int *similars; //Variable para la consulta de los similares de un fichero
  const short *langsimilars; //Variable para la consulta del idioma de los similares de los ficheros
  unsigned int *numfilesperlang; //Variable que indica cuantos ficheros de cada idioma hay originalmente 
  int **renumeration; //Variable que relaciona cada fichero con su nuevo numero tras el borrado de las paginas sin similares
  int *auxrenumeration; //Variable para el crecimiento del vector
  
  const int *similarspage; //Auxiliar para la renumeracion de similares
  const short *langsimilarspage; //Auxiliar para la renumeracion de similares
  
  if(directory!=NULL)
   {
    urls=new char**[kNumLanguages];
    sizeurl=new unsigned int[kNumLanguages];
    renumeration=new int*[kNumLanguages];
    numfilesperlang=new unsigned int[kNumLanguages];
    n_erased=new unsigned int[kNumLanguages];
    n_nonerased=new unsigned int[kNumLanguages];
    n_similars=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      n_erased[i]=0;
      n_similars[i]=0;
      n_nonerased[i]=0;
      urls[i]=new char*[kIniNumFilesPerLanguage];
      renumeration[i]=new int[kIniNumFilesPerLanguage];
      urls[i][0]=NULL;
      sizeurl[i]=kIniNumFilesPerLanguage;
      fprintf(stdout,"Deleting \'%s\' pages without similars: ",LanguageName(i));
      n=0;
      m=0;
      for(j=1,k=0;j<kMaxFilesPerLanguage && k<kNumErrorsOpeningAllowed;j++)
       {
        if((j-k)%100==0)
         {
          fprintf(stdout,". ");
          fflush(stdout);
         }
        
        if(j==sizeurl[i])
         {
          auxurls=new char*[sizeurl[i]+kIncNumFilesPerLanguage];
          auxrenumeration=new int[sizeurl[i]+kIncNumFilesPerLanguage];
          for(l=0;l<sizeurl[i];l++)
           {
            auxurls[l]=urls[i][l];
            auxrenumeration[l]=renumeration[i][l];
           }
          delete urls[i];
          delete renumeration[i];
          urls[i]=auxurls;
          renumeration[i]=auxrenumeration;
          sizeurl[i]+=kIncNumFilesPerLanguage;
         }
        
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(!(fent=fopen(filename,"r")))
         {
          k++;//Permitimos errores hasta en kNumErrorsOpeningAllowed ficheros consecutivos
          urls[i][j]=NULL;
         }
         else
          {
           n=j;
           fclose(fent);
           k=0;
           aux=new WebPage();
           if(aux->LoadHeader(filename))
            {//Miramos a ver si tiene similares
             n_similars[i]+=aux->N_Similars();
             if(aux->N_Similars()==0)
              {
               renumeration[i][j]=-1; //Este fichero sera borrado
               m++;
              }
              else
               {
                renumeration[i][j]=n_nonerased[i]+1;
                n_nonerased[i]++;
               }
            }
           delete aux;
          }
       }
      numfilesperlang[i]=n;
      fprintf(stdout,"(%d/%d deleted)\n",m,n);
     }
     
    //Recorremos de nuevo todos los ficheros para renumerar sus ficheros
    //similares de acuerdo a la renumeracion que han sufrido
    for(i=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nRenumerating similars in \'%s\' pages: ",LanguageName(i));
      for(j=1;j<numfilesperlang[i]+1;j++)
       {
        if(j%100==0)
         {
          if(j%1000!=0)
            fprintf(stdout,". ");
           else
             fprintf(stdout,"%d ",j);
          fflush(stdout);
         }
        
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(renumeration[i][j]!=-1)
         {//Solo para los ficheros que no se van a borrar
          if((fent=fopen(filename,"r"))!=NULL)
           {
            fclose(fent);
            aux=new WebPage();
            if(aux->Load(filename))
             {//Reemplazamos sus similares por los renombrados
              similarspage=aux->Similars();
              langsimilarspage=aux->LangSimilars();
              for(k=0;k<aux->N_Similars();k++)
                aux->ChangeSimilar(langsimilarspage[k],similarspage[k],renumeration[langsimilarspage[k]][similarspage[k]]);
              sprintf(filename2,"%s/%s/%06d.xml",directory,LanguageName(i),renumeration[i][j]);
              urls[i][renumeration[i][j]]=aux->PageFullURL();
              aux->AddApplication("cleaner","1.0");
              aux->Save(filename2);
              if((unsigned)renumeration[i][j]!=j)
               {//Se ha renumerado por lo que borramos el archivo original
                urls[i][j]=NULL;
                if(unlink(filename)!=0)
                 {
                  if(unlink(filename)!=0)
                   {
                    fprintf(stderr,"Error: The file %s could not be deleted\n",filename);
                   }
                 }
               }
             }
            delete aux;
           }
         }
         else
          {
           n_erased[i]++;
           urls[i][j]=NULL;
           if(unlink(filename)!=0)
            {
             if(unlink(filename)!=0)
              {
               fprintf(stderr,"Error: The file %s could not be deleted\n",filename);
              }
            }
          }
       }
      numfilesperlang[i]=numfilesperlang[i]-n_erased[i];
      fprintf(stdout,"There were %d files without similar files in %s/%s/\n",n_erased[i],directory,LanguageName(i));
      fprintf(stdout,"The remaining %d have a total of %d similar files\n",n_nonerased[i],n_similars[i]);
     }
    
    for(i=0;i<kNumLanguages;i++)
     {
      delete renumeration[i];
     }
    delete renumeration;
     
    //Ahora escribimos el fichero de indice que asocia a cada url sus urls similares
    for(i=0;i<kNumLanguages;i++)
     {
      if(numfilesperlang[i]>0)
       {
        fprintf(stdout,"Writing similar list for \'%s\' pages... ",LanguageName(i));
        sprintf(filename,"%s/%s/index",directory,LanguageName(i));
        if(!(fsal=fopen(filename,"w")))
         {
          fprintf(stderr,"Error: The Index file \"%s\" could not be opened\n",filename);
         }
         else
          {
           for(j=1;j<numfilesperlang[i];j++)
            {
             if(urls[i][j]!=NULL)
              {
               sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
               aux=new WebPage();
               if(aux->LoadHeader(filename) && aux->N_Similars()>0)
                {
                 similars=aux->Similars();
                 langsimilars=aux->LangSimilars();   
                 fprintf(fsal,"%s :",urls[i][j]);
                 fprintf(fsal," %s(%s)",urls[langsimilars[0]][similars[0]],LanguageName(langsimilars[0]));
                 for(k=1;k<aux->N_Similars();k++)
                  {
                   fprintf(fsal,", %s(%s)",urls[langsimilars[k]][similars[k]],LanguageName(langsimilars[k]));
                  }
                 fprintf(fsal,"\n");
                }
               delete aux;
              }
            }
           fclose(fsal);
          }
        fprintf(stdout,"\n");
       }
     }
    delete sizeurl;
    delete urls;
    delete n_erased;
    delete n_nonerased;
    delete n_similars;
    delete numfilesperlang;
   }
 }
 
void AlignWebsite(char *directory)
 {
  unsigned int i,j,k,l;
  char filename[kSizeAuxStrings],filename2[kSizeAuxStrings],filename3[kSizeAuxStrings];
  FILE *fent;
  WebPage *aux;
  #ifndef USINGBASH
  FILE *fsal;
  WebPage *aux2;
  #endif
  const int *similars; //It contains the numbers of the similars of a file
  const short *langsimilars; //It contains the languages of the similars of a file
  unsigned int *numfilesperlang; //Number of files in each language
  #ifndef USINGBASH
  const char *alignment; //Result of the alignment
  #endif
  unsigned int nalignments[kNumLanguages][kNumLanguages]; //Number of alignments of each type
  bool languagefound[kNumLanguages]; //Indicates the languages that has been aligned of the current page
  TagAligner2_1 *tagaligner;
  
  unsigned int loaded; //Number of files that have been processed
  unsigned int n_tmx; //Number of tmx generated
  
  if(directory!=NULL)
   {
    //Creation of the TMX directory
    sprintf(filename,"%s/%s",directory,kDirFilename);
    mkdir(filename,0777);
    
    tagaligner=new TagAligner2_1();
    loaded=0;
    n_tmx=0;
    numfilesperlang=new unsigned int[kNumLanguages];
    for(i=0;i<kNumLanguages;i++)
     {
      fprintf(stdout,"\nAligning \'%s\' webpages: ",LanguageName(i));
      fflush(stdout);
      for(j=i;j<kNumLanguages;j++)
        nalignments[i][j]=0;
      for(j=1,k=0;j<kMaxFilesPerLanguage && k<kNumErrorsOpeningAllowed;j++)
       {
        if((j-k)%25==0)
         {
          fprintf(stdout,". ");
          fflush(stdout);
         }
        
        sprintf(filename,"%s/%s/%06d.xml",directory,LanguageName(i),j);
        if(!(fent=fopen(filename,"r")))
          k++; //kNumErrorsOpeningAllowed consecutive errors are allowed
         else
          {
           fclose(fent);
           k=0;
           aux=new WebPage();
           if(aux->Load(filename))
            {
             loaded++;
             similars=aux->Similars();
             langsimilars=aux->LangSimilars();
             for(l=0;l<kNumLanguages;l++)
               languagefound[l]=false;
             for(l=0;l<aux->N_Similars();l++)              
              {//Alignment with the first similar of each language
               if((unsigned short)langsimilars[l]>i && !languagefound[langsimilars[l]])
                {                 
                 sprintf(filename2,"%s/%s/%06d.xml",directory,LanguageName(langsimilars[0]),similars[0]);
                 
#ifdef USINGBASH

#ifdef CONF_FILE
                 sprintf(filename3,"%s/../bin/bt-psbrtagaligner %s %s %s %s >%s/%s/%s-%s%06d.tmx",CONF_FILE,filename,LanguageName(i),filename2,
                       LanguageName(langsimilars[l]),directory,kDirFilename,LanguageName(i),LanguageName(langsimilars[l]),nalignments[i][langsimilars[l]]+1);  
#else
                 sprintf(filename3,"./bt-psbrtagaligner %s %s %s %s >%s/%s/%s-%s%06d.tmx",filename,LanguageName(i),filename2,LanguageName(langsimilars[l]),
                                   argv[1],kDirFilename,LanguageName(i),LanguageName(langsimilars[l]),nalignments[i][langsimilars[l]]+1);
#endif
                 system(filename3);
                 n_tmx++;
                 languagefound[langsimilars[l]]=true;
                 nalignments[i][langsimilars[l]]++;
#else                 
                 
                 
                 if(!(fent=fopen(filename2,"r")))
                   fprintf(stderr,"Error: File \"%s\" could not be opened while processing \"%s\"\n",filename2,filename);
                  else
                   {
                    fclose(fent);
                    aux2=new WebPage();
                    if(aux2->LoadContent(filename2))
                     {//Both files are loaded and the alignment can be performed
                      if(tagaligner->Align(aux->Content(),aux2->Content()))
                       {
                        alignment=tagaligner->GenerateTMX(LanguageName(i),LanguageName(langsimilars[0])); //Generate output as TMX
                        //alignment=tagaligner->GenerateTextAligned(); //Generate output as a plain text file
                      
                        //Write the alignment into harddisk
                        sprintf(filename3,"%s/%s/%s-%s%06d.tmx",directory,kDirFilename,LanguageName(i),LanguageName(langsimilars[l]),nalignments[i][langsimilars[l]]+1);
                        if(!(fsal=fopen(filename3,"w")))
                          fprintf(stderr,"Error: File \"%s\" could not be generated while processing \"%s\"\n",filename3,filename);
                         else
                          {
                           fprintf(fsal,"%s\n",alignment);
                           fclose(fsal);
                           n_tmx++;
                           languagefound[langsimilars[l]]=true;
                           nalignments[i][langsimilars[l]]++;
                          }
                        delete alignment;
                        alignment=NULL;
                       }
                     }
                    delete aux2;
                    aux2=NULL;
                   }
#endif                
                }
              }
            }
           delete aux;
           aux=NULL;
          }
       }
     }
    fprintf(stdout,"\nProcess finished: %d tmx generated in %d files processed\n",n_tmx,loaded);
    delete numfilesperlang;
    delete tagaligner;
   } 
 }
 
int main(int argc,char *argv[])
 {  
  Collector *collector;
  bool showuse;
  bool paramactivator[kNumPossibleOptions];
  char directory[kSizeAuxStrings]; //Directorio en el que se almacenan las paginas descargadas
  char directoryname[kSizeAuxStrings]; //Directorio que contiene el corpus descargado
  char statsfilename[kSizeAuxStrings]; //Path del fichero de configuracion
  int i;
  unsigned int numsimilars; //Contendra el numero de similares por defecto o el indicado por el usuario
  
  std::set_new_handler(OutOfMemory);
  
  numsimilars=0;
  collector=NULL;
  //Obtenemos la informacion de los parametros
  if(argc>1 && argc<8)
   {
    showuse=false;
    for(i=0;i<kNumPossibleOptions;i++)
      paramactivator[i]=false;

    for(i=1;i<argc;i++)
     {
      if(strcmp(argv[i],"-nbs")==0)
       {
        if(!paramactivator[0] && i<argc-1)
          paramactivator[0]=true;
         else
           showuse=true;
       }
       else
        {
         if(strcmp(argv[i],"-nsim")==0)
          {
           if(!paramactivator[1] && i<argc-2)
            {
             paramactivator[1]=true;
             numsimilars=atoi(argv[i+1]);
             i++;
            }
            else
              showuse=true;
          }
          else
           {
            if(strcmp(argv[i],"-nalign")==0)
             {
              if(!paramactivator[2] && !paramactivator[3] && i<argc-1)
                paramactivator[2]=true;
               else
                 showuse=true;
             }
             else
              {
               if(strcmp(argv[i],"-jalign")==0)
                {
                 if(!paramactivator[2] && !paramactivator[3] && i<argc-1)
                   paramactivator[3]=true;
                  else
                    showuse=true;
                }
              }
           }
        }
     }
   }
   else
     showuse=true;
  
  if(!showuse)
   {//Building the corpus from downloaded webs
    strcpy(directoryname,argv[argc-1]);
    if(directoryname[strlen(directoryname)-1]=='/')
      directoryname[strlen(directoryname)-1]='\0';
    sprintf(statsfilename,"%s/%s",directoryname,kNameStatsFile);
    
    if(!paramactivator[3])
     {
      collector=new Collector();
      if(collector->LoadResultsFile(statsfilename))
       {
        collector->GenerateCorpus(directoryname,kWholeCorpusDir);
        sprintf(directory,"%s/%s",directoryname,kWholeCorpusDir);
        if(directory!=NULL)
         {
          if(directory[strlen(directory)-1]=='/')//Eliminamos la '/' del final del directorio si existe
            directory[strlen(directory)-1]='\0';
          
          if(!paramactivator[0])
           {
            fprintf(stdout,"Building two ways similarity...\n");
            BuildSimilars(directory);
           }
          
          fprintf(stdout,"Choosing the best similars...\n");        
          if(!paramactivator[1])
            DiscardSimilars(20,directory);
           else
             DiscardSimilars(numsimilars*5,directory);
        
        
          fprintf(stdout,"Filtering similars using heavy heuristics...\n");
          SimilarsFilter(directory);
          fprintf(stdout,"Discarding further similars...\n");
          DiscardSimilars(1,directory);
          fprintf(stdout,"Erasing files without similars...\n");
          Cleaner(directory);
        
          if(!paramactivator[2])
           {
            fprintf(stdout,"Aligning and generating TMX files...\n");
            AlignWebsite(directory);
            fprintf(stdout,"The TMX has been successfully generated in \"./%s/tmx\"\n",directory);
           }
         }
       }
       else
        {//The file that contained the results of the last downloaded corpus was not correct
         fprintf(stderr,"Error: configuration file \'%s\' could not be loaded\n",statsfilename);
        }
      
     }
     else
      {
       fprintf(stdout,"Aligning and generating TMX files...\n");
       AlignWebsite(directoryname);
       fprintf(stdout,"The TMX has been successfully generated in \"./%s/tmx\"\n",directoryname);
      }
      
    delete collector;
   }
  if(showuse)
   {
    fprintf(stdout,"Usage:\t%s [options] downloaded_website\n",argv[0]);
    fprintf(stdout,"Options:\n");
    fprintf(stdout,"\t-nbs\t\t\tDo not build two ways similarity\n");
    fprintf(stdout,"\t-nsim <num>\t\tMax. similar number remaining after filtering\n");
    fprintf(stdout,"\t-nalign\t\t\tDo not perform the alignment\n");
    fprintf(stdout,"\t-jalign\t\t\tJust perform the alignment\n");
   }
  return(0);
 }
