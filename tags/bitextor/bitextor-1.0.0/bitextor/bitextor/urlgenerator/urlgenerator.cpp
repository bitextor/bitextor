/*
   Clase que se encarga de adivinar el idioma en el que esta un determinado
   fichero de texto llano o etiquetado. La adivinacion se basa en trigramas
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "urlgenerator.h"

URLGenerator::URLGenerator()
 {
  changepatterns=NULL;
  nchangepatterns=0;
 }

URLGenerator::URLGenerator(const char *localfilename)
 {
  if(!Load(localfilename))
   {//Actuamos como el constructor normal
    changepatterns=NULL;
    nchangepatterns=0;
   }
 }

URLGenerator::~URLGenerator()
 {
  ResetPatterns();
 }
 
void URLGenerator::ResetPatterns()
 {
  unsigned int i,j;
  
  if(changepatterns!=NULL)
   {
    for(i=0;i<nchangepatterns && changepatterns!=NULL;i++)
     {
      if(changepatterns[i].text!=NULL)
        delete changepatterns[i].text;
      if(changepatterns[i].changes!=NULL)
       {
        for(j=0;j<changepatterns[i].nchanges;j++)
         {
          if(changepatterns[i].changes[j]!=NULL)
            delete changepatterns[i].changes[j];
         }
        delete changepatterns[i].changes;
       }
     }    
    delete changepatterns;
    changepatterns=NULL;
    nchangepatterns=0;
   }
 } 

bool URLGenerator::Load(const char *filename)
 {
  bool result;
  FILE *fent;
  char buffer[kSizeAuxStrings]; //Buffer para la lectura del fichero
  unsigned int i,j;
  ChangePattern aux[kMaxNumberPatterns]; //Auxiliar para los patrones
  unsigned int naux; //Numero de patrones insertados
  char buffer2[kMaxChangesInVariation][kSizeAuxStrings];//Buffer para cdenas de cambios
  bool endpattern; //Indica que se ha acabado de leer un patron
  
  result=true;
  i=0;
  if(!(fent=fopen(filename,"r")))
    result=false;
   else
    {
     naux=0;
     do
      {
       buffer[0]=fgetc(fent);
       if(buffer[0]!='\n' && !feof(fent))
        {//Se coge el idioma           
         j=0;           
         while(buffer[j]!=' ')
          {             
           j++;             
           buffer[j]=fgetc(fent);
          };
         buffer[j]='\0';
         aux[naux].language=LanguageCode(buffer);
         if(aux[naux].language==-1)
          {//Error en el patron, no pertenece a ninguna lengua conocida
           result=false;
           fprintf(stderr,"Error: Loading patterns (language) line %d\n",naux+1);
          }
          else
           {
            //Leemos hasta la primera comilla (incluida)
            for(buffer[0]=fgetc(fent);buffer[0]!='\"' && buffer[0]!='\n' && !feof(fent);buffer[0]=fgetc(fent));
            if(buffer[0]!='\"')
             {//Error en el patron, no se encontro "
              result=false;
              fprintf(stderr,"Error: Loading patterns (pattern) line %d\n",naux+1);
             }
             else
              {            
               //Leemos y almacenamos el patron que contendra hasta la siguiente comilla no incluida
               for(buffer[0]=fgetc(fent),j=0;buffer[j]!='\"' && buffer[j]!='\n' && !feof(fent);j++,buffer[j]=fgetc(fent));
               if(buffer[j]!='\"')
                {//Error en el patron, no se encontro "
                 result=false;
                 fprintf(stderr,"Error: Loading patterns (pattern) line %d\n",naux+1);
                }
                else
                 {            
                  buffer[j]='\0';
                  aux[naux].text=new char[1+strlen(buffer)];
                  strcpy(aux[naux].text,buffer);
            
                  //Leemos el ->
                  for(buffer[0]=fgetc(fent);buffer[0]!='-' && buffer[0]!='\n' && !feof(fent);buffer[0]=fgetc(fent));
                  if(buffer[0]!='-')
                   {//Error en el patron, no se encontro ->
                    result=false;
                    fprintf(stderr,"Error: Loading patterns (->) line %d\n",naux+1);
                   }
                   else
                    {
                     buffer[0]=fgetc(fent);
                     if(buffer[0]!='>')
                      {//Error en el patron, no se encontro ->
                       result=false;
                       fprintf(stderr,"Error: Loading patterns (->) line %d\n",naux+1);             
                      }
                      else
                       {
                        endpattern=false;
                        aux[naux].nchanges=0;
                        while(!endpattern && result && aux[naux].nchanges<kMaxChangesInVariation)
                         {
                          //Leemos hasta la primera comilla (incluida)
                          for(buffer[0]=fgetc(fent);buffer[0]!='\"' && buffer[0]!='\n' && !feof(fent);buffer[0]=fgetc(fent));
                          if(buffer[0]!='\"' && aux[naux].nchanges==0)
                           {//Error en el patron, no se encontro "
                            result=false;
                            fprintf(stderr,"Error: Loading patterns (change) line %d\n",naux+1);
                           }
                           else
                            {
                             if(buffer[0]=='\n')
                              {
                               endpattern=true;//Hemos terminado de leer cambios
                              }
                              else
                               {//Almacenamos el cambio
                                for(buffer[0]=fgetc(fent),j=0;buffer[j]!='\"' && buffer[j]!='\n' && !feof(fent);j++,buffer[j]=fgetc(fent));
                                if(buffer[j]!='\"')
                                 {//Error en el patron, no se encontro "
                                  result=false;
                                  fprintf(stderr,"Error: Loading patterns (pattern) line %d\n",naux+1);
                                 }
                                 else
                                  {            
                                   buffer[j]='\0';
                                   strcpy(buffer2[aux[naux].nchanges],buffer);
                                   aux[naux].nchanges++;
                                  }
                               }
                            }
                         };
                         
                        //Almacenamos adecuadamente los cambios
                        aux[naux].changes=new char*[aux[naux].nchanges];
                        for(i=0;i<aux[naux].nchanges;i++)
                         {
                          aux[naux].changes[i]=new char[1+strlen(buffer2[i])];
                          strcpy(aux[naux].changes[i],buffer2[i]);
                         }
                       }
                    }                 
                 }              
              }
           }
         if(result)
           naux++;
        }
      } while(!feof(fent) && i<kMaxNumberPatterns && result);
     
     if(result)
      {//Ahora copiamos la estructura temporal en la original
       ResetPatterns();
       changepatterns=new ChangePattern[naux];
       for(i=0;i<naux;i++)
        {
         changepatterns[i].language=aux[i].language;
         changepatterns[i].text=aux[i].text;
         changepatterns[i].nchanges=aux[i].nchanges;
         changepatterns[i].changes=aux[i].changes;
        }
       nchangepatterns=naux;
      }
    }
  
  return(result);
 }

bool URLGenerator::Save(const char *filename)
 {
  bool result;
  FILE *fsal; //Fichero de entrada
  unsigned short i,j;

  if(changepatterns!=NULL)
   {
    if((fsal=fopen(filename,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: The output file \"%s\" could not be opened\n",filename);
     }
     else
      {
       for(i=0;i<nchangepatterns;i++)
        {
         fprintf(fsal,"%s \"%s\" ->",LanguageName(changepatterns[i].language),changepatterns[i].text);
         for(j=0;j<changepatterns[i].nchanges;j++)
           fprintf(fsal," \"%s\"",changepatterns[i].changes[j]);
         fprintf(fsal,"\n");
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
 
char** URLGenerator::GenerateVariants1(short language,const char *url,unsigned int *numvariants)
 {
  char **result,*aux[kMaxGenerations];
  unsigned int nvariants; //Indica las variantes que llevamos generadas
  unsigned int i,j,k,l,m,n;
  char actualvariant[kSizeAuxStrings]; //Variante que se esta contruyendo
  unsigned int npossiblechanges; //Numero de cambios que se pueden realizar en una cadena
  unsigned int possiblechanges[kMaxChangesInVariation]; //Posiciones en las que se encuentran esos cambios
  
  bool equals; //Indica si el patron se puede aplicar o si no
  bool found;
  
  if(changepatterns!=NULL && url!=NULL && language>=0 && language>=0)
   {
    nvariants=0;
    for(i=0;i<nchangepatterns && nvariants<kMaxGenerations;i++)
     {
      npossiblechanges=0;
      if(language==changepatterns[i].language)
       {//Solo se aplica un patron si coincide con el idioma de la pagina
        for(j=0,k=0;j<strlen(url);j++)
         {
          actualvariant[k]=url[j];
          k++;
          if(url[j]==changepatterns[i].text[0])
           {//Puede coincidir
            for(l=j+1,equals=true;l<strlen(url) && l-j<strlen(changepatterns[i].text) && equals;l++)
             {
              if(url[l]!=changepatterns[i].text[l-j])
                equals=false;
             }
            if(l-j!=strlen(changepatterns[i].text))
              equals=false;
            if(equals)
             {//El patron se puede aplicar asi que guardamos su posicion
              possiblechanges[npossiblechanges]=j;
              npossiblechanges++;
             }
           }
         }
        if(npossiblechanges>0)
         {
          for(j=0;j<changepatterns[i].nchanges && nvariants<kMaxGenerations;j++)
           {
            for(k=0,l=0,n=0;k<strlen(url);)
             {
              if(n<npossiblechanges && possiblechanges[n]==l)
               {//Se aplica el patron y se reemplaza
                for(m=0;m<strlen(changepatterns[i].changes[j]);m++)
                 {
                  actualvariant[k]=changepatterns[i].changes[j][m];
                  k++;
                 }
                l=l+strlen(changepatterns[i].text);
                n++;
               }
               else
                {
                 actualvariant[k]=url[l];
                 l++;
                 k++;
                }
             }
            //Ya tenemos una variante asi que la añadimos a la lista, no sin
            //antes comprobar que no este repetida
            for(k=0,found=false;k<nvariants && found==false;k++)
             {
              if(strcmp(aux[k],actualvariant)==0)
                found=true;
             }
            aux[nvariants]=new char[1+strlen(actualvariant)];
            strcpy(aux[nvariants],actualvariant);
            nvariants++;
           }
         }
       }
     }
    //Ahora guardamos la solucion en result y numvariants
    *numvariants=nvariants;
    if(nvariants>0)
     {
      result=new char*[nvariants];
      for(i=0;i<nvariants;i++)
        result[i]=aux[i];
     }
     else
       result=NULL;
   }
   else
     result=NULL;
  
  return(result);
 }

char** URLGenerator::GenerateVariants2(short language,const char *url,unsigned int *numvariants)
 {
  char **result,*aux[kMaxGenerations];
  unsigned int nvariants; //Indica las variantes que llevamos generadas
  unsigned int i,j,k,l,m,n;
  char actualvariant[kSizeAuxStrings]; //Variante que se esta contruyendo
  unsigned int npossiblechanges; //Numero de cambios que se pueden realizar en una cadena
  unsigned int possiblechanges[kMaxChangesInVariation]; //Posiciones en las que se encuentran esos cambios
  
  bool equals; //Indica si el patron se puede aplicar o si no
  bool found;
  
  if(changepatterns!=NULL && url!=NULL && language>=0 && language>=0)
   {
    nvariants=0;
    for(i=0;i<nchangepatterns && nvariants<kMaxGenerations;i++)
     {
      npossiblechanges=0;
      if(language==changepatterns[i].language)
       {//Solo se aplica un patron si coincide con el idioma de la pagina
        for(j=0,k=0;j<strlen(url);j++)
         {
          actualvariant[k]=url[j];
          k++;
          if(url[j]==changepatterns[i].text[0])
           {//Puede coincidir
            for(l=j+1,equals=true;l<strlen(url) && l-j<strlen(changepatterns[i].text) && equals;l++)
             {
              if(url[l]!=changepatterns[i].text[l-j])
                equals=false;
             }
            if(l-j!=strlen(changepatterns[i].text))
              equals=false;
            if(equals)
             {//El patron se puede aplicar asi que guardamos su posicion
              possiblechanges[npossiblechanges]=j;
              npossiblechanges++;
             }
           }
         }
        if(npossiblechanges>0)
         {
          for(j=0;j<changepatterns[i].nchanges && nvariants<kMaxGenerations;j++)
           {
            for(n=0;n<npossiblechanges;n++)
             {
              
              for(k=0,l=0;k<strlen(url);)
               {
                if(possiblechanges[n]==l)
                 {//Se aplica el patron y se reemplaza
                  for(m=0;m<strlen(changepatterns[i].changes[j]);m++)
                   {
                    actualvariant[k]=changepatterns[i].changes[j][m];
                    k++;
                   }
                  l=l+strlen(changepatterns[i].text);
                 }
                 else
                  {
                   actualvariant[k]=url[l];
                   l++;
                   k++;
                  }
               }
              
              
              
              
              /*strcpy(actualvariant,url);
              for(k=possiblechanges[n],m=0;m<strlen(changepatterns[i].changes[j]);m++)
               {
                actualvariant[k]=changepatterns[i].changes[j][m];
                k++;
               }*/
              //Ya tenemos una variante asi que la añadimos a la lista, no sin
              //antes comprobar que no este repetida
              for(k=0,found=false;k<nvariants && found==false;k++)
               {
                if(strcmp(aux[k],actualvariant)==0)
                  found=true;
               }
              aux[nvariants]=new char[1+strlen(actualvariant)];
              strcpy(aux[nvariants],actualvariant);
              nvariants++;
             }
           }
         }
       }
     }
    //Ahora guardamos la solucion en result y numvariants
    *numvariants=nvariants;
    if(nvariants>0)
     {
      result=new char*[nvariants];
      for(i=0;i<nvariants;i++)
        result[i]=aux[i];
     }
     else
       result=NULL;
   }
   else
     result=NULL;
  
  return(result);
 }
