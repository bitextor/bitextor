#include "genericutils.h"
 
char* RemoveAmpEntities(const char *stringamp)
 {
  char *result;
  char *aux;
  int i,j,len;

#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"BeginRemoveAmpEntities(%s)\n",stringamp);
   else
     fprintf(stderr,"BeginRemoveAmpEntities(NULL)\n");
#endif    
  if(stringamp!=NULL)
   {
    aux=new char[1+strlen(stringamp)];
    len=strlen(stringamp);
    for(i=0,j=0;i<len-5;i++,j++)
     {
      aux[j]=stringamp[i];
      if(stringamp[i]=='&' && (stringamp[i+1]=='a' || stringamp[i+1]=='A') &&
         (stringamp[i+2]=='M' || stringamp[i+2]=='m') &&
         (stringamp[i+3]=='P' || stringamp[i+3]=='p') &&
         stringamp[i+4]==';')
        i=i+4;
     }
    for(;(unsigned)i<strlen(stringamp);i++,j++)
       aux[j]=stringamp[i];
    aux[j]='\0';
    result=new char[1+strlen(aux)];
    strcpy(result,aux);    
    delete aux;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"EndRemoveAmpEntities(%s) -> %s\n",stringamp,result);
   else
     fprintf(stderr,"EndRemoveAmpEntities(NULL) -> %s\n",result);
#endif    
     
  return(result);
 }

char* InsertAmpEntities(const char *normalstring)
 {
  char *result;
  char *aux;
  int i,j,len;

#ifdef TRAZANDO_GenericUtils
  if(normalstring!=NULL)
    fprintf(stderr,"BeginInsertAmpEntities(%s)\n",normalstring);
   else
     fprintf(stderr,"BeginInsertAmpEntities(NULL)\n");
#endif    
  if(normalstring!=NULL)
   {
    aux=new char[1+5*strlen(normalstring)];
    len=strlen(normalstring);
    for(i=0,j=0;i<len;i++,j++)
     {
      aux[j]=normalstring[i];
      if(normalstring[i]=='&' && (!(i<len-4 && normalstring[i+1]=='a' && 
                                    normalstring[i+2]=='m' &&
                                    normalstring[i+3]=='p' && 
                                    normalstring[i+4]==';')))
       {
        j++;
        aux[j]='a';
        j++;
        aux[j]='m';
        j++;
        aux[j]='p';
        j++;
        aux[j]=';';
       }
     }
    aux[j]='\0';
    result=new char[1+strlen(aux)];
    strcpy(result,aux);
    delete aux;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(normalstring!=NULL)
    fprintf(stderr,"EndInsertAmpEntities(%s) -> %s\n",normalstring,result);
   else
     fprintf(stderr,"EndInsertAmpEntities(NULL) -> %s\n",result);
#endif    
     
  return(result);
 }

char* RemoveAmpLtGtEntities(const char *stringamp)
 {
  char *result;
  char *aux;
  int i,j,len;

#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"BeginRemoveAmpLtGtEntities(%s)\n",stringamp);
   else
     fprintf(stderr,"BeginRemoveAmpLtGtEntities(NULL)\n");
#endif    
  if(stringamp!=NULL)
   {
    aux=new char[1+strlen(stringamp)];
    len=strlen(stringamp);
    for(i=0,j=0;i<len-5;i++,j++)
     {
      aux[j]=stringamp[i];
      if(stringamp[i]=='&' && (stringamp[i+1]=='a' || stringamp[i+1]=='A') &&
         (stringamp[i+2]=='M' || stringamp[i+2]=='m') &&
         (stringamp[i+3]=='P' || stringamp[i+3]=='p') &&
         stringamp[i+4]==';')
        i=i+4;
       else
        {
         if(stringamp[i]=='&' && (stringamp[i+1]=='l' || stringamp[i+1]=='L') &&
           (stringamp[i+2]=='T' || stringamp[i+2]=='t') &&
            stringamp[i+3]==';')
          {
           aux[j]='<';
           i=i+3;
          }
          else
           {
            if(stringamp[i]=='&' && (stringamp[i+1]=='g' || stringamp[i+1]=='G') &&
              (stringamp[i+2]=='T' || stringamp[i+2]=='t') &&
               stringamp[i+3]==';')
             {
              aux[j]='>';
              i=i+3;
             }
           }
        }
     }
    for(;(unsigned)i<strlen(stringamp);i++,j++)
       aux[j]=stringamp[i];
    aux[j]='\0';
    result=new char[1+strlen(aux)];
    strcpy(result,aux);    
    delete aux;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"EndRemoveAmpLtGtEntities(%s) -> %s\n",stringamp,result);
   else
     fprintf(stderr,"EndRemoveAmpLtGtEntities(NULL) -> %s\n",result);
#endif    
     
  return(result);
 } 
 
char* InsertAmpLtGtEntities(const char *normalstring)
 {
  char *result;
  char *aux;
  int i,j,len;

#ifdef TRAZANDO_GenericUtils
  if(normalstring!=NULL)
    fprintf(stderr,"BeginInsertAmpLtGtEntities(%s)\n",normalstring);
   else
     fprintf(stderr,"BeginInsertAmpLtGtEntities(NULL)\n");
#endif    
  if(normalstring!=NULL)
   {
    aux=new char[1+5*strlen(normalstring)];
    len=strlen(normalstring);
    for(i=0,j=0;i<len;i++,j++)
     {
      aux[j]=normalstring[i];
      if(normalstring[i]=='&' && (!(i<len-4 && normalstring[i+1]=='a' && 
                                    normalstring[i+2]=='m' &&
                                    normalstring[i+3]=='p' && 
                                    normalstring[i+4]==';')))
       {
        j++;
        aux[j]='a';
        j++;
        aux[j]='m';
        j++;
        aux[j]='p';
        j++;
        aux[j]=';';
       }
       else
        {
         if(normalstring[i]=='<')
          {
           aux[j]='&';
           j++;
           aux[j]='l';
           j++;
           aux[j]='t';
           j++;
           aux[j]=';';
          }
          else
           {
            if(normalstring[i]=='>')
             {
              aux[j]='&';
              j++;
              aux[j]='g';
              j++;
              aux[j]='t';
              j++;
              aux[j]=';';
             }
           }
        }
     }
    aux[j]='\0';
    result=new char[1+strlen(aux)];
    strcpy(result,aux);
    delete aux;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(normalstring!=NULL)
    fprintf(stderr,"EndInsertAmpLtGtEntities(%s) -> %s\n",normalstring,result);
   else
     fprintf(stderr,"EndInsertAmpLtGtEntities(NULL) -> %s\n",result);
#endif    
     
  return(result);
 } 
 
char* XMLToLatin1(const xmlChar* entrada)
 {
  char *result;
  unsigned char *salida;
  int lensalida,lenentrada;
  
#ifdef TRAZANDO_GenericUtils
  if(entrada!=NULL)
    fprintf(stderr,"BeginXMLToLatin1(%s)\n",entrada);
   else
     fprintf(stderr,"BeginXMLToLatin1(NULL)\n");
#endif    
  
  if(entrada!=NULL)
   {
    lensalida=xmlStrlen(entrada)+1;
    lenentrada=xmlStrlen(entrada);  
    salida=new unsigned char[lensalida];
    if(UTF8Toisolat1(salida, &lensalida, entrada, &lenentrada)<0)
     {
      fprintf(stderr,"Error: Cannot convert encoding from UTF-8 to ISO-8859-1.");
      result=NULL;
     }
     else
      {
       salida[lensalida]='\0';
       result=new char[1+strlen((char*)salida)];
       strcpy(result,(char*)salida);
      }   
    delete salida;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(entrada!=NULL)
    fprintf(stderr,"EndXMLToLatin1(%s) -> %s\n",entrada,result);
   else
     fprintf(stderr,"EndXMLToLatin1(NULL) -> %s\n",result);
#endif    
     
  return(result);  
 }

char* RemoveURLReference(const char *inputurl)
 {
  char *result;
  char *aux;
  int i,len;

#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"BeginRemoveURLReference(%s)\n",inputurl);
   else
     fprintf(stderr,"BeginRemoveURLReference(NULL)\n");
#endif    
  if(inputurl!=NULL)
   {
    aux=new char[1+strlen(inputurl)];
    len=strlen(inputurl);
    for(i=0;i<len && inputurl[i]!='#';i++)
      aux[i]=inputurl[i];
    aux[i]='\0';
    result=new char[1+strlen(aux)];
    strcpy(result,aux);    
    delete aux;
   }
   else
     result=NULL;
#ifdef TRAZANDO_GenericUtils
  if(stringamp!=NULL)
    fprintf(stderr,"EndRemoveURLReference(%s) -> %s\n",inputurl,result);
   else
     fprintf(stderr,"EndRemoveURLReference(NULL) -> %s\n",result);
#endif    
     
  return(result);
 }
