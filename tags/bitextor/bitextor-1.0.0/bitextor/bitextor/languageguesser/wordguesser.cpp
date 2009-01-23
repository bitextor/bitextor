/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano. La adivinacion se basa en palabras frecuentes
   caracteristicas de un solo idioma
   Implementado por Enrique Sánchez Villamil
   Email: esvillamil@dlsi.ua.es
   Año 2004
*/

#include "wordguesser.h"

WordLangGuesser::WordLangGuesser(unsigned short nlanguages)
 {
  int i;
  numlanguages=nlanguages;;
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
    possiblelang[i]=true;     
  
  hashdict=NULL;
  wordsperlang=NULL;
  moduleperlang=NULL;
  dictsize=0;
  dictused=0;
  dictocur=0;
  pointslang=NULL;
  colissions=0;
 }

WordLangGuesser::WordLangGuesser(unsigned short nlanguages,const char *localfilename)
 {
  int i;
  
  numlanguages=nlanguages;
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
    possiblelang[i]=true;     
     
  if(!Load(localfilename))
   {//Actuamos como el constructor normal
    numlanguages=nlanguages;
    hashdict=NULL;
    wordsperlang=NULL;
    moduleperlang=NULL;
    dictsize=0;
    dictused=0;
    pointslang=NULL;  
   }
 }

WordLangGuesser::~WordLangGuesser()
 {
  unsigned k;
  if(hashdict!=NULL)
   {
    for(k=0;k<dictsize;k++)
     {
      if(hashdict[k]!=NULL)
       {
        delete hashdict[k]->word;
        delete hashdict[k]->languages;
        delete hashdict[k]->occurences;
        delete hashdict[k];        
       }
     }
    delete hashdict;
    hashdict=NULL;
   }
  if(wordsperlang==NULL)
   {
    delete wordsperlang;
    wordsperlang=NULL;
   }
  if(moduleperlang==NULL)
   {
    delete moduleperlang;
    moduleperlang=NULL;
   }
  if(pointslang!=NULL)
   {
    delete pointslang;
    pointslang=NULL;
   }
  if(possiblelang!=NULL)
   {
    delete possiblelang;
    possiblelang=NULL;
   }
 }
 
char* WordLangGuesser::ReadCharacter(FILE *file)
 {
  char *result,aux,buffer[kSizeAuxStrings];
  int i;
  
  if(!feof(file))
   {
    aux=fgetc(file);
    if((aux>='A' && aux<='Z') || (aux>='a' && aux<='z') || 
        aux=='ñ' || aux=='Ñ' || aux=='ç' || aux=='Ç' ||
        aux=='á' || aux=='é' || aux=='í' || aux=='ó' || aux=='ú' ||
        aux=='à' || aux=='è' || aux=='ì' || aux=='ò' || aux=='ù' ||
        aux=='ä' || aux=='ë' || aux=='ï' || aux=='ö' || aux=='ü' ||
        aux=='â' || aux=='ê' || aux=='î' || aux=='ô' || aux=='û' ||
        aux=='Á' || aux=='É' || aux=='Í' || aux=='Ó' || aux=='Ú' ||
        aux=='À' || aux=='È' || aux=='î' || aux=='Ò' || aux=='Ù' ||
        aux=='Ä' || aux=='Ë' || aux=='Ï' || aux=='Ö' || aux=='Ü' ||
        aux=='Â' || aux=='Ê' || aux=='Î' || aux=='Ô' || aux=='Û' ||
        aux=='ã' || aux=='Ã' || aux=='Õ' || aux=='õ' || aux=='&')
     {
      if(aux!='&')
       {        
        result=new char[2];
        result[0]=aux;
        result[1]='\0';
       }
       else
        {
         buffer[0]=aux;
         aux=fgetc(file);
         for(i=1;i<kSizeAuxStrings-1 && ((aux>='A' && aux<='Z') || (aux>='a' && aux<='z')
                                          || (aux>='0' && aux<='9') || aux=='#') && !feof(file);i++)
          {
           buffer[i]=aux;
           aux=fgetc(file);
          }
         buffer[i]=';';
         buffer[i+1]='\0';
         result=new char[strlen(buffer)+1];
         strcpy(result,buffer);
        }
     }
     else
      {
       if(aux=='<')
        {//Quitamos la etiqueta
         while(aux!='>' && !feof(file))
           aux=fgetc(file);
        }
       result=NULL;
      }
    }
    else
      result=NULL;
  return(result);
 }
 
char* WordLangGuesser::ReadCharacter(const char *text,short *inc,short where)
 {
  char *result,aux;
  int i;
    
  *inc=0;
  if(where>2 || where<0)
    where=where%3;
  if(text[*inc]!='\0')
   {
    aux=text[*inc];
    (*inc)++;
    if((aux>='A' && aux<='Z') || (aux>='a' && aux<='z') || 
        aux=='ñ' || aux=='Ñ' || aux=='ç' || aux=='Ç' ||
        aux=='á' || aux=='é' || aux=='í' || aux=='ó' || aux=='ú' ||
        aux=='à' || aux=='è' || aux=='ì' || aux=='ò' || aux=='ù' ||
        aux=='ä' || aux=='ë' || aux=='ï' || aux=='ö' || aux=='ü' ||
        aux=='â' || aux=='ê' || aux=='î' || aux=='ô' || aux=='û' ||
        aux=='Á' || aux=='É' || aux=='Í' || aux=='Ó' || aux=='Ú' ||
        aux=='À' || aux=='È' || aux=='î' || aux=='Ò' || aux=='Ù' ||
        aux=='Ä' || aux=='Ë' || aux=='Ï' || aux=='Ö' || aux=='Ü' ||
        aux=='Â' || aux=='Ê' || aux=='Î' || aux=='Ô' || aux=='Û' ||
        aux=='ã' || aux=='Ã' || aux=='Õ' || aux=='õ' || aux=='&')
     {
      if(aux!='&')
       {
        auxstrings[where][0]=aux;
        auxstrings[where][1]='\0';
        result=auxstrings[where];
       }
       else
        {
         auxstrings[where][0]=aux;
         aux=text[*inc];
         (*inc)++;
         for(i=1;i<kSizeAuxStrings-1 && ((aux>='A' && aux<='Z') || (aux>='a' && aux<='z') ||
                 (aux>='0' && aux<='9') || aux=='#') && text[*inc]!='\0';i++)
          {
           auxstrings[where][i]=aux;
           aux=text[*inc];
           (*inc)++;
          }
         auxstrings[where][i]=';';
         auxstrings[where][i+1]='\0';
         result=auxstrings[where];
        }
     }
     else
      {
       if(aux=='<')
        {//Quitamos la etiqueta
         while(aux!='>' && text[*inc]!='\0')
          {
           aux=text[*inc];
           (*inc)++;
          }
        }
       result=NULL;
      }
    }
    else
      result=NULL;
  return(result);
 }

int WordLangGuesser::HashFunction(const char *word,int size)
 {
  int result;

  result=hash_string(word);
  if(result<0)
    result=(result*(-1))%size;
   else
     result=result%size;
  return(result);
 }

bool WordLangGuesser::InsertDictionaryWord(const char* word,short language,unsigned int occur)
 {
  bool result=false;
  unsigned int i,j,hash,posic;
  DictionaryWord **auxhashdict;
  
  hash=HashFunction(word,dictsize);
  posic=hash;
  for(i=0,result=false;result==false && i<dictsize;i++)
   {
    if(hashdict[posic]!=NULL && strcmp(hashdict[posic]->word,word)!=0)
     {//Se ha producido una colision
      posic=RedisperseFunction(hash,i+1,dictsize);
      colissions++;
     }
     else
      {
       if(hashdict[posic]==NULL)
        {//Es la primera vez que encontramos la palabra, asi que la insertamos
         hashdict[posic]=new DictionaryWord;
         hashdict[posic]->word=new char[1+strlen(word)];
         strcpy(hashdict[posic]->word,word);
         hashdict[posic]->languages=new bool[numlanguages];
         for(j=0;j<numlanguages;j++)
           hashdict[posic]->languages[j]=false;
         hashdict[posic]->languages[language]=true;
         hashdict[posic]->occurences=new unsigned int[numlanguages];
         for(j=0;j<numlanguages;j++)
           hashdict[posic]->occurences[j]=0;	 
         hashdict[posic]->occurences[language]=occur;
         dictused++;
        }
        else
         {//La palabra ya estaba, asi que incrementamos su numero de ocurrencias
          hashdict[posic]->languages[language]=true;
          hashdict[posic]->occurences[language]+=occur;
         }
       dictocur++;
       wordsperlang[language]+=occur;
       result=true;
      }
   }
  
  //Ahora falta ver los tamaños y comprobar si hay que redimensionar la tabla
  if(dictused>=dictsize*kMaxFullHash)
   {//Hay que incrementar su tamaño
    auxhashdict=new DictionaryWord*[2*dictsize];
    for(i=0;i<2*dictsize;i++)
      auxhashdict[i]=NULL;
    
    for(i=0;i<dictsize;i++)
     {      
      if(hashdict[i]!=NULL)
       {//Insertamos la palabra
        hash=HashFunction(word,2*dictsize);
        posic=hash;
  
        for(j=0,result=false;result==false && j<2*dictsize;j++)
         { 
          if(auxhashdict[posic]!=NULL)
           {//Se ha producido una colision
            posic=RedisperseFunction(hash,j,2*dictsize);
           }
           else
            {
             auxhashdict[posic]=hashdict[i];
             result=true;
            }
         }
        if(!result)
          fprintf(stderr,"Fatal error: word \"%s\" could not be inserted while resizing hash table",word);
       }
     }
    delete hashdict;
    hashdict=auxhashdict;
    dictsize=2*dictsize;
   }
  return(result);
 }

const DictionaryWord* WordLangGuesser::SearchWord(const char* inputword)
 {
  const DictionaryWord *result;
  bool found;
  unsigned int i,hash,posic,len;
  char *word;
  
  if(inputword!=NULL)
   {
    len=strlen(inputword);
    word=new char[1+len];
    word[len]='\0';
    for(i=0;i<len;i++)
     {//Se pasa la cadena a minusculas
      if(inputword[i]>='A' && inputword[i]<='Z')
        word[i]=inputword[i]+32;
       else
        {
         if(inputword[i]=='Ñ')
           word[i]='ñ';
          else
           {
            if(inputword[i]=='Ç')
              word[i]='ç';
             else
              {
               if(inputword[i]=='Á')
                 word[i]='á';
                else
                 {
                  if(inputword[i]=='É')
                    word[i]='é';
                   else
                    {
                     if(inputword[i]=='Í')
                       word[i]='í';
                      else
                       {
                        if(inputword[i]=='Ó')
                          word[i]='ó';
                         else
                          {
                           if(inputword[i]=='Ú')
                             word[i]='ú';
                            else
                             {
                              if(inputword[i]=='À')
                                word[i]='à';
                               else
                                {
                                 if(inputword[i]=='È')
                                   word[i]='è';
                                  else
                                   {
                                    if(inputword[i]=='Ì')
                                      word[i]='ì';
                                     else
                                      {
                                       if(inputword[i]=='Ò')
                                         word[i]='ò';
                                        else
                                         {
                                          if(inputword[i]=='Ù')
                                            word[i]='ù';
                                           else
                                            {
                                             if(inputword[i]=='Ù')
                                               word[i]='ù';
                                              else
                                               {
                                                if(inputword[i]=='Ä')
                                                  word[i]='ä';
                                                 else
                                                  {
                                                   if(inputword[i]=='Ë')
                                                     word[i]='ë';
                                                    else
                                                     {
                                                      if(inputword[i]=='Ï')
                                                        word[i]='ï';
                                                       else
                                                        {
                                                         if(inputword[i]=='Ö')
                                                           word[i]='ö';
                                                          else
                                                           {
                                                            if(inputword[i]=='Ü')
                                                              word[i]='ü';
                                                             else
                                                              {
                                                               if(inputword[i]=='Â')
                                                                 word[i]='â';
                                                                else
                                                                 {
                                                                  if(inputword[i]=='Ê')
                                                                    word[i]='ê';
                                                                   else
                                                                    {
                                                                     if(inputword[i]=='Î')
                                                                       word[i]='î';
                                                                      else
                                                                       {
                                                                        if(inputword[i]=='Ô')
                                                                          word[i]='ô';
                                                                         else
                                                                          {
                                                                           if(inputword[i]=='Û')
                                                                             word[i]='û';
                                                                            else
                                                                             {
                                                                              if(inputword[i]=='Ã')
                                                                                word[i]='ã';
                                                                               else
                                                                                {
                                                                                 if(inputword[i]=='Õ')
                                                                                   word[i]='õ';
                                                                                  else
                                                                                   {
                                                                                    word[i]=inputword[i];
                                                                                   }
                                                                                }
                                                                             }
                                                                          }
                                                                       }
                                                                    }
                                                                 }
                                                              }
                                                           }
                                                        }
                                                     }
                                                  }
                                               }
                                            }
                                         }
                                      }
                                   }
                                }
                             }
                          }
                       }
                    }
                 }
              }
           }
        }
     }
    hash=HashFunction(word,dictsize);
    posic=hash;
    for(i=0,result=NULL,found=false;result==NULL && !found && i<dictsize;i++)
     {
      if(hashdict[posic]!=NULL && strcmp(hashdict[posic]->word,word)!=0)
       {//Se ha producido una colision
        posic=RedisperseFunction(hash,i,dictsize);
       }
       else
        {
         if(hashdict[posic]==NULL)
          {//La palabra no estaba
           found=true;
          }
          else
           {//Se ha encontrado la palabra
            found=true;
            result=hashdict[posic];
           }
        }
     }
    delete word;
   }
   else
     result=NULL;
  return(result);
 }
 
bool WordLangGuesser::Save(const char *file)
 {
  bool result;
  FILE *fsal; //Fichero de entrada
  int j;
  unsigned int k;
  char *posic; //Auxiliares para escribir en disco

  if(file!=NULL)
   {
    if((fsal=fopen(file,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: The output file could not be opened\n");
     }
     else
      {
       if(hashdict!=NULL)
        {//Guardamos la estructura del diccionario       
         fprintf(fsal,"%d %d\n%d",dictsize,dictused,wordsperlang[0]);
         for(k=1;k<numlanguages;k++)
           fprintf(fsal," %d",wordsperlang[k]);
         fprintf(fsal,"\n");
         posic=(char*)&k;
         for(k=0;k<dictsize;k++)
          {
           if(hashdict[k]!=NULL)
            {
             fprintf(fsal,"%c%c%c%c",posic[0],posic[1],posic[2],posic[3]);
             for(j=0;j<numlanguages;j++)
              {
               if(hashdict[k]->languages[j])
                 fprintf(fsal,"1");
                else
                  fprintf(fsal,"0");
              }
             fprintf(fsal," ");
             for(j=0;j<numlanguages;j++)
               fprintf(fsal,"%u ",hashdict[k]->occurences[j]);
             fprintf(fsal,"%s\n",hashdict[k]->word);
            }
          }
        }
       result=true;
       fclose(fsal);
      }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: There is not any train set loaded\n");
    }
  return(result);
 }

bool WordLangGuesser::Load(const char *file)
 {
  bool result;
  FILE *fent;
  char aux2[100];//Auxiliares para la lectura del fichero
  char data[10000],*posic;
  int j,k;
  unsigned int l,m;
  unsigned long long int accumulator[numlanguages];
  
  result=true;
  posic=new char[10];
  if(!(fent=fopen(file,"r")))
   {
    result=false;
    //fprintf(stderr,"Error: The input file could not be opened\n");
   }
   else
    {
     if(hashdict!=NULL)
      {
       for(l=0;l<dictsize;l++)
        {
         if(hashdict[l]!=NULL)
          {
           delete hashdict[l]->word;
           delete hashdict[l]->languages;
           delete hashdict[l]->occurences;
           delete hashdict[l];        
          }
        }
       delete hashdict;
       hashdict=NULL;       
      }
     if(wordsperlang!=NULL)
      {
       delete wordsperlang;
       wordsperlang=NULL;
      }
     if(moduleperlang!=NULL)
      {
       delete moduleperlang;
       moduleperlang=NULL;
      }
      
     //Ahora cargamos el diccionario
     wordsperlang=new unsigned int[numlanguages];
     moduleperlang=new double[numlanguages];
     for(m=0;m<numlanguages;m++)
       accumulator[m]=0;
       
     if(fscanf(fent,"%d %d\n%d",&dictsize,&dictused,&wordsperlang[0])!=3)
      {
       fprintf(stderr,"Error: Input training file format is not valid.\n");
       result=false;
      }
      else
       {
        dictocur=wordsperlang[0];
        for(m=1;m<numlanguages;m++)
         {
          if(fscanf(fent," %d",&wordsperlang[m])!=1)
           {
            fprintf(stderr,"Error: Input training file format is not valid.\n");
            result=false;
           }
           else
             dictocur+=wordsperlang[m];
         }
        fgetc(fent);//Quitamos el caracter de fin de linea
        
        hashdict=new DictionaryWord*[dictsize];
        for(m=0;m<dictsize;m++)
          hashdict[m]=NULL;             
        for(m=0;m<dictused && !feof(fent) && result;m++)
         {
          if(fscanf(fent,"%c%c%c%c",posic,posic+1,posic+2,posic+3)==4)
           {
            j=*((int*)posic);
            if(j>=0 && j<(signed)dictsize)
             {
              hashdict[j]=new DictionaryWord;
              hashdict[j]->languages=new bool[numlanguages];
              hashdict[j]->occurences=new unsigned int[numlanguages];
              hashdict[j]->word=NULL;
              for(k=0;k<numlanguages && result;k++)
               {
                if(fscanf(fent,"%c",data)==1)
                 {
                  if(data[0]=='1')
                    hashdict[j]->languages[k]=true;
                   else
                     hashdict[j]->languages[k]=false;
                 }
                 else
                  {
                   result=false;
                   fprintf(stderr,"Error: Input training file format is not valid.\n");
                  }                      
               }
              if(result)
               {
                fscanf(fent," ");
                for(k=0;k<numlanguages && result;k++)
                 {
                  if(fscanf(fent,"%u",&l)==1)
                   {
                    hashdict[j]->occurences[k]=l;
                    accumulator[k]+=(l*l);
                   }
                   else
                    {
                     result=false;
                     fprintf(stderr,"Error: Input training file format is not valid.\n");
                    }                      
                 }
                if(result)
                 {
                  fgetc(fent);
                  for(k=0;k<100 && !feof(fent) && (k==0 || aux2[k-1]!='\n');k++)
                    aux2[k]=fgetc(fent);
                   aux2[k-1]='\0';
                  if(!feof(fent))
                   {
                    hashdict[j]->word=new char[1+strlen(aux2)];
                    strcpy(hashdict[j]->word,aux2);
                   }
                   else
                    {
                     result=false;
                     fprintf(stderr,"Error: Input training file format is not valid.\n");
                    }
                 }
               }
             }
             else
              {
               result=false;
               fprintf(stderr,"Error: Input training file format is not valid.\n");
              }
           }
           else
            {
             result=false;
             fprintf(stderr,"Error: Input training file format is not valid.\n");
            }
         }
        
        //Finalizamos el calculo de los modulos
        for(k=0;k<numlanguages;k++)
          moduleperlang[k]=sqrt(accumulator[k]);
       }
     fclose(fent);
    }
     
  if(!result)
   {
    if(hashdict!=NULL)
     {
      for(l=0;l<dictsize;l++)
       {
        if(hashdict[l]!=NULL)
         {
          delete hashdict[l]->word;
          delete hashdict[l]->languages;
          delete hashdict[l]->occurences;
          delete hashdict[l];        
         }
       }
      delete hashdict;
      hashdict=NULL;
     }
    if(wordsperlang!=NULL)
     {
      delete wordsperlang;
      wordsperlang=NULL;
     }
    if(moduleperlang!=NULL)
     {
      delete moduleperlang;
      moduleperlang=NULL;
     }
   }
  delete posic;
  return(result);
 }

short WordLangGuesser::GuessText(const char *text)
 { 
  short result;
  char *auxcharacter;
  char aux[10000]; //Auxiliar para la construccion de palabras
  unsigned int i,j;
  float totalpoints,betterprob,betterprob2;
  const DictionaryWord *word;
  
  const char *auxtext; //Variable para el recorrido del texto
  short inc; //Incremento en el recorrido del fichero
  
  if(text!=NULL)
   {
    result=-1;
    auxtext=text;
    if(pointslang!=NULL)
      delete pointslang;
    pointslang=new float[numlanguages];
    for(i=0;i<numlanguages;i++)
      pointslang[i]=0;
      
    aux[0]='\0';
    totalpoints=0;
    while((*auxtext)!='\0')
     {
      auxcharacter=ReadCharacter(auxtext,&inc);
      auxtext+=inc;
      if(auxcharacter!=NULL)
        strcat(aux,auxstrings[0]);
       else
        {
         if(strlen(aux)>0)
          {//Se ha acabado de leer una palabra
           word=SearchWord(aux);
           aux[0]='\0';
           if(word!=NULL)
            {
             for(i=0;i<numlanguages;i++)
              {
               if(word->languages[i] && possiblelang[i])
                {
                 (pointslang[i])++;
                 totalpoints++;
                }
              }
            }
          }
        }
     };

    //Normalizamos los valores
    if(totalpoints==0)
     {
      for(i=0,j=0;i<numlanguages;i++)
       {
        if(possiblelang[i])
          j++;
       }
      for(i=0;i<numlanguages;i++)
       {
        if(possiblelang[i])
          pointslang[i]=(float)1/j;
       }
     }
     else
      {
       for(i=0;i<numlanguages;i++)
         pointslang[i]=pointslang[i]/totalpoints;
      }
     
    //Ahora nos quedamos con la mas probable
    betterprob=0;
    for(i=0,betterprob2=-1;i<numlanguages;i++)
     {
      if(pointslang[i]>betterprob && possiblelang[i])
       {
        result=i;
        betterprob2=betterprob;
        betterprob=pointslang[i];
       }
       else
        {
         if(pointslang[i]>betterprob2 && possiblelang[i])
           betterprob2=pointslang[i];
        }
     }
    if(betterprob2+kAceptacionLang>betterprob || kMinPointsDifference>betterprob*totalpoints-betterprob2*totalpoints)
      result=-1;
   }
   else
    {
     result=-1;
     fprintf(stderr,"Error: The input file could not be opened\n");
    }
  return(result); 
 } 

bool WordLangGuesser::Train(const char *file,short language)
 {
  bool result;  
  FILE *fent;
  char aux[10000];//Auxiliares para la lectura del fichero 
  char *auxcharacter;
  unsigned int auxdictused;//Valor previo de la variable para compararla
  unsigned long long accumulator[numlanguages]; //Auxiliar para el calculo del modulo
  unsigned int i,j;
  
  result=true;
  if(!(fent=fopen(file,"r")))
   {
    result=false;
    fprintf(stderr,"Error: The input file \"%s\"could not be opened\n",file);
   }
   else
    {
     if(hashdict==NULL)
      {
       dictsize=kIniSizeHash;
       hashdict=new DictionaryWord*[dictsize];
       for(i=0;i<dictsize;i++)
         hashdict[i]=NULL;
       dictused=0;
       wordsperlang=new unsigned int[numlanguages];
       moduleperlang=new double[numlanguages];
       for(i=0;i<numlanguages;i++)
        {
         wordsperlang[i]=0;
         moduleperlang[i]=0;
        }
      }
     auxdictused=dictused;
     fprintf(stdout,"Generating dictionary");
     fflush(stdout);
     for(i=0,aux[0]='\0';!feof(fent);i++)
      {
       auxcharacter=ReadCharacter(fent);
       if(auxcharacter!=NULL)
         strcat(aux,auxcharacter);         
        else
         {
          if(strlen(aux)>0)
           {//Se ha acabado de leer una palabra
            if(i%100000==0 && i!=0)
             {//Cada 100000 palabras generamos un punto
              fprintf(stdout,".");
              fflush(stdout);
             }
            //Pasamos la palabra a minusculas
            for(j=0;j<strlen(aux);j++)
              aux[j]=(char)tolower((int)aux[j]);
            
            InsertDictionaryWord(aux,language);
            aux[0]='\0';
           }
         }
      }
     fclose(fent);
     
     for(i=0;i<dictsize;i++)
      {
       if(hashdict[i]!=NULL)
        {
         for(j=0;j<numlanguages;j++)
          {
           if(hashdict[i]->languages[j])
             accumulator[j]+=(hashdict[i]->occurences[j]*hashdict[i]->occurences[j]);
          }
        }
      }
      
     for(i=0;i<numlanguages;i++)
       moduleperlang[i]=sqrt(accumulator[i]);
     fprintf(stdout,"\nDictionary learned: %d new words and %d new ocurrences and %d colissions\n",dictused-auxdictused,dictocur,colissions);
     result=true;
    }  
  return(result);
 }

void WordLangGuesser::Normalize()
 {
  float suma;
  unsigned int i;
  
  for(i=0,suma=0;i<numlanguages;i++)
   {
    if(possiblelang[i])
      suma+=pointslang[i];
   }
  if(suma>0)
   {
    for(i=0;i<numlanguages;i++)
     {
      if(possiblelang[i])
        pointslang[i]=pointslang[i]/suma;
       else
         pointslang[i]=0;
     }
   }
 }
 
bool WordLangGuesser::SameMagnitude(int ocur1,int total1,int ocur2,int total2)
 { 
  bool result;
  float a,b;
  
  a=ocur1/(float)total1;
  b=ocur2/(float)total2;
  
  if(a/10>b || b/10>a)
    result=false;
   else
     result=true;  
  return(result);
 }
  
bool WordLangGuesser::CleanHash(unsigned int size)
 {
  bool result;
  DictionaryWord ***vectordict;//Utilizaremos un vector ordenado por frecuencia de palabra
  DictionaryWord **auxhashdict;//Auxiliar para la creacion de la nueva hash
  unsigned int auxdictsize; //Tamaño de la tabla antigua
  unsigned int i,j,k,l;
  bool found;
  
  result=true;
  vectordict=new DictionaryWord**[numlanguages];
  for(i=0;i<numlanguages;i++)
   {
    vectordict[i]=new DictionaryWord*[size];
    for(j=0;j<size;j++)
      vectordict[i][j]=NULL;
   }
  for(i=0;i<dictsize;i++)
   {
    if(hashdict[i]!=NULL)
     {//Miramos para cada idioma si se puede insertar en la nueva hash
      for(j=0;j<numlanguages;j++)
       {
        if(hashdict[i]->languages[j])
         {
          /*
          //Solo insertamos si la palabra es exclusiva de un idioma 
          for(k=0,found=false;k<numlanguages && !found;k++)
           {
            if(k!=j && hashdict[i]->languages[k])
              found=true;
           }
          */          
          
          //Solo insertamos si la palabra es solo muy frecuente en un idioma
          for(k=0,found=false;k<numlanguages && !found;k++)
           {
            if(k!=j)
             {
              //Comparamos las frecuencias de la palabra en estos idiomas
              if(hashdict[i]->occurences[k]/(float)wordsperlang[k]>hashdict[i]->occurences[j]/(float)wordsperlang[j] ||
                 SameMagnitude(hashdict[i]->occurences[k],wordsperlang[k],
                               hashdict[i]->occurences[j],wordsperlang[j]))
                found=true;
             }
           }

          //Recorremos la tabla para insertarla en la posicion adecuada
          for(k=0;k<size && !found;k++)
           {
            if(vectordict[j][k]==NULL)
             {//Insertamos directamente ya que habia hueco
              vectordict[j][k]=hashdict[i];
              found=true;
             }
             else
              {
               if(vectordict[j][k]->occurences[j]<hashdict[i]->occurences[j])
                {//Insertamos aqui desplazando las restantes a la derecha
                 found=true;
                 for(l=size-1;l>k;l--)
                   vectordict[j][l]=vectordict[j][l-1];
                 vectordict[j][k]=hashdict[i];
                }
              }
           }           
         }
       }
     }
   }
   
  //Ahora solo resta reinsertar todas las palabras en una hash pero mas pequeña
  auxhashdict=hashdict;
  auxdictsize=dictsize;
  dictsize=size*10*numlanguages;
  hashdict=new DictionaryWord*[dictsize];
  for(i=0;i<dictsize;i++)
    hashdict[i]=NULL;
  dictused=0;
  
  for(i=0;i<numlanguages;i++)
   {
    for(j=0;j<size;j++)
     {
      if(vectordict[i][j]!=NULL)
       {
        //Insertamos la palabra        
        InsertDictionaryWord(vectordict[i][j]->word,i,vectordict[i][j]->occurences[i]);
        fprintf(stdout,"%d:%s -> %d\n",i,vectordict[i][j]->word,vectordict[i][j]->occurences[i]);
       }
       else
        {//El vector no estaba completo
         result=false;
         j=size;
        }
     }
   }
  
  //Liberamos la memoria del antiguo vector
  for(i=0;i<auxdictsize;i++)
   {
    if(auxhashdict[i]!=NULL)
     {
      delete auxhashdict[i]->word;
      delete auxhashdict[i]->languages;
      delete auxhashdict[i]->occurences;
      delete auxhashdict[i];        
     }
   }
  delete auxhashdict;
  return(result);
 }

bool WordLangGuesser::SetPossibleLanguages(const char *langstring)
 {
  bool result;
  int i;
  
  if(langstring==NULL || strlen(langstring)!=(unsigned)numlanguages)
    result=false;
   else
    {
     for(i=0;i<numlanguages;i++)
      {
       if(langstring[i]=='0')
         possiblelang[i]=false;
        else
          possiblelang[i]=true;
      }
     result=true;     
    }
  return(result);
 }

unsigned short* WordLangGuesser::GivePossibleLanguages(unsigned int *npossiblelanguages)
 {
  unsigned short *result;
  int i,j;
  
  for(i=0,(*npossiblelanguages)=0;i<numlanguages;i++)
   {
    if(possiblelang[i])
      (*npossiblelanguages)++;
   }
  result=new unsigned short[*npossiblelanguages];
  for(i=0,j=0;i<numlanguages;i++)
   {
    if(possiblelang[i])
     {
      result[j]=i;
      j++;
     }
   }
  return(result);
 }

const float* WordLangGuesser::PointsPerLanguage()
 {
  const float *result;
  
  result=pointslang;
  return(result);
 }

bool WordLangGuesser::AddNewLanguage(const char *file)
 {
  bool result;
  FILE *fent;
  char aux2[100];//Auxiliares para la lectura del fichero
  char data[10000],*posic;
  int j,k;
  unsigned int l,m;
  unsigned long long int accumulator[numlanguages];
  
  result=true;
  posic=new char[10];
  if(!(fent=fopen(file,"r")))
   {
    result=false;
    //fprintf(stderr,"Error: The input file could not be opened\n");
   }
   else
    {
     if(hashdict!=NULL)
      {
       for(l=0;l<dictsize;l++)
        {
         if(hashdict[l]!=NULL)
          {
           delete hashdict[l]->word;
           delete hashdict[l]->languages;
           delete hashdict[l]->occurences;
           delete hashdict[l];        
          }
        }
       delete hashdict;
       hashdict=NULL;       
      }
     if(wordsperlang!=NULL)
      {
       delete wordsperlang;
       wordsperlang=NULL;
      }
     if(moduleperlang!=NULL)
      {
       delete moduleperlang;
       moduleperlang=NULL;
      }
      
     //Ahora cargamos el diccionario
     wordsperlang=new unsigned int[numlanguages];
     moduleperlang=new double[numlanguages];
     for(m=0;m<numlanguages;m++)
       accumulator[m]=0;
       
     if(fscanf(fent,"%d %d\n%d",&dictsize,&dictused,&wordsperlang[0])!=3)
      {
       fprintf(stderr,"Error: Input training file format is not valid.\n");
       result=false;
      }
      else
       {
        dictocur=wordsperlang[0];
        for(m=1;m<(unsigned int)(numlanguages-1);m++)
         {
          if(fscanf(fent," %d",&wordsperlang[m])!=1)
           {
            fprintf(stderr,"Error: Input training file format is not valid.\n");
            result=false;
           }
           else
             dictocur+=wordsperlang[m];
         }
        fgetc(fent); //Quitamos el caracter de fin de linea
        wordsperlang[m]=0; //Ponemos el valor del nuevo idioma
        
        hashdict=new DictionaryWord*[dictsize];
        for(m=0;m<dictsize;m++)
          hashdict[m]=NULL;             
        for(m=0;m<dictused && !feof(fent) && result;m++)
         {
          if(fscanf(fent,"%c%c%c%c",posic,posic+1,posic+2,posic+3)==4)
           {
            j=*((int*)posic);
            if(j>=0 && j<(signed)dictsize)
             {
              hashdict[j]=new DictionaryWord;
              hashdict[j]->languages=new bool[numlanguages];
              hashdict[j]->occurences=new unsigned int[numlanguages];
              hashdict[j]->word=NULL;
              for(k=0;k<numlanguages-1 && result;k++)
               {
                if(fscanf(fent,"%c",data)==1)
                 {
                  if(data[0]=='1')
                    hashdict[j]->languages[k]=true;
                   else
                     hashdict[j]->languages[k]=false;
                 }
                 else
                  {
                   result=false;
                   fprintf(stderr,"Error: Input training file format is not valid.\n");
                  }                      
               }
              hashdict[j]->languages[k]=false; //Ponemos el valor false a este nuevo idioma
              if(result)
               {
                fscanf(fent," ");
                for(k=0;k<numlanguages-1 && result;k++)
                 {
                  if(fscanf(fent,"%u",&l)==1)
                   {
                    hashdict[j]->occurences[k]=l;
                    accumulator[k]+=(l*l);
                   }
                   else
                    {
                     result=false;
                     fprintf(stderr,"Error: Input training file format is not valid.\n");
                    }                      
                 }
                if(result)
                 {
                  fgetc(fent);
                  for(k=0;k<100 && !feof(fent) && (k==0 || aux2[k-1]!='\n');k++)
                    aux2[k]=fgetc(fent);
                   aux2[k-1]='\0';
                  if(!feof(fent))
                   {
                    hashdict[j]->word=new char[1+strlen(aux2)];
                    strcpy(hashdict[j]->word,aux2);
                   }
                   else
                    {
                     result=false;
                     fprintf(stderr,"Error: Input training file format is not valid.\n");
                    }
                 }
               }
             }
             else
              {
               result=false;
               fprintf(stderr,"Error: Input training file format is not valid.\n");
              }
           }
           else
            {
             result=false;
             fprintf(stderr,"Error: Input training file format is not valid.\n");
            }
         }
        
        //Finalizamos el calculo de los modulos
        for(k=0;k<numlanguages-1;k++)
          moduleperlang[k]=sqrt(accumulator[k]);
        moduleperlang[k]=0; //Ponemos el valor cero a este nuevo idioma
       }
     fclose(fent);
    }
     
  if(!result)
   {
    if(hashdict!=NULL)
     {
      for(l=0;l<dictsize;l++)
       {
        if(hashdict[l]!=NULL)
         {
          delete hashdict[l]->word;
          delete hashdict[l]->languages;
          delete hashdict[l]->occurences;
          delete hashdict[l];        
         }
       }
      delete hashdict;
      hashdict=NULL;
     }
    if(wordsperlang!=NULL)
     {
      delete wordsperlang;
      wordsperlang=NULL;
     }
    if(moduleperlang!=NULL)
     {
      delete moduleperlang;
      moduleperlang=NULL;
     }
   }
   else
    {
     result=Save(file);
    }
  delete posic;
  return(result);
 }
