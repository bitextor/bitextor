/*
   Source code of the aligner 2-1. It is designed to allow webpage alignment after
   turning them into XHTML files. The alignment is performed in a single step, where
   sentences and tags are never aligned between them.

   Source code of the aligner 2 steps with alignment distance. It is designed
   to allow webpage alignment after turning them into XHTML files. The alignment
   is performed in two steps: 
    - In the first one, tags and text chunks are aligned between them.
    - In the second one, aligned text chunks are splitted in sentences and are aligned
      at the sentence-level.

   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

#include "tagaligner2-1.h"


TagAligner2_1::TagAligner2_1()
 {
  lphrases=NULL;
  rphrases=NULL;
  lvector=NULL;
  rvector=NULL;
  bestalignment=NULL;
  n_lphrases=0;
  n_rphrases=0;
  n_bestalignment=0;
  lsegments=NULL;
  rsegments=NULL;
  n_segments=0;
  
  EDInsStructural=1;
  EDInsFormat=0.75;
  EDInsContent=1.25;
  EDInsText=0.01;
  EDDelStructural=1;
  EDDelFormat=0.75;
  EDDelContent=1.25;
  EDDelText=0.01;
  EDSubsStrucStruc=1.5;
  EDSubsStrucFormat=1.75; //This substitution would have no sense
  EDSubsStrucContent=kBigValue;
  EDSubsFormatFormat=0.4; //The format could easily change between bitexts
  EDSubsFormatContent=kBigValue;
  EDSubsContentContent=kBigValue;
  EDSubsText=0.01; //It should be small to avoid significant interferences
  
  ALtextsizemin=20; 
  ALtextsizeminrelation=0.6;
 }

TagAligner2_1::~TagAligner2_1()
 {
  Reset();
 }

void TagAligner2_1::Reset()
 {
  unsigned int i;
  
  if(lphrases!=NULL)
   {
    for(i=0;i<n_lphrases;i++)
     {
      if(lphrases[i]!=NULL)
        delete lphrases[i];
     }
    delete lphrases;
    lphrases=NULL;
   }
  n_lphrases=0;
  if(rphrases!=NULL)
   {
    for(i=0;i<n_rphrases;i++)
     {
      if(rphrases[i]!=NULL)
        delete rphrases[i];
     }
    delete rphrases;
    rphrases=NULL;
   }
  n_rphrases=0;
  if(lvector!=NULL)
   {
    delete lvector;
    lvector=NULL;
   }
  if(rvector!=NULL)
   {
    delete rvector;
    rvector=NULL;
   }
  if(bestalignment!=NULL)
   {
    delete bestalignment;
    bestalignment=NULL;
   }
  n_bestalignment=0;  
  n_segments=0;
 }
 
char** TagAligner2_1::SplitFilterText(const char *text,unsigned int *numberphrases)
 {
  char **result;
  unsigned int len,i,j,k,l;
  char *auxiliarstring;
  char **auxresult;
  char *buffertext; //Used for storing text while reading it  
  char actualtagname[kSizeAuxStrings]; //Used for storing tags while reading them
  bool insidetag; //True when inside a tag
  bool insidecomment; //True when inside a comment
  bool insidecdata; //True when inside a <![CDATA[ ]]> structure
  bool recordingtag; //True when copying the tagname
  bool typetag; //True opening tag, False closing tag
  bool openingandclose; //True for tags of the <.../> type
  bool joiningtext; //Indicates if we are reading a text that has to be joined to the previous one (they were divided by an irrelevant tag)
  bool lookingtagend; //Indicates when the end of a tag style or script has been found
  
  if(text!=NULL)
   {
    result=NULL;
    *numberphrases=0;
    len=strlen(text);
    buffertext=new char[1+len];
    insidetag=false;
    k=0;
    insidecomment=false;
    insidecdata=false;
    recordingtag=false;
    typetag=true;
    openingandclose=false;
    joiningtext=false;
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
            k=0;
           }
         }
         else
          {
           if(insidecdata)
            {
             if(text[i]=='>' && text[i-1]==']' && text[i-2]==']')
              {
               joiningtext=true;
               insidecdata=false;
               insidetag=false;
               k=0;
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
                   
                   if(GetXHTMLTagType(GetXHTMLTagNumber(actualtagname))==kTAGTypeIrrelevant || strcmp(actualtagname,"style")==0)
                    {//This tag should be completely ignored
                     if(strcmp(actualtagname,"style")==0 || strcmp(actualtagname,"script")==0)
                      {//Special case: The text inside the tags style and script is discarded
                       for(i++,lookingtagend=false;!lookingtagend;i++)
                        {
                         if(text[i]=='>')
                          {
                           if(text[i-8]=='<' && text[i-7]=='/' && text[i-6]=='s' && text[i-5]=='c' && 
                              text[i-4]=='r' && text[i-3]=='i' && text[i-2]=='p' && text[i-1]=='t' && actualtagname[1]=='c')
                             lookingtagend=true;
                           if(text[i-7]=='<' && text[i-6]=='/' && text[i-5]=='s' && text[i-4]=='t' &&
                              text[i-3]=='y' && text[i-2]=='l' && text[i-1]=='e' && actualtagname[1]=='t')
                             lookingtagend=true;
                          }
                        }
                       i--;
                      }
                     
                     joiningtext=true;
                    }
                    else
                     {
                      joiningtext=false;
                      if(!openingandclose)
                        auxresult=new char*[*numberphrases+1];
                       else
                        auxresult=new char*[*numberphrases+2];
                      for(k=0;k<*numberphrases;k++)
                        auxresult[k]=result[k];
                      auxresult[k]=new char[strlen(actualtagname)+5];
                      if(openingandclose)
                       {//Duplicate the tag (tag /tag)
                        strcpy(auxresult[k],"<");
                        strcat(auxresult[k],actualtagname);
                        strcat(auxresult[k],">");
                        k++;
                        (*numberphrases)++;
                        auxresult[k]=new char[strlen(actualtagname)+5];
                       }                   
                      strcpy(auxresult[k],"<");
                      if(!typetag)
                        strcat(auxresult[k],"/");
                      strcat(auxresult[k],actualtagname);
                      strcat(auxresult[k],">");
                      (*numberphrases)++;
                      delete result;
                      result=auxresult;
                     }
                   k=0;
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
           buffertext[k]='\0';
           //We verify that the text has any non-blank character
           for(k=0,l=0;k<strlen(buffertext);k++)
            {
             if(buffertext[k]!=' ' && buffertext[k]!='\n' && buffertext[k]!='\t')
              {
               if(k>0 && (buffertext[k-1]==' ' || buffertext[k-1]=='\n' || buffertext[k-1]=='\t'))
                {
                 buffertext[l]=' ';
                 l++;
                }
               buffertext[l]=buffertext[k];
               l++;
              }
            }
           if(l>0)
            {//The text string is valid for segmentation purposes
             buffertext[l]='\0';
             if(joiningtext && result[*numberphrases-1][0]!='<')
              {//The text should be joined to the previous one
               auxiliarstring=new char[2+strlen(buffertext)+strlen(result[*numberphrases-1])];
               strcpy(auxiliarstring,result[*numberphrases-1]);
               strcat(auxiliarstring," ");
               strcat(auxiliarstring,buffertext);
               delete result[*numberphrases-1];
               result[*numberphrases-1]=auxiliarstring;
              }
              else
               {
                auxresult=new char*[*numberphrases+1];
                for(k=0;k<*numberphrases;k++)
                  auxresult[k]=result[k];
                auxresult[k]=new char[strlen(buffertext)+1];
                strcpy(auxresult[k],buffertext);
                delete result;
                result=auxresult;
                (*numberphrases)++;
               }
            }
           
           insidetag=true;
           recordingtag=true;
           typetag=true;
           openingandclose=false;
           j=0;
           buffertext[j]='\0';
          }
          else
           {
            if(text[i]=='&' && i<len && (text[i+1]=='#' && text[i+2]=='1' && text[i+3]=='6' && text[i+4]=='0') || 
                                        (text[i+1]=='n' && text[i+2]=='b' && text[i+3]=='s' && text[i+4]=='p') && text[i+5]==';')
             {//Substitution of &#160; per blank space
              i=i+5;
              buffertext[k]=' ';
              k++;
             }
             else
              {
               buffertext[k]=text[i];
               k++;
              }
           }
        }
     }
    
    delete buffertext;
   }
   else
     result=NULL;
  return(result);
 }
 
char* TagAligner2_1::GenerateTMX(const char** leftparts,const char *llang,const char** rightparts,const char *rlang,unsigned int n_segments)
 {
  char *result;
  char *auxresult;
  char *segment;
  unsigned int i,len;
  
  result=new char[500];
  strcpy(result,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<tmx version=\"1.4\">\n");
  strcat(result,"<header creationtool=\"tagaligner\" creationtoolversion=\"1.0\" datatype=\"PlainText\" segtype=\"sentence\" o-encoding=\"iso-8859-1\">\n</header>\n<body>\n");
  for(i=0;i<n_segments;i++)
   {
    if(leftparts[i]!=NULL)
     {
      if(rightparts[i]!=NULL)
       {
        len=strlen(leftparts[i])+strlen(rightparts[i]);
        segment=new char[1+len+300];
        sprintf(segment,"<tu tuid=\"%d\" datatype=\"Text\">\n  <tuv xml:lang=\"%s\">\n    <seg>",i,llang);
        strcat(segment,leftparts[i]);
        sprintf(segment,"%s</seg>\n  </tuv>\n  <tuv xml:lang=\"%s\">\n    <seg>",segment,rlang);
        strcat(segment,rightparts[i]);
        strcat(segment,"</seg>\n  </tuv>\n</tu>\n");
       }
       else
        {
         len=strlen(leftparts[i]);
         segment=new char[1+len+300];
         sprintf(segment,"<tu tuid=\"%d\" datatype=\"Text\">\n  <tuv xml:lang=\"%s\">\n    <seg>",i,llang);
         strcat(segment,leftparts[i]);
         strcat(segment,"</seg>\n  </tuv>\n</tu>\n");
        }
     }
     else
      {
       if(rightparts[i]!=NULL)
        {
         len=strlen(rightparts[i]);
         segment=new char[1+len+300];
         sprintf(segment,"<tu tuid=\"%d\" datatype=\"Text\">\n  <tuv xml:lang=\"%s\">\n    <seg>",i,rlang);
         strcat(segment,rightparts[i]);
         strcat(segment,"</seg>\n  </tuv>\n</tu>\n");
        }
        else
         {//This would be an error: an empty segment
          fprintf(stderr,"Warning there is an empty segment (%d)\n",i);
          segment=NULL;
         }
      }
    //Concatenation of the new segment
    if(segment!=NULL)
     {
      auxresult=new char[50+strlen(result)+strlen(segment)];
      strcpy(auxresult,result);
      strcat(auxresult,segment);
      delete segment;
      segment=NULL;
      delete result;
      result=auxresult;
     }
   }
  strcat(result,"</body>\n</tmx>\n");
  
  return(result);
 }

char* TagAligner2_1::GenerateTextAligned(const char** leftparts,const char** rightparts,unsigned int n_segments)
 {
  char *result;
  char *auxresult;
  char *segment;
  unsigned int i,len;
  
  result=new char[1];
  result[0]='\0';
  for(i=0;i<n_segments;i++)
   {
    if(leftparts[i]!=NULL)
     {
      if(rightparts[i]!=NULL)
       {
        len=strlen(leftparts[i])+strlen(rightparts[i]);
        segment=new char[1+len+300];
        sprintf(segment,"%s\n%s\n",leftparts[i],rightparts[i]);
       }
       else
        {
         len=strlen(leftparts[i]);
         segment=new char[1+len+300];
         sprintf(segment,"%s\n\n",leftparts[i]);
        }
     }
     else
      {
       if(rightparts[i]!=NULL)
        {
         len=strlen(rightparts[i]);
         segment=new char[1+len+300];
         sprintf(segment,"\n%s\n",rightparts[i]);
        }
        else
         {//This would be an error: an empty segment
          fprintf(stderr,"Warning there is an empty segment (%d)\n",i);
          segment=NULL;
         }
      }
    //Concatenation of the new segment
    if(segment!=NULL)
     {
      auxresult=new char[5+strlen(result)+strlen(segment)];
      strcpy(auxresult,result);
      strcat(auxresult,segment);
      delete segment;
      segment=NULL;
      delete result;
      result=auxresult;
     }
   }
  return(result);
 }

 
short TagAligner2_1::GetAssociatedNumber(const char *phrase)
 {
  short result;
  char buffer[kSizeAuxStrings];
  bool tagtype; //True: opening tag; False: closing tag
  
  if(phrase[0]=='<')
   {//It is a tag
    if(phrase[1]=='/')
     {//It is a closing tag
      strcpy(buffer,phrase+2);
      buffer[strlen(buffer)-1]='\0';
      tagtype=false;
     }
     else
      {//It is a opening tag
       strcpy(buffer,phrase+1);
       buffer[strlen(buffer)-1]='\0';
       tagtype=true;
      }
    result=GetXHTMLTagNumber(buffer);
    if(!tagtype)
      result=result*(-1);
   }
   else
    {//It is a text phrase
     result=kFirstLength+strlen(phrase);
    }
  return(result);
 }

char** TagAligner2_1::SplitInSentences(const char** splittext,unsigned int sizesplittext,unsigned int* resultsnumber)
 {
  char **result;
  unsigned i,j;
  char **auxresult,**auxresult2; //Auxiliars for storing the sentences
  unsigned int sizeauxresult; //Number of generated sentences
  
  *resultsnumber=0;
  result=NULL;
  for(i=0;i<sizesplittext;i++)
   {
    if(splittext[i]!=NULL && splittext[i][0]!='<')
     {//It is a text chunk
      auxresult=SentenceSplitter(splittext[i],&sizeauxresult);
      auxresult2=new char*[*resultsnumber+sizeauxresult];
      for(j=0;j<*resultsnumber;j++)
        auxresult2[j]=result[j];
      for(j=0;j<sizeauxresult;j++,(*resultsnumber)++)
        auxresult2[*resultsnumber]=auxresult[j];
      delete auxresult;
      delete result;
      result=auxresult2;
     }
     else
      {
       auxresult2=new char*[*resultsnumber+1];
       for(j=0;j<*resultsnumber;j++)
         auxresult2[j]=result[j];
       auxresult2[j]=new char[1+strlen(splittext[i])];
       strcpy(auxresult2[j],splittext[i]);
       (*resultsnumber)++;
       delete result;
       result=auxresult2;
      }
   }
  return(result);
 }
 
char* TagAligner2_1::EditDistancePath(const char **tts1,unsigned int ttssize1,const char **tts2,unsigned int ttssize2,double maxvalue)
 {
  char *result;
  unsigned int i,j,k;
  
  short *chunks1,*chunks2; //Identifying numbers associated to the tags and the text chunks
  char ***lsegments,***rsegments; //Sentences contained in the text chunks
  unsigned int *n_lsegments,*n_rsegments; //Number of sentences contained in the text chunks
  char *auxresult;
  double **matrix;
  char **route; //Prior element in the route of the minimal edit distance
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  double alignmenttextdistance; //Alignment distance between two text chunks
  
  if(tts1!=NULL && tts2!=NULL && ttssize1>0 && ttssize2>0)
   {
    //Initial splitting the text chunks in phrases (to avoid splitting more than once)
    chunks1=new short[ttssize1];
    lsegments=new char**[ttssize1];
    n_lsegments=new unsigned int[ttssize1];
    for(i=0;i<ttssize1;i++)
     {
      chunks1[i]=GetAssociatedNumber(tts1[i]);
      if(chunks1[i]>kFirstLength)
        lsegments[i]=SentenceSplitter(tts1[i],&n_lsegments[i]);
       else
        {
         lsegments[i]=NULL;
         n_lsegments[i]=0;
        }
     }
    chunks2=new short[ttssize2];
    rsegments=new char**[ttssize2];
    n_rsegments=new unsigned int[ttssize2];
    for(i=0;i<ttssize2;i++)
     {
      chunks2[i]=GetAssociatedNumber(tts2[i]);
      if(chunks2[i]>kFirstLength)
        rsegments[i]=SentenceSplitter(tts2[i],&n_rsegments[i]);
       else
        {
         rsegments[i]=NULL;
         n_rsegments[i]=0;
        }
     }
    
    //Initializing the algorithm
    route=new char*[ttssize1+1];
    matrix=new double*[2];//We keep only 2 rows of the matrix
    matrix[0]=NULL;
    matrix[1]=new double[ttssize2+1];

    startingcol=0; endingcol=ttssize2;//Initially the whole row is processed
    nextstartingcol=0; nextendingcol=ttssize2;
    
    //Inicialization of the first row: only insertion costs
    matrix[1][0]=0;
    route[0]=new char[ttssize2+1];
    route[0][0]='e';
    for(j=0;j<ttssize2;j++)
     {
      //Initialization of the first column of the row
      switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
       {
        case kTAGTypeStruct: matrix[1][j+1]=EDInsStructural+matrix[1][j];
                             break;
        
        case kTAGTypeFormat: matrix[1][j+1]=EDInsFormat+matrix[1][j];
                             break;
        
        case kTAGTypeContent: matrix[1][j+1]=EDInsContent+matrix[1][j];
                              break;
        
        default: if(chunks2[j]>kFirstLength)
                  {//It is a text
                   matrix[1][startingcol]=EDInsText*(chunks2[j]-kFirstLength);
                  }
                 break;
       }
      
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
    if(endingcol<ttssize2)
      endingcol++;//The ending column increases each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<ttssize2;j++)
  fprintf(stdout,"      %d",chunks2[j]);
fprintf(stdout,"\n0\n");
#endif
    
    for(i=0;i<ttssize1;i++)
     {
      route[i+1]=new char[ttssize2+1];
      delete matrix[0];
      matrix[0]=matrix[1];
      matrix[1]=new double[ttssize2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<ttssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",chunks1[i]);
#endif
      for(j=0;j<ttssize2+1;j++)
       {
        matrix[1][j]=-1;
        route[i+1][j]='\0';
       }

      //Initialization of the first column of the row
      if(GetXHTMLTagType((unsigned short)abs(chunks1[i]))==kTAGTypeStruct)
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
        
        switch(GetXHTMLTagType((unsigned short)abs(chunks1[i])))
         {
          case kTAGTypeStruct: switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
                                {
                                 case kTAGTypeStruct: if(chunks1[i]!=chunks2[j])
                                                        subscost+=EDSubsStrucStruc;
                                                      break;
                                 
                                 case kTAGTypeFormat: subscost+=EDSubsStrucFormat;
                                                      break;
                                                      
                                 case kTAGTypeContent: subscost+=EDSubsStrucContent;
                                                      break;
                                                      
                                 default: if(chunks2[j]>kFirstLength)
                                           {//It is a text
                                            subscost+=kBigValue; //It has no sense to replace a tag with text
                                           }
                                          break;
                                }
                               break;
          
          case kTAGTypeFormat: switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
                                {
                                 case kTAGTypeStruct: subscost+=EDSubsStrucFormat;
                                                      break;
                                 
                                 case kTAGTypeFormat: if(chunks1[i]!=chunks2[j])
                                                        subscost+=EDSubsFormatFormat;
                                                      break;
                                 
                                 case kTAGTypeContent: subscost+=EDSubsFormatContent;
                                                      break;
                                                      
                                 default: if(chunks2[j]>kFirstLength)
                                           {//It is a text
                                            subscost+=kBigValue; //It has no sense to replace a tag with text
                                           }
                                          break;
                                }
                               break;
          
          case kTAGTypeContent: switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
                                 {
                                  case kTAGTypeStruct: subscost+=EDSubsStrucContent;
                                                       break;
                                  
                                  case kTAGTypeFormat: subscost+=EDSubsFormatContent;
                                                       break;
                                 
                                  case kTAGTypeContent: if(chunks1[i]!=chunks2[j])
                                                          subscost+=EDSubsContentContent;
                                                        break;
                                                      
                                  default: if(chunks2[j]>kFirstLength)
                                            {//It is a text
                                             subscost+=kBigValue; //It has no sense to replace a tag with text
                                            }
                                           break;
                                 }
                                break;
          
          
          default: if(chunks1[i]>kFirstLength)
                    {//It is a text
                     switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
                      {
                       case kTAGTypeStruct: subscost+=kBigValue; //It has no sense to replace a tag with text
                                            break;
                                 
                       case kTAGTypeFormat: subscost+=kBigValue; //It has no sense to replace a tag with text
                                            break;
                                            
                       case kTAGTypeContent: subscost+=kBigValue; //It has no sense to replace a tag with text
                                            break;
                                
                       default: if(chunks2[j]>kFirstLength)
                                 {//It is a text
                                  //Substitution cost
                                  if((unsigned)chunks1[i]>ALtextsizemin || (unsigned)chunks2[i]>ALtextsizemin)
                                   {//The difference between the text sizes could be very high
                                    if((chunks1[i]-kFirstLength)<ALtextsizeminrelation*(chunks2[j]-kFirstLength) || (chunks2[j]-kFirstLength)<ALtextsizeminrelation*(chunks1[i]-kFirstLength))
                                      subscost+=kBigValue;
                                     else
                                      {
                                       alignmenttextdistance=AlignmentDistanceChunks((const char**)lsegments[i],n_lsegments[i],(const char**)rsegments[j],n_rsegments[j]);
                                       subscost+=EDSubsText*alignmenttextdistance;
                                      }
                                   }
                                   else
                                    {
                                     alignmenttextdistance=AlignmentDistanceChunks((const char**)lsegments[i],n_lsegments[i],(const char**)rsegments[j],n_rsegments[j]);
                                     subscost+=EDSubsText*alignmenttextdistance;
                                    }
                                  
                                  
                                 }
                                break;
                      }
                    }
                   break;
         }
                
        //Insertion cost
        inscost=matrix[1][j];
        switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
         {
          case kTAGTypeStruct: inscost+=EDInsStructural;
                               break;

          case kTAGTypeFormat: inscost+=EDInsFormat;
                               break;
                               
          case kTAGTypeContent: inscost+=EDInsContent;
                                break;

          default: if(chunks2[j]>kFirstLength)
                     inscost+=EDInsText*(chunks2[j]-kFirstLength);
                   break;
         }

        //Deletion cost
        if(endingcol!=j+1)
         {
          delcost=matrix[0][j+1];
          switch(GetXHTMLTagType((unsigned short)abs(chunks1[i])))
           {
            case kTAGTypeStruct: delcost+=EDDelStructural;
                                 break;
                                 
            case kTAGTypeFormat: delcost+=EDDelFormat;
                                 break;

            case kTAGTypeContent: delcost+=EDDelContent;
                                  break;
                                                                 
            default: if(chunks1[i]>kFirstLength)
                       delcost+=EDDelText*(chunks1[i]-kFirstLength);
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
      if(nextendingcol<ttssize2)
        nextendingcol++;//The ending column increase each row until the maximum
      endingcol=nextendingcol;
     }    

    for(;j<ttssize2;j++)
     {
      inscost=matrix[1][j];
      switch(GetXHTMLTagType((unsigned short)abs(chunks2[j])))
       {
        case kTAGTypeStruct: inscost+=EDInsStructural;
                             break;
        case kTAGTypeFormat: inscost+=EDInsFormat;
                             break;
        case kTAGTypeContent: inscost+=EDInsContent;
                             break;
        default: if(chunks2[j]>kFirstLength)
                   inscost+=EDInsText*(chunks2[j]-kFirstLength);
                 break;
       }
      matrix[1][j+1]=inscost;
      route[i][j+1]='i';
     }

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<ttssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif
    
    if(startingcol==endingcol || route[ttssize1][ttssize2]=='\0')
      result=NULL;
     else
      {//Building the path
       auxresult=new char[ttssize1+ttssize2+1];
       auxresult[ttssize1+ttssize2]='\0';
       i=ttssize1; j=ttssize2;
       for(k=ttssize1+ttssize2-1;route[i][j]!='e' && k>=0;k--)
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
      }
    
    delete matrix[0];
    delete matrix[1];
    delete matrix;
    matrix=NULL;
    for(i=0;i<ttssize1+1;i++)
      delete route[i];
    delete route;
    route=NULL;
    
    delete chunks1;
    chunks1=NULL;
    for(i=0;i<ttssize1;i++)
     {
      for(j=0;j<n_lsegments[i];j++)
       {
        if(lsegments[i][j]!=NULL)
          delete lsegments[i][j];
       }
      delete lsegments[i];
     }
    delete lsegments;
    lsegments=NULL;
    
    delete chunks2;
    chunks2=NULL;
    for(i=0;i<ttssize2;i++)
     {
      for(j=0;j<n_rsegments[i];j++)
       {
        if(rsegments[i][j]!=NULL)
          delete rsegments[i][j];
       }
      delete rsegments[i];
     }
    delete rsegments;
    rsegments=NULL;    
   }
   else
     result=NULL;
  return(result);
 }

unsigned short TagAligner2_1::WordsNumber(const char *text)
 {
  unsigned short result;
  unsigned int i;
  
  result=0;
  if(text!=NULL)
   {
    for(i=0;i<strlen(text);i++)
     {
      if(text[i]==' ' || text[i]=='\n' || text[i]=='\t')
       {//There is an empty space
        if(text[i-1]==' ' || text[i-1]=='\n' || text[i-1]=='\t')
         {//It was not the first one
          
         }
         else
           result++;
       }
       else
         result++;
     }
    if(text[i-1]!=' ' && text[i-1]!='\n' && text[i-1]!='\t')
      result++;
   }
  return(result);
 }

double TagAligner2_1::AlignmentDistanceChunks(const char** leftchunks,unsigned short nleftchunks,const char** rightchunks,unsigned short nrightchunks)
 {
  double result;
  short *lvector,*rvector; //Vector of characteristics of each segment to align them
  unsigned int i; //Several counters

  if(leftchunks!=NULL && rightchunks!=NULL)
   {
    //The characteristic value of each fragment is calculated (number of words)
    lvector=new short[nleftchunks];
    rvector=new short[nrightchunks];
    
#ifdef DistanceInWords        
    for(i=0;i<nleftchunks;i++)
      lvector[i]=WordsNumber(leftchunks[i]);
    for(i=0;i<nrightchunks;i++)
      rvector[i]=WordsNumber(rightchunks[i]);
#else
    for(i=0;i<nleftchunks;i++)
      lvector[i]=strlen(leftchunks[i]);
    for(i=0;i<nrightchunks;i++)
      rvector[i]=strlen(rightchunks[i]);
#endif    
     
    result=EditDistance(lvector,nleftchunks,rvector,nrightchunks);
    delete lvector;
    lvector=NULL;
    delete rvector;
    rvector=NULL;  
   }
   else
    {//One of the texts is empty so, the other one should be aligned to empty segments
     if(leftchunks!=NULL)
      {
       result=0;
#ifdef DistanceInWords
       for(i=0;i<nleftchunks;i++)
         result+=WordsNumber(leftchunks[i]);
#else
       for(i=0;i<nleftchunks;i++)
         result+=strlen(leftchunks[i]);
#endif         
      }
      else
       {
        if(rightchunks!=NULL)
         {
          result=0;
#ifdef DistanceInWords
       for(i=0;i<nrightchunks;i++)
         result+=WordsNumber(rightchunks[i]);
#else
          for(i=0;i<nrightchunks;i++)
            result+=strlen(rightchunks[i]);
#endif            
         }
         else
          {//Both texts were empty (this could be an error)
           result=0;          
          }
       }
    }
  return(result);
 }
 
double TagAligner2_1::EditDistance(const short *tts1,unsigned int ttsize1,const short *tts2,unsigned int ttssize2,double maxvalue)
 {
  double result;
  double **matrix;
  double subscost,inscost,delcost; //Temporary costs
  unsigned int startingcol,endingcol; //Indicate what columns of the row are processed
  unsigned int nextstartingcol,nextendingcol; //Indicate what columns of the row are processed in the next row
  unsigned int i,j;
  
  if(tts1!=NULL && tts2!=NULL && ttsize1>0 && ttssize2>0)
   {
    matrix=new double*[2];//We keep only 2 rows of the matrix
    matrix[0]=NULL;
    matrix[1]=new double[ttssize2+1];

    startingcol=0; endingcol=ttssize2;//Initially the whole row is processed
    nextstartingcol=0; nextendingcol=ttssize2;
    
    //Inicialization of the first row: only insertion costs
    matrix[1][0]=0;
    for(j=0;j<ttssize2;j++)
     {
      matrix[1][j+1]=matrix[1][j]+EDInsText*tts2[j];
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
    if(endingcol<ttssize2)
      endingcol++;//The ending column increase each row until the maximum

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t0");
for(j=0;j<ttssize2;j++)
  fprintf(stdout,"      %d",tts2[j]);
fprintf(stdout,"\n0\n");
#endif
    
    for(i=0;i<ttsize1;i++)
     {
      delete matrix[0];
      matrix[0]=matrix[1];
      matrix[1]=new double[ttssize2+1];

#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[0][0]);
for(j=1;j<ttssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[0][j]);
fprintf(stdout,"\n%d\n",tts1[i]);
#endif
      for(j=0;j<ttssize2+1;j++)
       {
        matrix[1][j]=-1;
       }

      //Initialization of the first column of the row
      matrix[1][startingcol]=matrix[0][startingcol]+EDDelText*tts1[i];
         
      if(matrix[1][startingcol]>maxvalue)
       {//This path should be discarded in the next row
        nextstartingcol++;
       }
      
      for(j=startingcol;j<endingcol;j++)
       {
        //Substitution cost
        subscost=matrix[0][j];
        if((unsigned)tts1[i]>ALtextsizemin || (unsigned)tts2[i]>ALtextsizemin)
         {//The difference between the text sizes could be very high
          if(tts1[i]<ALtextsizeminrelation*tts2[j] || tts2[j]<ALtextsizeminrelation*tts1[i])
            subscost+=kBigValue;
           else
             subscost+=EDSubsText*abs(tts1[i]-tts2[j]);
         }
         else
           subscost+=EDSubsText*abs(tts1[i]-tts2[j]);//The substitution cost is calculated directly

        //Insertion cost
        inscost=matrix[1][j]+EDInsText*tts2[j];

        //Deletion cost
        if(endingcol!=j+1)
          delcost=matrix[0][j+1]+EDDelText*tts1[i];
         else
           delcost=maxvalue;
        
        //Choosing the minimal cost
        if(subscost<=inscost)
         {
          if(subscost<=delcost)
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
      if(nextendingcol<ttssize2)
        nextendingcol++;//The ending column increase each row until the maximum
      endingcol=nextendingcol;
     }    

    for(;j<ttssize2;j++)
     {
      inscost=matrix[1][j]+EDInsText*tts2[j];
      matrix[1][j+1]=inscost;
     }
#ifdef TRAZANDO_HEAVYHEURISTICS
fprintf(stdout,"\t %.2f",matrix[1][0]);
for(j=1;j<ttssize2+1;j++)
  fprintf(stdout,"   %.2f",matrix[1][j]);
fprintf(stdout,"\n");
#endif

    if(startingcol==endingcol || matrix[1][ttssize2]==-1)
      result=-1;
     else
       result=matrix[1][ttssize2];
    delete matrix[0];
    delete matrix[1];
    delete matrix;
    matrix=NULL;
   }
   else
     result=-1;
  return(result);
 }

bool TagAligner2_1::Align(const char *ltext,const char *rtext)
 {
  bool result;
  
  char **lsplittext,**rsplittext; //Auxiliars to perform the splitting of the input text
  unsigned int n_lsplittext,n_rsplittext; //Sizes of the auxiliars to perform the splitting
  char *pathdistance; //Path of the edit distance
  unsigned int n_subs,n_ins,n_del; //Number of substutions, insertions and deletions in the edit distance process
  unsigned int n_sizepath; //Size of the the path of edit distance
  unsigned int i,j,k,l;
  unsigned int n_foundsegments; //Number of segments found until now
  char *trimbuffer; //Buffer used to remove initial and ending spaces
  
  if(ltext!=NULL && rtext!=NULL)
   {
    result=true;
    Reset();
    
    lsplittext=SplitFilterText(ltext,&n_lsplittext); //Splits where tags are found and remove irrelevant tags
    rsplittext=SplitFilterText(rtext,&n_rsplittext);
        
    lphrases=SplitInSentences((const char**)lsplittext,n_lsplittext,&n_lphrases); //Splits also the text chunks in sentences
    rphrases=SplitInSentences((const char**)rsplittext,n_rsplittext,&n_rphrases);

    //Elimination of initial and ending spaces in the phrases
    for(i=0;i<n_lphrases;i++)
     {
      if(lphrases[i]!=NULL)
       {
        for(j=0;lphrases[i][j]==' ' && j<strlen(lphrases[i]);j++);
        if(j>0)
         {//There are initial spaces
          for(k=strlen(lphrases[i])-1;lphrases[i][k]==' ' && k>=0;k--);
          trimbuffer=new char[2+k-j];
          for(l=j;l<k+1;l++)
            trimbuffer[l-j]=lphrases[i][l];
          trimbuffer[l-j]='\0';
          delete lphrases[i];
          lphrases[i]=trimbuffer;
          trimbuffer=NULL;
         }
         else
          {//There are not initial spaces
           for(k=strlen(lphrases[i])-1;lphrases[i][k]==' ' && k>=0;k--);
           lphrases[i][k+1]='\0';
          }
       }
     }
    for(i=0;i<n_rphrases;i++)
     {
      if(rphrases[i]!=NULL)
       {
        for(j=0;rphrases[i][j]==' ' && j<strlen(rphrases[i]);j++);
        if(j>0)
         {//There are initial spaces
          for(k=strlen(rphrases[i])-1;rphrases[i][k]==' ' && k>=0;k--);
          trimbuffer=new char[2+k-j];
          for(l=j;l<k+1;l++)
            trimbuffer[l-j]=rphrases[i][l];
          trimbuffer[l-j]='\0';
          delete rphrases[i];
          rphrases[i]=trimbuffer;
          trimbuffer=NULL;
         }
         else
          {//There are not initial spaces
           for(k=strlen(rphrases[i])-1;rphrases[i][k]==' ' && k>=0;k--);
           rphrases[i][k+1]='\0';
          }
       }
     }     
    
    pathdistance=EditDistancePath((const char**)lphrases,n_lphrases,(const char**)rphrases,n_rphrases,2*(n_lphrases+n_rphrases));
  
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
    n_sizepath=n_subs+n_del+n_ins;
  
    lsegments=new char*[n_lphrases+n_ins+n_del];
    rsegments=new char*[n_rphrases+n_ins+n_del];
  
    //Generate the tag alignment
    for(i=0,j=0,k=0;pathdistance[i]!='\0';i++)
     {
      switch(pathdistance[i])
       {
        case 's': lsegments[i]=lphrases[j];
                  rsegments[i]=rphrases[k];
                  j++;
                  k++;
                  break;
  
        case 'd': lsegments[i]=lphrases[j];
                  rsegments[i]=NULL;
                  j++;
                  break;
      
        case 'i': lsegments[i]=NULL;
                  rsegments[i]=rphrases[k];
                  k++;
                  break;
       }
     }
    delete pathdistance;
    pathdistance=NULL;
    n_foundsegments=i;
    
    //Tag cleaning
    for(i=0,n_segments=0;i<n_foundsegments;i++)
     {
      if(lsegments[i]!=NULL && lsegments[i][0]!='<' ||
         rsegments[i]!=NULL && rsegments[i][0]!='<')
       {//This is a text segment
        lsegments[n_segments]=lsegments[i];
        rsegments[n_segments]=rsegments[i];
        n_segments++;
       }
     }
    
    //Finally the text alignment is performed
   }
   else
     result=false;
  
  return(result);
 }
