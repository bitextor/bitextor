/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "webpage.h"

WebPage::WebPage()
 {
  unsigned int i;
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::WebPage()\n");
#endif
  
  localfile=NULL;
  domain=NULL;
  path=NULL;
  filename=NULL;
  fileext=NULL;
  fileparam=NULL;
  date=0;
  origdate=0;
  size=0;  
  language=-1;
  content=NULL;
  n_tags=0;
  for(i=0;i<kMaxLevelTags;i++)  
    n_leveltags[i]=0;  
  similars=NULL;
  langsimilars=NULL;
  distancesimilars=NULL;
  n_similars=0;
  applications=NULL;
  appversions=NULL;
  n_applications=0;
  n_links=0;
  links=NULL;
  n_outlinks=0;
  n_images=0;
  guessers=NULL;
  n_guessers=0;
  n_languagesperguesser=NULL;
  id_languagesperguesser=NULL;
  prob_languagesperguesser=NULL;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::WebPage()\n");
#endif   
 }

WebPage::WebPage(const char *localfilename)
 {
  unsigned int i;
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::WebPage(%s)\n",localfilename);
#endif   
  
  if(Load(localfilename))
   {
    localfile=new char[1+strlen(localfilename)];
    strcpy(localfile,localfilename);
   }
   else
    {//Actuamos como el constructor normal
     localfile=NULL;
     domain=NULL;
     path=NULL;
     filename=NULL;
     fileext=NULL;
     fileparam=NULL;
     date=0;
     origdate=0;
     size=0;
     language=0;
     content=NULL;
     n_tags=0;
     for(i=0;i<kMaxLevelTags;i++)  
       n_leveltags[i]=0;  
     similars=NULL;
     langsimilars=NULL;
     distancesimilars=NULL;
     n_similars=0;
     applications=NULL;
     appversions=NULL;
     n_applications=0;
     n_links=0;
     links=NULL;
     n_outlinks=0;
     n_images=0;
     guessers=NULL;
     n_guessers=0;
     n_languagesperguesser=NULL;
     id_languagesperguesser=NULL;
     prob_languagesperguesser=NULL;
    }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::WebPage(%s)\n",localfilename);
#endif    
 }

WebPage::~WebPage()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::~WebPage()\n");
#endif   
 
  Reset();

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::~WebPage()\n");
#endif   
  
 }
 
void WebPage::Reset()
 {
  unsigned int i;
  int j;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::~Reset()\n");
#endif 
  
  if(localfile!=NULL)
   {
    delete localfile;
    localfile=NULL;
   }
  if(domain!=NULL)
   {
    delete domain;
    domain=NULL;
   }
  if(path!=NULL)
   {
    delete path;
    path=NULL;
   }
  if(filename!=NULL)
   {
    delete filename;
    filename=NULL;
   }
  if(fileext!=NULL)
   {
    delete fileext;
    fileext=NULL;
   }
  if(fileparam!=NULL)
   {
    delete fileparam;
    fileparam=NULL;
   }
  if(content!=NULL)  
   { 
    delete content;
    content=NULL;
   }
  if(similars!=NULL)
   {
    delete similars;
    similars=NULL;
   }
  if(langsimilars!=NULL)
   {
    delete langsimilars;
    langsimilars=NULL;
   }
  if(distancesimilars!=NULL)
   {
    delete distancesimilars;
    distancesimilars=NULL;
   }
  if(applications!=NULL)
   {
    for(j=0;j<n_applications;j++)
     {
      delete applications[j];
     }
    delete applications;
    applications=NULL;
   }
  if(appversions!=NULL)
   {
    for(j=0;j<n_applications;j++)
     {
      delete appversions[j];
     }
    delete appversions;
    appversions=NULL;
   }
  if(links!=NULL)
   {
    for(i=0;i<n_links;i++)
     {
      if(links[i]!=NULL)
        delete links[i];
     }
    delete links;
    links=NULL;
   }
  date=0;
  origdate=0;
  size=0;
  language=-1;
  n_tags=0;
  for(i=0;i<kMaxLevelTags;i++)  
    n_leveltags[i]=0;
  n_similars=0;
  n_applications=0;
  n_links=0;
  n_outlinks=0;
  n_images=0;
    
  for(j=0;j<n_guessers;j++)
   {
    if(guessers[j]!=NULL)
      delete guessers[j];
   }
  delete guessers;
  guessers=NULL;
  for(j=0;j<n_guessers;j++)
   {
    if(id_languagesperguesser!=NULL && id_languagesperguesser[j]!=NULL)
      delete id_languagesperguesser[j];
    if(prob_languagesperguesser!=NULL && prob_languagesperguesser[j]!=NULL)
      delete prob_languagesperguesser[j];
   }
  if(id_languagesperguesser!=NULL)
   {
    delete id_languagesperguesser;
    id_languagesperguesser=NULL;
   }
  if(prob_languagesperguesser!=NULL)
   {
    delete prob_languagesperguesser;
    prob_languagesperguesser=NULL;
   }
  if(n_languagesperguesser!=NULL)
   {
    delete n_languagesperguesser;
    n_languagesperguesser=NULL;
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Reset()\n");
#endif       
 }

bool WebPage::Save(const char *file)
 {
  bool result;
  int i,j;
  FILE *fsal; //Fichero de entrada
  char *aux; //Auxiliar para las transformaciones de & en &amp;
  char strleveltags[kMaxLevelTags*10]; //Auxiliar para almacenar la cadena de nivel de etiquetas

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Save(%s)\n",file);
#endif
  
  if(domain!=NULL && file!=NULL)
   {
    if((fsal=fopen(file,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: output file \"%s\" could not be opened\n",file);
     }
     else
      {
       if(localfile!=NULL)
         delete localfile;
       localfile=new char[1+strlen(file)];
       strcpy(localfile,file);
       fprintf(fsal,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
       
/*       fprintf(fsal,"<!DOCTYPE PSBR SYSTEM \"file://~/robot/xmlresources/psbr.dtd\"[\n");
       fprintf(fsal,"  <!ENTITY %% ISOlat1 SYSTEM \"file://~/robot/xmlresources/isolat1.ent\">\n");
       fprintf(fsal,"%%ISOlat1;\n");
       fprintf(fsal,"  <!ENTITY %% ISOlat2 SYSTEM \"file://~/robot/xmlresources/isolat2.ent\">\n");
       fprintf(fsal,"%%ISOlat2;\n");
       fprintf(fsal,"  <!ENTITY %% ISOpub SYSTEM \"file://~/robot/xmlresources/isopub.ent\">\n");
       fprintf(fsal,"%%ISOpub;\n");
       fprintf(fsal,"  <!ENTITY %% ISOnum SYSTEM \"file://~/robot/xmlresources/isonum.ent\">\n");
       fprintf(fsal,"%%ISOnum; ]>\n");      
*/       
       fprintf(fsal,"<!DOCTYPE psbr SYSTEM \"file://psbr.dtd\">\n");       
       
       fprintf(fsal,"<psbr>\n");
       fprintf(fsal," <metainfo>\n");
       fprintf(fsal,"  <source>\n");

       aux=InsertAmpEntities(domain);
       if(aux!=NULL)
         fprintf(fsal,"   <location domain=\"%s\" path=\"",aux);
        else
          fprintf(fsal,"   <location domain=\"\" path=\"");
       delete aux;
       
       aux=InsertAmpEntities(path);
       if(aux!=NULL)
         fprintf(fsal,"%s\" file=\"",aux);
        else
          fprintf(fsal,"\" file=\"");
       delete aux;
       
       aux=InsertAmpEntities(filename);
       fprintf(fsal,"%s\" ext=\"",aux);
       delete aux;
       aux=InsertAmpEntities(fileext);
       fprintf(fsal,"%s\" param=\"",aux);
       delete aux;
       aux=InsertAmpLtGtEntities(fileparam);
       fprintf(fsal,"%s\"/>\n",aux);
       delete aux;
       
       fprintf(fsal,"   <date type=\"generation\" value=\"%d\"/>\n",origdate);
       fprintf(fsal,"  </source>\n");
       fprintf(fsal,"  <local>\n");      
       fprintf(fsal,"   <date type=\"retrieval\" value=\"%d\"/>\n",date);
       fprintf(fsal,"   <size value=\"%d\" text=\"%d\"/>\n",size,textsize);
       fprintf(fsal,"   <tagnumber value=\"%d\"/>\n",n_tags);
       
       sprintf(strleveltags,"%d",n_leveltags[0]);
       for(i=1;i<kMaxLevelTags-1;i++)
         sprintf(strleveltags,"%s_%d",strleveltags,n_leveltags[i]);
       fprintf(fsal,"   <taglevelnumber value=\"%s\"/>\n",strleveltags);
       
       fprintf(fsal,"   <linksnumber value=\"%d\"/>\n",n_links);
       fprintf(fsal,"   <imagesnumber value=\"%d\"/>\n",n_images);
       
       fprintf(fsal,"  </local>\n");
      
       fprintf(fsal,"  <language code=\"%s\">\n",LanguageName(language));
       for(i=0;i<n_guessers;i++)
        {
         fprintf(fsal,"    <guesser name=\"%s\">\n",guessers[i]);
         for(j=0;j<n_languagesperguesser[i];j++)
           fprintf(fsal,"      <lang code=\"%s\" probability=\"%f\"/>\n",LanguageName(id_languagesperguesser[i][j]),prob_languagesperguesser[i][j]);
         fprintf(fsal,"    </guesser>\n");
        }
       fprintf(fsal,"  </language>\n");
       
       fprintf(fsal,"  <process>\n");
       for(i=0;i<n_applications;i++)
         fprintf(fsal,"   <application name=\"%s\" version=\"%s\"/>\n",applications[i],appversions[i]);
       fprintf(fsal,"  </process>\n");
      
       fprintf(fsal,"  <similar>\n");
       for(i=0;i<n_similars;i++)
         fprintf(fsal,"   <id langcode=\"%s\" value=\"%d\" distance=\"%f\"/>\n",LanguageName(langsimilars[i]),similars[i],distancesimilars[i]);
       fprintf(fsal,"  </similar>\n");
      
       fprintf(fsal," </metainfo>\n");
       fprintf(fsal," <content>\n");
       fprintf(fsal,"%s",content);
       fprintf(fsal,"\n</content>\n");
       fprintf(fsal,"</psbr>\n");
       fclose(fsal);
       result=true;
      }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: There is no file loaded to save\n");
    }
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Save(%s) -> %d\n",file,result);
#endif    
  return(result);
 }

bool WebPage::Refresh()
 {
  bool result;
  char *auxfile;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Refresh()\n");
#endif
    
  if(localfile!=NULL)
   {
    result=false;
    if(!LoadContent(localfile))
     {
      fprintf(stderr,"Error: File \"%s\" could not be loaded\n",localfile);
      result=false;
     }
     else
      {
       auxfile=new char[1+strlen(localfile)];
       strcpy(auxfile,localfile);
       result=Save(auxfile);
       delete auxfile;
       auxfile=NULL;
      }
   }
   else
     result=false;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Refresh() -> %d\n",result);
#endif
       
  return(result);
 }

bool WebPage::Load(const char* file)
 {
  bool result;
  
  if(LoadHeader(file))
   {
    if(LoadContent(file))
     {
      result=true;
     }
     else
       result=false;
   }
   else
     result=false;
  return(result);
 }

bool WebPage::LoadHeader(const char* file)
 {
  bool result;
  bool finished;
  int ret;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::LoadHeader(%s)\n",file);
#endif
  
  reader=xmlReaderForFile(file,NULL,0);
  if(reader==NULL)
   {
    fprintf(stderr,"Error: Cannot open \"%s\".\n",file);
    result=false;
   }
   else
    {
     result=true;
     finished=false;
     ret=xmlTextReaderRead(reader);
     while(ret==1 && result && !finished)
      {
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(tag!=NULL)
        {
         if(strcmp(tag,"PSBR")==0) 
          {//Es el DOCTYPE
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"psbr")==0)
             {
              delete tag;
              tag=NULL;
              result=ProcessPSBR();
              finished=true;
             }
             else
              {
               if(strcmp(tag,"#text")==0)
                {
                 delete tag;
                 tag=NULL;
                }
                else
                 {
                  delete tag;
                  tag=NULL;
                  result=false;
                  fprintf(stderr,"File \"%s\" does not contain <psbr> tag.\n",file);
                 }
              }
           }
        }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }

     if(ret!=1)
      {
       fprintf(stderr,"Error: Parse error at the end of input.\n");
       result=false;
      }
     xmlFreeTextReader(reader);
     //xmlCleanupParser();
    }
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::LoadHeader(%s) -> %d\n",file,result);
#endif
  if(result)
   {
    localfile=new char[1+strlen(file)];
    strcpy(localfile,file);
    
   } 
  return(result);
 } 
 
bool WebPage::ProcessPSBR()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessPSBR()\n");
#endif
  
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"metainfo")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessMETAINFO();
           }
           else
            {
             if(strcmp(tag,"content")==0)
              {
               delete tag;
               tag=NULL;
               finished=true;//El contenido del content se ignora y tan solo se marca el final
              }
              else
               {
                if(strcmp(tag,"psbr")==0)
                 {//Ignoramos el doctype
                  delete tag;
                  tag=NULL;
                 }
                 else
                  {
                   delete tag;
                   tag=NULL;
                   result=false;
                  }
               }
            }
         }
       if(result && !finished)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessPSBR() -> %d\n",result);
#endif
     
  return(result);
 }

bool WebPage::ProcessMETAINFO()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessMETAINFO()\n");
#endif
    
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"source")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessSOURCE();
           }
           else
            {
             if(strcmp(tag,"local")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessLOCAL();
              }
              else
               {
                if(strcmp(tag,"language")==0)
                 {
                  delete tag;
                  tag=NULL;
                  result=ProcessLANGUAGE();
                 }
                 else
                  {
                   if(strcmp(tag,"process")==0)
                    {
                     delete tag;
                     tag=NULL;
                     result=ProcessPROCESS();
                    }
                    else 
                     {                
                      if(strcmp(tag,"similar")==0)
                       {
                        delete tag;
                        tag=NULL;
                        result=ProcessSIMILAR();
                       }
                       else 
                        {
                         delete tag;
                         tag=NULL;
                         result=false;
                        }
                     }
                  }
               }
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessMETAINFO() -> %d\n",result);
#endif
     
  return(result);
 } 
  
bool WebPage::ProcessSOURCE()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessSOURCE()\n");
#endif
    
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"location")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessLOCATION();
           }
           else
            {
             if(strcmp(tag,"date")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessDATE();
              }
              else
               {
                delete tag;
                tag=NULL;
                result=false;
               }
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessSOURCE()\n");
#endif
  
  return(result);
 } 

bool WebPage::ProcessLOCAL()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessLOCAL()\n");
#endif
    
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"date")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessDATE();
           }
           else
            {
             if(strcmp(tag,"size")==0)
              {
               delete tag;
               tag=NULL;
               result=ProcessSIZE();
              }
              else
               {
                if(strcmp(tag,"tagnumber")==0)
                 {
                  delete tag;
                  tag=NULL;
                  result=ProcessTAGNUMBER();
                 }
                 else
                  {
                   if(strcmp(tag,"taglevelnumber")==0)
                    {
                     delete tag;
                     tag=NULL;
                     result=ProcessTAGLEVELNUMBER();
                    }
                    else
                     {
                      if(strcmp(tag,"linksnumber")==0)
                       {
                        delete tag;
                        tag=NULL;
                        result=ProcessLINKSNUMBER();
                       }
                       else
                        {
                         if(strcmp(tag,"imagesnumber")==0)
                          {
                           delete tag;
                           tag=NULL;
                           result=ProcessIMAGESNUMBER();
                          }
                          else
                           {
                            delete tag;
                            tag=NULL;
                            result=false;
                           }
                        }
                     }
                  }
               }
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessLOCAL() -> %d\n",result);
#endif
     
  return(result);
 } 

bool WebPage::ProcessLANGUAGE()
 {
  bool result;
  bool finished;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessLANGUAGE()\n");
#endif
    
  result=true;
  //Leemos el atributo code con el idioma de la pagina
  nameattrib=xmlCharStrdup("code");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
  
  if(attrib!=NULL)
   {  
    language=LanguageCode(attrib);
    delete attrib;
    attrib=NULL;
    if(language==-1)
      result=false;
   }
   else
     result=false;
 
  if(result)
   {
    finished=false;
    ret=xmlTextReaderRead(reader);
    while(result && ret==1 && !finished)
     {
      type=xmlTextReaderNodeType(reader);
      if(type==XML_READER_TYPE_END_ELEMENT)
        finished=true;
       else
        {    
         xtag=xmlTextReaderConstName(reader);
         tag=XMLToLatin1(xtag);
         if(strcmp(tag,"#text")==0)
          {
           delete tag;
           tag=NULL;
          }
          else
           {
            if(strcmp(tag,"guesser")==0)
             {
              delete tag;
              tag=NULL;
              result=ProcessGUESSER();
             }
             else
              {
               delete tag;
               tag=NULL;
               result=false;
              }
           }
         if(result)
           ret=xmlTextReaderRead(reader);
        }
     }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessLANGUAGE() -> %d\n",result);
#endif
     
  return(result);
 }

bool WebPage::ProcessPROCESS()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessPROCESS()\n");
#endif
    
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"application")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessAPPLICATION();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }
   
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessPROCESS() -> %d\n",result);
#endif
   
  return(result);
 }

bool WebPage::ProcessSIMILAR()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessSIMILAR()\n");
#endif
    
  result=true;
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
     {
      finished=true;
     }
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"id")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessID();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessSIMILAR() -> %d\n",result);
#endif
     
  return(result);
 }
 
bool WebPage::ProcessLOCATION()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *aux; //Auxiliar para la conversion de entitades &amp; en &
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessLOCATION()\n");
#endif
    
  nameattrib=xmlCharStrdup("domain");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  aux=XMLToLatin1(xattrib);
  domain=RemoveAmpEntities(aux);
  delete aux;
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("path");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  aux=XMLToLatin1(xattrib);
  path=RemoveAmpEntities(aux);
  delete aux;
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("file");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  aux=XMLToLatin1(xattrib);
  filename=RemoveAmpEntities(aux);
  delete aux;
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("ext");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  aux=XMLToLatin1(xattrib);
  fileext=RemoveAmpEntities(aux);
  delete aux;
  xmlFree(xattrib);
  
  nameattrib=xmlCharStrdup("param");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  aux=XMLToLatin1(xattrib);
  fileparam=RemoveAmpLtGtEntities(aux);
  delete aux;
  xmlFree(xattrib);
  
  if(domain==NULL || path==NULL  ||  filename==NULL || fileext==NULL || fileparam==NULL)
    result=false;
   else
     result=true;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessLOCATION() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool WebPage::ProcessDATE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  bool typedate; //Indica si es de generacion o de bajada de la pagina
  int datevalue;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessDATE()\n");
#endif
  
  nameattrib=xmlCharStrdup("type");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);
  
  typedate=0;
  if(attrib!=NULL)
   {
    result=true;
    if(strcmp(attrib,"generation")==0)
      typedate=0;
     else
      {
       if(strcmp(attrib,"retrieval")==0)
         typedate=1;
        else
         {
          fprintf(stderr,"Error: wrong date type while loading XML file\n");
          result=false;
         }
      }
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
  if(result)
   {
    nameattrib=xmlCharStrdup("value");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=XMLToLatin1(xattrib);
    xmlFree(xattrib);    
    
    if(attrib!=NULL)
     {
      //Realizamos la funcion atoi pero detectando errores
      for(i=0,datevalue=0;i<(signed)strlen(attrib);i++)
       {
        if(!isdigit(attrib[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (date value)\n");
         }
         else
           datevalue=datevalue*10+(attrib[i]-48);
       }
    
      if(typedate==0)
        origdate=datevalue;
       else
         date=datevalue;
      delete attrib;
      attrib=NULL;
     }
     else
       result=false;
   }
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessDATE() -> %d\n",result);
#endif

  return(result);
 }
 
bool WebPage::ProcessSIZE()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int i;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessSIZE()\n");
#endif
  
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    //Realizamos la funcion atoi pero detectando errores
    for(i=0,size=0;i<(signed)strlen(attrib);i++)
     {
      if(!isdigit(attrib[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file (size)\n");
       }
       else
         size=size*10+(attrib[i]-48);
     }    
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
  if(result)
   {
    nameattrib=xmlCharStrdup("text");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=XMLToLatin1(xattrib);
    xmlFree(xattrib);    
    
    if(attrib!=NULL)
     {
      //Realizamos la funcion atoi pero detectando errores
      for(i=0,textsize=0;i<(signed)strlen(attrib);i++)
       {
        if(!isdigit(attrib[i]))
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (date value)\n");
         }
         else
           textsize=textsize*10+(attrib[i]-48);
       }    
      delete attrib;
      attrib=NULL;
     }
     else
       result=false;
   }
     
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessSIZE() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool WebPage::ProcessTAGNUMBER()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessTAGNUMBER()\n");
#endif
  
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    //Realizamos la funcion atoi pero detectando errores
    for(i=0,n_tags=0;i<(signed)strlen(attrib);i++)
     {
      if(!isdigit(attrib[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file (tagnumber)\n");
       }
       else
         n_tags=n_tags*10+(attrib[i]-48);
     }    
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessTAGNUMBER() -> %d\n",result);
#endif
  
  return(result);
 }

bool WebPage::ProcessTAGLEVELNUMBER()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int i;
  unsigned int level;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessTAGLEVELNUMBER()\n");
#endif
    
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    
    for(i=0;i<kMaxLevelTags;i++)  
       n_leveltags[i]=0;  
     
    //Realizamos la funcion atoi pero detectando errores
    for(i=0,level=0;i<(signed)strlen(attrib) && level<kMaxLevelTags;i++)
     {
      if(!isdigit(attrib[i]))
       {
        if(attrib[i]=='_')
         {
          level++;
          n_leveltags[level]=0;
         }
         else
          {
           if(attrib[i]=='-')
            {//Este caso no deberia darse (hay un recuento negativo)
             //Se ignora este menos
            }
            else
             {
              result=false;
              fprintf(stderr,"Error: Invalid format while loading file (tagtreenumber)\n");
             }
          }
       }
       else
         n_leveltags[level]=n_leveltags[level]*10+(attrib[i]-48);
    
     }    
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessTAGLEVELNUMBER() -> %d\n",result);
#endif
  
  return(result);
 }

bool WebPage::ProcessLINKSNUMBER()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessLINKSNUMBER()\n");
#endif
  
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    //Realizamos la funcion atoi pero detectando errores
    for(i=0,n_links=0;i<(signed)strlen(attrib);i++)
     {
      if(!isdigit(attrib[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file (linksnumber)\n");
       }
       else
         n_links=n_links*10+(attrib[i]-48);
     }    
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessLINKSNUMBER() -> %d\n",result);
#endif
  
  return(result);
 }

 bool WebPage::ProcessIMAGESNUMBER()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessIMAGESNUMBER()\n");
#endif
  
  nameattrib=xmlCharStrdup("value");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    //Realizamos la funcion atoi pero detectando errores
    for(i=0,n_images=0;i<(signed)strlen(attrib);i++)
     {
      if(!isdigit(attrib[i]))
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file (imagesnumber)\n");
       }
       else
         n_images=n_images*10+(attrib[i]-48);
     }    
    delete attrib;
    attrib=NULL;
   }
   else
     result=false;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessIMAGESNUMBER() -> %d\n",result);
#endif
  
  return(result);
 }


bool WebPage::ProcessGUESSER()
 {
  bool result;
  bool finished;
  xmlChar const *xtag; //Tag de un nodo tal como lo muestra la libreria
  char *tag; //Tag una vez convertido
  int ret,type;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  char **aux; //Variable para hacer crecer el vector guessers
  int *aux2; //Variable para hacer crecer el vector n_languagesperguesser
  int **aux3; //Variable para hacer crecer el vector id_languagesperguesser
  float **aux4; //Variable para hacer crecer el vector prob_languagesperguesser
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessGUESSER()\n");
#endif
    
  nameattrib=xmlCharStrdup("name");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);  
  if(attrib!=NULL)
   {
    result=true;
    aux=new char*[n_guessers+1];
    aux2=new int[n_guessers+1];
    aux3=new int*[n_guessers+1];
    aux4=new float*[n_guessers+1];
    for(i=0;i<n_guessers;i++)
     {
      aux[i]=guessers[i];
      aux2[i]=n_languagesperguesser[i];
      aux3[i]=id_languagesperguesser[i];
      aux4[i]=prob_languagesperguesser[i];
     }
    aux[i]=attrib;
    aux2[i]=0;
    aux3[i]=NULL;
    aux4[i]=NULL;
    delete guessers;
    guessers=aux;
    delete n_languagesperguesser;
    n_languagesperguesser=aux2;
    delete id_languagesperguesser;
    id_languagesperguesser=aux3;
    delete prob_languagesperguesser;
    prob_languagesperguesser=aux4;
    n_guessers++;        
   }
   else
     result=false;
  
  finished=false;
  ret=xmlTextReaderRead(reader);
  while(result && ret==1 && !finished)
   {
    type=xmlTextReaderNodeType(reader);
    if(type==XML_READER_TYPE_END_ELEMENT)
      finished=true;
     else
      {    
       xtag=xmlTextReaderConstName(reader);
       tag=XMLToLatin1(xtag);
       if(strcmp(tag,"#text")==0)
        {
         delete tag;
         tag=NULL;
        }
        else
         {
          if(strcmp(tag,"lang")==0)
           {
            delete tag;
            tag=NULL;
            result=ProcessLANG();
           }
           else
            {
             delete tag;
             tag=NULL;
             result=false;
            }
         }
       if(result)
         ret=xmlTextReaderRead(reader);
      }
   }
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessGUESSER() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool WebPage::ProcessLANG()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int langcode;
  float langprobability;
  int dotplace; //Lugar del punto al leer un numero en punto flotante
  int *aux;
  float *aux2;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessLANG()\n");
#endif
    
  nameattrib=xmlCharStrdup("code");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    langcode=LanguageCode(attrib);
    delete attrib;
    attrib=NULL;
    if(langcode==-1)
      result=false;
    
    if(result)
     {
      nameattrib=xmlCharStrdup("probability");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=XMLToLatin1(xattrib);
      xmlFree(xattrib);
    
      if(attrib!=NULL)
       {
        for(i=0,langprobability=0,dotplace=1000;result && i<(signed)strlen(attrib);i++)
         {                           
          if(!isdigit(attrib[i]) && attrib[i]!='.')
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (lang)\n");
           }
           else
            {
             if(attrib[i]!='.')
               langprobability=langprobability*10+(attrib[i]-48);                                             
              else
                dotplace=i;
            }                                           
         }
        if(result)
         {
          for(i--;i>dotplace;i--)
            langprobability=langprobability/10;
          
          aux=new int[n_languagesperguesser[n_guessers-1]+1];
          aux2=new float[n_languagesperguesser[n_guessers-1]+1];
          for(i=0;i<n_languagesperguesser[n_guessers-1];i++)
           {
            aux[i]=id_languagesperguesser[n_guessers-1][i];
            aux2[i]=prob_languagesperguesser[n_guessers-1][i];
           }
          aux[i]=langcode;
          aux2[i]=langprobability;          
          delete id_languagesperguesser[n_guessers-1];
          id_languagesperguesser[n_guessers-1]=aux;
          delete prob_languagesperguesser[n_guessers-1];
          prob_languagesperguesser[n_guessers-1]=aux2;
          (n_languagesperguesser[n_guessers-1])++;
         }
        delete attrib;
        attrib=NULL;
       }
       else
         result=false;
     }
     else
       result=false;
   }
   else
     result=false;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessLANG() -> %d\n",result);
#endif
       
  return(result);
 }

bool WebPage::ProcessAPPLICATION()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  char *appname;
  char **aux,**aux2;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessAPPLICATION()\n");
#endif
    
  nameattrib=xmlCharStrdup("name");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  appname=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(appname!=NULL)
   {
    result=true;
    nameattrib=xmlCharStrdup("version");
    xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
    xmlFree(nameattrib);
    attrib=XMLToLatin1(xattrib);
    xmlFree(xattrib);  
    
    if(attrib!=NULL)
     {
      aux=new char*[n_applications+1];
      aux2=new char*[n_applications+1];
      for(i=0;i<n_applications;i++)
       {
        aux[i]=applications[i];
        aux2[i]=appversions[i];
       }
      aux[i]=appname;
      aux2[i]=attrib;
      delete applications;
      applications=aux;
      delete appversions;
      appversions=aux2;
      n_applications++;
     }
     else
      {
       delete appname;
       appname=NULL;
       result=false;
      }
    
   }
   else
     result=false;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessAPPLICATION() -> %d\n",result);
#endif
  
  return(result);
 }
 
bool WebPage::ProcessID()
 {
  bool result;
  xmlChar *nameattrib;
  xmlChar *xattrib; //Nombre del atributo tal como lo muestra la libreria
  char *attrib; //Valor del atributo una vez convertido
  int *aux;
  int value;
  short *aux2;
  float *aux3;
  short langcode;
  float distance;
  int dotplace; //Lugar del punto al leer un numero en punto flotante
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessID()\n");
#endif
    
  nameattrib=xmlCharStrdup("langcode");
  xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
  xmlFree(nameattrib);
  attrib=XMLToLatin1(xattrib);
  xmlFree(xattrib);    
    
  if(attrib!=NULL)
   {
    result=true;
    langcode=LanguageCode(attrib);
    delete attrib;
    attrib=NULL;
    if(langcode==-1)
      result=false;
   
    if(result)
     {   
      nameattrib=xmlCharStrdup("value");
      xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
      xmlFree(nameattrib);
      attrib=XMLToLatin1(xattrib);
      xmlFree(xattrib);  
    
      if(attrib!=NULL)
       {
        //Realizamos la funcion atoi pero detectando errores
        for(i=0,value=0;i<(signed)strlen(attrib);i++)
         {
          if(!isdigit(attrib[i]))
           {
            result=false;
            fprintf(stderr,"Error: Invalid format while loading file (id lang)\n");
           }
           else
              value=value*10+(attrib[i]-48);    
         }    
        delete attrib;
        attrib=NULL;
        
        nameattrib=xmlCharStrdup("distance");
        xattrib=xmlTextReaderGetAttribute(reader,nameattrib);
        xmlFree(nameattrib);
        attrib=XMLToLatin1(xattrib);
        xmlFree(xattrib);    
    
        distance=0;
        if(attrib!=NULL)
         {
          for(i=0,dotplace=1000;result && i<(signed)strlen(attrib);i++)
           {                           
            if(!isdigit(attrib[i]) && attrib[i]!='.')
             {
              result=false;
              fprintf(stderr,"Error: Invalid format while loading file (id)\n");
             }
             else
              {
               if(attrib[i]!='.')
                 distance=distance*10+(attrib[i]-48);                                             
                else
                  dotplace=i;
              }                                           
           }
          if(result)
           {
            for(i--;i>dotplace;i--)
              distance=distance/10;
           }
         }
        delete attrib;
        attrib=NULL;
        
        if(result)
         {
          aux=new int[n_similars+1];
          aux2=new short[n_similars+1];
          aux3=new float[n_similars+1];
          for(i=0;i<n_similars;i++)
           {
            aux[i]=similars[i];
            aux2[i]=langsimilars[i];
            aux3[i]=distancesimilars[i];
           }
          aux[i]=value;
          aux2[i]=langcode;
          aux3[i]=distance;
          delete similars;
          similars=aux;
          delete langsimilars;
          langsimilars=aux2;
          delete distancesimilars;
          distancesimilars=aux3;
          n_similars++;
         }         
       }
       else
         result=false;
     }    
   }
   else
     result=false;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessID() -> %d\n",result);
#endif
       
  return(result);
 }

bool WebPage::LoadContent(const char *file)
 {
  bool result;
  int i;
  FILE *fent;
  bool found;
  char buffer[kSizeFileBuffer+1]; //Tamaño del buffer de lectura del fichero para el content
  char *bufferaux; //Tamaño del buffer auxiliar para ir incrementando el tamaño del content
  int bytesleidos; //Numero de bytes leidos en el buffer

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::LoadContent(%s)\n",file);
#endif
    
  result=true;
  if(!(fent=fopen(file,"r")))
    result=false;
   else
    {
     bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);
     buffer[bytesleidos]='\0';
     for(i=8,found=false;i<(signed)strlen(buffer) && !found;i++)
      {
       if(buffer[i-8]=='<' && buffer[i-7]=='c' && buffer[i-6]=='o' &&
          buffer[i-5]=='n' && buffer[i-4]=='t' && buffer[i-3]=='e' &&
          buffer[i-2]=='n' && buffer[i-1]=='t' && buffer[i]=='>')
        {
         found=true;
         i++;//Descartamos tambien el fin de linea tras el <content>
        }
      }
     
     if(content!=NULL)
       delete content;
     content=new char[strlen(buffer)-i+1];
     strcpy(content,buffer+i);
                        
     //Ahora leemos el resto de caracteres hasta el final del fichero
     while(!feof(fent))
      {
       bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);
       if(bytesleidos==kSizeFileBuffer)
        {
         buffer[kSizeFileBuffer]='\0';                               
         bufferaux=new char[kSizeFileBuffer+1+strlen(content)];
         strcpy(bufferaux,content);
         strcat(bufferaux,buffer);
         delete content;
         content=new char[1+strlen(bufferaux)];
         strcpy(content,bufferaux);
         delete bufferaux;
         bufferaux=NULL;
        }
        else
         {//Ya hemos leido todo el fichero
          buffer[bytesleidos]='\0';
          bufferaux=new char[bytesleidos+1+strlen(content)];
          strcpy(bufferaux,content);
          strcat(bufferaux,buffer);
          delete content;
          content=new char[1+strlen(bufferaux)];
          strcpy(content,bufferaux);
          delete bufferaux;
          bufferaux=NULL;
         }
      }
     //Ahora quitamos del content las etiquetas del final del content y del psbr.
                           
     //Nos quedamos con la posicion de cierre de la ultima etiqueta
     for(i=strlen(content)-1;content[i]!='>';i--);
     if(content[i-6]=='<' && content[i-5]=='/' &&
        content[i-4]=='p' && content[i-3]=='s' &&
        content[i-2]=='b' && content[i-1]=='r')
      {
       //Buscamos la posicion de cierre de la etiqueta anterior
       for(i=i-7;content[i]!='>';i--);
       if(content[i-9]=='<' && content[i-8]=='/' &&
          content[i-7]=='c' && content[i-6]=='o' &&
          content[i-5]=='n' && content[i-4]=='t' &&
          content[i-3]=='e' && content[i-2]=='n' && 
          content[i-1]=='t')
        {//Aceptamos el fichero y ya lo tenemos almacenado
         bufferaux=new char[i-10+1];
         content[i-10]='\0';
         strcpy(bufferaux,content);
         delete content;
         content=bufferaux;
        }
        else
         {
          result=false;
          fprintf(stderr,"Error: Invalid format while loading file (content)\n");
         }
      }
      else
       {
        result=false;
        fprintf(stderr,"Error: Invalid format while loading file at end of file (psbr)\n");
       }
     fclose(fent);
    }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::LoadContent(%s) -> %d\n",file,result);
#endif
        
  return(result); 
 }

bool WebPage::ProcessPage()
 {
  bool result;
  int i;
  unsigned int j,k;
  int level;//Nivel de la etiqueta actual dentro del arbol de etiquetas
  int len; //Longitud de la cadena
  char buffer[kSizeAuxStrings*10];
  bool filled; //Indica si se ha rellenado al menos parcialmente el buffer
  bool notfound; //Indica si se descarta el encontrar una determinada secuencia
  bool found; //Indica si se ha encontrado la etiqueta que se buscaba
  char buffer2[kSizeAuxStrings]; //Buffer para links
  bool redirecting,refreshing;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ProcessPage()\n");
#endif
  result=true;
  if(localfile==NULL)
    result=false;
   else
    {
     n_tags=0;
     n_links=0;
     n_outlinks=0;
     n_images=0;
     textsize=0;
     level=0;

     for(i=0,len=strlen(content);i<len && level>=0;i++)
      {
       if(content[i]=='<')
        {
         if(level<kMaxLevelTags)
           n_leveltags[level]++;
         level++;
         n_tags++;
          
         //Estamos dentro de una etiqueta
         for(j=0,i++,filled=false;i<len && ((content[i]!=' ' && content[i]!='\n' && content[i]!='\t') ||  !filled) && content[i]!='>';i++,j++)
          {
           if(content[i]!=' ')
            {
             buffer[j]=content[i];
             filled=true;
            }
            else
              j--;
          }
         buffer[j]='\0';
         //Una vez almacenado el comienzo de la etiqueta (o toda completa)
         //vemos a ver que contiene
         switch(buffer[0])
          {
           case '/': //Es un cierre de etiqueta
                     n_tags--;
                     level--;
                     if(level<kMaxLevelTags)
                       n_leveltags[level]--;
                     level--;
                     break;

           case '%': //Es un codigo de servidor que deberia ignorarse
                     n_tags--;
                     level--;
                     if(level<kMaxLevelTags)
                       n_leveltags[level]--;
                     for(;i<len && !(content[i-1]=='%' && content[i]=='>');i++);
                     break;
                     
           case '!': //Es un comentario o etiqueta meta
                     n_tags--; 
                     level--;
                     if(level<kMaxLevelTags)
                       n_leveltags[level]--;
                     found=false;
                     if(buffer[1]=='-')
                      {
                       if(buffer[2]=='-')
                        {//Es un comentario asi que se elimina su contenido
                         found=true;
                         for(;i<len && !(content[i-2]=='-' && content[i-1]=='-' &&
                                           content[i]=='>');i++);
                        }
                      }
                     if(!found)
                      {
                       for(;i<len && content[i]!='>';i++);
                       if(content[i-1]=='/')
                         level--;
                      }
                     break;
          
           case 'a': if(buffer[1]=='\0')
                      {//Es una etiqueta a por lo que buscamos el href
                       notfound=false;
                       while(!notfound  && i<len)
                        {
                         //Quitamos los espacios antes de leer el atributo
                         for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                       
                         //Cogemos el atributo
                         for(j=0;i<len && content[i]!=' ' && content[i]!='\n' && content[i]!='\t' && content[i]!='>' && content[i]!='=';i++,j++)
                           buffer[j]=content[i];
                         buffer[j]='\0';
                       
                         //Quitamos los espacios tras el atributo
                         for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                         
                         if(content[i]=='=')
                          {//El atributo tiene valor
                           if(buffer[0]=='h' && buffer[1]=='r' &&
                              buffer[2]=='e' && buffer[3]=='f')
                            {//Cogemos el atributo
                             //Avanzamos hasta la comilla
                             for(;i<len && content[i]!='\"' && content[i]!='\'';i++);

                             for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                               buffer[j]=content[i];
                             buffer[j]='\0';
                             InsertLink(buffer);
                             if(ExternalLink(buffer))
                               n_outlinks++;
                            }
                            else
                             {//Este atributo no nos interesa
                              for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                              for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                              i++;
                             }
                          }
                          else
                           {
                            if(content[i]=='>')
                             {
                              notfound=true;
                              if(content[i-1]=='/')
                                level--;
                             }
                             else
                               i++;
                           }
                        };
                      }
                      else
                       {
                        for(;i<len && content[i]!='>';i++);
                        if(content[i-1]=='/')
                          level--;
                       }
                     break; 
           
           case 'f': found=false;
                     if(buffer[1]=='r')
                      {
                       if(buffer[2]=='a')
                        {
                         if(buffer[3]=='m')
                          {
                           if(buffer[4]=='e')
                            {
                             if(buffer[5]=='\0')
                              {//Es una etiqueta frame por lo que buscamos el src
                               found=true;
                               notfound=false;
                               while(!notfound  && i<len)
                                {
                                 //Quitamos los espacios antes de leer el atributo
                                 for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                       
                                 //Cogemos el atributo
                                 for(j=0;i<len && content[i]!=' ' && content[i]!='\n' && content[i]!='\t' && content[i]!='>' && content[i]!='=';i++,j++)
                                   buffer[j]=content[i];
                                 buffer[j]='\0';
                       
                                 //Quitamos los espacios tras el atributo
                                 for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                         
                                 if(content[i]=='=')
                                  {//El atributo tiene valor
                                   if(buffer[0]=='s' && buffer[1]=='r' &&
                                      buffer[2]=='c')
                                    {//Cogemos el atributo
                                     //Avanzamos hasta la comilla
                                     for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                      
                                     for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                                       buffer[j]=content[i];
                                     buffer[j]='\0';
                                     InsertLink(buffer);
                                     if(ExternalLink(buffer))
                                       n_outlinks++;
                                    }
                                    else
                                     {//Este atributo no nos interesa
                                      for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                      for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                      i++;
                                     }
                                  }
                                  else
                                   {
                                    if(content[i]=='>')
                                     {
                                      notfound=true;
                                      if(content[i-1]=='/')
                                        level--;
                                     }
                                     else
                                       i++;
                                   }
                                };
                              
                              }
                            }
                          }
                        }
                      }
                     if(!found)
                      {
                       for(;i<len && content[i]!='>';i++);
                       if(content[i-1]=='/')
                         level--;
                      }
                     break;
           
           case 'i': found=false;
                     if(buffer[1]=='m')
                      {
                       if(buffer[2]=='g')
                        {
                         if(buffer[3]=='\0')
                          {//Es una etiqueta img por lo que buscamos el src
                           found=true;
                           
                           notfound=false;                           
                           while(!notfound && i<len)
                            {
                             //Quitamos los espacios antes de leer el atributo
                             for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                       
                             //Cogemos el atributo
                             for(j=0;i<len && content[i]!=' ' && content[i]!='\n' && content[i]!='\t' && content[i]!='>' && content[i]!='=';i++,j++)
                               buffer[j]=content[i];
                             buffer[j]='\0';
                       
                             //Quitamos los espacios tras el atributo
                             for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                         
                             if(content[i]=='=')
                              {//El atributo tiene valor
                               if(buffer[0]=='s' && buffer[1]=='r' &&
                                  buffer[2]=='c')
                                {
                                 n_images++;
                                 //Cogemos el atributo
                                 //Avanzamos hasta la comilla
                                 for(;i<len && content[i]!='\"' && content[i]!='\'';i++);

                                 for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                                   buffer[j]=content[i];
                                 buffer[j]='\0';    
                                }
                                else
                                 {//Este atributo no nos interesa
                                  for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                  for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                 }
                              }
                              else
                               {
                                if(content[i]=='>')
                                 {
                                  notfound=true;
                                  if(content[i-1]=='/')
                                    level--;
                                 }
                                 else
                                   i++;
                               }
                            }
                          }
                        }
                      }
                      else
                       {
                        if(buffer[1]=='f')
                         {
                          if(buffer[2]=='r')
                           {
                            if(buffer[3]=='a')
                             {
                              if(buffer[4]=='m')
                               {
                                if(buffer[5]=='e')
                                 {
                                  if(buffer[6]=='\0')
                                   {//Es una etiqueta iframe por lo que buscamos el atributo src
                                    found=true;
                                    notfound=false;
                                    while(!notfound  && i<len)
                                     {
                                      //Quitamos los espacios antes de leer el atributo
                                      for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                       
                                      //Cogemos el atributo
                                      for(j=0;i<len && content[i]!=' ' && content[i]!='\n' && content[i]!='\t' && content[i]!='>' && content[i]!='=';i++,j++)
                                        buffer[j]=content[i];
                                      buffer[j]='\0';
                       
                                      //Quitamos los espacios tras el atributo
                                      for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                         
                                      if(content[i]=='=')
                                       {//El atributo tiene valor
                                        if(buffer[0]=='s' && buffer[1]=='r' &&
                                           buffer[2]=='c')
                                         {//Cogemos el atributo
                                          //Avanzamos hasta la comilla
                                          for(;i<len && content[i]!='\"' && content[i]!='\'';i++);

                                          for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                                            buffer[j]=content[i];
                                          buffer[j]='\0';
                                          InsertLink(buffer);
                                          if(ExternalLink(buffer))
                                            n_outlinks++;
                                         }
                                         else
                                          {//Este atributo no nos interesa
                                           for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                           for(i++;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                           i++;
                                          }
                                       }
                                       else
                                        {
                                         if(content[i]=='>')
                                          {
                                           notfound=true;
                                           if(content[i-1]=='/')
                                             level--;
                                          }
                                          else
                                            i++;
                                        }
                                     };
                                   }
                                 }
                               }
                             }
                           }
                         }
                       }
                     if(!found)
                      {
                       for(;i<len && content[i]!='>';i++);
                       if(content[i-1]=='/')
                         level--;
                      }
                     break;
           
           
           case 's': //Podria ser una etiqueta script o style que hay que ignorar su contenido
                     found=false;
                     if(buffer[1]=='c')
                      {
                       if(buffer[2]=='r')
                        {
                         if(buffer[3]=='i')
                          {
                           if(buffer[4]=='p')
                            {
                             if(buffer[5]=='t')
                              {
                               if(buffer[6]=='\0')
                                {
                                 found=true;
                                 //Seguimos leyendo hasta el cierre de la etiqueta
                                 for(;i<len && !(content[i-8]=='<' && content[i-7]=='/' &&
                                                 content[i-6]=='s' && content[i-5]=='c' &&
                                                 content[i-4]=='r' && content[i-3]=='i' &&
                                                 content[i-2]=='p' && content[i-1]=='t' &&
                                                 content[i]=='>');i++);
                                 level--;
                                }
                              }
                            }
                          }
                        }
                      }
                      else
                       {
                        if(buffer[1]=='t')
                         {
                          if(buffer[2]=='y')
                           {
                            if(buffer[3]=='l')
                             {
                              if(buffer[4]=='e')
                               {
                                if(buffer[5]=='\0')
                                 {
                                  found=true;
                                  //Seguimos leyendo hasta el cierre de la etiqueta                                  
                                  for(;i<len && !(content[i-7]=='<' && content[i-6]=='/' &&
                                                  content[i-5]=='s' && content[i-4]=='t' &&
                                                  content[i-3]=='y' && content[i-2]=='l' &&
                                                  content[i-1]=='e' && content[i]=='>');i++);
                                  level--;
                                 }
                               }
                             }
                           }
                         }
                       }
                     if(!found)
                      {
                       for(;i<len && content[i]!='>';i++);
                       if(content[i-1]=='/')
                         level--;
                      }
                     break;
           
           case 'm': //Podria ser una etiqueta meta con una redireccion del estilo
                     //<meta http-equiv="refresh" content="0;URL=http://...">
                     found=false;
                     if(buffer[1]=='e')
                      {
                       if(buffer[2]=='t')
                        {
                         if(buffer[3]=='a')
                          {
                           if(buffer[4]=='\0')
                            {//Seguimos leyendo para ver si es una redireccion
                             found=true;
                             //Buscamos los atributos http-equiv y content
                             notfound=false;
                             redirecting=false;
                             refreshing=false;
                             while(!notfound && i<len)
                              {//Quitamos los espacios antes de leer el atributo
                               for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                       
                               //Cogemos el atributo
                               for(j=0;i<len && content[i]!=' ' && content[i]!='\n' && content[i]!='\t' && content[i]!='>' && content[i]!='=';i++,j++)
                                 buffer[j]=content[i];
                               buffer[j]='\0';
                       
                               //Quitamos los espacios tras el atributo
                               for(;i<len && (content[i]==' ' || content[i]=='\n' || content[i]=='\t');i++);
                         
                               if(content[i]=='=')
                                {//El atributo tiene valor
                                 if(buffer[0]=='c' && buffer[1]=='o' &&
                                    buffer[2]=='n' && buffer[3]=='t' &&
                                    buffer[4]=='e' && buffer[5]=='n' &&
                                    buffer[6]=='t')
                                  {//Cogemos el atributo
                                   //Avanzamos hasta la comilla
                                   for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                 
                                   for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                                     buffer[j]=content[i];
                                   buffer[j]='\0';
                                 
                                   for(j=0;j<strlen(buffer) && buffer[j-4]!=';' &&
                                           (buffer[j-3]!='U' || buffer[j-3]!='u') &&
                                           (buffer[j-2]!='R' || buffer[j-2]!='r') &&
                                           (buffer[j-1]!='L' || buffer[j-1]!='l') &&
                                           buffer[j]!='=';j++);
                                   if(j!=strlen(buffer))
                                    {//Era una redireccion
                                     for(k=0,j++;j<strlen(buffer);k++,j++)
                                       buffer2[k]=buffer[j];
                                     buffer2[k]='\0';
                                     redirecting=true;
                                    }
                                  }
                                  else
                                   {
                                    if(buffer[0]=='h' && buffer[1]=='t' &&
                                       buffer[2]=='t' && buffer[3]=='p' &&
                                       buffer[4]=='-' && buffer[5]=='e' &&
                                       buffer[6]=='q' && buffer[7]=='u' &&
                                       buffer[8]=='i' && buffer[9]=='v')
                                     {//Cogemos el atributo
                                      //Avanzamos hasta la comilla
                                      for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                 
                                      for(i++,j=0;i<len && content[i]!='"' && content[i]!='\'';i++,j++)
                                        buffer[j]=content[i];
                                      buffer[j]='\0';
                                      
                                      if(strcmp(buffer,"refresh")==0)
                                        refreshing=true;
                                     }
                                     else
                                      {//Este atributo no nos interesa
                                       for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                       for(;i<len && content[i]!='\"' && content[i]!='\'';i++);
                                      }
                                   }
                                }
                                else
                                 {
                                  if(content[i]=='>')
                                   {
                                    notfound=true;
                                    if(content[i-1]=='/')
                                      level--;
                                   }
                                 }
                               } 
                             if(redirecting && refreshing)
                              {
                               InsertLink(buffer2);
                               if(ExternalLink(buffer2))
                                 n_outlinks++;
                              }
                               
                            }
                          }
                        }
                      }
                     if(!found)
                      {
                       for(;i<len && content[i]!='>';i++);
                       if(content[i-1]=='/')
                         level--;
                      }
                     break;
                     
           default: //Vamos hacia el cierre de la etiqueta
                    for(;i<len && content[i]!='>';i++);
                    if(content[i-1]=='/')
                      level--;
          }
            
        }
        else
         {
          if(content[i]!=' ' && content[i]!='\t' && content[i]!='\n')
            textsize++;
         }         
      }

     if(level!=0)
      {
       fprintf(stderr,"Error: Page was not xhtml (tree bad formed) %d\n",level);
       result=false;
      }
      else
        result=true;
     if(n_leveltags[kMaxLevelTags-1]<0)
       n_leveltags[kMaxLevelTags-1]=0;
         
    }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ProcessPage() -> %d\n",result);
#endif
        
  return(result);
 } 
 
bool WebPage::NewSimilar(short language,int numfile,float distance)
 {
  bool result;
  int i;
  int *auxsimilars;
  short *auxlanguages;
  float *auxdistances;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::NewSimilar(%d,%d,%d)\n",language,numfile,distance);
#endif
  
  result=true;
  if(n_similars>0)
   {
    auxsimilars=new int[n_similars+1];
    auxlanguages=new short[n_similars+1];
    auxdistances=new float[n_similars+1];
    for(i=0,result=true;i<n_similars && result;i++)
     {
      if(similars[i]==numfile && langsimilars[i]==language)
        result=false;//El similar estaba repetido
       else
        {
         auxsimilars[i]=similars[i];
         auxlanguages[i]=langsimilars[i];
         auxdistances[i]=distancesimilars[i];
        }
     }
    if(result)
     {
      auxsimilars[i]=numfile;
      auxlanguages[i]=language;
      auxdistances[i]=distance;
      delete similars;
      similars=auxsimilars;
      delete langsimilars;
      langsimilars=auxlanguages;
      delete distancesimilars;
      distancesimilars=auxdistances;
      n_similars++;
     }
     else
      {
       delete auxsimilars;
       auxsimilars=NULL;
       delete auxlanguages;
       auxlanguages=NULL;
       delete auxdistances;
       auxdistances=NULL;
      }
   }
   else
    {
     similars=new int;
     langsimilars=new short;
     distancesimilars=new float;
     similars[0]=numfile;
     langsimilars[0]=language;
     distancesimilars[0]=distance;
     n_similars++;
    }
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::NewSimilar(%d,%d,%d) -> %d\n",language,numfile,distance,result);
#endif
  
  return(result);
 }

bool WebPage::ChangeSimilar(short language,int numorigfile,int numdestfile)
 {
  bool result;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ChangeSimilar(%d,%d,%d)\n",language,numorigfile,numdestfile);
#endif
    
  if(numorigfile!=numdestfile)
   {
    if(n_similars>0)
     {
      if(numdestfile>=0)
       {
        for(i=0,result=false;i<n_similars && !result;i++)
         {
          if(similars[i]==numorigfile && langsimilars[i]==language)
           {
            similars[i]=numdestfile;
            result=true;
           }
         }       
       }
       else
        {//Hay que borrar el similar si se encuentra
         for(i=0,result=false;i<n_similars && !result;i++)
          {
           if(similars[i]==numorigfile && langsimilars[i]==language)
             result=true;
          }
         if(result)
          {//Desplazamos el resto hacia la izquierda
           n_similars--;
           for(i--;i<n_similars;i++)
            {
             similars[i]=similars[i+1];
             langsimilars[i]=langsimilars[i+1];
             distancesimilars[i]=distancesimilars[i+1];
            }
          }
        }
     }
     else
       result=false;
   }
   else
     result=true;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ChangeSimilar(%d,%d,%d) -> %d\n",language,numorigfile,numdestfile,result);
#endif
  
  return(result);
 } 

bool WebPage::RefreshSimilar(short language,int numorigfile,float newdistance)
 {
  bool result;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::RefreshSimilar(%d,%d,%d)\n",language,numorigfile,newdistance);
#endif
    
  if(n_similars>0)
   {
    for(i=0,result=false;i<n_similars && !result;i++)
     {
      if(similars[i]==numorigfile && langsimilars[i]==language)
       {
        distancesimilars[i]=newdistance;
        result=true;
       }
     }
   }
   else
     result=true;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::RefreshSimilar(%d,%d,%d) -> %d\n",language,numorigfile,newdistance,result);
#endif
  
  return(result);
 } 

unsigned int WebPage::ResetSimilarListTo(unsigned int sizelist,unsigned short* languagelist,unsigned int* numberlist)
 {
  unsigned int result;
  bool found;
  int i,k;
  unsigned int j;
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ResetSimilarListTo(%d,%p,%p) -> %d\n",sizelist,languagelist,numberlist);
#endif

  result=0;
  for(i=0;i<n_similars;i++)
   {
    for(found=false,j=0;j<sizelist && !found;j++)
     {
      if(numberlist[j]==(unsigned)similars[i] &&
         languagelist[j]==langsimilars[i])
        found=true;
     }
    if(!found)
     {//Borramos el elemento
      n_similars--;
      for(k=i;k<n_similars;k++)
       {
        similars[k]=similars[k+1];
        langsimilars[k]=langsimilars[k+1];
        distancesimilars[k]=distancesimilars[k+1];
       }
      i--;
      result++;
     }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ResetSimilarListTo(%d,%p,%p) -> %d\n",sizelist,languagelist,numberlist,result);
#endif
  return(result);
 }
  
void WebPage::FijarURL(const char *url)
 {
  unsigned int len; //longitud de la cadena url
  unsigned int i,j;
  char aux[kSizeAuxStrings];
  bool finishing; //Se activa cuando leemos un '?' ya que solo restan parametros
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::FijarURL(%s)\n",url);
#endif
  
  //La URL puede o no tener "http://"
  len=strlen(url);
  if(len>7 && url[0]=='h' && url[1]=='t' &&
     url[2]=='t' && url[3]=='p' && url[4]==':' &&
     url[5]=='/' && url[6]=='/')
    j=7;
   else
     j=0;
  
  //Cogemos el domain
  aux[0]='\0';
  for(i=j;i<len && url[i]!='/' && url[i]!='?';i++)
    aux[i-j]=url[i];
  aux[i-j]='\0';
  if(domain!=NULL)
    delete domain;
  domain=new char[1+strlen(aux)];
  strcpy(domain,aux);
  if(url[i]=='?')
    finishing=true;
   else
     finishing=false;

  if(!finishing)
   {//Cogemos el path
    aux[0]='\0';
    j=i+1;
    for(i=j;i<len && url[i]!='?' && url[i]!='#';i++)
      aux[i-j]=url[i];
    for(i--;i>j && url[i]!='/';i--);//Retrocedemos hasta el anterior '/'
    if(j>=i)
      aux[0]='\0';
     else 
       aux[i-j]='\0';
    if(path!=NULL)
      delete path;
    path=new char[1+strlen(aux)];
    strcpy(path,aux);
    if(url[i]=='?')
      finishing=true;
   }
   else
    {
     if(path!=NULL)
       delete path;
     path=new char[1];
     strcpy(path,"");
    }
  
  if(!finishing)
   {//Cogemos el filename
    if(strlen(path)>0)
      j=i+1;
    aux[0]='\0';
    for(i=j;i<len && url[i]!='?' && url[i]!='#';i++)
      aux[i-j]=url[i];
    aux[i-j]='\0';
    for(i--;i>j && url[i]!='.';i--);//Retrocedemos hasta el anterior '.'
    if(j!=i && i>j)
      aux[i-j]='\0';      
    
    if(filename!=NULL)
      delete filename;
    filename=new char[1+strlen(aux)];
    
    strcpy(filename,aux);
    if(url[i]=='?')
      finishing=true;
   }
   else
    {
     if(filename!=NULL)
       delete filename;
     filename=new char[1];
     strcpy(filename,"");
    }
  
  if(!finishing)
   {//Cogemos la extension
    if(strlen(filename)>0)
      j=i+1;
    aux[0]='\0';
    for(i=j;i<len && url[i]!='?' && url[i]!='#';i++)
      aux[i-j]=url[i];
    if(j>=i)
      aux[0]='\0';
     else 
       aux[i-j]='\0';
    if(fileext!=NULL)
      delete fileext;
    fileext=new char[1+strlen(aux)];
    strcpy(fileext,aux);
   }
   else
    {
     if(fileext!=NULL)
       delete fileext;
     fileext=new char[1];
     strcpy(fileext,"");
    }

  //Cogemos los parametros
  if(strlen(fileext)>0)
    j=i+1;
  aux[0]='\0';
  for(i=j;i<len && url[i]!='#';i++)
   {
    aux[i-j]=url[i];
   }
  if(j>=i)
    aux[0]='\0';
   else 
     aux[i-j]='\0';
  if(fileparam!=NULL)
    delete fileparam;
  fileparam=new char[1+strlen(aux)];
  strcpy(fileparam,aux);

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::FijarURL(%s)\n",url);
#endif
 }

struct tm* WebPage::GetDateString(const char *datestring)
 {
  struct tm *result;
  int i,j,len;
  int sec,min,hour,mday,mon,year,wday;//Auxiliares para los valores
  bool error; //Indica que hay un error en la cadena
  char buffer[kSizeAuxStrings];
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::GetDateString(%s)\n",datestring);
#endif
  
  result=NULL;
  len=strlen(datestring);
  error=false;
  
  for(i=0;i<len && datestring[i]!=',';i++)//Empezamos por el dia de la semana
    buffer[i]=datestring[i];
  buffer[i]='\0';
    
  if(strcmp(buffer,"Sun")==0)
    wday=0;
   else
    {
     if(strcmp(buffer,"Mon")==0)
       wday=1;
      else
       {
        if(strcmp(buffer,"Tue")==0)
          wday=2;
         else
          {
           if(strcmp(buffer,"Wed")==0)
             wday=3;
            else
             {
              if(strcmp(buffer,"Thu")==0)
                wday=4;
              else
                {
                 if(strcmp(buffer,"Fri")==0)
                   wday=5;
                  else
                   {
                    if(strcmp(buffer,"Sat")==0)
                      wday=6;
                     else
                      {//Hay un error con la cadena
                       error=true;
                      }
                   }                                 
                }
             }           
          }    
       }    
    }
        
  if(!error)
   {
    for(i=i+2,j=i;i<len && !error && datestring[i]!=' ';i++)//Seguimos por el dia del mes
     {
      if(!isdigit(datestring[i]))
        error=true;
       else 
         buffer[i-j]=datestring[i];
     }
    if(!error)
     {
      buffer[i-j]='\0';
      mday=atoi(buffer);
      
      for(i++,j=i;i<len && !error && datestring[i]!=' ';i++)//Seguimos por el mes
        buffer[i-j]=datestring[i];
      buffer[i]='\0';

      if(strcmp(buffer,"Jan")==0)
        mon=0;
       else
        {
         if(strcmp(buffer,"Feb")==0)
           mon=1;
          else
           {
            if(strcmp(buffer,"Mar")==0)
              mon=2;
             else
              {
               if(strcmp(buffer,"Apr")==0)
                 mon=3;
               else
                 {
                  if(strcmp(buffer,"May")==0)
                    mon=4;
                   else
                    {
                     if(strcmp(buffer,"Jun")==0)
                       mon=5;
                      else
                       {
                        if(strcmp(buffer,"Jul")==0)
                          mon=6;
                         else
                          {
                           if(strcmp(buffer,"Ago")==0)
                             mon=7;
                            else
                             {
                              if(strcmp(buffer,"Sep")==0)
                                mon=8;
                               else
                                {
                                 if(strcmp(buffer,"Oct")==0)
                                   mon=9;
                                  else
                                   {
                                    if(strcmp(buffer,"Nov")==0)
                                      mon=10;
                                     else
                                      {
                                       if(strcmp(buffer,"Dic")==0)
                                         mon=11;
                                        else
                                         {//Hay un error con la cadena
                                          error=true;
                                          mon=12;
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
      if(!error)
       {
        for(i++,j=i;i<len && !error && datestring[i]!=' ';i++)//Seguimos por el año
         {
          if(!isdigit(datestring[i]))
            error=true;
           else 
             buffer[i-j]=datestring[i];
         }
        if(!error)        
         {
          buffer[i-j]='\0';
          year=atoi(buffer);
          
          for(i++,j=i;i<len && !error && datestring[i]!=':';i++)//Seguimos por la hora
           {
            if(!isdigit(datestring[i]))
              error=true;
             else 
               buffer[i-j]=datestring[i];
           } 
          if(!error)        
           {
            buffer[i-j]='\0';
            hour=atoi(buffer);
          
            for(i++,j=i;i<len && !error && datestring[i]!=':';i++)//Seguimos por los minutos
             {
              if(!isdigit(datestring[i]))
                error=true;
               else 
                 buffer[i-j]=datestring[i];
             } 
            if(!error)        
             {
              buffer[i-j]='\0';
              min=atoi(buffer);
          
              for(i++,j=i;i<len && !error && datestring[i]!=' ';i++)//Seguimos por los segundos
               {
                if(!isdigit(datestring[i]))
                  error=true;
                 else 
                   buffer[i-j]=datestring[i];
               }
              if(!error)        
               {
                buffer[i-j]='\0';
                sec=atoi(buffer);
               }
             }
           }
         }
       }
     }    
   }
  
  if(!error)
   {
    result=new struct tm;
    result->tm_sec=sec;
    result->tm_min=min;
    result->tm_hour=hour;
    result->tm_mday=mday;
    result->tm_mon=mon;
    result->tm_year=year;
    result->tm_wday=wday;
    result->tm_yday=-1;
    result->tm_isdst=-1;
   }
   
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::GetDateString(%s) -> (%d:%d:%d %d,%d-%d-%d)\n",datestring,hour,min,sec,wday,mday,mon,year);
#endif
   
  return(result);
 } 
 
bool WebPage::ExternalLink(const char *url)
 {
  bool result;
  unsigned int i,j;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ExternalLink(%s)\n",url);
#endif
    
  for(i=1,result=false;i<strlen(url) && !result;i++)
   {
    if(url[i-1]=='/' && url[i]=='/')
      result=true;
   }
  if(!result)
    i=0;
  for(j=0,result=false;i<strlen(url) && j<strlen(domain) && !result && 
                       url[i]!=':' && domain[j]!=':';i++,j++)
   {
    if(url[i]!=domain[j])
      result=true;
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ExternalLink(%s) -> %d\n",url,result);
#endif
      
  return(result);
 }

bool WebPage::InsertLink(const char *url)
 {
  bool result;  
  char *newurl; //URL
  unsigned int i;
  char **auxlinks; //Auxiliar para que crezca el vector de enlaces

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::InsertLink(%s)\n",url);
#endif
    
  if(url!=NULL)
   {
    auxlinks=new char*[n_links+1];
    for(i=0;i<n_links;i++)
      auxlinks[i]=links[i];
    newurl=new char[1+strlen(url)];
    strcpy(newurl,url);
    auxlinks[i]=newurl;
    newurl=NULL;
    delete links;
    links=auxlinks;
    n_links++;
    result=true;
   }
   else
     result=false;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::InsertLink(%s) -> %d\n",url,result);
#endif
  
  return(result);
 }

bool WebPage::ImportWget(const char *file,const char *href)
 {
  bool result;
  bool found;
  int i;
  unsigned int k;
  char buffer[kSizeFileBuffer+1];//Buffer para la lectura del fichero
#ifdef SearchLastModified   
  int j;
  char lastmodified[kSizeAuxStrings];//Buffer para almacenar la cadena con la fecha
  struct tm *auxdate;//Auxiliar para las fechas
#endif  
  unsigned int bytesleidos;//Bytes rellenados en el buffer
  unsigned int byteswritten,byteswritten2;//Bytes escritos en disco
  int pidactual;//PID del programa actual para crear el fichero temporal
  char tempfilename[kSizeAuxStrings];//Nombre que tendra el fichero temporal que se creara
  FILE *fent,*ftemp1,*ftemp2;
  bool writeerror;
  char systemcall[kSizeAuxStrings];//Nombre de la llamada al sistema que se realiza al tidy
  char *bufferaux;//Buffer auxiliar para la carga del content

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ImportWget(%s,%s)\n",file,href);
#endif  
    
  result=true;
  i=0;
  if(!(fent=fopen(file,"r")))
    result=false;
   else
    {
     Reset();
     date=time(0);
     FijarURL(href);
     
     //Leemos el fichero (o al menos en parte)     
     bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);     
     
#ifdef SearchLastModified
     //Buscamos en el fichero la fecha de modificacion
     for(i=15,found=false;i<bytesleidos && !found;i++)
      {
       if(buffer[i-15]=='\n' && buffer[i-14]=='L' &&
          buffer[i-13]=='a' && buffer[i-12]=='s' &&
          buffer[i-11]=='t' && buffer[i-10]=='-' &&
          buffer[i-9]=='M' && buffer[i-8]=='o' &&
          buffer[i-7]=='d' && buffer[i-6]=='i' &&
          buffer[i-5]=='f' && buffer[i-4]=='i' &&
          buffer[i-3]=='e' && buffer[i-2]=='d' &&
          buffer[i-1]==':' && buffer[i]==' ')
        {//Hemos encontrado el Last-Modified
         found=true;
         for(j=i+1;buffer[j]!='\n';j++)
           lastmodified[j-i-1]=buffer[j];
        }
      }
     if(!found)
      {
       fprintf(stderr,"Warning: Last-Modified not found in %s\n",file);
       origdate=date;       
      }
      else
       {//Convertimos la cadena con el tiempo en un numero
        auxdate=GetDateString(lastmodified);
        if(auxdate!=NULL)
         {
          origdate=mktime(auxdate);
          if(origdate==(unsigned)(time_t)(-1))
           {
            fprintf(stderr,"Warning: Last-Modified not valid in %s\n",file);
            origdate=date;
           }
          delete auxdate;
          auxdate=NULL;
         }
         else
          {
           fprintf(stderr,"Warning: Last-Modified not valid in %s\n",file);
           origdate=date; 
          }
       }
     //Pasamos al analisis del contenido
          
     //Buscamos en el fichero la fecha de modificacion
     if(!found)
       i=5;
#endif
#ifndef SearchLastModified
     origdate=date; 
#endif     
         
     for(found=false;i<(signed)bytesleidos && !found;i++)
      {
       if(buffer[i-4]=='<' && (buffer[i-3]=='h' || buffer[i-3]=='H') &&
          (buffer[i-2]=='t' || buffer[i-2]=='T') && 
          (buffer[i-1]=='m' || buffer[i-1]=='M') &&
          (buffer[i]=='l' || buffer[i]=='L'))
        {//Hemos encontrado el inicio de la pagina
         found=true;
         i=i-5;
        }
      }
     if(!found && fileext!=NULL && 
        (fileext[0]=='h' || fileext[0]=='H') &&
        (fileext[1]=='t' || fileext[1]=='T') &&
        (fileext[2]=='m' || fileext[2]=='M') &&
        (fileext[3]=='l' || fileext[3]=='L' || fileext[3]=='\0') &&
        (fileext[3]=='\0' || fileext[4]=='\0'))
      {
       fprintf(stderr,"Error: File %s does not contain <html> tag\n",file);
       result=false;
      }
      else
       {//Creamos el fichero temporal para pasarle el tidy
        pidactual=getpid();
        sprintf(tempfilename,"/tmp/wptmptidya%d",pidactual);
        if(!(ftemp1=fopen(tempfilename,"w")))
         {
          fprintf(stderr,"Error: Temporal file \"%s\" could not be created\n",tempfilename);
          result=false;
         }
         else
          {//Escribimos el content del fichero en el temporal
           byteswritten=fwrite(buffer+i,1,bytesleidos-i,ftemp1);
           if((bytesleidos-i)!=byteswritten)
            {
             byteswritten2=fwrite(buffer+i,1,bytesleidos-i-byteswritten,ftemp1);
             if(byteswritten2!=(bytesleidos-i-byteswritten))
              {
               fprintf(stderr,"Error: Unable to write in temporal file \"%s\"\n",tempfilename);
               result=false;               
              }
             fclose(ftemp1);
            }
            else
             {
              writeerror=false;
              while(bytesleidos==kSizeFileBuffer && !writeerror)
               {
                bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);
                if(bytesleidos!=fwrite(buffer,1,bytesleidos,ftemp1))
                 {
                  fprintf(stderr,"Error: Unable to write in temporal file \"%s\"\n",tempfilename);
                  writeerror=true;
                 }
               };
              fclose(ftemp1);
              if(writeerror)
               {
                result=false;
               }
               else
                {//Pasamos el tidy por el fichero temporal
                 //sprintf(systemcall,"tidy -i -asxhtml --force-output true < /tmp/wptmptidya%d > /tmp/wptmptidyb%d 2>/dev/null",pidactual,pidactual);
                 sprintf(systemcall,"./convertpage.sh /tmp/wptmptidya%d /tmp/wptmptidyb%d",pidactual,pidactual);
                 system(systemcall);
                 usleep(100);
                 
                 sprintf(systemcall,"/tmp/wptmptidya%d",pidactual);
                 if(unlink(systemcall)!=0)//Borramos el primer fichero temporal
                  {
                   unlink(systemcall);
                  }
                                  
                 //Abrimos el fichero temporal con la solucion del tidy
                 sprintf(tempfilename,"/tmp/wptmptidyb%d",pidactual);
                 if(!(ftemp2=fopen(tempfilename,"r")))
                  {
                   fprintf(stderr,"Error: Temporal file \"%s\" could not be opened\n",tempfilename);
                   result=false;                                   
                  }
                  else
                   {//Cargamos el contenido del nuevo fichero en content                    
                    content=new char[1];
                    content[0]='\0';
                    while(!feof(ftemp2))
                     {
                      bytesleidos=fread(buffer,1,kSizeFileBuffer,ftemp2);
                      if(bytesleidos==kSizeFileBuffer)
                       {
                        buffer[kSizeFileBuffer]='\0';                               
                        bufferaux=new char[kSizeFileBuffer+1+strlen(content)];
                        strcpy(bufferaux,content);
                        strcat(bufferaux,buffer);
                        delete content;
                        content=new char[1+strlen(bufferaux)];
                        strcpy(content,bufferaux);
                        delete bufferaux;
                        bufferaux=NULL;
                       }
                       else
                        {//Ya hemos leido todo el fichero
                         buffer[bytesleidos]='\0';
                         bufferaux=new char[bytesleidos+1+strlen(content)];
                         sprintf(bufferaux,"%s%s",content,buffer);
                         delete content;
                         content=new char[1+strlen(bufferaux)];
                         strcpy(content,bufferaux);
                         delete bufferaux;
                         bufferaux=NULL;
                        }
                     }                   
                    fclose(ftemp2);
                    
                    //Comprobamos que el fichero no contenga codigo de servidor
                    //no valido como "<% ... %> ni nada parecido
                    for(found=false,i=1;i<(signed)strlen(content) && !found;i++)
                     {
                      if(content[i-2]=='<' && content[i-1]=='%' && content[i]=='@')
                       {//Hemos encontrado codigo no valido
                        found=true;
                       }
                     }
                    if(found)
                     {
                      fprintf(stderr,"Error: File contained non-valid server code\n");
                      result=false;    
                     }
                     else
                      {
                       sprintf(systemcall,"/tmp/wptmptidyb%d",pidactual);
                       if(unlink(systemcall)!=0)//Borramos el segundo fichero temporal
                        {
                         unlink(systemcall);
                        }
                       //Quitamos el <!DOCTYPE ...> del documento puesto que no es valido
                       //dentro del xml y ademas es el que pone el tidy, es decir, siempre
                       //el mismo
                       if(strlen(content)>150)
                        {
                         for(k=0,found=false;k<strlen(content) && k<150 && !found;k++)
                          {
                           if(content[k]=='<' && content[k+1]=='!' &&
                              content[k+2]=='D' && content[k+3]=='O' &&
                              content[k+4]=='C' && content[k+5]=='T' &&
                              content[k+6]=='Y' && content[k+7]=='P' &&
                              content[k+8]=='E')
                             found=true;
                          }
                         if(found)
                          {
                           for(found=false;k<strlen(content) && !found;k++)
                            {
                             if(content[k]=='>')
                               found=true;
                            }
                           bufferaux=new char[strlen(content)-k+1];
                           strcpy(bufferaux,content+k);
                           delete content;
                           content=bufferaux;
                          }
                        }
                    
                       localfile=new char[1+strlen(file)];
                       strcpy(localfile,file);                    
                       size=strlen(content);
                       result=ProcessPage();
                       if(result)
                        {                         
                         n_similars=0;
                         similars=NULL;
                         n_applications=2;                    
                         applications=new char*[2];
                         appversions=new char*[2];
                         applications[0]=new char[1+strlen("tidy")];
                         strcpy(applications[0],"tidy");
                         appversions[0]=new char[1+strlen("1st April 2002")];
                         strcpy(appversions[0],"1st April 2002");
                         applications[1]=new char[1+strlen("unicode2ascii")];
                         strcpy(applications[1],"unicode2ascii");
                         appversions[1]=new char[1+strlen("1.0")];
                         strcpy(appversions[1],"1.0");
                        }
                      }
                   }
                }
             }
          }
       }
     fclose(fent);
    }
  if(!result)
    Reset();

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ImportWget(%s,%s) -> %d\n",file,href,result);
#endif  
      
  return(result);
 }


bool WebPage::ImportConvertedPage(const char *file,const char *href)
 {
  bool result;
  bool found;
  int i;
  unsigned int k;
  char buffer[kSizeFileBuffer+1];//Buffer para la lectura del fichero
  unsigned int bytesleidos;//Bytes rellenados en el buffer
  FILE *fent;
  char *bufferaux;//Buffer auxiliar para la carga del content

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::ImportConvertedPage(%s,%s)\n",file,href);
#endif  
      
  if(!(fent=fopen(file,"r")))
    result=false;
   else
    {
     Reset();
     date=time(0);
     FijarURL(href);

     //Leemos el fichero (o al menos en parte)     
     bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);
     buffer[bytesleidos]='\0';
          
     //Pasamos al analisis del contenido
     origdate=date;
         
     for(i=0,found=false;i<kSizeFileBuffer && !found;i++)
      {
       if(buffer[i-4]=='<' && (buffer[i-3]=='h' || buffer[i-3]=='H') &&
          (buffer[i-2]=='t' || buffer[i-2]=='T') && 
          (buffer[i-1]=='m' || buffer[i-1]=='M') &&
          (buffer[i]=='l' || buffer[i]=='L'))
        {//Hemos encontrado el inicio de la pagina
         found=true;
         i=i-5;
        }
      }

     //Cargamos el contenido del nuevo fichero en content                    
     content=new char[1+strlen(buffer)];
     strcpy(content,buffer);
     while(!feof(fent))
      {
       bytesleidos=fread(buffer,1,kSizeFileBuffer,fent);
       if(bytesleidos==kSizeFileBuffer)
        {
         buffer[kSizeFileBuffer]='\0';                               
         bufferaux=new char[kSizeFileBuffer+1+strlen(content)];
         strcpy(bufferaux,content);
         strcat(bufferaux,buffer);
         delete content;
         content=new char[1+strlen(bufferaux)];
         strcpy(content,bufferaux);
         delete bufferaux;
         bufferaux=NULL;
        }
        else
         {//Ya hemos leido todo el fichero
          buffer[bytesleidos]='\0';
          bufferaux=new char[bytesleidos+1+strlen(content)];
          sprintf(bufferaux,"%s%s",content,buffer);
          delete content;
          content=new char[1+strlen(bufferaux)];
          strcpy(content,bufferaux);
          delete bufferaux;
          bufferaux=NULL;
         }
      }                   
                   
     //Quitamos el <!DOCTYPE ...> del documento puesto que no es valido
     //dentro del xml y ademas es el que pone el tidy, es decir, siempre
     //el mismo
     if(strlen(content)>150)
      {
       for(k=0,found=false;k<strlen(content) && k<150 && !found;k++)
        {
         if(content[k]=='<' && content[k+1]=='!' &&
            content[k+2]=='D' && content[k+3]=='O' &&
            content[k+4]=='C' && content[k+5]=='T' &&
            content[k+6]=='Y' && content[k+7]=='P' &&
            content[k+8]=='E')
           found=true;
        }
       if(found)
        {
         for(found=false;k<strlen(content) && !found;k++)
          {
           if(content[k]=='>')
             found=true;
          }
         bufferaux=new char[strlen(content)-k+1];
         strcpy(bufferaux,content+k);
         delete content;
         content=bufferaux;
        }
        
       //Quitamos el <?xml ...> del documento puesto que no es valido
       //dentro del contenido del xml
       for(k=0,found=false;k<strlen(content) && k<500 && !found;k++)
        {
         if(content[k]=='<' && content[k+1]=='?' &&
            content[k+2]=='x' && content[k+3]=='m' &&
            content[k+4]=='l' && content[k+5]==' ')
           found=true;
        }
       if(found)
        {
         for(found=false;k<strlen(content) && !found;k++)
          {
           if(content[k]=='>')
             found=true;
          }
         bufferaux=new char[strlen(content)-k+1];
         strcpy(bufferaux,content+k);
         delete content;
         content=bufferaux;
        }
       
       //Quitamos el <%@ ...> del documento puesto que no es valido dentro
       //del contenido del xml
       for(k=0,found=false;k<strlen(content) && k<500 && !found;k++)
        {
         if(content[k]=='<' && content[k+1]=='%' &&
            content[k+2]=='@' && content[k+3]==' ')
           found=true;
        }
       if(found)
        {
         for(found=false;k<strlen(content) && !found;k++)
          {
           if(content[k]=='>')
             found=true;
          }
         bufferaux=new char[strlen(content)-k+1];
         strcpy(bufferaux,content+k);
         delete content;
         content=bufferaux;
        }
      }

     localfile=new char[1+strlen(file)];
     strcpy(localfile,file);                    
     size=strlen(content);
     ProcessPage();

     n_similars=0;
     n_applications=2;
     applications=new char*[2];
     appversions=new char*[2];
     applications[0]=new char[1+strlen("tidy")];
     strcpy(applications[0],"tidy");
     appversions[0]=new char[1+strlen("1st April 2002")];
     strcpy(appversions[0],"1st April 2002");
     applications[1]=new char[1+strlen("unicode2ascii")];
     strcpy(applications[1],"unicode2ascii");
     appversions[1]=new char[1+strlen("1.0")];
     strcpy(appversions[1],"1.0");
     result=true;
     fclose(fent);
    }
  if(!result)
    Reset();

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::ImportConvertedPage(%s,%s) -> %d\n",file,href,result);
#endif  

  return(result);
 } 

short WebPage::WhatLanguage()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::WhatLanguage()\n");
#endif  
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::WhatLanguage() -> %d\n",language);
#endif  
  
  return(language);
 }

unsigned int WebPage::N_Tags()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_Tags()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::N_Tags() -> %d\n",n_tags);
#endif  
  
  return(n_tags);
 }

const unsigned int* WebPage::N_LevelTags()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_LevelTags()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::N_LevelTags() -> %p\n",n_tags);
#endif  
  
  return((const unsigned int*)n_leveltags);
 }
 
unsigned int WebPage::N_Links()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_Links()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::N_Links() -> %d\n",n_links);
#endif  
 
  return(n_links);
 }
 
const char** WebPage::Links()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Links()\n");
#endif

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Links() -> %p\n",links);
#endif

  return((const char**)links);
 } 

unsigned int WebPage::N_Similars()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_Similars()\n");
#endif  
 
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::N_Similars() -> %d\n",n_similars);
#endif  
  
  return(n_similars);
 }

const int* WebPage::Similars()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Similars()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_Similars() -> %p\n",similars);
#endif  
 
  return(similars);
 }

const short* WebPage::LangSimilars()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::LangSimilars()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::LangSimilars() -> %d\n",langsimilars);
#endif  
 
  return(langsimilars);
 }
  
const float* WebPage::DistanceSimilars()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::DistanceSimilars()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::DistanceSimilars() -> %d\n",langsimilars);
#endif  
 
  return(distancesimilars);
 }

unsigned int WebPage::CountSimilars(short language)
 {
  unsigned int result;
  int i;
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::CountSimilars()\n");
#endif  
  
  for(i=0,result=0;i<n_similars;i++)
   {
    if(langsimilars[i]==language)
      result++;
   }  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::CountSimilars() -> %d\n",langsimilars);
#endif  
 
  return(result);
 }

unsigned int WebPage::N_Images()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::N_Images()\n");
#endif  
 
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::N_Images() -> %d\n",n_images);
#endif  
  
  return(n_images);
 } 

unsigned int WebPage::LocalFileId()
 {
  unsigned int result;
  char id[kSizeAuxStrings];
  unsigned int len;
  int i,j;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::LocalFileId()\n");
#endif  
  
  if(localfile!=NULL)
   {
    len=strlen(localfile);
    for(i=len-1;i>=0 && (localfile[i]<'0' || localfile[i]>'9');i--);
    for(;localfile[i]>='0' && localfile[i]<='9';i--);
    for(j=0,i++;localfile[i]>='0' && localfile[i]<='9';i++,j++)
      id[j]=localfile[i];
    id[j]='\0';
    result=strtol(id,NULL,10);
   }
   else
     result=0;
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::LocalFileId() -> %d\n",result);
#endif  
  
  return(result);    
 }

unsigned int WebPage::Size()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Size()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Size() -> %d\n",size);
#endif  
 
  return(size);
 }

unsigned int WebPage::TextSize()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::TextSize()\n");
#endif  

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::TextSize() -> %d\n",textsize);
#endif  
 
  return(textsize);
 } 
 
char* WebPage::PageFullURL()
 {
  char* result;
  char aux[kSizeAuxStrings];

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::PageFullURL()\n");
#endif  
    
  if(domain!=NULL && strlen(domain)>0)
   {
    sprintf(aux,"%s",domain);
    if(path!=NULL && strlen(path)>0)
      sprintf(aux,"%s/%s",aux,path);
    if(filename!=NULL && strlen(filename)>0)
      sprintf(aux,"%s/%s",aux,filename);
    if(fileext!=NULL && strlen(fileext)>0)
      sprintf(aux,"%s.%s",aux,fileext);
    if(fileparam!=NULL && strlen(fileparam)>0)
      sprintf(aux,"%s?%s",aux,fileparam);
    result=new char[1+strlen(aux)];
    strcpy(result,aux);      
   }
   else
     result=NULL;

#ifdef TRAZANDO_WebPage
  if(result!=NULL)
    fprintf(stderr,"EndWebPage::PageFullURL() -> %s\n",result);
   else
     fprintf(stderr,"EndWebPage::PageFullURL() -> NULL\n");
#endif  

  return(result);
 }

const char* WebPage::Content()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Content()\n");
#endif

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::Content() -> %p\n",content);//Aqui no mostramos toda la cadena devuelta porque es enorme
#endif  

  return(content);
 }

void WebPage::SetLanguage(short lang)
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::SetLanguage(%d)\n",lang);
#endif  
 
  if(lang>=-1)
    language=lang;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::SetLanguage(%d)\n",lang);
#endif          
 }

const char* WebPage::Domain()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Domain()\n");
#endif  

#ifdef TRAZANDO_WebPage
  if(domain!=NULL)
    fprintf(stderr,"EndWebPage::Domain() -> %s\n",domain);
   else
     fprintf(stderr,"EndWebPage::Domain() -> NULL\n");
#endif  
 
  return(domain);
 }

const char* WebPage::Path()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Path()\n");
#endif 

#ifdef TRAZANDO_WebPage
  if(path!=NULL)
    fprintf(stderr,"EndWebPage::Path() -> %s\n",path);
   else
     fprintf(stderr,"EndWebPage::Path() -> NULL\n");
#endif 
  return(path);
 }

const char* WebPage::Filename()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Filename()\n");
#endif  

#ifdef TRAZANDO_WebPage
  if(filename!=NULL)
    fprintf(stderr,"EndWebPage::Filename() -> %s\n",filename);
   else
     fprintf(stderr,"EndWebPage::Filename() -> NULL\n");
#endif  
 
  return(filename);
 }

const char* WebPage::Extension()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Extension()\n");
#endif

#ifdef TRAZANDO_WebPage
  if(fileext!=NULL)
    fprintf(stderr,"EndWebPage::Extension() -> %s\n",fileext);
   else
     fprintf(stderr,"EndWebPage::Extension() -> NULL\n");
#endif
  return(fileext);
 }

const char* WebPage::Parameters()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::Parameters()\n");
#endif 

#ifdef TRAZANDO_WebPage
  if(fileparam!=NULL)
    fprintf(stderr,"EndWebPage::Parameters() -> %s\n",fileparam);
   else
     fprintf(stderr,"EndWebPage::Parameters() -> NULL\n");
#endif
  return(fileparam);
 }
 
void WebPage::CleanContent()
 {
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::CleanContent()\n");
#endif 
  if(content!=NULL)
   {
    delete content;
    content=NULL;
   }
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::CleanContent()\n");
#endif    
 }

void WebPage::FreeUselessFields()
 {
  unsigned int i;
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::FreeUselessFields()\n");
#endif 
  if(links!=NULL)
   {
    for(i=0;i<n_links;i++)
      delete links[i];
    delete links;
    links=NULL;    
   }
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::FreeUselessFields()\n");
#endif    
 }
 
void WebPage::AddNewGuesser(const char *guessername,unsigned int n_languages,unsigned short *languages,const float *points)
 {
  char **auxguessers;
  int *auxn_languagesperguesser;
  int **auxid_languagesperguesser;
  float **auxprob_languagesperguesser;
  int i;
  unsigned int j;

#ifdef TRAZANDO_WebPage
  if(guessername!=NULL)
    fprintf(stderr,"BeginWebPage::AddNewGuesser(%s)\n",guessername);
   else
     fprintf(stderr,"BeginWebPage::AddNewGuesser(NULL)\n");
#endif 
    
  auxguessers=new char*[n_guessers+1];
  auxn_languagesperguesser=new int[n_guessers+1];
  auxid_languagesperguesser=new int*[n_guessers+1];
  auxprob_languagesperguesser=new float*[n_guessers+1];
  
  for(i=0;i<n_guessers;i++)
   {
    auxguessers[i]=guessers[i];
    auxn_languagesperguesser[i]=n_languagesperguesser[i];
    auxid_languagesperguesser[i]=id_languagesperguesser[i];
    auxprob_languagesperguesser[i]=prob_languagesperguesser[i];
   }
  auxguessers[i]=new char[1+strlen(guessername)];
  strcpy(auxguessers[i],guessername);
  auxn_languagesperguesser[i]=n_languages;
  auxid_languagesperguesser[i]=new int[n_languages];
  auxprob_languagesperguesser[i]=new float[n_languages];
  
  for(j=0;j<n_languages;j++)
   {
    auxid_languagesperguesser[i][j]=languages[j];
    auxprob_languagesperguesser[i][j]=points[languages[j]];
   }  
  
  delete guessers;
  guessers=auxguessers;
  delete n_languagesperguesser;
  n_languagesperguesser=auxn_languagesperguesser;
  delete id_languagesperguesser;
  id_languagesperguesser=auxid_languagesperguesser;
  delete prob_languagesperguesser;
  prob_languagesperguesser=auxprob_languagesperguesser;
  
  n_guessers++;
#ifdef TRAZANDO_WebPage
  if(guessername!=NULL)
    fprintf(stderr,"EndWebPage::AddNewGuesser(%s)\n",guessername);
   else
     fprintf(stderr,"EndWebPage::AddNewGuesser(NULL)\n");
#endif 
  
 }

void WebPage::SetProbsGuesser(const char *guessername,unsigned int n_languages,unsigned short *languages,const float *points)
 {
  int i;
  int idguesser;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::SetProbsGuesser(%d,%d,%p,%p)\n",idguesser,n_languages,languages,points);
#endif 

  //Buscamos si tenemos el guesser cuyos valores queremos actualizar
  for(idguesser=-1,i=0;i<n_guessers && idguesser==-1;i++)
   {
    if(strcmp(guessername,guessers[i])==0)
      idguesser=i;
   }
   
  if(idguesser!=-1)
   {
    n_languagesperguesser[idguesser]=n_languages;
    if(id_languagesperguesser[idguesser]!=NULL)
      delete id_languagesperguesser[idguesser];
    id_languagesperguesser[idguesser]=new int[n_languages];
    if(prob_languagesperguesser[idguesser]!=NULL)
      delete prob_languagesperguesser[idguesser];
    prob_languagesperguesser[idguesser]=new float[n_languages];
    for(i=0;i<n_languagesperguesser[idguesser];i++)
     {
      id_languagesperguesser[idguesser][i]=languages[i];
      prob_languagesperguesser[idguesser][i]=points[languages[i]];
     }
   }

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::SetProbsGuesser(%d,%d,%p,%p)\n",idguesser,n_languages,languages,points);
#endif 
      
 }

void WebPage::AddApplication(const char *appname,const char *appversion)
 {
  char **aux,**aux2;
  int i;

#ifdef TRAZANDO_WebPage
  fprintf(stderr,"BeginWebPage::AddApplication(%s,%s)\n",appname,appversion);
#endif
    
  if(appname!=NULL && appversion!=NULL)
   {
    aux=new char*[n_applications+1];
    aux2=new char*[n_applications+1];
    for(i=0;i<n_applications;i++)
     {
      aux[i]=applications[i];
      aux2[i]=appversions[i];
     }
    aux[i]=new char[1+strlen(appname)];
    strcpy(aux[i],appname);
    aux2[i]=new char[1+strlen(appversion)];
    strcpy(aux2[i],appversion);
    delete applications;
    applications=aux;
    delete appversions;
    appversions=aux2;
    n_applications++;
   }
  
#ifdef TRAZANDO_WebPage
  fprintf(stderr,"EndWebPage::AddApplication(%s,%s)\n",appname,appversion);
#endif
 }

char* WebPage::FilterContent()
 {
  char *result;
  unsigned int len,i,j,k,l;
  char *auxresult;
  char *buffertext; //Used for storing text while reading it  
  char actualtagname[kSizeAuxStrings]; //Used for storing tags while reading them
  bool insidetag; //True when inside a tag
  bool insidecomment; //True when inside a comment
  bool insidecdata; //True when inside a <![CDATA[ ]]> structure
  bool recordingtag; //True when copying the tagname
  bool typetag; //True opening tag, False closing tag
  bool openingandclose; //True for tags of the <.../> type
  bool lookingtagend; //Indicates when the end of a tag style or script has been found
  
  if(content!=NULL)
   {
    result=new char[1];
    result[0]='\0';
    len=strlen(content);
    buffertext=new char[1+len];
    insidetag=false;
    k=0;
    insidecomment=false;
    insidecdata=false;
    recordingtag=false;
    typetag=true;
    openingandclose=false;
    for(i=0,j=0;i<len;i++)
     {
      if(insidetag)
       {
        if(insidecomment)
         {
          if(content[i]=='>' && content[i-1]=='-' && content[i-2]=='-')
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
             if(content[i]=='>' && content[i-1]==']' && content[i-2]==']')
              {
               insidecdata=false;
               insidetag=false;
               k=0;
              }
            }
            else
             {
              if(content[i]=='/')
               {
                if(content[i+1]=='>' || j==0)
                 {
                  typetag=false;
                  if(j>0)
                    openingandclose=true;
                 }
               }
               else
                {
                 if(content[i]=='>')
                  {
                   insidetag=false;
                   recordingtag=false;
                   actualtagname[j]='\0';
                   
                   if(strcmp(actualtagname,"style")==0 || strcmp(actualtagname,"script")==0)
                    {//Special case: The content inside the tags style and script is discarded
                     for(i++,lookingtagend=false;!lookingtagend;i++)
                      {
                       if(content[i]=='>')
                        {
                         if(content[i-8]=='<' && content[i-7]=='/' && content[i-6]=='s' && content[i-5]=='c' && 
                            content[i-4]=='r' && content[i-3]=='i' && content[i-2]=='p' && content[i-1]=='t' && actualtagname[1]=='c')
                           lookingtagend=true;
                         if(content[i-7]=='<' && content[i-6]=='/' && content[i-5]=='s' && content[i-4]=='t' &&
                            content[i-3]=='y' && content[i-2]=='l' && content[i-1]=='e' && actualtagname[1]=='t')
                           lookingtagend=true;
                        }
                      }
                     i--;
                    }
                   k=0;
                  }
                  else
                   {
                    if(j==0 && content[i]=='!' && i<len-2 && content[i+1]=='-' && content[i+2]=='-')
                      insidecomment=true;
                     else
                      {
                       if(j==0 && content[i]=='!' && i<len-7 && content[i+1]=='[' &&
                          content[i+2]=='C' && content[i+3]=='D' && content[i+4]=='A' &&
                          content[i+5]=='T' && content[i+6]=='A' && content[i+7]=='[')
                         insidecdata=true;
                        else
                         {
                          if(recordingtag)
                           {
                            if(content[i]!=' ' && content[i]!='\t' && content[i]!='\n')
                             {
                              actualtagname[j]=content[i];
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
         if(content[i]=='<')
          {
           buffertext[k]='\0';
           //We verify that the content has any non-blank character
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
            {//The content string is valid for segmentation purposes
             buffertext[l]='\0';
             auxresult=new char[2+strlen(buffertext)+strlen(result)];
             strcpy(auxresult,result);
             strcat(auxresult," ");
             strcat(auxresult,buffertext);
             delete result;
             result=auxresult;
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
            if(content[i]=='&' && i<len && (content[i+1]=='#' && content[i+2]=='1' && content[i+3]=='6' && content[i+4]=='0') || 
                                           (content[i+1]=='n' && content[i+2]=='b' && content[i+3]=='s' && content[i+4]=='p') && content[i+5]==';')
             {//Substitution of &#160; or &nbsp; per blank space
              i=i+5;
              buffertext[k]=' ';
              k++;
             }
             else
              {
               buffertext[k]=content[i];
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
