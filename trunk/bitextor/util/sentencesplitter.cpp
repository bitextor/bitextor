/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/


#include "sentencesplitter.h"


bool IsAlphaNumeric(const char c)
 {
  bool result;
  
  if((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') ||
     c=='¢' || c=='£' || c=='¤' || c=='¥' || c=='À' || c=='Á' || c=='Ç' ||
     c=='È' || c=='É' || c=='Ì' || c=='Í' || c=='Ñ' || c=='Ò' || c=='Ó' ||
     c=='Ù' || c=='Ú' || c=='Ü' || c=='à' || c=='á' || c=='ç' || c=='è' || 
     c=='é' || c=='ì' || c=='í' || c=='ñ' || c=='ò' || c=='ó' || c=='ù' || 
     c=='ú' || c=='ü')
    result=true;
   else
     result=false;   
  return(result);
 }

inline bool IsBlankSpace(const char *str)
 {
  bool result;
  
  if(str!=NULL)
   {
    if(str[0]==' ' || str[0]=='\t' || str[0]=='\n' || 
       (strlen(str)>5 && str[0]=='&' && str[1]=='#' && str[2]=='1' &&
        str[3]=='6' && str[4]=='0' && str[5]==';'))
      result=true;
     else
       result=false;
   }
   else
     result=true;
  return(result);
 }

bool IsCapitalLetter(const char c)
 {
  bool result;
  
  if((c>='A' && c<='Z') || c=='À' || c=='Á' || c=='Ç' || c=='È' || c=='É' || c=='Ì' ||
      c=='Í' || c=='Ñ' || c=='Ò' || c=='Ó' || c=='Ù' || c=='Ú' || c=='Ü' )
    result=true;
   else
     result=false;   
  return(result);
 }

bool IsNonCapitalLetter(const char c)
 {
  bool result;
  
  if((c>='a' && c<='z') || c=='à' || c=='á' || c=='ç' || c=='è' || c=='é' || c=='ì' ||
      c=='í' || c=='ñ' || c=='ò' || c=='ó' || c=='ù' || c=='ú' || c=='ü' )
    result=true;
   else
     result=false;   
  return(result);
 }

char* TrimText(const char *text)
 {
  char *result;
  unsigned int i,j,length;
  bool blankspacereaded;
  
  length=strlen(text);
  result=new char[1+length];
  for(i=0,j=0,blankspacereaded=false;i<length;i++)
   {
    if(IsBlankSpace(&text[i]))
     {
      if(!blankspacereaded)
       {
        result[j]=' ';
        j++;
       }
      blankspacereaded=true;
     }
     else
      {
       blankspacereaded=false;
       result[j]=text[i];
       j++;
      }
   }
  result[j]='\0';
  return(result);
 }

unsigned int TextLength(const char *text)
 {
  unsigned int result;
  unsigned int i;
 
  for(i=0,result=0;i<strlen(text);i++)
   {
    if(text[i]=='&')
     {
      for(i++;text[i]!=';' && i<strlen(text);i++);
      result++;
     }
     else
       result++;
   }
  return(result);
 } 

char** QuickSentenceSplitter(const char *text,unsigned int *n_segments)
 {
  char **result;
  char **auxresult;
  char sentence[kMaxSizeSentences];
  unsigned int i,j,k;

  *n_segments=0;
  result=NULL;
  for(i=0,j=0;text[i]!='\0' && j<kMaxSizeSentences;i++)
   {
    if(text[i]=='!' || text[i]=='?' || (text[i]=='.' && text[i+1]==' ' && IsAlphaNumeric(text[i+2]) && !IsCapitalLetter(text[i-1])))
     {//Split the sentence
      sentence[j]=text[i];
      j++;
      sentence[j]='\0';
//      if(j>1)
//       {
        auxresult=new char*[*n_segments+1];
        for(k=0;k<*n_segments;k++)
          auxresult[k]=result[k];
        auxresult[k]=new char[1+strlen(sentence)];
        strcpy(auxresult[k],sentence);
        (*n_segments)++;
        delete result;
        result=auxresult;
//       }
      j=0;
     }
     else
      {
       sentence[j]=text[i];
       j++;
      }
   }
  if(j==kMaxSizeSentences)
   {
    fprintf(stderr,"Error: Sentece too long while splitting sentences\n");
   }
   else
    {//Store the last sentence if it was not yet stored
     sentence[j]='\0';
     if(j>0)
      {
       auxresult=new char*[*n_segments+1];
       for(k=0;k<*n_segments;k++)
         auxresult[k]=result[k];
       auxresult[k]=new char[1+strlen(sentence)];
       strcpy(auxresult[k],sentence);
       (*n_segments)++;
       delete result;
       result=auxresult;
      }     
    }   
  return(result);
 }

char** DeepSentenceSplitter(const char *text,unsigned int *n_segments)
 {
  char **result;
  char **auxresult;
  unsigned int *aux;
  unsigned int *positions; //Positions where the dots are found
  unsigned int n_positions; //Number of dots that were found
  float *pointsperposition; //Number of points that has each of the dots
  char sentence[kMaxSizeSentences]; //Buffer to store each of the sentences
  unsigned int i,j,k;
  
  //Find possible split points
  positions=NULL;
  for(n_positions=0,i=0;i<strlen(text);i++)
   {
    if(text[i]=='.')
     {
      aux=new unsigned int[1+n_positions];
      for(j=0;j<n_positions;j++)
        aux[j]=positions[j];
      aux[j]=i;
      delete positions;
      positions=aux;
      n_positions++;
     }
   }
  pointsperposition=new float[n_positions];
  for(i=0;i<n_positions;i++)
    pointsperposition[i]=0;
  
  //Give points to them applying heuristics
  for(i=0;i<n_positions;i++)
   {
    if(positions[i]<strlen(text)-1)
     {
      //Heuristic 1: Dot followed by a number -> -0.5
      if(text[positions[i]+1]>='0' && text[positions[i]+1]<='9')
        pointsperposition[i]-=0.5;
      
      //Heuristic 2: Dot followed by blank space -> +0.5
      if(IsBlankSpace(&(text[positions[i]+1])))
        pointsperposition[i]+=0.5;
      
      //Heuristic 3: Dot followed by a non capital letter -> -0.2
      if(IsNonCapitalLetter(text[positions[i]+1]))
        pointsperposition[i]-=0.2;
        
      //Heuristic 4: Dot followed by another dot -> -0.5
      if(text[positions[i]+1]=='.')
        pointsperposition[i]-=0.5;
        
      if(positions[i]<strlen(text)-2)
       {
        //Heuristic 5: Dot followed by a blank space and capital letter -> +0.5
        if(IsBlankSpace(&(text[positions[i]+1])) && IsCapitalLetter(text[positions[i]+2]))
          pointsperposition[i]+=0.5;
        
        //Heuristic 6: Dot followed by a blank space and non-capital letter -> -0.2
        if(IsBlankSpace(&(text[positions[i]+1])) && IsNonCapitalLetter(text[positions[i]+2]))
          pointsperposition[i]-=0.2;
       }
     }
    
    if(positions[i]>0)
     {
      //Heuristic 7: Dot that follows a capital letter -> -0.5
      if(IsCapitalLetter(text[positions[i]-1]))
        pointsperposition[i]-=0.5;
      
      //Heuristic 8: Dot that follows a very short word (max 3 char) in capital letters -> -0.5
      if((IsCapitalLetter(text[positions[i]-1]) && (positions[i]==1 || IsBlankSpace(&(text[positions[i]-2])))) ||
         (IsCapitalLetter(text[positions[i]-1]) && IsCapitalLetter(text[positions[i]-2]) && (positions[i]==2 || IsBlankSpace(&(text[positions[i]-3])))) ||
         (IsCapitalLetter(text[positions[i]-1]) && IsCapitalLetter(text[positions[i]-2]) && IsCapitalLetter(text[positions[i]-3]) && (positions[i]==3 || IsBlankSpace(&(text[positions[i]-4])))))
        pointsperposition[i]-=0.5;
      
      //Heuristic 9: Dot that follows a blank space -> +0.2
      if(IsBlankSpace(&(text[positions[i]-1])))
        pointsperposition[i]+=0.2;
      
      //Heuristic 10: Dot that follows a ' or a " character and it is followed by another one of them-> -0.5
      if((text[positions[i]-1]=='\'' || text[positions[i]-1]=='\"') && positions[i]<strlen(text)-1 && 
         (text[positions[i]+1]=='\'' || text[positions[i]-1]=='\"'))
        pointsperposition[i]-=0.2;
      
      //Heuristic 11: Dot that follows another dot -> +0.4
      if(text[positions[i]-1]=='.')
        pointsperposition[i]+=0.2;
      
      if(positions[i]>1)
       {
        //Heuristic 12: Dot that follows an abbreviature -> -2 
        //Not implemented
        
       }
     }
   }
 
  //Split the text only if the points of each dot are high enough
  *n_segments=0;
  result=NULL;
  for(i=0,j=0,k=0;text[i]!='\0' && j<kMaxSizeSentences;i++)
   {
    if(k<n_positions && i==positions[k])
     {
      if(pointsperposition[k]>-0.2)
       {//Split the sentence
        sentence[j]=text[i];
        j++;
        sentence[j]='\0';
//        if(j>1)
//         {
          auxresult=new char*[*n_segments+1];
          for(k=0;k<*n_segments;k++)
            auxresult[k]=result[k];
          auxresult[k]=new char[1+strlen(sentence)];
          strcpy(auxresult[k],sentence);
          (*n_segments)++;
          delete result;
          result=auxresult;
//         }
        j=0;
       }
       else
        {
         sentence[j]=text[i];
         j++;
        }
      k++;
     }
     else
      {
       sentence[j]=text[i];
       j++;
      }
   }
  if(j==kMaxSizeSentences)
    fprintf(stderr,"Error: Sentence too long while splitting sentences\n");
   else
    {//Store the last sentence if it was not yet stored
     sentence[j]='\0';
     if(j>0)
      {
       auxresult=new char*[*n_segments+1];
       for(k=0;k<*n_segments;k++)
         auxresult[k]=result[k];
       auxresult[k]=new char[1+strlen(sentence)];
       strcpy(auxresult[k],sentence);
       (*n_segments)++;
       delete result;
       result=auxresult;
      }     
    }  
  delete pointsperposition;
  delete positions;
  return(result);
 }
 
char** SentenceSplitter(const char *text,unsigned int *n_segments)
 {
  char **result;
  char *auxtext,**auxresult,**auxresult2,**auxresult3;
  unsigned int n_auxsegm;
  unsigned int i,j,k,l;
  
  auxtext=TrimText(text);
  auxresult=QuickSentenceSplitter(auxtext,&n_auxsegm);
  delete auxtext;
  auxresult3=new char*[10000*n_auxsegm];
  for(i=0,j=0;i<n_auxsegm;i++)
   {
    auxresult2=DeepSentenceSplitter(auxresult[i],&k);
    delete auxresult[i];
    for(l=0;l<k;l++,j++)
      auxresult3[j]=auxresult2[l];
    delete auxresult2;
   }
  delete auxresult;
  
  *n_segments=j;
  result=new char*[j];
  for(i=0;i<j;i++)
    result[i]=auxresult3[i];
  delete auxresult3;
  return(result);
 }
