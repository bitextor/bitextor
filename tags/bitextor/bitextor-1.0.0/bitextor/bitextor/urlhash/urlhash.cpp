/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
*/

#include "urlhash.h"

URLHash::URLHash()
 {
  unsigned int i;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::URLHash()\n");
#endif  
    
  sizevector=kSizeHash;  
  hash=new URLContent*[sizevector];
  for(i=0;i<sizevector;i++)
    hash[i]=NULL;
  
  hash[0]=NULL;
  urlnumber=0;
  urlsinqueue=0;
  queuehead=NULL;
  queuetail=NULL;

  depthhead=0;
  for(i=0;i<kMaxDeepTree;i++)
    levelshead[i]=NULL;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::URLHash()\n");
#endif  
    
 }

URLHash::~URLHash()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::~URLHash()\n");
#endif  
  
  Reset();
  sizevector=0;
  delete hash;
  hash=NULL;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::~URLHash()\n");
#endif   
 }
 
void URLHash::Reset()
 {
  unsigned int i;
  NodeQueue *aux;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Reset()\n");
#endif   
  
  for(i=1;i<sizevector;i++)
   {
    if(hash[i]!=NULL)
     {
      delete hash[i]->urlcontent;
      delete hash[i];
     }
   }
  urlnumber=0;
  urlsinqueue=0;
  while(queuehead!=NULL)  
   {
    aux=queuehead;
    queuehead=queuehead->sig;
    delete aux;
   }
  queuehead=NULL;
  queuetail=NULL;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::Reset()\n");
#endif  
 }

//Funcion que transforma una cadena cualquiera de caracteres en un entero
int URLHash::HashFunction(const char *cad)
 {
  int result;
  unsigned int i;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::HashFunction(%s)\n",cad);
#endif   
  
  result=0;
  if(cad!=NULL)
   {
    for(i=0;i<strlen(cad);i++)
      result=result+(i+1)*((int) cad[i]);
    result=result%kSizeHash;
   }
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::HashFunction(%s) -> %d\n",cad,result);
#endif   
  
  return(result);
 } 

char* URLHash::FixURL(const char *url)
 {
  char *result,*aux;
  unsigned int i,j;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::FixURL(%s)\n",url);
#endif   
  
  if(url!=NULL)
   {
    aux=new char[1+strlen(url)];
    for(i=0,j=0;i<strlen(url);i++)
     {
      if(url[i-1]!='.' || url[i]!='/')
       {
        aux[j]=url[i];
        j++;
       }
       else
        {
         if(url[i-2]=='/')
           j=j-1;
          else
           {
            if(url[i-2]=='.' && url[i-3]=='/')
             {
              for(j=j-3;j>0 && aux[j]!='/';j--);
              if(j==0)
                fprintf(stderr,"Warning: Malformed URL %s\n",url);
             }
             else
              {
               aux[j]=url[i];
               j++;
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
     
#ifdef TRAZANDO_URLHash
 if(result!=NULL)
   fprintf(stderr,"EndURLHash::FixURL(%s) -> %s\n",url,result);
  else
    fprintf(stderr,"EndURLHash::FixURL(%s) -> NULL\n",url);
#endif   
     
  return(result);
 }

unsigned int URLHash::InsertPerDepth(const char *origurl,void *content)
 {
  int pos,hashvalue;
  bool found;
  unsigned int result,i;
  unsigned int depth,j;
  char *url;
  NodeQueue *newone;
  
#ifdef TRAZANDO_URLHash
  if(content!=NULL)
    fprintf(stderr,"BeginURLHash::InsertPerDepth(%s,%p)\n",origurl,content);
   else
     fprintf(stderr,"BeginURLHash::InsertPerDepth(%s,NULL)\n",origurl);
#endif

  result=0;
  found=false;
  url=FixURL(origurl);
  hashvalue=HashFunction(url);
  pos=hashvalue;
  if(pos==0)
    pos++;
  i=0;
  do
   {
    if(hash[pos]==NULL)
     { //Hemos dado con una posicion libre del vector asi que insertamos aqui
      hash[pos]=new URLContent;
      hash[pos]->urlcontent=new char[1+strlen(url)];
      strcpy(hash[pos]->urlcontent,url);
      if(content==NULL)
       {
        hash[pos]->visited=false;
        depth=Depth(origurl);
        
        if(depth>=kMaxDeepTree)
          depth=kMaxDeepTree-1;
        
        //Insertamos la nueva url en la cola
        if(levelshead[depth]!=NULL)
         {
          newone=new NodeQueue;
          newone->place=pos;
          newone->sig=levelshead[depth]->sig;
          if(levelshead[depth]->sig==NULL)
            queuetail=newone;
          levelshead[depth]->sig=newone;
          levelshead[depth]=newone;
          for(j=depth+1;j<kMaxDeepTree;j++)
           {
            if(levelshead[depth]==levelshead[j])
              levelshead[j]=newone;
           }
         }
         else
          {
           if(queuehead==NULL)
            {//La cola estaba vacia
             queuehead=new NodeQueue;
             queuehead->place=pos;
             queuehead->sig=NULL;
             for(j=depth;j<kMaxDeepTree;j++)
               levelshead[j]=queuehead;
            }
            else
             {//Se inserta al principio
              levelshead[depth]=new NodeQueue;
              levelshead[depth]->place=pos;
              levelshead[depth]->sig=queuehead;
              queuehead=levelshead[depth];
              depthhead=depth;
             }
          }
        urlsinqueue++;
       }
       else
         hash[pos]->visited=true;
      
      hash[pos]->content=content;
      urlnumber++;
      found=true;
      result=pos;
     }
     else
      {
       if(strcmp(hash[pos]->urlcontent,url)!=0)
        { //La posicion que nos ha tocado esta ocupada por otra url
         i++;
         pos=(hashvalue+i*kRedisperseHash)%sizevector;
         if(pos==hashvalue || i>sizevector)
          { //El vector estaba lleno y el usuario no cabe
           found=true;
          }
          else
           {
            if(pos==0)
              pos++;
           } 
        }
        else
         { //La url ya estaba en la hash asi que la actualizamos si procede
          hash[pos]->content=content;
          found=true;
          result=pos;
         }
      }
   } while(!found);  
  delete url;

#ifdef TRAZANDO_URLHash
  if(content!=NULL)
    fprintf(stderr,"EndURLHash::InsertPerDepth(%s,%p) -> %d\n",origurl,content,result);
   else
     fprintf(stderr,"EndURLHash::InsertPerDepth(%s,NULL) -> %d\n",origurl,result);
#endif    
    
  return(result);
 }


unsigned int URLHash::InsertEnd(const char *origurl,void *content)
 {
  int pos,hashvalue;
  bool found;
  unsigned int result,i;
  char *url;
  
#ifdef TRAZANDO_URLHash
  if(content!=NULL)
    fprintf(stderr,"BeginURLHash::InsertEnd(%s,%p)\n",origurl,content);
   else
     fprintf(stderr,"BeginURLHash::InsertEnd(%s,NULL)\n",origurl);
#endif

  result=0;
  found=false;
  url=FixURL(origurl);
  hashvalue=HashFunction(url);
  pos=hashvalue;
  if(pos==0)
    pos++;
  i=0;
  do
   {
    if(hash[pos]==NULL)
     { //Hemos dado con una posicion libre del vector asi que insertamos aqui
      hash[pos]=new URLContent;
      hash[pos]->urlcontent=new char[1+strlen(url)];
      strcpy(hash[pos]->urlcontent,url);
      if(content==NULL)
       {
        hash[pos]->visited=false;
        //Insertamos la nueva url en la cola
        if(queuetail!=NULL)
         {
          queuetail->sig=new NodeQueue;
          queuetail->sig->place=pos;
          queuetail->sig->sig=NULL;
          queuetail=queuetail->sig;
         }
         else
          {//La cola estaba vacia
           queuehead=new NodeQueue;
           queuetail=queuehead;
           queuetail->place=pos;
           queuetail->sig=NULL;
          }
        urlsinqueue++;
       }
       else
         hash[pos]->visited=true;
      
      hash[pos]->content=content;
      urlnumber++;
      found=true;
      result=pos;
     }
     else
      {
       if(strcmp(hash[pos]->urlcontent,url)!=0)
        { //La posicion que nos ha tocado esta ocupada por otra url
         i++;
         pos=(hashvalue+i*kRedisperseHash)%sizevector;
         if(pos==hashvalue || i>sizevector)
          { //El vector estaba lleno y la url no cabe
           found=true;
          }
          else
           {
            if(pos==0)
              pos++;
           } 
        }
        else
         { //La url ya estaba en la hash asi que la actualizamos si procede
          hash[pos]->content=content;
          found=true;
          result=pos;
         }
      }
   } while(!found);  
  delete url;

#ifdef TRAZANDO_URLHash
  if(content!=NULL)
    fprintf(stderr,"EndURLHash::InsertEnd(%s,%p) -> %d\n",origurl,content,result);
   else
     fprintf(stderr,"EndURLHash::InsertEnd(%s,NULL) -> %d\n",origurl,result);
#endif    
    
  return(result);
 }

unsigned int URLHash::Search(const char* url)
 {
  int pos,hashvalue;
  bool found;
  unsigned int result,i;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Search(%s)\n",url);
#endif    
  
  result=0;
  found=false;
  hashvalue=HashFunction(url);
  pos=hashvalue;
  if(pos==0)
    pos++;
  i=0;
  do
   {
    if(hash[pos]!=NULL && strcmp(hash[pos]->urlcontent,url)==0)
     { //Ya lo hemos encontrado
      result=pos;
      found=true;
     }
     else
      {
       if(hash[pos]==NULL)
         found=true; //Hemos llegado a una posicion vacia y no es la buscada, o sea que no se encuentra
        else
         {          
          if(i<sizevector)
           { //Buscamos en otra posicion del vector
            i++;
            pos=(hashvalue+i)%sizevector;
            if(pos==0)
              pos++;
           }
           else
             found=true; //Hemos recorrido todo el vector sin encontrar la url
         }
      }
   } while(!found);
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::Search(%s) -> %d\n",url,result);
#endif    
  
  return(result);
 }
 
char* URLHash::Search(unsigned int idurl)
 {
  char *result;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Search(%d)\n",idurl);
#endif    
    
  if(idurl>0 && idurl<sizevector && hash[idurl]!=NULL && hash[idurl]->urlcontent!=NULL)
   {     
    result=new char[1+strlen(hash[idurl]->urlcontent)];
    strcpy(result,hash[idurl]->urlcontent);
   }
   else
     result=NULL;

#ifdef TRAZANDO_URLHash
  if(result!=NULL)
    fprintf(stderr,"EndURLHash::Search(%d) -> %s\n",idurl,result);
   else
     fprintf(stderr,"EndURLHash::Search(%d) -> NULL\n",idurl);
#endif    
  
  return(result);
 }

unsigned int URLHash::NextNode()
 {  
  unsigned int result,i;
  NodeQueue *aux;
  bool found;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::NextNode()\n");
#endif
    
  if(urlsinqueue>0 && queuehead!=NULL)
   {
    result=queuehead->place;
    for(i=depthhead,found=false;i<kMaxDeepTree && !found;i++)
     {
      if(levelshead[depthhead]==queuehead)
       {
        levelshead[depthhead]=NULL;
        depthhead++;
       }
       else
         found=true;
     }
    aux=queuehead;
    queuehead=queuehead->sig;
    if(queuehead==NULL)
      queuetail=NULL;
    delete aux;
    urlsinqueue--;
   }
   else
     result=0;
     
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::NextNode() -> %d\n",result);
#endif    
     
  return(result);
 }

void* URLHash::Content(unsigned int idurl)
 {
  void *result;
  
#ifdef TRAZANDO_URLHash
  //fprintf(stderr,"BeginURLHash::Content(%d)\n",idurl);
#endif    
  
  if(idurl>0 && idurl<sizevector && hash[idurl]!=NULL)
    result=hash[idurl]->content;
   else
     result=NULL;
     
#ifdef TRAZANDO_URLHash
  //fprintf(stderr,"EndURLHash::Content(%d) -> %p\n",idurl,result);
#endif    
     
  return(result);
 }

bool URLHash::SetContentNULL(unsigned int idurl)
 {
  bool result;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::SetContentNULL(%d)\n",idurl);
#endif    
    
  if(idurl>0 && idurl<sizevector && hash[idurl]!=NULL)
   {
    hash[idurl]->content=NULL;
    result=true;
   }
   else
     result=false;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::SetContentNULL(%d) -> %d\n",idurl,result);
#endif    
       
  return(result);
 } 

unsigned int URLHash::NURLs()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::NURLs()\n");
#endif    
 
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::NURLs() -> %d\n",urlnumber);
#endif    
  
  return(urlnumber);
 }

unsigned int URLHash::Size()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Size()\n");
#endif    
 
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::Size() -> %d\n",sizevector);
#endif    
 
  return(sizevector);
 } 

unsigned int URLHash::QueueSize()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::QueueSize()\n");
#endif
 
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::QueueSize() -> %d\n",urlsinqueue);
#endif
 
  return(urlsinqueue);
 }  
  
unsigned int URLHash::Depth(const char* url)
 {
  unsigned int result,i;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Depth(%s)\n",url);
#endif    
     
  if(url!=NULL)
   {
    for(i=0,result=0;i<strlen(url) && url[i]!='#';i++)
     {
      if(url[i]=='/')
        result++;
     }
    result++;
   }
   else
     result=0;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::Depth(%s) -> %d\n",url,result);
#endif    
          
  return(result);
 }

unsigned int URLHash::Depth(unsigned int idurl)
 {
  unsigned int result;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"BeginURLHash::Depth(%d)\n",idurl);
#endif    
    
  if(idurl>0 && idurl<sizevector && hash[idurl]!=NULL)
   {
    result=Depth(hash[idurl]->urlcontent);
   }
   else
     result=0;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"EndURLHash::Depth(%d) -> %d\n",idurl,result);
#endif    
  
  return(result);
 }
