/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano o etiquetado. La adivinacion se basa en trigramas
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "trigramguesser.h"

float ReadFloat(FILE *file)
 {
  float result;
  char buffer;
  int i,dotplace;
  bool finished;
  
  for(i=0,result=0,dotplace=-1,finished=false;!feof(file) && !finished;i++)
   {                           
    buffer=fgetc(file);
    if(!isdigit(buffer) && buffer!='.' && !feof(file))
      finished=true;//Se ha acabado de leer el float
     else
      {
       if(buffer!='.')
         result=result*10+(buffer-48);
        else
          dotplace=i;
      }
   }
                                    
  if(i==1)
   {
    result=0;
    fprintf(stderr,"Error: Invalid format while reading floats\n");
   }
   else
    {
     for(i=i-2;i>dotplace && dotplace!=-1;i--)
       result=result/10;
    }
  return(result);
 }

int CompareTrigramFrequency(const void* a,const void* b)
 {
  int result;
  if(((Trigrama*)a)->frequency==((Trigrama*)b)->frequency)
    result=0;
   else
    {
     if(((Trigrama*)a)->frequency<((Trigrama*)b)->frequency)
       result=-1;
      else
        result=1; 
    }
  return(result);
 }

TrigramLangGuesser::TrigramLangGuesser(unsigned short nlanguages)
 {
  int i;
  numlanguages=nlanguages;
  numfrequents=kNumFrequents;
  frequents=NULL;
  ntrigrams=NULL;
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
    possiblelang[i]=true; 
  pointslang=NULL;
 }

TrigramLangGuesser::TrigramLangGuesser(unsigned short nlanguages,const char *localfilename)
 {
  int i;
  
  numlanguages=nlanguages;
  numfrequents=kNumFrequents;
  frequents=NULL;
  ntrigrams=NULL;
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
    possiblelang[i]=true;     
     
  if(!Load(localfilename))
   {//Actuamos como el constructor normal
    numlanguages=nlanguages;
    numfrequents=kNumFrequents;
    frequents=NULL;
    ntrigrams=NULL;
    pointslang=NULL;  
   }
 }

TrigramLangGuesser::~TrigramLangGuesser()
 {
  int i,j;
  if(frequents!=NULL)
   {
    for(i=0;i<numlanguages;i++)
     {
      if(frequents[i]!=NULL)
       {
        if(ntrigrams!=NULL && ntrigrams[i]>0)
         {
          for(j=1;j<numfrequents;j++)
           {
            if(frequents[i][j].a!=NULL)
             {
              delete frequents[i][j].a;
              delete frequents[i][j].b;
              delete frequents[i][j].c;
             }
           }
         }
        delete frequents[i];
       }
     } 
    delete frequents;
    frequents=NULL;
   }
  if(ntrigrams!=NULL)
   {
    delete ntrigrams;
    ntrigrams=NULL;
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
 
char* TrigramLangGuesser::ReadCharacter(FILE *file)
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
 
char* TrigramLangGuesser::ReadCharacter(const char *text,short *inc,short where)
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
 
bool TrigramLangGuesser::Save(const char *file)
 {
  bool result;
  FILE *fsal; //Fichero de entrada
  int i,j;

  if(file!=NULL)
   {
    if((fsal=fopen(file,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: The output file could not be opened\n");
     }
     else
      {
       if(frequents!=NULL)
        {//Guardamos la estructura de trigramas
         fprintf(fsal,"%d %d\n%d",numlanguages,kNumFrequents,ntrigrams[0]);
         for(i=1;i<numlanguages;i++)
         fprintf(fsal," %d",ntrigrams[i]);
         fprintf(fsal,"\n");
         for(i=0;i<numlanguages;i++)
          {
           if(ntrigrams[i]>0)
            {
             fprintf(fsal,"%f\n",frequents[i][0].frequency);
             for(j=1;j<numfrequents;j++)
               fprintf(fsal,"%s%s%s %f\n",frequents[i][j].a,frequents[i][j].b,frequents[i][j].c,frequents[i][j].frequency);
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

bool TrigramLangGuesser::Load(const char *file)
 {
  bool result;
  FILE *fent;
  char *aux;//Auxiliares para la lectura del fichero
  char *posic;
  int i,j;
  int nlanguages;

  result=true;
  posic=new char[10];
  if(!(fent=fopen(file,"r")))
   {
    result=false;
    fprintf(stderr,"Error: The input file could not be opened\n");
   }
   else
    {
     if(frequents!=NULL)
      {
       for(i=0;i<numlanguages;i++)
        {
         if(frequents[i]!=NULL)
          {
           if(ntrigrams!=NULL && ntrigrams[i]>0)
            {
             for(j=1;j<numfrequents;j++)
              {
               if(frequents[i][j].a!=NULL)
                {
                 delete frequents[i][j].a;
                 delete frequents[i][j].b;
                 delete frequents[i][j].c;
                }
              }
            }      
           delete frequents[i];
          }
        } 
       delete frequents;
       frequents=NULL;
      }
     if(ntrigrams!=NULL)
      {
       delete ntrigrams;
       ntrigrams=NULL;
      }  
      
     //Cargamos primero la estructura de trigramas     
     fscanf(fent,"%d %d\n",&i,&numfrequents);
     ntrigrams=new int[i];
     
     fscanf(fent,"%d",&(ntrigrams[0]));
     for(j=1;j<i && !feof(fent);j++)
       fscanf(fent," %d",&(ntrigrams[j]));
     fscanf(fent,"\n");
     nlanguages=(short)i;
     
     frequents=new Trigrama*[numlanguages];
     for(j=0;j<numlanguages;j++)
      {
       frequents[j]=new Trigrama[numfrequents];
       for(i=0;i<numfrequents;i++)
        {
         frequents[j][i].frequency=0;
         frequents[j][i].a=NULL;
         frequents[j][i].b=NULL;
         frequents[j][i].c=NULL;
        }
      }
   
     for(i=0;i<nlanguages;i++)
      {
       if(ntrigrams[i]>0)
        {
         frequents[i][0].frequency=ReadFloat(fent);
         for(j=1;j<numfrequents && !feof(fent);j++)
          {         
           aux=ReadCharacter(fent);
           if(aux!=NULL)
             frequents[i][j].a=aux;
            else
             {
              frequents[i][j].a=new char[2];
              strcpy(frequents[i][j].a," ");
             }
           aux=ReadCharacter(fent);              
           if(aux!=NULL)
             frequents[i][j].b=aux;
            else
             {
              frequents[i][j].b=new char[2];
              strcpy(frequents[i][j].b," ");
             }
           aux=ReadCharacter(fent);
           if(aux!=NULL)
             frequents[i][j].c=aux;
            else
             {
              frequents[i][j].c=new char[2];
              strcpy(frequents[i][j].c," ");
             }
           fgetc(fent);
           frequents[i][j].frequency=ReadFloat(fent);           
          }
        }
      }
     if(i!=nlanguages || j!=numfrequents)
       result=false;
     fclose(fent);
    }
   
   if(!result)
    {
     if(frequents!=NULL)
      {
       for(i=0;i<numlanguages;i++)
        {
         if(frequents[i]!=NULL)
          {
           if(ntrigrams!=NULL && ntrigrams[i]>0)
            {
             for(j=1;j<numfrequents;j++)
              {
               if(frequents[i][j].a!=NULL)
                {
                 delete frequents[i][j].a;
                 delete frequents[i][j].b;
                 delete frequents[i][j].c;
                }
              }
            }      
           delete frequents[i];
          }
        } 
       delete frequents;
       frequents=NULL;
      }
     if(ntrigrams!=NULL)
      {
       delete ntrigrams;
       ntrigrams=NULL;
      }
    }
  delete posic;
  return(result);
 }

bool TrigramLangGuesser::Train(const char* file,unsigned short language)
 {
  bool result;
  FILE *fent;
  char *a,*b,*c;//Variables para los caracteres
  int i;
  Trigrama filetrigrams[2*numfrequents]; //Auxiliar para contar los trigramas del fichero
  short temptrigrams[numfrequents]; //Indica el instante en que se inserto el trigrama en el array
  float placetrigrams[numfrequents]; //Indica la puntuacion del trigrama para determinar cual quitar
  float minimvalue; //Indica el valor minimo del placetrigrams
  int minimplace; //Indica la posicion donde ira el nuevo trigrama
  unsigned int timestep; //Indica el numero de trigrama del fichero que se inserta
  bool inserted; //Indica que ya se ha insertado el trigrama
  bool arrayfull; //Indica si ya se ha llenado el vector de trigramas del fichero
  short newtrigrams; //Indica el numero de nuevos trigramas encontrados hasta el momento
  bool updatedtrigrams[numfrequents]; //Indica los trigramas ya actualizados
  float suma; //Variable que sirve para actualizar el resto de trigramas

  int j=0;
  bool found; //Indica si se ha encontrado el trigrama que buscabamos
  
  result=true;
  arrayfull=false;
  if((fent=fopen(file,"r")) && language<numlanguages)
   {
    if(frequents==NULL)
     {
      frequents=new Trigrama*[numlanguages];
      ntrigrams=new int[numlanguages];
      for(i=0;i<numlanguages;i++)
       {
        frequents[i]=new Trigrama[numfrequents];
        ntrigrams[i]=0;
        for(j=0;j<numfrequents;j++)
         {
          frequents[i][j].a=NULL;
          frequents[i][j].b=NULL;
          frequents[i][j].c=NULL;
          frequents[i][j].frequency=0;
         }
       }
     }
    
    timestep=0;
    minimvalue=999999999;
    minimplace=-1;
    b=new char[2];
    strcpy(b," ");
    c=new char[2];
    strcpy(c," ");
    
    filetrigrams[0].frequency=0;
    
    for(i=1;i<numfrequents;i++)
      filetrigrams[i].a=NULL;
    
    while(!feof(fent))
     {//Leemos trigrama a trigrama
      a=b;
      b=c;
      c=ReadCharacter(fent);
      if(c==NULL)
       {
        if(strcmp(b," ")==0)
         {//Hemos leido dos nulos seguidos asi que seguimos leyendo
          do
           {
            c=ReadCharacter(fent);
           } while(c==NULL && !feof(fent));
          if(feof(fent))
           {
            c=new char[2];
            strcpy(c," ");
           }
         }
         else
          {
           c=new char[2];
           strcpy(c," ");
          }
       }
                        
      //Insertamos el trigrama si hay hueco
      for(i=1,inserted=false;!inserted && i<numfrequents;i++)
       {
        if(filetrigrams[i].a!=NULL)
         {
          if(strcmp(filetrigrams[i].a,a)==0 &&
             strcmp(filetrigrams[i].b,b)==0 &&
             strcmp(filetrigrams[i].c,c)==0)
           {//Es el mismo trigrama asi que se incrementa su frecuencia
            filetrigrams[i].frequency++;
            temptrigrams[i]=timestep;
            inserted=true;
           }
          placetrigrams[i]=kAjusteLG*filetrigrams[i].frequency/(timestep-temptrigrams[i]); 
          if(i==1)
           {             
            minimvalue=placetrigrams[1];
            minimplace=1;
           }
           else
            {
             if(minimvalue>placetrigrams[i])
              {
               minimvalue=placetrigrams[i];
               minimplace=i;
              }
            }
         }
         else
          {//Aun no estaba lleno el vector de trigramas, asi que insertamos aqui
           filetrigrams[i].a=new char[1+strlen(a)];
           strcpy(filetrigrams[i].a,a);
           filetrigrams[i].b=new char[1+strlen(b)];
           strcpy(filetrigrams[i].b,b);
           filetrigrams[i].c=new char[1+strlen(c)];
           strcpy(filetrigrams[i].c,c);
           filetrigrams[i].frequency=1;
           temptrigrams[i]=timestep;
           inserted=true;
          }
       }
        
      if(!inserted)
       {//El vector estaba lleno, asi que lo insertamos reemplazando
        filetrigrams[0].frequency=filetrigrams[0].frequency+filetrigrams[minimplace].frequency;
        delete filetrigrams[minimplace].a;
        delete filetrigrams[minimplace].b;
        delete filetrigrams[minimplace].c;
        filetrigrams[minimplace].a=new char[1+strlen(a)];
        strcpy(filetrigrams[minimplace].a,a);
        filetrigrams[minimplace].b=new char[1+strlen(b)];
        strcpy(filetrigrams[minimplace].b,b);
        filetrigrams[minimplace].c=new char[1+strlen(c)];
        strcpy(filetrigrams[minimplace].c,c);
        filetrigrams[minimplace].frequency=1;
        temptrigrams[minimplace]=timestep;
       }
      delete a;
      a=NULL;
      timestep++;
     }
    delete b;
    b=NULL;
    delete c;
    c=NULL;
    //Normalizamos las frecuencias
    for(i=0;i<numfrequents;i++)
     {
      filetrigrams[i].frequency=filetrigrams[i].frequency/timestep;
      updatedtrigrams[i]=false;//Inicializamos de paso este vector
     }
    
#ifdef probando
  //Realizamos el sumatorio
  for(suma=0,i=0;i<numfrequents;i++)
   {
    suma+=filetrigrams[i].frequency;
   }
  if(suma<1-0.001 || suma>1+0.001)
    fprintf(stderr,"Warning: La suma de las probabilidades no es 1 sino %f\n",suma);
#endif

    
    newtrigrams=0;

    if(ntrigrams[language]>0)
     {//Ahora combinamos los resultados          
      filetrigrams[0].frequency=(filetrigrams[0].frequency*timestep+frequents[language][0].frequency*ntrigrams[language])/(timestep+ntrigrams[language]);
      updatedtrigrams[0]=true;
    
      for(i=1;i<numfrequents;i++)
       {
        if(frequents[language][i].a!=NULL)
         {//El trigrama existe asi que intentamos compararlo
          for(j=1,found=false;j<numfrequents && !found;j++)
           {
            if(strcmp(filetrigrams[j].a,frequents[language][i].a)==0 &&
               strcmp(filetrigrams[j].b,frequents[language][i].b)==0 &&
               strcmp(filetrigrams[j].c,frequents[language][i].c)==0)
             {//Es el mismo trigrama asi que se incrementa su frecuencia
#ifdef probando
              if(updatedtrigrams[j])
                fprintf(stderr,"Warning: updating a previously updated trigram (repeated trigram?)\n");
#endif                
              filetrigrams[j].frequency=(filetrigrams[j].frequency*timestep+frequents[language][i].frequency*ntrigrams[language])/(timestep+ntrigrams[language]);
              found=true;
              updatedtrigrams[j]=true;
             }
           }
          if(!found)
           {//Insertamos el trigrama antiguo
            filetrigrams[numfrequents+newtrigrams].a=new char[1+strlen(frequents[language][i].a)];
            strcpy(filetrigrams[numfrequents+newtrigrams].a,frequents[language][i].a);
            filetrigrams[numfrequents+newtrigrams].b=new char[1+strlen(frequents[language][i].b)];
            strcpy(filetrigrams[numfrequents+newtrigrams].b,frequents[language][i].b);
            filetrigrams[numfrequents+newtrigrams].c=new char[1+strlen(frequents[language][i].c)];
            strcpy(filetrigrams[numfrequents+newtrigrams].c,frequents[language][i].c);
            filetrigrams[numfrequents+newtrigrams].frequency=(frequents[language][i].frequency*ntrigrams[language])/(timestep+ntrigrams[language]);
            newtrigrams++;
           }
         }
       }

      //Ahora quedaria actualizar las frecuencias nuevas
      for(i=1;i<numfrequents;i++)
       {
        if(!updatedtrigrams[i])
          filetrigrams[i].frequency=(filetrigrams[i].frequency*timestep)/(timestep+ntrigrams[language]);
       }
     }
         
    //Por ultimo hay que reordenar el vector para quedarnos con los trigramas mas frecuentes    
    qsort(&(filetrigrams[1]),numfrequents+newtrigrams-1,sizeof(Trigrama),CompareTrigramFrequency);
    
    //Actualizamos el valor del resto
    for(suma=0,i=1;i<numfrequents;i++)
      suma+=filetrigrams[i].frequency;
    filetrigrams[0].frequency=1-suma;
    
    //Nos quedamos con los trigramas mas frecuentes
    frequents[language][0].frequency=filetrigrams[0].frequency;
    
    for(i=1;i<numfrequents;i++)
     {
      frequents[language][i].frequency=filetrigrams[numfrequents-i].frequency;
      if(frequents[language][i].a!=NULL)
       {
        delete frequents[language][i].a;
        delete frequents[language][i].b;
        delete frequents[language][i].c;
       }
      frequents[language][i].a=new char[1+strlen(filetrigrams[numfrequents-i].a)];
      strcpy(frequents[language][i].a,filetrigrams[numfrequents-i].a);
      frequents[language][i].b=new char[1+strlen(filetrigrams[numfrequents-i].b)];
      strcpy(frequents[language][i].b,filetrigrams[numfrequents-i].b);
      frequents[language][i].c=new char[1+strlen(filetrigrams[numfrequents-i].c)];
      strcpy(frequents[language][i].c,filetrigrams[numfrequents-i].c);
     }
    ntrigrams[language]=ntrigrams[language]+timestep;
    
#ifdef probando
  //Realizamos el sumatorio
  for(suma=0,i=0;i<numfrequents;i++)
    suma+=frequents[language][i].frequency;
  if(suma<1-0.001 || suma>1+0.001)
    fprintf(stderr,"Warning: La suma actualizada de las probabilidades no es 1 sino %f\n",suma);
#endif
    fclose(fent);
   }
   else
    {
     result=false;
     if(language<numlanguages)
       fprintf(stderr,"Error: The input file could not be opened\n");
      else
        fprintf(stderr,"Error: The language is not valid\n");
    }
  return(result);
 }

short TrigramLangGuesser::GuessText(const char* text)
 {
  short result;
  int i,j,k;
  char *readresult;
  Trigrama filetrigrams[numfrequents]; //Auxiliar para contar los trigramas del fichero
  short temptrigrams[numfrequents]; //Indica el instante en que se inserto el trigrama en el array
  float placetrigrams[numfrequents]; //Indica la puntuacion del trigrama para determinar cual quitar
  float minimvalue; //Indica el valor minimo del placetrigrams
  int minimplace; //Indica la posicion donde ira el nuevo trigrama
  unsigned int timestep; //Indica el numero de trigrama del fichero que se inserta
  bool inserted; //Indica que ya se ha insertado el trigrama
  bool arrayfull; //Indica si ya se ha llenado el vector de trigramas del fichero
  bool found; //Indica si se ha encontrado el trigrama que buscabamos
  float totalpoints; //Numero total de puntos que se asignan a cada idioma
  float betterprob; //Indica la maxima probabilidad que tiene un idioma
  float betterprob2; //Indica la segunda probabilidad mas probable que tiene un idioma
  short betterlang; //Indica el idioma mas probable
 
  const char *auxtext; //Variable para el recorrido del texto
  short inc; //Incremento en el recorrido del fichero
  
  result=-1;
  arrayfull=false;
  auxtext=text;
  if(frequents!=NULL && text!=NULL)
   {
    strcpy(auxstrings[2]," ");
    strcpy(auxstrings[1]," ");
    strcpy(auxstrings[0]," ");
    
    filetrigrams[0].frequency=0;
    
    for(i=1;i<numfrequents;i++)
      filetrigrams[i].a=NULL;
    
    minimvalue=999999999;
    minimplace=-1;
    for(timestep=0;timestep<kMaxTrigramNumber && (*auxtext)!='\0';timestep++)
     {//Leemos trigrama a trigrama
      readresult=ReadCharacter(auxtext,&inc,timestep%3);
      auxtext+=inc;
      if(readresult==NULL)
       {
        if(strcmp(auxstrings[(timestep+2)%3]," ")==0)
         {//Hemos leido dos nulos seguidos asi que seguimos leyendo
          do
           {
            readresult=ReadCharacter(auxtext,&inc,timestep%3);
            auxtext+=inc;
           } while(readresult==NULL && (*auxtext)!='\0');
          if((*auxtext)=='\0')
            strcpy(auxstrings[timestep%3]," ");
         }
         else
           strcpy(auxstrings[timestep%3]," ");
       }

      //Insertamos el trigrama si hay hueco
      for(i=1,inserted=false;!inserted && i<numfrequents;i++)
       {
        if(filetrigrams[i].a!=NULL)
         {
          if(strcmp(filetrigrams[i].a,auxstrings[(timestep+1)%3])==0 &&
             strcmp(filetrigrams[i].b,auxstrings[(timestep+2)%3])==0 &&
             strcmp(filetrigrams[i].c,auxstrings[timestep%3])==0)
           {//Es el mismo trigrama asi que se incrementa su frecuencia
            filetrigrams[i].frequency++;
            temptrigrams[i]=timestep;
            inserted=true;
           }
          placetrigrams[i]=kAjusteLG*filetrigrams[i].frequency/(timestep-temptrigrams[i]); 
          if(i==1)
           {             
            minimvalue=placetrigrams[1];
            minimplace=1;
           }
           else
            {
             if(minimvalue>placetrigrams[i])
              {
               minimvalue=placetrigrams[i];
               minimplace=i;
              }
            }
         }
         else
          {//Aun no estaba lleno el vector de trigramas, asi que insertamos aqui
           filetrigrams[i].a=new char[1+strlen(auxstrings[(timestep+1)%3])];
           strcpy(filetrigrams[i].a,auxstrings[(timestep+1)%3]);
           filetrigrams[i].b=new char[1+strlen(auxstrings[(timestep+2)%3])];
           strcpy(filetrigrams[i].b,auxstrings[(timestep+2)%3]);
           filetrigrams[i].c=new char[1+strlen(auxstrings[timestep%3])];
           strcpy(filetrigrams[i].c,auxstrings[timestep%3]);
           filetrigrams[i].frequency=1;
           temptrigrams[i]=timestep;
           inserted=true;
          }
       }
        
      if(!inserted)
       {//El vector estaba lleno, asi que lo insertamos reemplazando
        filetrigrams[0].frequency=filetrigrams[0].frequency+filetrigrams[minimplace].frequency;
        delete filetrigrams[minimplace].a;
        delete filetrigrams[minimplace].b;
        delete filetrigrams[minimplace].c;
        
        filetrigrams[minimplace].a=new char[1+strlen(auxstrings[(timestep+1)%3])];
        strcpy(filetrigrams[minimplace].a,auxstrings[(timestep+1)%3]);
        filetrigrams[minimplace].b=new char[1+strlen(auxstrings[(timestep+2)%3])];
        strcpy(filetrigrams[minimplace].b,auxstrings[(timestep+2)%3]);
        filetrigrams[minimplace].c=new char[1+strlen(auxstrings[timestep%3])];
        strcpy(filetrigrams[minimplace].c,auxstrings[timestep%3]);
        filetrigrams[minimplace].frequency=1;
        temptrigrams[minimplace]=timestep;
       }
     }
    //Normalizamos las frecuencias
    for(i=0;i<numfrequents;i++)
      filetrigrams[i].frequency=filetrigrams[i].frequency/timestep;
    
#ifdef probando
  float suma;
  //Realizamos el sumatorio
  for(suma=0,i=0;i<numfrequents;i++)
   {
    suma+=filetrigrams[i].frequency;
   }
  if(suma<1-0.001 || suma>1+0.001)
    fprintf(stderr,"Warning: La suma de las probabilidades no es 1 sino %f\n",suma);
  fprintf(stdout,"(The file contents %d trigrams) ",timestep);
#endif
    
    totalpoints=0;
    if(pointslang!=NULL)
      delete pointslang;
    pointslang=new float[numlanguages];
    for(i=0,betterprob=0,betterlang=-1,betterprob2=-1;i<numlanguages;i++)
     {//Realizamos el producto escalar con cada idioma
      if(ntrigrams[i]>0 && possiblelang[i])
       {
        pointslang[i]=0;
        
        for(j=1;j<numfrequents;j++)
         {
          if(filetrigrams[j].a!=NULL)
           {//El trigrama existe asi que intentamos compararlo
            for(k=1,found=false;k<numfrequents && !found;k++)
             {
              if(frequents[i][k].a!=NULL)
               {
                if(strcmp(filetrigrams[j].a,frequents[i][k].a)==0 &&
                   strcmp(filetrigrams[j].b,frequents[i][k].b)==0 &&
                   strcmp(filetrigrams[j].c,frequents[i][k].c)==0)
                 {
                  pointslang[i]=pointslang[i]+filetrigrams[j].frequency*frequents[i][k].frequency;
                  found=true;
                 }
               }
             }            
           }
         }
#ifdef probando        
        //fprintf(stderr,"Resultado producto escalar (%d) = %f\n",i,pointslang[i]);
#endif        
        if(pointslang[i]>=betterprob)
         {
          betterlang=i;
          betterprob2=betterprob;
          betterprob=pointslang[i];
         }
         else
          {
           if(pointslang[i]>betterprob2)
               betterprob2=pointslang[i];
          }
        totalpoints+=pointslang[i];
       }
       else
         pointslang[i]=0;
     }
    
    for(j=1;j<numfrequents;j++)//Borramos la estructura de frecuencias
     {
      if(filetrigrams[j].a!=NULL)
       {
        delete filetrigrams[j].a;
        filetrigrams[j].a=NULL;
        delete filetrigrams[j].b;
        filetrigrams[j].b=NULL;
        delete filetrigrams[j].c;
        filetrigrams[j].c=NULL;
       }
     }
     
    if(betterprob2+kAceptacionLang>betterprob)
      result=betterlang;
     else
       result=-1;
    
    //Normalizamos los valores
    for(i=0;i<numlanguages;i++)
      pointslang[i]=pointslang[i]/totalpoints;
   }
   else
    {
     if(frequents==NULL)
       fprintf(stderr,"Error: Parameters were not loaded before guessing\n");       
    }
  return(result);
 }

void TrigramLangGuesser::PrintTrigrams(short language)
 {
  int i;
  
  fprintf(stdout,"Idioma: %d\n",language);
  for(i=1;i<numfrequents;i++)
   {
    fprintf(stdout,"\t%s%s%s -> %f",frequents[language][i].a,
                                    frequents[language][i].b,
                                    frequents[language][i].c,
                                    frequents[language][i].frequency);
    if(i%3==0)
      fprintf(stdout,"\n");
   }
  fprintf(stdout,"\tResto -> %f\n",frequents[language][0].frequency);  
 }

void TrigramLangGuesser::Normalize()
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

bool TrigramLangGuesser::SetPossibleLanguages(const char *langstring)
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

unsigned short* TrigramLangGuesser::GivePossibleLanguages(unsigned int *npossiblelanguages)
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

const float* TrigramLangGuesser::PointsPerLanguage()
 {
  const float *result;
  
  result=pointslang;
  return(result);
 }
