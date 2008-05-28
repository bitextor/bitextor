/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

#include "heavyheuristics.h"

#define cout std::cout
#define cerr std::cerr
#define endl std::endl

//Uncomment this define to show all tags contained in the texts processed that
//are not xhtml standard tags
//#define SHOW_NON_XHTML_TAGS

HeavyHeuristics::HeavyHeuristics()
 {
  EDInsStructural=1;
  EDInsFormat=0.75;
  EDDelStructural=1;
  EDDelFormat=0.75;
  EDSubsStrucStruc=1.5;
  EDSubsStrucFormat=1.75; //This substitution would have no sense
  EDSubsFormatFormat=0.4; //The format could easily change between bitexts
  EDWeightperText=0.01; //It should be small to avoid significant interferences
  
  EDLightIns=1;
  EDLightDel=1;
  EDLightSubs=10; //Substitutions are not wanted in the light edit distance
  
  ALtextsizemin=20; 
  ALtextsizeminrelation=0.7;
 }
 
HeavyHeuristics::~HeavyHeuristics()
 {
 
 }
 
char* HeavyHeuristics::ObtainTagSequence(const char *text)
 {
  char *result;
  char *auxresult;
  char *actualtagname;
  unsigned int len,i,j;
  bool insidetag; //True when inside a tag
  bool insidecomment; //True when inside a comment
  bool insidecdata; //True when inside a <![CDATA[ ]]> structure
  bool recordingtag; //True when copying the tagname
  bool typetag; //True opening tag, False closing tag
  bool openingandclose; //True for tags of the <.../> type
  
  if(text!=NULL)
   {
    len=strlen(text);
    auxresult=new char[1+len];
    actualtagname=new char[kMaxSizeTag];
    insidetag=false;
    insidecomment=false;
    insidecdata=false;
    recordingtag=false;
    typetag=true;
    openingandclose=false;    
    for(i=0,j=0;i<len;i++)
     {
      if(insidetag)
       {
        if(insidecomment)
         {
          if(text[i]=='>' && text[i-1]=='-' && text[i-2]=='-')
           {
            insidecomment=false;
            insidetag=false;
           }
         }
         else
          {
           if(insidecdata)
            {
             if(text[i]=='>' && text[i-1]==']' && text[i-2]==']')
              {
               insidecdata=false;
               insidetag=false;
              }
            }
            else
             {
              if(text[i]=='/')
               {
                if(text[i+1]=='>' || j==0)
                 {
                  typetag=false;
                  if(j>0)
                    openingandclose=true;
                 }
               }
               else
                {
                 if(text[i]=='>')
                  {
                   insidetag=false;
                   recordingtag=false;
                   actualtagname[j]='\0';
                   if(openingandclose)
                    {//Duplicate the tag (tag /tag)
                     strcat(auxresult,actualtagname);
                     strcat(auxresult," ");
                    }
                   if(!typetag)
                     strcat(auxresult,"/");
                   strcat(auxresult,actualtagname);
                   strcat(auxresult," ");
                  }
                  else
                   {
                    if(j==0 && text[i]=='!' && i<len-2 && text[i+1]=='-' && text[i+2]=='-')
                      insidecomment=true;
                     else
                      {
                       if(j==0 && text[i]=='!' && i<len-7 && text[i+1]=='[' &&
                          text[i+2]=='C' && text[i+3]=='D' && text[i+4]=='A' &&
                          text[i+5]=='T' && text[i+6]=='A' && text[i+7]=='[')
                         insidecdata=true;
                        else
                         {
                          if(recordingtag)
                           {
                            if(text[i]!=' ' && text[i]!='\t' && text[i]!='\n')
                             {
                              actualtagname[j]=text[i];
                              j++;
                             }
                             else
                               recordingtag=false;
                           }
                         }
                      }
                   }
                }
             }
          }
       }
       else
        {
         if(text[i]=='<')
          {
           insidetag=true;
           recordingtag=true;
           typetag=true;
           openingandclose=false;
           j=0;
          }
        }
     }
    
    auxresult[strlen(auxresult)-1]='\0'; //Deleting the last blank
    result=new char[1+strlen(auxresult)];
    strcpy(result,auxresult);
    delete auxresult;
    delete actualtagname;
   }
   else
     result=NULL;
  return(result);
 }
 
short* HeavyHeuristics::ObtainNumericTagSequence(const char *text,unsigned int *ntags)
 {
  short *result;
  short *auxresult;
  short tagnumber;
  char *actualtagname;
  unsigned int len,i,j;
  bool insidetag; //True when inside a tag
  bool insidecomment; //True when inside a comment
  bool insidecdata; //True when inside a <![CDATA[ ]]> structure
  bool recordingtag; //True when copying the tagname
  bool typetag; //True opening tag, False closing tag
  bool openingandclose; //True for tags of the <.../> type
  
  if(text!=NULL)
   {
    len=strlen(text);
    auxresult=new short[1+len/4]; //4 would be the minimum size of a tag (<a/>)
    actualtagname=new char[kMaxSizeTag];
    insidetag=false;
    insidecomment=false;
    insidecdata=false;
    recordingtag=false;
    typetag=true;
    openingandclose=false;
    *ntags=0;
    for(i=0,j=0;i<len;i++)
     {
      if(insidetag)
       {
        if(insidecomment)
         {
          if(text[i]=='>' && text[i-1]=='-' && text[i-2]=='-')
           {
            insidecomment=false;
            insidetag=false;
           }
         }
         else
          {
           if(insidecdata)
            {
             if(text[i]=='>' && text[i-1]==']' && text[i-2]==']')
              {
               insidecdata=false;
               insidetag=false;
              }
            }
            else
             {
              if(text[i]=='/')
               {
                if(text[i+1]=='>' || j==0)
                 {
                  typetag=false;
                  if(j>0)
                    openingandclose=true;
                 }
               }
               else
                {
                 if(text[i]=='>')
                  {
                   insidetag=false;
                   recordingtag=false;
                   actualtagname[j]='\0';
                   tagnumber=GetXHTMLTagNumber(actualtagname);
                   if(tagnumber!=0)
                    {
                     if(openingandclose)
                      {//Duplicate the tag (tag /tag)
                       auxresult[*ntags]=tagnumber;
                       (*ntags)++;
                      }                   
                     auxresult[*ntags]=tagnumber;
                     if(!typetag)
                       auxresult[*ntags]=(-1)*auxresult[*ntags];
                     (*ntags)++;
                    }
                    else
                     {
#ifdef SHOW_NON_XHTML_TAGS
                      fprintf(stderr,"Warning: Unknown tag \'%s\'\n",actualtagname);
#endif                       
                     }
                  }
                  else
                   {
                    if(j==0 && text[i]=='!' && i<len-2 && text[i+1]=='-' && text[i+2]=='-')
                      insidecomment=true;
                     else
                      {
                       if(j==0 && text[i]=='!' && i<len-7 && text[i+1]=='[' &&
                          text[i+2]=='C' && text[i+3]=='D' && text[i+4]=='A' &&
                          text[i+5]=='T' && text[i+6]=='A' && text[i+7]=='[')
                         insidecdata=true;
                        else
                         {
                          if(recordingtag)
                           {
                            if(text[i]!=' ' && text[i]!='\t' && text[i]!='\n')
                             {
                              actualtagname[j]=text[i];
                              j++;
                             }
                             else
                               recordingtag=false;
                           }
                         }
                      }
                   }
                }
             }
          }
       }
       else
        {
         if(text[i]=='<')
          {
           insidetag=true;
           recordingtag=true;
           typetag=true;
           openingandclose=false;
           j=0;
          }
        }
     }
    
    result=new short[*ntags];
    memcpy(result,auxresult,(*ntags)*sizeof(short));
    delete auxresult;
    delete actualtagname;
   }
   else
     result=NULL;
  return(result);
 }

short* HeavyHeuristics::FilterTagSequence(const short* ts,unsigned int tssize,unsigned int* tssizeresult)
 {
  short *result;
  short *auxresult;
  unsigned int i;
  unsigned short tagtype;
  
  auxresult=new short[tssize];
  for(i=0,*tssizeresult=0;i<tssize;i++)
   {
    tagtype=GetXHTMLTagType((unsigned short)abs(ts[i]));
    if(tagtype==kTAGTypeStruct || tagtype==kTAGTypeFormat || ts[i]>kFirstLength)
     {
      auxresult[*tssizeresult]=ts[i];
      (*tssizeresult)++;
     }
   }
  result=new short[*tssizeresult];
  memcpy(result,auxresult,(*tssizeresult)*sizeof(short));
  delete auxresult;
  return(result);
 }

unsigned short* HeavyHeuristics::CountTagsFromSequence(const short* ts,unsigned int tssize)
 {
  unsigned short *result;
  unsigned int i;

  result=new unsigned short[kNumberTags+1];
  for(i=0;i<kNumberTags+1;i++)
    result[i]=0;
  for(i=0;i<tssize;i++)
   {
    if(ts[i]<0)
      result[0]++;
     else
       result[ts[i]]++;
   }
   
  /* //This code allows to count also the number of closing tags
     //instead of only the opening ones
  result=new unsigned short[2*(kNumberTags+1)];
  for(i=0;i<kNumberTags+1;i++)
    result[i]=0;
  for(i=0;i<tssize;i++)
   {
    if(ts[i]<0)
      result[ts[i]*2*(-1)]++;
     else
       result[ts[i]]++;
   }*/
   
  return(result);
 }

double HeavyHeuristics::CountsDistance(const unsigned short *count1,const unsigned short *count2)
 {
  double result;
  unsigned int i;
    
  if(count1!=NULL && count2!=NULL)
   {
    for(i=1,result=0;i<kNumberTags+1;i++)
      result+=pow(count1[i]-count2[i],2);
    result=sqrt(result);
   }
   else
     result=-1;
  return(result);
 }

double HeavyHeuristics::LightEditDistance(const char* text1,const char* text2)
 {
  double result;
  double **matrix;
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  unsigned int i,j;
  unsigned int len1,len2;
  
  if(text1!=NULL && text2!=NULL)
   {
    len1=strlen(text1);
    len2=strlen(text2);
    if(len1>0 && len2>0)
     {
      matrix=new double*[2];//We keep only 2 rows of the matrix
      matrix[0]=NULL;
      matrix[1]=new double[len2+1];

      startingcol=0; endingcol=len2;//Initially the whole row is processed
      nextstartingcol=0; nextendingcol=len2;
    
      //Inicialization of the first row: only insertion costs
      matrix[1][0]=0;
      for(j=0;j<len2;j++)
        matrix[1][j+1]=matrix[1][j]+EDLightIns;
      
      startingcol=nextstartingcol;
      endingcol=nextendingcol;
      if(endingcol<len2)
        endingcol++;//The ending column increase each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<len2;j++)
  fprintf(stdout,"      %d",text2[j]);
fprintf(stdout,"\n0\n");
#endif
    
      for(i=0;i<len1;i++)
       {
        delete matrix[0];
        matrix[0]=matrix[1];
        matrix[1]=new double[len2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<len2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",text1[i]);
#endif
        for(j=0;j<len2+1;j++)
          matrix[1][j]=-1;

            
        //Initialization of the first column of the row
        matrix[1][startingcol]=EDLightDel+matrix[0][startingcol];
         
        if(matrix[1][startingcol]>len1+len2)
         {//This path should be discarded in the next row
          nextstartingcol++;
         }
      
        for(j=startingcol;j<endingcol;j++)
         {
          //Substitution cost
          subscost=matrix[0][j];
          if(text1[i]!=text2[j])
            subscost+=EDLightSubs;
        
          //Insertion cost
          inscost=matrix[1][j]+EDLightIns;

          //Deletion cost
          delcost=matrix[0][j+1]+EDLightDel;
        
          //Choosing the minimal cost
          if(subscost<inscost)
           {
            if(subscost<delcost)
              matrix[1][j+1]=subscost;
             else
               matrix[1][j+1]=delcost;
           }
           else
            {
             if(inscost<delcost)
               matrix[1][j+1]=inscost;
              else
                matrix[1][j+1]=delcost;
            }
         }
        startingcol=nextstartingcol;
        if(nextendingcol<len2)
          nextendingcol++;//The ending column increase each row until the maximum
        endingcol=nextendingcol;
       }    

      for(;j<len2;j++)
        matrix[1][j+1]=matrix[1][j]+EDLightIns;
     
#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<len2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif
    
      if(startingcol==endingcol || matrix[1][len2]==-1)
        result=len1+len2;
       else
         result=matrix[1][len2];
      delete matrix[0];
      delete matrix[1];
      delete matrix;
      matrix=NULL;
     }
     else
       result=-1;
   }
   else
     result=-1;
  return(result);
 }

inline double HeavyHeuristics::EditDistanceTagSequence(const char* text1,const char* text2,double maxvalue)
 {
  double result;
  short *ts1,*ts2;
  unsigned int tssize1,tssize2;
  short *fts1,*fts2;
  unsigned int ftssize1,ftssize2;
  
  ts1=ObtainNumericTagSequence(text1,&tssize1);
  ts2=ObtainNumericTagSequence(text2,&tssize2);
  if(ts1!=NULL && ts2!=NULL)
   {
    fts1=FilterTagSequence(ts1,tssize1,&ftssize1);
    fts2=FilterTagSequence(ts2,tssize2,&ftssize2);
    result=EditDistanceNumTagSeq(fts1,ftssize1,fts2,ftssize2,maxvalue);
    delete fts1;
    delete fts2;
   }
   else
     result=-1;
  delete ts1;
  delete ts2;
  return(result);
 }

double HeavyHeuristics::EditDistanceNumTagSeq(const short* ts1,unsigned int tssize1,const short* ts2,unsigned int tssize2,double maxvalue)
 {
  double result;
  double **matrix;
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  unsigned int i,j;
  
  if(ts1!=NULL && ts2!=NULL && tssize1>0 && tssize2>0)
   {
    matrix=new double*[2];//We keep only 2 rows of the matrix
    matrix[0]=NULL;
    matrix[1]=new double[tssize2+1];

    startingcol=0; endingcol=tssize2;//Initially the whole row is processed
    nextstartingcol=0; nextendingcol=tssize2;
    
    //Inicialization of the first row: only insertion costs
    matrix[1][0]=0;
    for(j=0;j<tssize2;j++)
     {
      if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
        matrix[1][j+1]=matrix[1][j]+EDInsStructural;
       else
         matrix[1][j+1]=matrix[1][j]+EDInsFormat;
         
      if(matrix[1][j+1]>maxvalue)
       {//This path should be discarded in the next row
        if(startingcol==j)
          nextstartingcol++;
         else
           nextendingcol--;
       }
     }
    startingcol=nextstartingcol;
    endingcol=nextendingcol;
    if(endingcol<tssize2)
      endingcol++;//The ending column increase each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<tssize2;j++)
  fprintf(stdout,"      %d",ts2[j]);
fprintf(stdout,"\n0\n");
#endif
    
    for(i=0;i<tssize1;i++)
     {
      delete matrix[0];
      matrix[0]=matrix[1];
      matrix[1]=new double[tssize2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",ts1[i]);
#endif
      for(j=0;j<tssize2+1;j++)
        matrix[1][j]=-1;

            
      //Initialization of the first column of the row
      if(GetXHTMLTagType((unsigned short)abs(ts1[i]))==kTAGTypeStruct)
        matrix[1][startingcol]=EDDelStructural+matrix[0][startingcol];
       else
         matrix[1][startingcol]=EDDelFormat+matrix[0][startingcol];
         
      if(matrix[1][startingcol]>maxvalue)
       {//This path should be discarded in the next row
        nextstartingcol++;
       }
      
      for(j=startingcol;j<endingcol;j++)
       {
        //Substitution cost
        subscost=matrix[0][j];
        if(GetXHTMLTagType((unsigned short)abs(ts1[i]))==kTAGTypeStruct)
         {
          if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
           {
            if(ts1[i]!=ts2[j])
             subscost+=EDSubsStrucStruc;
           }
           else
             subscost+=EDSubsStrucFormat;
         }
         else
          {
           if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
             subscost+=EDSubsStrucFormat;
            else
             {
              if(ts1[i]!=ts2[j])
                subscost+=EDSubsFormatFormat;
             }
          }
        
        //Insertion cost
        inscost=matrix[1][j];
        if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
          inscost+=EDInsStructural;
         else
           inscost+=EDInsFormat;

        //Deletion cost
        if(endingcol!=j+1)
         {
          delcost=matrix[0][j+1];
          if(GetXHTMLTagType((unsigned short)abs(ts1[i]))==kTAGTypeStruct)
            delcost+=EDDelStructural;
           else
             delcost+=EDDelFormat;
         }
         else
           delcost=maxvalue;
        
        //Choosing the minimal cost
        if(subscost<inscost)
         {
          if(subscost<delcost)
            matrix[1][j+1]=subscost;
           else
             matrix[1][j+1]=delcost;
         }
         else
          {
           if(inscost<delcost)
             matrix[1][j+1]=inscost;
            else
              matrix[1][j+1]=delcost;
          }
        if(matrix[1][j+1]>maxvalue)
         {//This path should be discarded in the next row
          if(startingcol==j)
            nextstartingcol++;
           else
             nextendingcol--;
         }
        
       }
      startingcol=nextstartingcol;
      if(nextendingcol<tssize2)
        nextendingcol++;//The ending column increase each row until the maximum
      endingcol=nextendingcol;
     }    

    for(;j<tssize2;j++)
     {
      inscost=matrix[1][j];
      if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
        inscost+=EDInsStructural;
       else
         inscost+=EDInsFormat;
      matrix[1][j+1]=inscost;
     }
     
#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif
    
    if(startingcol==endingcol || matrix[1][tssize2]==-1)
      result=maxvalue;
     else
       result=matrix[1][tssize2];
    delete matrix[0];
    delete matrix[1];
    delete matrix;
    matrix=NULL;
   }
   else
     result=-1;
  return(result);
 }

char* HeavyHeuristics::EditDistancePathNumTagSeq(const short *ts1,unsigned int tssize1,const short *ts2,unsigned int tssize2,double maxvalue)
 {
  char *result;
  char *auxresult;
  double **matrix;
  char **route; //Prior element in the route of the minimal edit distance
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  unsigned int i,j,k;
  
  if(ts1!=NULL && ts2!=NULL && tssize1>0 && tssize2>0)
   {
    route=new char*[tssize1+1];
    matrix=new double*[2];//We keep only 2 rows of the matrix
    matrix[0]=NULL;
    matrix[1]=new double[tssize2+1];

    startingcol=0; endingcol=tssize2;//Initially the whole row is processed
    nextstartingcol=0; nextendingcol=tssize2;
    
    //Inicialization of the first row: only insertion costs
    matrix[1][0]=0;
    route[0]=new char[tssize2+1];
    route[0][0]='e';
    for(j=0;j<tssize2;j++)
     {
      if(GetXHTMLTagType((unsigned short)abs(ts2[j]))==kTAGTypeStruct)
        matrix[1][j+1]=matrix[1][j]+EDInsStructural;
       else
         matrix[1][j+1]=matrix[1][j]+EDInsFormat;
      route[0][j+1]='i';   
      if(matrix[1][j+1]>maxvalue)
       {//This path should be discarded in the next row
        if(startingcol==j)
          nextstartingcol++;
         else
           nextendingcol--;
       }
     }
    startingcol=nextstartingcol;
    endingcol=nextendingcol;
    if(endingcol<tssize2)
      endingcol++;//The ending column increase each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<tssize2;j++)
  fprintf(stdout,"      %d",ts2[j]);
fprintf(stdout,"\n0\n");
#endif
    
    for(i=0;i<tssize1;i++)
     {
      route[i+1]=new char[tssize2+1];
      delete matrix[0];
      matrix[0]=matrix[1];
      matrix[1]=new double[tssize2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",ts1[i]);
#endif
      for(j=0;j<tssize2+1;j++)
       {
        matrix[1][j]=-1;
        route[i+1][j]='\0';
       }

      //Initialization of the first column of the row
      if(GetXHTMLTagType((unsigned short)abs(ts1[i]))==kTAGTypeStruct)
        matrix[1][startingcol]=EDDelStructural+matrix[0][startingcol];
       else
         matrix[1][startingcol]=EDDelFormat+matrix[0][startingcol];
      route[i+1][startingcol]='d';
         
      if(matrix[1][startingcol]>maxvalue)
       {//This path should be discarded in the next row
        nextstartingcol++;
       }
      
      for(j=startingcol;j<endingcol;j++)
       {
        //Substitution cost
        subscost=matrix[0][j];
        
        switch(GetXHTMLTagType((unsigned short)abs(ts1[i])))
         {
          case kTAGTypeStruct: switch(GetXHTMLTagType((unsigned short)abs(ts2[j])))
                                {
                                 case kTAGTypeStruct: if(ts1[i]!=ts2[j])
                                                        subscost+=EDSubsStrucStruc;
                                                      break;
                                 
                                 case kTAGTypeFormat: subscost+=EDSubsStrucFormat;
                                                      break;
                                 default: if(ts2[j]>kFirstLength)
                                           {//It is a text
                                            subscost+=kBigValue; //It has no sense to replace a tag with text
                                           }
                                          break;
                                }
                               
                               break;
          
          case kTAGTypeFormat: switch(GetXHTMLTagType((unsigned short)abs(ts2[j])))
                                {
                                 case kTAGTypeStruct: subscost+=EDSubsStrucFormat;
                                                      break;
                                 
                                 case kTAGTypeFormat: if(ts1[i]!=ts2[j])
                                                        subscost+=EDSubsFormatFormat;
                                                      break;
                                 default: if(ts2[j]>kFirstLength)
                                           {//It is a text
                                            subscost+=kBigValue; //It has no sense to replace a tag with text
                                           }
                                          break;
                                }
                               
                               break;
          
          default: if(ts1[i]>kFirstLength)
                    {//It is a text
                     switch(GetXHTMLTagType((unsigned short)abs(ts2[j])))
                      {
                       case kTAGTypeStruct: subscost+=kBigValue; //It has no sense to replace a tag with text
                                            break;
                                 
                       case kTAGTypeFormat: subscost+=kBigValue; //It has no sense to replace a tag with text
                                            break;
                                
                       default: if(ts2[j]>kFirstLength)
                                 {//It is a text
                                  subscost+=EDWeightperText*abs(ts1[i]-ts2[j]); //The substitution is the difference in size
                                 }
                                break;
                      }
                    }
                   break;
         }
                
        //Insertion cost
        inscost=matrix[1][j];
        switch(GetXHTMLTagType((unsigned short)abs(ts2[j])))
         {
          case kTAGTypeStruct: inscost+=EDInsStructural;
                               break;
          case kTAGTypeFormat: inscost+=EDInsFormat;
                               break;
          default: if(ts2[j]>kFirstLength)
                     inscost+=EDWeightperText*(ts2[j]-kFirstLength);
                   break;
         }

        //Deletion cost
        if(endingcol!=j+1)
         {
          delcost=matrix[0][j+1];
          switch(GetXHTMLTagType((unsigned short)abs(ts1[i])))
           {
            case kTAGTypeStruct: delcost+=EDDelStructural;
                                 break;
            case kTAGTypeFormat: delcost+=EDDelFormat;
                                 break;
            default: if(ts1[i]>kFirstLength)
                       delcost+=EDWeightperText*(ts1[i]-kFirstLength);
                     break;
           }
         }
         else
           delcost=maxvalue;
        
        //Choosing the minimal cost
        if(subscost<=inscost)
         {
          if(subscost<=delcost)
           {
            matrix[1][j+1]=subscost;
            route[i+1][j+1]='s';
           }
           else
            {
             matrix[1][j+1]=delcost;
             route[i+1][j+1]='d';
            }
         }
         else
          {
           if(inscost<delcost)
            {
             matrix[1][j+1]=inscost;
             route[i+1][j+1]='i';
            }
            else
             {
              matrix[1][j+1]=delcost;
              route[i+1][j+1]='d';
             }
          }
        if(matrix[1][j+1]>maxvalue)
         {//This path should be discarded in the next row
          if(startingcol==j)
            nextstartingcol++;
           else
             nextendingcol--;
         }
        
       }
      startingcol=nextstartingcol;
      if(nextendingcol<tssize2)
        nextendingcol++;//The ending column increase each row until the maximum
      endingcol=nextendingcol;
     }    

    for(;j<tssize2;j++)
     {
      inscost=matrix[1][j];
      switch(GetXHTMLTagType((unsigned short)abs(ts2[j])))
       {
        case kTAGTypeStruct: inscost+=EDInsStructural;
                             break;
        case kTAGTypeFormat: inscost+=EDInsFormat;
                             break;
        default: if(ts2[j]>kFirstLength)
                   inscost+=EDWeightperText*(ts2[j]-kFirstLength);
                 break;
       }
      matrix[1][j+1]=inscost;
      route[i][j+1]='i';
     }
#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif
    
    if(startingcol==endingcol || route[tssize1][tssize2]=='\0')
      result=NULL;
     else
      {//Building the path
       auxresult=new char[tssize1+tssize2+1];
       auxresult[tssize1+tssize2]='\0';
       i=tssize1; j=tssize2;
       for(k=tssize1+tssize2-1;route[i][j]!='e' && k>=0;k--)
        {
         auxresult[k]=route[i][j];
         switch(route[i][j])
          {
           case 'd': i--;
                     break;
                     
           case 'i': j--;
                     break;
           
           case 's': i--;
                     j--;
                     break;
          }
        }
       k++;
       result=new char[1+strlen(auxresult+k)];
       strcpy(result,auxresult+k);
       delete auxresult;
       auxresult=NULL;
      }
    
    delete matrix[0];
    delete matrix[1];
    delete matrix;
    matrix=NULL;
    for(i=0;i<tssize1+1;i++)
      delete route[i];
    delete route;
    route=NULL;
   }
   else
     result=NULL;
  return(result);
 }



char* HeavyHeuristics::EditDistancePathSizeText(const short *ts1,unsigned int tssize1,const short *ts2,unsigned int tssize2,double maxvalue)
 {
  char *result;
  char *auxresult;
  double **matrix;
  char **route; //Prior element in the route of the minimal edit distance
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  unsigned int i,j,k;
  
  if(ts1!=NULL && ts2!=NULL && tssize1>0 && tssize2>0)
   {
    route=new char*[tssize1+1];
    matrix=new double*[2];//We keep only 2 rows of the matrix
    matrix[0]=NULL;
    matrix[1]=new double[tssize2+1];

    startingcol=0; endingcol=tssize2;//Initially the whole row is processed
    nextstartingcol=0; nextendingcol=tssize2;
    
    //Inicialization of the first row: only insertion costs
    matrix[1][0]=0;
    route[0]=new char[tssize2+1];
    route[0][0]='e';
    for(j=0;j<tssize2;j++)
     {
      matrix[1][j+1]=matrix[1][j]+EDWeightperText*ts2[j];
      route[0][j+1]='i';   
      if(matrix[1][j+1]>maxvalue)
       {//This path should be discarded in the next row
        if(startingcol==j)
          nextstartingcol++;
         else
           nextendingcol--;
       }
     }
    startingcol=nextstartingcol;
    endingcol=nextendingcol;
    if(endingcol<tssize2)
      endingcol++;//The ending column increase each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<tssize2;j++)
  fprintf(stdout,"      %d",ts2[j]);
fprintf(stdout,"\n0\n");
#endif
    
    for(i=0;i<tssize1;i++)
     {
      route[i+1]=new char[tssize2+1];
      delete matrix[0];
      matrix[0]=matrix[1];
      matrix[1]=new double[tssize2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",ts1[i]);
#endif
      for(j=0;j<tssize2+1;j++)
       {
        matrix[1][j]=-1;
        route[i+1][j]='\0';
       }

      //Initialization of the first column of the row
      matrix[1][startingcol]=matrix[0][startingcol]+EDWeightperText*ts1[i];
      route[i+1][startingcol]='d';
         
      if(matrix[1][startingcol]>maxvalue)
       {//This path should be discarded in the next row
        nextstartingcol++;
       }
      
      for(j=startingcol;j<endingcol;j++)
       {
        //Substitution cost
        subscost=matrix[0][j];
        if((unsigned)ts1[i]>ALtextsizemin || (unsigned)ts2[i]>ALtextsizemin)
         {//The difference between the text sizes could be very high
          if(ts1[i]<ALtextsizeminrelation*ts2[j] || ts2[j]<ALtextsizeminrelation*ts1[i])
            subscost+=kBigValue;
           else
             subscost+=EDWeightperText*abs(ts1[i]-ts2[j]);
         }
         else
           subscost+=EDWeightperText*abs(ts1[i]-ts2[j]);//The substitution cost is calculated directly

        //Insertion cost
        inscost=matrix[1][j]+EDWeightperText*ts2[j];

        //Deletion cost
        if(endingcol!=j+1)
          delcost=matrix[0][j+1]+EDWeightperText*ts1[i];
         else
           delcost=maxvalue;
        
        //Choosing the minimal cost
        if(subscost<=inscost)
         {
          if(subscost<=delcost)
           {
            matrix[1][j+1]=subscost;
            route[i+1][j+1]='s';
           }
           else
            {
             matrix[1][j+1]=delcost;
             route[i+1][j+1]='d';
            }
         }
         else
          {
           if(inscost<delcost)
            {
             matrix[1][j+1]=inscost;
             route[i+1][j+1]='i';
            }
            else
             {
              matrix[1][j+1]=delcost;
              route[i+1][j+1]='d';
             }
          }
        if(matrix[1][j+1]>maxvalue)
         {//This path should be discarded in the next row
          if(startingcol==j)
            nextstartingcol++;
           else
             nextendingcol--;
         }
        
       }
      startingcol=nextstartingcol;
      if(nextendingcol<tssize2)
        nextendingcol++;//The ending column increase each row until the maximum
      endingcol=nextendingcol;
     }    

    for(;j<tssize2;j++)
     {
      inscost=matrix[1][j]+EDWeightperText*ts2[j];
      matrix[1][j+1]=inscost;
      route[i][j+1]='i';
     }
#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<tssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif
    
    if(startingcol==endingcol || route[tssize1][tssize2]=='\0')
      result=NULL;
     else
      {//Building the path
       auxresult=new char[tssize1+tssize2+1];
       auxresult[tssize1+tssize2]='\0';
       i=tssize1; j=tssize2;
       for(k=tssize1+tssize2-1;route[i][j]!='e' && k>=0;k--)
        {
         auxresult[k]=route[i][j];
         switch(route[i][j])
          {
           case 'd': i--;
                     break;
                     
           case 'i': j--;
                     break;
           
           case 's': i--;
                     j--;
                     break;
          }
        }
       k++;
       result=new char[1+strlen(auxresult+k)];
       strcpy(result,auxresult+k);
       delete auxresult;
       auxresult=NULL;
      }
    
    delete matrix[0];
    delete matrix[1];
    delete matrix;
    matrix=NULL;
    for(i=0;i<tssize1+1;i++)
      delete route[i];
    delete route;
    route=NULL;
   }
   else
     result=NULL;
  return(result);
 }

char** HeavyHeuristics::AlignText(const char* ltext,const char* rtext,unsigned int* n_segm)
 {
  char **result;
  char **lsegments,**rsegments; //Segments in which the texts are divided
  unsigned int n_lsegments,n_rsegments; //Number of segments in which the texts are divided
  short *lvector,*rvector; //Vector of characteristics of each segment to align them
  char *pathdistance;//Path of the edit distance
  char **lalignments,**ralignments;//Vector de segmentos alineados
  unsigned int n_ins,n_subs,n_del,i,j,k; //Several counters

  if(ltext!=NULL && rtext!=NULL)
   {
    //Both texts are splitted into sentences
    lsegments=SentenceSplitter(ltext,&n_lsegments);
    rsegments=SentenceSplitter(rtext,&n_rsegments);

    //The characteristic value of each fragment is calculated (number of words)
    lvector=new short[n_lsegments];
    rvector=new short[n_rsegments];
    for(i=0;i<n_lsegments;i++)
     {
      lvector[i]=0;
      for(j=1;j<strlen(lsegments[i]);j++)
       {
        if(lsegments[i][j]==' ' || lsegments[i][j]=='\n' || lsegments[i][j]=='\t')
         {//There is an empty space
          if(lsegments[i][j-1]==' ' || lsegments[i][j-1]=='\n' || lsegments[i][j-1]=='\t')
           {//It was not the first one
            
           }
           else
             lvector[i]++;
         }
         else
           lvector[i]++;
       }
      if(lsegments[i][j-1]!=' ' && lsegments[i][j-1]!='\n' && lsegments[i][j-1]!='\t')
        lvector[i]++;
     }
    for(i=0;i<n_rsegments;i++)
     {
      rvector[i]=0;
      for(j=1;j<strlen(rsegments[i]);j++)
       {
        if(rsegments[i][j]==' ' || rsegments[i][j]=='\n' || rsegments[i][j]=='\t')
         {//There is an empty space
          if(rsegments[i][j-1]==' ' || rsegments[i][j-1]=='\n' || rsegments[i][j-1]=='\t')
           {//It was not the first one
            
           }
           else
             rvector[i]++;
         }
         else
           rvector[i]++;
       }
      if(rsegments[i][j-1]!=' ' && rsegments[i][j-1]!='\n' && rsegments[i][j-1]!='\t')
        rvector[i]++;
     }
     
    //pathdistance=EditDistancePathNumTagSeq(lvector,n_lsegments,rvector,n_rsegments,1000);
    pathdistance=EditDistancePathSizeText(lvector,n_lsegments,rvector,n_rsegments,1000);
    delete lvector;
    lvector=NULL;
    delete rvector;
    rvector=NULL;
  
    if(pathdistance!=NULL)
     {
      for(i=0,n_subs=0,n_del=0,n_ins=0;pathdistance[i]!='\0';i++)
       {
        switch(pathdistance[i])
         {
          case 's': n_subs++;
                    break;
          case 'd': n_del++;
                    break;
          case 'i': n_ins++;
                    break;
         }
       }
    
      //Generate the alignment
      lalignments=new char*[n_lsegments+n_ins+n_del];
      ralignments=new char*[n_rsegments+n_ins+n_del];
      for(i=0,j=0,k=0;pathdistance[i]!='\0';i++)
       {
        switch(pathdistance[i])
         {
          case 's': lalignments[i]=lsegments[j];
                    ralignments[i]=rsegments[k];
                    j++;
                    k++;
                    break;
  
          case 'd': lalignments[i]=lsegments[j];
                    ralignments[i]=NULL;
                    j++;
                    break;
      
          case 'i': lalignments[i]=NULL;
                    ralignments[i]=rsegments[k];
                    k++;
                    break;
         }
       }
      *n_segm=i;

      result=new char*[2*i];
      for(j=0;j<i;j++)
        result[j]=lalignments[j];
      for(j=0;j<i;j++)
        result[j+i]=ralignments[j];
      delete lalignments;
      lalignments=NULL;
      delete ralignments;
      ralignments=NULL;
      delete pathdistance;
      pathdistance=NULL;
     }
     else
      {
       *n_segm=0;
       result=NULL;
      }
    delete lsegments;
    lsegments=NULL;
    delete rsegments;
    rsegments=NULL;
   }
   else
    {//One of the texts is empty so, the other one should be aligned to empty segments
     if(ltext!=NULL)
      {
       lsegments=SentenceSplitter(ltext,&n_lsegments);
       result=new char*[2*n_lsegments];
       for(j=0;j<n_lsegments;j++)
         result[j]=lsegments[j];
       for(j=0;j<n_lsegments;j++)
         result[j+n_lsegments]=NULL;
       delete lsegments;
       lsegments=NULL;
       *n_segm=n_lsegments;
      }
      else
       {
        if(rtext!=NULL)
         {
          rsegments=SentenceSplitter(rtext,&n_rsegments);
          result=new char*[2*n_rsegments];
          for(j=0;j<n_rsegments;j++)
            result[j]=NULL;
          for(j=0;j<n_rsegments;j++)
            result[j+n_rsegments]=rsegments[j];
          delete rsegments;
          rsegments=NULL;
          *n_segm=n_rsegments;
         }
         else
          {//Both texts were empty (this could be an error)
           *n_segm=0;
           result=NULL;          
          }
       }
    }
  return(result);
 }
