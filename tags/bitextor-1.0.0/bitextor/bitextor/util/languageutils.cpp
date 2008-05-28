#include "languageutils.h"

const char* LanguageName(short language)
 {
  switch(language)
   {    
    case 1: return("en");
    case 2: return("ca");
    case 3: return("pt");
    case 4: return("gl");
    default: return("es");
   }
 }

short LanguageCode(const char *language)
 {
  short result;
  
  if(language!=NULL)
   {
    if(strcmp(language,"es")==0)
      result=0;
     else
      {
       if(strcmp(language,"en")==0)
         result=1;
        else
         {
          if(strcmp(language,"ca")==0)
            result=2;
           else
            {
             if(strcmp(language,"pt")==0)
               result=3;
              else
               {
                if(strcmp(language,"gl")==0)
                  result=4;
                 else
                  {
                   result=-1;
                  }
               }
            }
         }
      }
   }
   else
     result=-1;
  return(result);
 }
