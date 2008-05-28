/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "langguesser.h"
#include "trigramguesser.h"
#include "wordguesser.h"
#include "urlguesser.h"
#include <string.h>

#define cout std::cout
#define cerr std::cerr
#define endl std::endl

/** Funcion CHAPUZA que devuelve un entero asociado al numero que contiene la cadena
 *  Esta funcion tan solo convierte una cadena que contendra un numero entre
 *  el 0 y el 9 al formato entero.
 *
 *  @param number la cadena con el numero a convertir
 *  @returns el numero resultante de la conversion o -1 si no era valido
 */
int GiveNumber(char *number)
 {
  int result;
  unsigned int i;
  
  for(i=0,result=0;i<strlen(number) && result!=-1;i++)
   {
    if(number[i]>='0' && number[i]<='9')
     {
      result=result*10+number[i]-48;
     }
     else
       result=-1;
   }
  return(result);
 }

int main(int argc,char *argv[])
 {
  LangGuesser<TrigramLangGuesser> *trigramlangguesser;
  LangGuesser<WordLangGuesser> *wordlangguesser;
  WordLangGuesser *wordlangguesser2; //Util para las funciones especificas
  LangGuesser<URLLangGuesser> *urllangguesser;
  URLLangGuesser *urllangguesser2; //Util para las funciones especificas
  int language=-1,newsize;
  bool showuse;
  int result=-2;

  showuse=false;
  if(argc>2 && argc<6)
   {
    if(strcmp(argv[1],"-tt")==0 && argc==5)
     {//Entrenamiento para trigramas
      language=GiveNumber(argv[2]);
      if(language>-1 && language<kNumLanguages)
       {
        trigramlangguesser=new LangGuesser<TrigramLangGuesser>(kNumLanguages);
        trigramlangguesser->Load(argv[4]);
        trigramlangguesser->Train(argv[3],language);
        trigramlangguesser->Save(argv[4]);
        fprintf(stdout,"Trigram training successfully finished for language %d\n",language);
        //delete trigramlangguesser; //Segmentation fault to be fixed
       }
       else
        {
         if(language>-1)
           fprintf(stdout,"Error: Wrong parameter (lang_id)\n");
          else
            fprintf(stdout,"Error: There are not so many languages defined (lang_id)\n");
        }
     }
     else
      {
       if(strcmp(argv[1],"-tu")==0 && argc==5)
        {//Entrenamiento para urls
         language=GiveNumber(argv[2]);
         if(language>-1)
          {
           urllangguesser=new LangGuesser<URLLangGuesser>(kNumLanguages);
           urllangguesser->Load(argv[4]);
           urllangguesser->Train(argv[3],language);
           urllangguesser->Save(argv[4]);
           fprintf(stdout,"URL training successfully finished for language %d\n",language);
           delete urllangguesser;
          }
          else
            fprintf(stdout,"Error: Wrong parameter (lang_id)\n");              
        }
        else
         {
          if(strcmp(argv[1],"-td")==0 && argc==5)
           {//Carga de diccionario
            language=GiveNumber(argv[2]);
            if(language>-1)
             {
              wordlangguesser=new LangGuesser<WordLangGuesser>(kNumLanguages);
              wordlangguesser->Load(argv[4]);
              wordlangguesser->Train(argv[3],language);
              wordlangguesser->Save(argv[4]);
              fprintf(stdout,"Word dictionary successfully built for language %d\n",language);
              delete wordlangguesser;
             }
             else
               fprintf(stdout,"Error: Wrong parameter (lang_id)\n");
           }
           else
            {
             if(strcmp(argv[1],"-rd")==0 && argc==4)
              {//Compresion del diccionario
               newsize=atoi(argv[2]);
               if(newsize>0 && newsize<10000000)
                {
                 wordlangguesser2=new WordLangGuesser(kNumLanguages);
                 if(wordlangguesser2->Load(argv[3]))
                  {
                   wordlangguesser2->CleanHash(newsize);
                   wordlangguesser2->Save(argv[3]);
                   fprintf(stdout,"Word dictionary properly reduced\n");
                  }
                  else
                    fprintf(stderr,"Error: The input training file could not be loaded\n");
                 delete wordlangguesser2;
                }
                else
                  fprintf(stdout,"Error: Wrong parameter (new_size)\n");
              }
              else
               {              
                if(strcmp(argv[1],"-ru")==0 && argc==3)
                 {//Reduccion de ngramas no relevantes
                  urllangguesser2=new URLLangGuesser(kNumLanguages);
                  if(urllangguesser2->Load(argv[2]))
                   {
                    urllangguesser2->ReduceNgrams();
                    urllangguesser2->Save(argv[2]);
                    fprintf(stdout,"Reducing URL ngrams successfully finished\n",language);
                   }
                   else
                     fprintf(stderr,"Error: The input training file could not be loaded\n");
                  delete urllangguesser2;                  
                 }              
                 else
                  {
                   if(strcmp(argv[1],"-gt")==0 && argc==4)
                    {//Desambiguacion solo con trigramas
                     trigramlangguesser=new LangGuesser<TrigramLangGuesser>(kNumLanguages);
                     if(trigramlangguesser->Load(argv[2]))
                      {
                       result=trigramlangguesser->GuessFile(argv[3]);
                       cout<<result<<endl;
                      }
                      else
                        fprintf(stderr,"Error: The input parameter file could not be loaded\n");
                     delete trigramlangguesser;
                    }
                    else
                     {
                      if(strcmp(argv[1],"-gu")==0 && argc==4)
                       {//Desambiguacion solo con caracteres especiales
                        urllangguesser=new LangGuesser<URLLangGuesser>(kNumLanguages);
                        if(urllangguesser->Load(argv[2]))
                         {
                          result=urllangguesser->GuessText(argv[3]);
                          cout<<result<<endl;
                         }
                         else
                           fprintf(stderr,"Error: The input parameter file could not be loaded\n");
                        delete urllangguesser;
                       }
                       else
                        {
                         if(strcmp(argv[1],"-gd")==0 && argc==4)
                          {//Desambiguacion solo con diccionario
                           wordlangguesser=new LangGuesser<WordLangGuesser>(kNumLanguages);
                           if(wordlangguesser->Load(argv[2]))
                            {
                             result=wordlangguesser->GuessFile(argv[3]);
                             cout<<result<<endl;
                            }
                            else
                              fprintf(stderr,"Error: The input parameter file could not be loaded\n");
                           delete wordlangguesser;
                          }
                          else
                           {
                            if(strcmp(argv[1],"-nd")==0 && argc==3)
                             {//Adicion de un nuevo idioma al diccionario
                              wordlangguesser2=new WordLangGuesser(kNumLanguages);
                              if(wordlangguesser2->AddNewLanguage(argv[2]))
                               {
                                fprintf(stdout,"Word dictionary properly updated to handle a new language\n");
                               }
                               else
                                 fprintf(stderr,"Error: The input parameter file could not be loaded to be updated\n");
                              delete wordlangguesser2;
                             }
                             else
                               showuse=true;
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
     showuse=true;
  if(showuse)
   {
    cerr<<"Training:(Trigram) "<<argv[0]<<" -tt <lang_id> <file_training> <file_parameters>"<<endl;
    cerr<<"\t(URL) "<<argv[0]<<" -tu <lang_id> <file_training> <file_parameters>"<<endl;
    cerr<<"\t(Reducing URL ngrams) "<<argv[0]<<" -ru <file_parameters>"<<endl;
    cerr<<"\t(Dictionary) "<<argv[0]<<" -td <lang_id> <file_dict> <file_parameters>"<<endl;
    cerr<<"\t(Reducing dictionary) "<<argv[0]<<" -rd <new_size> <file_parameters>"<<endl;
    cerr<<"Guessing:(Trigrams only) "<<argv[0]<<" -gt <file_parameters> <file_to_guess>"<<endl;
    cerr<<"\t(URLs only) "<<argv[0]<<" -gu <file_parameters> <url_to_guess>"<<endl;
    cerr<<"\t(Dictionary only) "<<argv[0]<<" -gd <file_parameters> <file_to_guess>"<<endl;
   }
  return(result);
 }
