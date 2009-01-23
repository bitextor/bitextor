/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "../webpage/webpage.h"
#include "../util/genericutils.h"
#include "tagaligner2-1.h"

int main(int argc,char *argv[])
 {
  WebPage *lwebpage,*rwebpage;
  FILE *fout;
  bool showuse;
  char *tagaligneroutput;
//  TagAligner2step_ad *tagalignerA;
//  TagAligner2step_l *tagalignerB;
  TagAligner2_1 *tagalignerC;
  unsigned short option; //Indicates if the user has introduced an option
  unsigned int alignertype; //Indicates the aligner that will be used

  if(argc==5 || argc==6 || argc==7)
   {
    if(strcmp(argv[1],"-ad")==0)
     {
      option=1;
      alignertype=1;
     }
     else
      {
       if(strcmp(argv[1],"-l")==0)
        {
         option=1;
         alignertype=2;
        }
        else
         {
          option=0;
          alignertype=3;
         }
      }
    
    lwebpage=new WebPage();
    rwebpage=new WebPage();
    tagaligneroutput=NULL;
    if(lwebpage->Load(argv[option+1]) && rwebpage->Load(argv[option+3]))
     {
      showuse=false;
      if(argc==6+option)
       {//The output is a file
        if(!(fout=fopen(argv[option+5],"w")))
         {//There were errors opening the output file
          fprintf(stderr,"File \"%s\" could not be created\n",argv[option+3]); 
         }
         else
          {
           /*
           switch(alignertype)
            {
             case 1: //2 steps aligner with alignment distance
                     tagalignerA=new TagAligner2step_ad();
                     if(tagalignerA->Align(lwebpage->Content(),rwebpage->Content()))
                      {
                       tagaligneroutput=tagalignerA->GenerateTMX(argv[option+2],argv[option+4]);
                      }
                      else
                        tagaligneroutput=NULL;
                     delete tagalignerA;
                     break;
             case 2: //2 steps aligner with length distance
                     tagalignerB=new TagAligner2step_l();
                     if(tagalignerB->Align(lwebpage->Content(),rwebpage->Content()))
                      {
                       tagaligneroutput=tagalignerB->GenerateTMX(argv[option+2],argv[option+4]);
                      }
                      else
                        tagaligneroutput=NULL;
                     delete tagalignerB;
                     break;
             case 3: //2-1 aligner */
                     tagalignerC=new TagAligner2_1();
                     if(tagalignerC->Align(lwebpage->Content(),rwebpage->Content()))
                      {
                       tagaligneroutput=tagalignerC->GenerateTMX(argv[option+2],argv[option+4]);
                      }
                      else
                        tagaligneroutput=NULL;
                     delete tagalignerC;
//                     break;
//            }
           
           if(tagaligneroutput!=NULL)
            {
             fprintf(fout,"%s",tagaligneroutput);
             delete tagaligneroutput;
            }
           fclose(fout);
          }
       }
       else
        {//The output is the stdout
         
/*         switch(alignertype)
          {
           case 1: //2 steps aligner with alignment distance
                   tagalignerA=new TagAligner2step_ad();
                   if(tagalignerA->Align(lwebpage->Content(),rwebpage->Content()))
                    {
                     tagaligneroutput=tagalignerA->GenerateTMX(argv[option+2],argv[option+4]);
                    }
                    else
                      tagaligneroutput=NULL;
                   delete tagalignerA;
                   break;
           case 2: //2 steps aligner with length distance
                   tagalignerB=new TagAligner2step_l();
                   if(tagalignerB->Align(lwebpage->Content(),rwebpage->Content()))
                    {
                     tagaligneroutput=tagalignerB->GenerateTMX(argv[option+2],argv[option+4]);
                    }
                    else
                      tagaligneroutput=NULL;
                   delete tagalignerB;
                   break;
           case 3: //2-1 aligner */
                   tagalignerC=new TagAligner2_1();
                   if(tagalignerC->Align(lwebpage->Content(),rwebpage->Content()))
                    {
                     tagaligneroutput=tagalignerC->GenerateTMX(argv[option+2],argv[option+4]);
                    }
                    else
                      tagaligneroutput=NULL;
                   delete tagalignerC;
//                   break;
//          }
         if(tagaligneroutput!=NULL)
          {
           fprintf(stdout,"%s",tagaligneroutput);
           delete tagaligneroutput;
          }
        }
     }
     else
       showuse=true;
    delete lwebpage;
    delete rwebpage;
   }
   else
     showuse=true;
  
  if(showuse)
   {
    fprintf(stdout,"Use:\t%s <option> <left_file> <left_language> <right_file> <right_language>\n",argv[0]);
    fprintf(stdout,"\t%s <option> <left_file> <left_language> <right_file> <right_language> <output_file>\n",argv[0]);
    fprintf(stdout,"\n\tOptions:\n");
    fprintf(stdout,"\t\t-ad : 2 steps aligner with alignment distance text comparison\n");
    fprintf(stdout,"\t\t-l : 2 steps aligner with text length comparison\n");
    fprintf(stdout,"\t\t-direct : 2-1 aligner that aligns text sentences and tags at the same time (default)\n");
   }
  return(0);
 }
