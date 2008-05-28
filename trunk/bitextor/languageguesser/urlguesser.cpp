/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano o etiquetado. La adivinacion se basa en trigramas
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "urlguesser.h"

int CompareNGramFrequency(const void* a,const void* b)
 {
  int result;
  if(((NGram*)a)->frequency==((NGram*)b)->frequency)
    result=0;
   else
    {
     if(((NGram*)a)->frequency<((NGram*)b)->frequency)
       result=-1;
      else
        result=1; 
    }
  return(result);
 }

URLLangGuesser::URLLangGuesser(unsigned short nlanguages)
 {
  short i,j,k;
  
  numlanguages=nlanguages;
  numfrequents=kNumNgrams;
  maxsizengrams=kMaxSizeNGrams;
  frequents=new NGram**[numlanguages];
  numngrams=new int*[numlanguages];
  totalngrams=new int*[numlanguages];
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
   {
    frequents[i]=new NGram*[maxsizengrams];
    numngrams[i]=new int[maxsizengrams];
    totalngrams[i]=new int[maxsizengrams];
    possiblelang[i]=true;
    for(j=0;j<maxsizengrams;j++)
     {
      frequents[i][j]=new NGram[numfrequents];
      numngrams[i][j]=0;
      totalngrams[i][j]=0;
      for(k=0;k<numfrequents;k++)
       {
        frequents[i][j][k].ngram=NULL;
        frequents[i][j][k].frequency=0;
       }
     }
   }
 }

URLLangGuesser::URLLangGuesser(unsigned short nlanguages,const char *localfilename)
 {
  short i,j,k;
  
  numlanguages=nlanguages;
  numfrequents=kNumNgrams;
  maxsizengrams=kMaxSizeNGrams;
  possiblelang=new bool[numlanguages];
  for(i=0;i<numlanguages;i++)
    possiblelang[i]=true;
  frequents=NULL;
  numngrams=NULL;
  totalngrams=NULL;
     
  if(!Load(localfilename))
   {//Actuamos como el constructor normal
    numlanguages=nlanguages;
    numfrequents=kNumNgrams;
    maxsizengrams=kMaxSizeNGrams;
    frequents=new NGram**[numlanguages];
    numngrams=new int*[numlanguages];
    totalngrams=new int*[numlanguages];
    for(i=0;i<numlanguages;i++)
     {
      frequents[i]=new NGram*[maxsizengrams];
      numngrams[i]=new int[maxsizengrams];
      totalngrams[i]=new int[maxsizengrams];
      for(j=0;j<maxsizengrams;j++)
       {
        frequents[i][j]=new NGram[numfrequents];
        numngrams[i][j]=0;
        totalngrams[i][j]=0;
        for(k=0;k<numfrequents;k++)
         {
          frequents[i][j][k].ngram=NULL;
          frequents[i][j][k].frequency=0;
         }
       }
     } 
   }
 }

URLLangGuesser::~URLLangGuesser()
 {
  short i,j,k;
  if(frequents!=NULL)
   {
    for(i=0;i<numlanguages;i++)
     {
      for(j=0;j<maxsizengrams;j++)
       {
        for(k=1;k<numfrequents;k++)
         {
          if(frequents[i][j][k].ngram!=NULL)
           {
            delete frequents[i][j][k].ngram;
            frequents[i][j][k].ngram=NULL;
           }
         }
        delete frequents[i][j];
       }
      delete frequents[i];
     } 
    delete frequents;
    frequents=NULL;
   }
  if(numngrams!=NULL)
   {
    for(i=0;i<numlanguages;i++)
      delete numngrams[i];
    delete numngrams;
    numngrams=NULL;
   }
  if(totalngrams!=NULL)
   {
    for(i=0;i<numlanguages;i++)
      delete totalngrams[i];
    delete totalngrams;
    totalngrams=NULL;
   }
  if(pointslang!=NULL)
   {
    delete pointslang;
    pointslang=NULL;
   }
 }
 
bool URLLangGuesser::Save(const char *file)
 {
  bool result;
  FILE *fsal; //Fichero de entrada
  short i,j,k;

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
        {//Guardamos primero la estructura de trigramas
         fprintf(fsal,"%d %d %d\n",numlanguages,maxsizengrams,numfrequents);
         
         //Guardamos el numero de trigramas que se guardan para cada idioma y tamaño
         for(i=0;i<numlanguages;i++)
          {
           for(j=0;j<maxsizengrams;j++)
             fprintf(fsal," %d",numngrams[i][j]);
          }
         fprintf(fsal,"\n");
         
         //Guardamos el numero de trigramas que se han usado en el entrenamiento para cada idioma y tamaño
         for(i=0;i<numlanguages;i++)
          {
           for(j=0;j<maxsizengrams;j++)
             fprintf(fsal," %d",totalngrams[i][j]);
          }
         fprintf(fsal,"\n");
         
         
         for(i=0;i<numlanguages;i++)
          {
           if(possiblelang[i])
            {
             for(j=0;j<maxsizengrams;j++)
              {
               fprintf(fsal,"%f\n",frequents[i][j][0].frequency);//Guardamos el resto
               for(k=1;k<numngrams[i][j];k++)
                 fprintf(fsal,"%s %f\n",frequents[i][j][k].ngram,frequents[i][j][k].frequency);
              }
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

bool URLLangGuesser::Load(const char *file)
 {
  bool result;
  FILE *fent;
  char buffer[kSizeAuxStrings]; //Buffer para la lectura del fichero
  int i,j,k,languagesinfile,l;
  
  result=true;
  if(!(fent=fopen(file,"r")))
    result=false;
   else
    {
     fscanf(fent,"%d %d %d\n",&i,&j,&k);     
     if(i<0 || i>numlanguages || j<0 || j>kMaxSizeNGrams || k<0 || k>1000000)
       result=false;
      else
       {
        languagesinfile=i;
        maxsizengrams=j;
        numfrequents=k;
        frequents=new NGram**[numlanguages];
        numngrams=new int*[numlanguages];
        totalngrams=new int*[numlanguages];
        possiblelang=new bool[numlanguages];
        for(i=0;i<numlanguages;i++)
         {
          frequents[i]=new NGram*[maxsizengrams];
          numngrams[i]=new int[maxsizengrams];
          totalngrams[i]=new int[maxsizengrams];
          possiblelang[i]=true;
          for(j=0;j<maxsizengrams;j++)
           {
            frequents[i][j]=new NGram[numfrequents];
            numngrams[i][j]=0;
            totalngrams[i][j]=0;
            for(k=0;k<numfrequents;k++)
             {
              frequents[i][j][k].ngram=NULL;
              frequents[i][j][k].frequency=0;
             }
           }
         }
        
        //Cargamos el numero de ngramas almacenados por cada idioma y tamaño
        for(j=0;j<languagesinfile && !feof(fent);j++)
         {
          for(k=0;k<maxsizengrams;k++)
           {
            fscanf(fent,"%d",&l);
            numngrams[j][k]=l;
           }
         }
        fscanf(fent,"\n");
        
        //Cargamos el numero de ngramas utilizados en el entrenamiento por cada idioma y tamaño
        for(j=0;j<languagesinfile && !feof(fent);j++)
         {
          for(k=0;k<maxsizengrams;k++)
           {
            fscanf(fent,"%d",&l);
            totalngrams[j][k]=l;
           }
         }
        fscanf(fent,"\n");
     
        for(i=0;i<languagesinfile;i++)
         {
          for(j=0;j<maxsizengrams;j++)
           {
            if(numngrams[i][j]>0)
             {
              fscanf(fent,"%f\n",&(frequents[i][j][0].frequency));
              for(k=1;k<numngrams[i][j] && !feof(fent);k++)
               {
                fscanf(fent,"%s %f\n",buffer,&(frequents[i][j][k].frequency));
                if(frequents[i][j][k].frequency>0)
                 {
                  frequents[i][j][k].ngram=new char[1+strlen(buffer)];
                  strcpy(frequents[i][j][k].ngram,buffer);
                 }
                 else
                   frequents[i][j][k].ngram=NULL;
               }
             }
             else
               possiblelang[i]=false;
           }
         }
        if(i!=languagesinfile || j!=maxsizengrams)
          result=false;
       }
       
     if(!result)
      {
       if(frequents!=NULL)
        {
         for(i=0;i<numlanguages;i++)
          {
           for(j=0;j<maxsizengrams;j++)
            {
             for(k=1;k<numfrequents;k++)
              {
               if(frequents[i][j][k].ngram!=NULL)
                {
                 delete frequents[i][j][k].ngram;
                 frequents[i][j][k].ngram=NULL;
                }
              }
            }
           delete frequents[i];
          } 
         delete frequents;
         frequents=NULL;
        }
       if(numngrams!=NULL)
        {
         for(i=0;i<numlanguages;i++)
           delete numngrams[i];
         delete numngrams;
         numngrams=NULL;
        }
       if(totalngrams!=NULL)
        {
         for(i=0;i<numlanguages;i++)
           delete totalngrams[i];
         delete totalngrams;
         totalngrams=NULL;
        }
      }
     fclose(fent);
    }
  return(result);
 }

bool URLLangGuesser::Train(const char* file,short language)
 {
  bool result;
  FILE *fent;
  char a;//Variable para los caracteres
  char **filebuffer;//Variables para los n-gramas que se cargan de disco
  int i,j,k;
  NGram **filengrams; //Auxiliar para contar los n-gramas del fichero
  short **tempngrams; //Indica el instante en que se inserto el n-grama en el array
  float **placengrams; //Indica la puntuacion del n-grama para determinar cual quitar
  unsigned int *timeperngrams; //Indica el numero de ngramas de disco que se han leido para cada tamaño
  float minimvalue; //Indica el valor minimo del placengrams
  int minimplace; //Indica la posicion donde ira el nuevo n-grama
  unsigned int timestep; //Indica el numero de n-grama del fichero que se inserta
  bool inserted; //Indica que ya se ha insertado el n-grama
  bool arrayfull; //Indica si ya se ha llenado el vector de n-gramas del fichero
  short newngrams; //Indica el numero de nuevos n-gramas encontrados hasta el momento
  bool **updatedngrams; //Indica los n-gramas ya actualizados
  float suma; //Variable que sirve para actualizar el resto de n-gramas
  bool found; //Indica si se ha encontrado el n-grama que buscabamos
  
  filengrams=new NGram*[maxsizengrams];
  filebuffer=new char*[maxsizengrams];
  tempngrams=new short*[maxsizengrams];
  placengrams=new float*[maxsizengrams];
  updatedngrams=new bool*[maxsizengrams];
  timeperngrams=new unsigned int[maxsizengrams];
  for(i=0;i<maxsizengrams;i++)
   {
    filengrams[i]=new NGram[numfrequents];
    filengrams[i][0].frequency=0;    
    for(j=0;j<numfrequents;j++)
     {
      filengrams[i][j].ngram=NULL;
      filengrams[i][j].frequency=0;
     }
    filebuffer[i]=new char[i+1];
    for(j=0;j<i+1;j++)
      filebuffer[i][j]=' ';
    filebuffer[i][j]='\0';
    tempngrams[i]=new short[numfrequents];
    placengrams[i]=new float[numfrequents];
    updatedngrams[i]=new bool[numfrequents];
    timeperngrams[i]=0;    
   }

  result=true;
  arrayfull=false;
  if(language<numlanguages && (fent=fopen(file,"r")))
   {
    timestep=0;    
    minimvalue=999999999;
    minimplace=-1;
    while(!feof(fent))
     {//Leemos caracter a caracter
      a=fgetc(fent);
      if(a!='\n' && !feof(fent))
       {
        for(i=0;i<maxsizengrams;i++)
         {
          for(j=0;j<i;j++)//Desplazamos 
           {
            filebuffer[i][j]=filebuffer[i][j+1];
           }
          filebuffer[i][j]=a;
#ifdef probando          
          if(strlen(filebuffer[i])<(unsigned)i)
            fprintf(stderr,"N-grama no corresponde: \"%s\"\n",filebuffer[i]);
#endif            
          if(filebuffer[i][0]!=' ')//Nos aseguramos de que el ngrama este completo
           {                                         
            //Insertamos el ngrama si hay hueco
            for(j=1,inserted=false;!inserted && j<numfrequents;j++)
             {
              if(filengrams[i][j].ngram!=NULL)
               {
                if(strcmp(filengrams[i][j].ngram,filebuffer[i])==0)
                 {//Es el mismo ngrama asi que se incrementa su frecuencia
                  filengrams[i][j].frequency=filengrams[i][j].frequency+1;
                  tempngrams[i][j]=timestep;
                  inserted=true;
                 }
                 else
                  {
                   placengrams[i][j]=kAjusteLG*filengrams[i][j].frequency/(timestep-tempngrams[i][j]);
                   if(j==1)
                    {
                     minimvalue=placengrams[i][1];
                     minimplace=1;
                    }
                    else
                     {
                      if(minimvalue>placengrams[i][j])
                       {
                        minimvalue=placengrams[i][j];
                        minimplace=j;
                       }
                     }
                  }
               }
               else
                {//Aun no estaba lleno el vector de trigramas, asi que insertamos aqui
                 filengrams[i][j].ngram=new char[1+strlen(filebuffer[i])];
                 strcpy(filengrams[i][j].ngram,filebuffer[i]);
                 filengrams[i][j].frequency=1;
                 tempngrams[i][j]=timestep;
                 inserted=true;
                }
             }
        
            if(!inserted)
             {//El vector estaba lleno, asi que lo insertamos reemplazando
              filengrams[i][0].frequency=filengrams[i][0].frequency+filengrams[i][minimplace].frequency;
              delete filengrams[i][minimplace].ngram;
              filengrams[i][minimplace].ngram=new char[1+strlen(filebuffer[i])];
              strcpy(filengrams[i][minimplace].ngram,filebuffer[i]);
              filengrams[i][minimplace].frequency=1;
              tempngrams[i][minimplace]=timestep;
             }
            timeperngrams[i]++;
           }
         }
#ifdef probando
  //Realizamos el sumatorio
  for(i=0;i<maxsizengrams;i++)
   {
    for(suma=0,j=0;j<numfrequents;j++)
     {
      suma+=filengrams[i][j].frequency;
     }
    if(suma<timeperngrams[i]-0.001 || suma>timeperngrams[i]+0.001)
      fprintf(stderr,"Warning: La suma de las probabilidades no es %d sino %f\n",timeperngrams[i],suma);
   }
#endif        
       }
       else
        {
         for(i=0;i<maxsizengrams;i++)
          {
           for(j=0;j<i+1;j++)
             filebuffer[i][j]=' ';
           filebuffer[i][j]='\0';           
          }
        }
      timestep++;
     }
        
    //Normalizamos las frecuencias
    for(i=0;i<maxsizengrams;i++)
     {
      for(j=0;j<numfrequents;j++)
       {
        filengrams[i][j].frequency=filengrams[i][j].frequency/timeperngrams[i];
        updatedngrams[i][j]=false;//Inicializamos de paso este vector
       }
     }
    
#ifdef probando
  //Realizamos el sumatorio
  for(i=0;i<maxsizengrams;i++)
   {
    for(suma=0,j=0;j<numfrequents;j++)
     {
      suma+=filengrams[i][j].frequency;
     }
    if(suma<1-0.001 || suma>1+0.001)
      fprintf(stderr,"Warning: La suma de las probabilidades no es 1 sino %f\n",suma);
   }
#endif

    for(i=0;i<maxsizengrams;i++)
     {
      newngrams=0;

      if(totalngrams[language][i]>0)
       {//Ahora combinamos los resultados          
        filengrams[i][0].frequency=(filengrams[i][0].frequency*timestep+frequents[language][i][0].frequency*totalngrams[language][i])/(timestep+totalngrams[language][i]);
        updatedngrams[i][0]=true;
    
        for(j=1;j<numfrequents;j++)
         {
          if(frequents[language][i][j].ngram!=NULL)
           {//El trigrama existe asi que intentamos compararlo
            for(k=1,found=false;k<numfrequents && !found;k++)
             {
              if(strcmp(filengrams[i][k].ngram,frequents[language][i][j].ngram)==0)
               {//Es el mismo trigrama asi que se incrementa su frecuencia
#ifdef probando
                if(updatedngrams[i][k])
                  fprintf(stderr,"Warning: updating a previously updated trigram (repeated trigram?)\n");
#endif                
                filengrams[i][k].frequency=(filengrams[i][k].frequency*timestep+frequents[language][i][j].frequency*totalngrams[language][i])/(timestep+totalngrams[language][i]);
                found=true;
                updatedngrams[i][k]=true;
               }
             }
            if(!found)
             {//Insertamos el trigrama antiguo
              filengrams[i][numfrequents+newngrams].ngram=new char[1+strlen(frequents[language][i][j].ngram)];
              strcpy(filengrams[i][numfrequents+newngrams].ngram,frequents[language][i][j].ngram);
              filengrams[i][numfrequents+newngrams].frequency=(frequents[language][i][j].frequency*totalngrams[language][i])/(timestep+totalngrams[language][i]);
              newngrams++;
             }
           }
         }

        //Ahora quedaria actualizar las frecuencias nuevas
        for(j=1;j<numfrequents;j++)
         {
          if(!updatedngrams[i][j])
            filengrams[i][j].frequency=(filengrams[i][j].frequency*timestep)/(timestep+totalngrams[language][i]);
         }
       }
         
      //Por ultimo hay que reordenar el vector para quedarnos con los trigramas mas frecuentes    
      qsort(&(filengrams[i][1]),numfrequents+newngrams-1,sizeof(NGram),CompareNGramFrequency);
    
      //Actualizamos el valor del resto
      for(suma=0,j=1;j<numfrequents;j++)
        suma+=filengrams[i][j].frequency;
      if(1-suma<0)
        filengrams[i][0].frequency=0;
       else
         filengrams[i][0].frequency=1-suma;
      
    
      //Nos quedamos con los trigramas mas frecuentes
      frequents[language][i][0].frequency=filengrams[i][0].frequency;
    
      for(j=1;j<numfrequents;j++)
       {
        frequents[language][i][j].frequency=filengrams[i][numfrequents-j].frequency;
        if(frequents[language][i][j].ngram!=NULL)
          delete frequents[language][i][j].ngram;
        if(filengrams[i][numfrequents-j].ngram!=NULL)
         {
          frequents[language][i][j].ngram=new char[1+strlen(filengrams[i][numfrequents-j].ngram)];
          strcpy(frequents[language][i][j].ngram,filengrams[i][numfrequents-j].ngram);
         }
         else
           frequents[language][i][j].ngram=NULL;
       }
      totalngrams[language][i]=totalngrams[language][i]+timeperngrams[i];
     }
    
#ifdef probando
  //Realizamos el sumatorio
  for(i=0;i<maxsizengrams;i++)
   {
    for(suma=0,j=0;j<numfrequents;j++)
     {
      suma+=frequents[language][i][j].frequency;
     }
    if(suma<1-0.001 || suma>1+0.001)
      fprintf(stderr,"Warning: La suma de las probabilidades no es 1 sino %f\n",suma);
   }
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
    
  for(i=0;i<maxsizengrams;i++)
   {
    for(j=0;j<numfrequents;j++)
     {
      if(filengrams[i][j].ngram!=NULL)
        delete filengrams[i][j].ngram;
     }
    delete filengrams[i];
    delete filebuffer[i];
    delete tempngrams[i];
    delete placengrams[i];      
    delete updatedngrams[i];
   }
  delete filengrams;
  delete filebuffer;
  delete tempngrams;
  delete placengrams;
  delete updatedngrams;
  delete timeperngrams;  
  return(result);
 }

short URLLangGuesser::GuessText(const char* text)
 {
  short result;
  char a;//Variable para los caracteres
  char **filebuffer;//Variables para los n-gramas que se cargan de disco
  int i,j,k,l;
  unsigned int recorretexto; //Variable para el recorrido de la cadena de entrada
  NGram **filengrams; //Auxiliar para contar los n-gramas del fichero
  short **tempngrams; //Indica el instante en que se inserto el n-grama en el array
  float **placengrams; //Indica la puntuacion del n-grama para determinar cual quitar
  unsigned int *timeperngrams; //Indica el numero de ngramas de disco que se han leido para cada tamaño
  float minimvalue; //Indica el valor minimo del placengrams
  int minimplace; //Indica la posicion donde ira el nuevo n-grama
  unsigned int timestep; //Indica el numero de n-grama del fichero que se inserta
  bool inserted; //Indica que ya se ha insertado el n-grama
  bool arrayfull; //Indica si ya se ha llenado el vector de n-gramas del fichero
  short newngrams; //Indica el numero de nuevos n-gramas encontrados hasta el momento
  bool found; //Indica si se ha encontrado el n-grama que buscabamos
  float betterprob; //Indica la maxima probabilidad que tiene un idioma
  float betterprob2; //Indica la segunda probabilidad mas probable que tiene un idioma
  float **pointslangng; //Indica los puntos para cada idioma de cada ngrama por separado

#ifdef probando  
  float suma; //Variable que sirve para actualizar el resto de n-gramas
#endif
    
  filengrams=new NGram*[maxsizengrams];
  filebuffer=new char*[maxsizengrams];
  tempngrams=new short*[maxsizengrams];
  placengrams=new float*[maxsizengrams];
  timeperngrams=new unsigned int[maxsizengrams];
  for(i=0;i<maxsizengrams;i++)
   {
    filengrams[i]=new NGram[numfrequents];
    filengrams[i][0].frequency=0;    
    for(j=0;j<numfrequents;j++)
     {
      filengrams[i][j].ngram=NULL;
      filengrams[i][j].frequency=0;
     }
    filebuffer[i]=new char[i+1];
    for(j=0;j<i+1;j++)
      filebuffer[i][j]=' ';
    filebuffer[i][j]='\0';
    tempngrams[i]=new short[numfrequents];
    placengrams[i]=new float[numfrequents];
    timeperngrams[i]=0;
   }
  
  result=-1;
  arrayfull=false;
  if(text!=NULL)
   {
    timestep=0;
    minimvalue=999999999;
    minimplace=-1;
    for(recorretexto=0;recorretexto<strlen(text);recorretexto++)
     {//Leemos caracter a caracter
      a=text[recorretexto];
      if(a!='\n')
       {
        for(i=0;i<maxsizengrams;i++)
         {
          for(j=0;j<i;j++)//Desplazamos 
           {
            filebuffer[i][j]=filebuffer[i][j+1];
           }
          filebuffer[i][j]=a;
#ifdef probando          
          if(strlen(filebuffer[i])<(unsigned)i)
            fprintf(stderr,"N-grama no corresponde: \"%s\"\n",filebuffer[i]);
#endif            
          if(filebuffer[i][0]!=' ')//Nos aseguramos de que el ngrama este completo
           {                                         
            //Insertamos el ngrama si hay hueco
            for(j=1,inserted=false;!inserted && j<numfrequents;j++)
             {
              if(filengrams[i][j].ngram!=NULL)
               {
                if(strcmp(filengrams[i][j].ngram,filebuffer[i])==0)
                 {//Es el mismo ngrama asi que se incrementa su frecuencia
                  filengrams[i][j].frequency=filengrams[i][j].frequency+1;
                  tempngrams[i][j]=timestep;
                  inserted=true;
                 }
                 else
                  {
                   placengrams[i][j]=kAjusteLG*filengrams[i][j].frequency/(timestep-tempngrams[i][j]);
                   if(j==1)
                    {
                     minimvalue=placengrams[i][1];
                     minimplace=1;
                    }
                    else
                     {
                      if(minimvalue>placengrams[i][j])
                       {
                        minimvalue=placengrams[i][j];
                        minimplace=j;
                       }
                     }
                  }
               }
               else
                {//Aun no estaba lleno el vector de trigramas, asi que insertamos aqui
                 filengrams[i][j].ngram=new char[1+strlen(filebuffer[i])];
                 strcpy(filengrams[i][j].ngram,filebuffer[i]);
                 filengrams[i][j].frequency=1;
                 tempngrams[i][j]=timestep;
                 inserted=true;
                }
             }
        
            if(!inserted)
             {//El vector estaba lleno, asi que lo insertamos reemplazando
              filengrams[i][0].frequency=filengrams[i][0].frequency+filengrams[i][minimplace].frequency;
              delete filengrams[i][minimplace].ngram;
              filengrams[i][minimplace].ngram=new char[1+strlen(filebuffer[i])];
              strcpy(filengrams[i][minimplace].ngram,filebuffer[i]);
              filengrams[i][minimplace].frequency=1;
              tempngrams[i][minimplace]=timestep;
             }
            timeperngrams[i]++;
           }
         }
#ifdef probando
  //Realizamos el sumatorio
  for(i=0;i<maxsizengrams;i++)
   {
    for(suma=0,j=0;j<numfrequents;j++)
     {
      suma+=filengrams[i][j].frequency;
     }
    if(suma<timeperngrams[i]-0.001 || suma>timeperngrams[i]+0.001)
      fprintf(stderr,"Warning: La suma de las probabilidades no es %d sino %f\n",timeperngrams[i],suma);
   }
#endif        
       }
       else
        {
         for(i=0;i<maxsizengrams;i++)
          {
           for(j=0;j<i+1;j++)
             filebuffer[i][j]=' ';
           filebuffer[i][j]='\0';           
          }
        }
      timestep++;
     }
        
    //Normalizamos las frecuencias
    for(i=0;i<maxsizengrams;i++)
     {
      for(j=0;j<numfrequents;j++)
        filengrams[i][j].frequency=filengrams[i][j].frequency/timeperngrams[i];
     }
    
#ifdef probando
  //Realizamos el sumatorio
  for(i=0;i<maxsizengrams;i++)
   {
    for(suma=0,j=0;j<numfrequents;j++)
     {
      suma+=filengrams[i][j].frequency;
     }
    if(suma<1-0.001 || suma>1+0.001)
      fprintf(stderr,"Warning: La suma de las probabilidades no es 1 sino %f\n",suma);
   }
#endif

    if(pointslang!=NULL)
      delete pointslang;
    pointslang=new float[numlanguages];
    pointslangng=new float*[numlanguages];
    betterprob=0;
    betterprob2=0;
    for(i=0;i<numlanguages;i++)
     {
      newngrams=0;
      pointslangng[i]=new float[maxsizengrams];

      for(j=0;j<maxsizengrams;j++)
       {
        pointslangng[i][j]=0;
        if(possiblelang[i])
         {
          pointslangng[i][j]=0;
          
          for(k=1;k<numfrequents;k++)
           {
            if(filengrams[j][k].ngram!=NULL)
             {//El n-grama existe asi que intenamos compararlo
              for(l=1,found=false;l<numfrequents && !found;l++)
               {
                if(frequents[i][j][k].ngram!=NULL)
                 {
                  if(strcmp(filengrams[j][k].ngram,frequents[i][j][k].ngram)==0)
                   {
                    pointslangng[i][j]=pointslangng[i][j]+filengrams[j][k].frequency*frequents[i][j][k].frequency;
                    found=true;
                   }
                 }
               }               
             }
           }          
#ifdef probando
          fprintf(stderr,"Resultado producto escalar (%d,%d) = %f\n",i,j,pointslangng[i][j]);
#endif                                 
         }        
       }
     
      //Combinamos los resultados de los ngramas
      for(j=0;j<maxsizengrams;j++)
       {
        pointslang[i]=(j+1)*pointslangng[i][j];
       }
#ifdef probando
      fprintf(stderr,"Resultado combi producto escalar (%d) = %f\n",i,pointslang[i]);
#endif         
      if(pointslang[i]>betterprob)
       {
        result=i;
        betterprob2=betterprob;
        betterprob=pointslang[i];
       }
       else
        {
         if(pointslang[i]>betterprob2)
           betterprob2=pointslang[i];
        }                             
     }
    for(i=0;i<numlanguages;i++)
      delete pointslangng[i];
    delete pointslangng;       
   }
   else
    {
     result=-1;
     fprintf(stderr,"Error: The input string was empty\n");
    }
    
  for(i=0;i<maxsizengrams;i++)
   {
    for(j=0;j<numfrequents;j++)
     {
      if(filengrams[i][j].ngram!=NULL)
        delete filengrams[i][j].ngram;
     }
    delete filengrams[i];
    delete filebuffer[i];
    delete tempngrams[i];
    delete placengrams[i];      
   }
  delete filengrams;
  delete filebuffer;
  delete tempngrams;
  delete placengrams;
  delete timeperngrams;
  return(result);
 }

void URLLangGuesser::Normalize()
 {
  float suma;
  int i;
  
  for(i=0,suma=0;i<numlanguages;i++)
    suma+=pointslang[i];
  if(suma>0)
   {
    for(i=0;i<numlanguages;i++)
      pointslang[i]=pointslang[i]/suma;
   }
 }

bool URLLangGuesser::SetPossibleLanguages(const char *langstring)
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

unsigned short* URLLangGuesser::GivePossibleLanguages(unsigned int *npossiblelanguages)
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
  
bool URLLangGuesser::SameMagnitude(float freq1,float freq2)
 { 
  bool result;
  
  if(freq1/10>freq2 || freq2/10>freq1)
    result=false;
   else
     result=true;  
  return(result);
 }

void URLLangGuesser::ReduceNgrams()
 {
  int i,j,k,l,m;
  NGram ***newfrequents;
  int **numnewfrequents;
  bool found;
  float suma;//Util para recalcular el resto
  
  newfrequents=new NGram**[numlanguages];
  numnewfrequents=new int*[numlanguages];
  for(i=0;i<numlanguages;i++)
   {
    newfrequents[i]=new NGram*[maxsizengrams];
    numnewfrequents[i]=new int[maxsizengrams];
    for(j=0;j<maxsizengrams;j++)
     {
      newfrequents[i][j]=new NGram[numfrequents];
      numnewfrequents[i][j]=0;
      for(k=0;k<numfrequents;k++)
        newfrequents[i][j][k].ngram=NULL;
     }
   }
  
  for(i=0;i<maxsizengrams;i++)
   {
    for(j=0;j<numlanguages;j++)
     {
      for(k=1;k<numfrequents;k++)
       {
        if(frequents[j][i][k].ngram!=NULL)
         {//Buscamos a ver si la frecuencia de este ngrama es similar en otros idiomas
          for(l=0,found=false;l<numlanguages && !found;l++)
           {
            if(j!=l && possiblelang[j])
             {
              for(m=0;m<numfrequents && !found;m++)
               {
                if(frequents[l][i][m].ngram!=NULL && strcmp(frequents[j][i][k].ngram,frequents[l][i][m].ngram)==0)
                 {//Comprobamos que la frecuencia no sea similar
                  if(frequents[j][i][k].frequency < frequents[l][i][m].frequency ||
                     SameMagnitude(frequents[l][i][m].frequency,frequents[j][i][k].frequency))
                   {//Hay que descartar este ngrama
                    found=true;
                   }
                 }
               }
             }
           }
          if(!found)
           {//Metemos el ngrama en la nueva estructura
            newfrequents[j][i][numnewfrequents[j][i]].ngram=new char[1+strlen(frequents[j][i][k].ngram)];
            strcpy(newfrequents[j][i][numnewfrequents[j][i]].ngram,frequents[j][i][k].ngram);
            newfrequents[j][i][numnewfrequents[j][i]].frequency=frequents[j][i][k].frequency;
            (numnewfrequents[j][i])++;
           }
         }
       }
      //Recalculamos el valor del resto en funcion de los descartados
      for(suma=0,k=1;k<numfrequents;k++)
        suma+=newfrequents[j][i][k].frequency;
      if(1-suma<0)
        newfrequents[j][i][0].frequency=0;
       else
         newfrequents[j][i][0].frequency=1-suma;

     }
   }
   
  for(i=0;i<numlanguages;i++)
   {
    for(j=0;j<maxsizengrams;j++)
     {
      for(k=0;k<numfrequents;k++)
       {
        if(frequents[i][j][k].ngram!=NULL)
          delete frequents[i][j][k].ngram;
       }
      delete frequents[i][j];
     }
    delete frequents[i];
    delete numngrams[i];
   }
  delete frequents;
  delete numngrams;
  frequents=newfrequents;
  numngrams=numnewfrequents;
 }

bool URLLangGuesser::GeneratePatterns(const char *filename)
 {
  bool result;
  FILE *fsal;
  unsigned int i,j,k,l,m;
  
  if(filename!=NULL)
   {
    if((fsal=fopen(filename,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: The output file \"%s\"could not be opened\n",filename);
     }
     else
      {
       for(i=2;i<kMaxSizeNGrams;i++)//Con n-gramas menores de 3 letras no se trabaja
        {
         for(j=0;j<numlanguages;j++)
          {
           if(possiblelang[j])
            {
             for(k=1;k<1+kGenerateVariants;k++)
              {
               for(l=1;l<1+kGenerateVariants;l++)
                {
                 fprintf(fsal,"%s \"%s\" ->",LanguageName(j),frequents[j][i][k].ngram);
                 for(m=0;m<numlanguages;m++)
                  {
                   if(m!=j && l<(unsigned)numngrams[m][i])
                     fprintf(fsal," \"%s\"",frequents[m][i][l]);
                  }
                 fprintf(fsal,"\n");
                }
              }
            }
          }
        }
       fclose(fsal);
       result=true;
      }      
   }
   else
     result=false;
  
  return(result);
 }

const float* URLLangGuesser::PointsPerLanguage()
 {
  const float *result;
  
  result=pointslang;
  return(result);
 }
