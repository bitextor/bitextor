#include "downloadmanager.h"

int sonspin;

void KillAlarmTimer(int sig)
 {//This function is executed when the timer for wget expires
  kill(sonspin,SIGTERM);
#ifndef HIDEMESSAGES
  fprintf(stderr,"Killing WGET...\n");
#endif  
 }


DownloadManager::DownloadManager()
 {
  unsigned int i; 
  char directoryname[1000];
 
  //Creation of the general cache directory (if it did not exist)
  strcpy(directoryname,kLocalDirectory);
  if(directoryname[strlen(directoryname)-1]=='/')
    directoryname[strlen(directoryname)-1]='\0';//Removing the ending '/'
  mkdir(directoryname,0777);
  
  //Creation of the renaming cache directory (if it did not exist)
  strcat(directoryname,"/");
  strcat(directoryname,kURLRenamePrefix);
  if(directoryname[strlen(directoryname)-1]=='/')
    directoryname[strlen(directoryname)-1]='\0';//Removing the ending '/'
  mkdir(directoryname,0777);
  
  cachefaults=0;
  cachehits=0;
  downloadedpages=0;
  notfoundpages=0;
  cacheread=true;
  cachegeneration=true;
  onlycache=false;
  
  sizebigurlhash=kSizeCacheURLHash;
  bigurlhash=new char*[sizebigurlhash];
  for(i=0;i<sizebigurlhash;i++)
    bigurlhash[i]=NULL;
  n_urlsinhash=0;
  
  bigurlhashfilename=new char[2+strlen(kLocalDirectory)+strlen(kHashfilename)];
  
  strcpy(bigurlhashfilename,kLocalDirectory);
  if(bigurlhashfilename[strlen(bigurlhashfilename)-1]!='/')
    strcat(bigurlhashfilename,"/");
  strcat(bigurlhashfilename,kHashfilename);
  LoadHashCache(bigurlhashfilename);
  
#ifdef CONF_FILE
  scriptspath=new char[1+strlen(CONF_FILE)];
  strcpy(scriptspath,CONF_FILE);
#endif
#ifndef CONF_FILE  
  scriptspath=new char[2];
  strcpy(scriptspath,".");
#endif
 }
 
DownloadManager::~DownloadManager()
 {
  unsigned int i;
  
  for(i=0;i<sizebigurlhash;i++)
   {
    if(bigurlhash[i]!=NULL)
      delete bigurlhash[i];
   }
  delete bigurlhash;
  bigurlhash=NULL;  
  delete scriptspath;
  scriptspath=NULL;
 }
  
int DownloadManager::HashFunction(const char *str)
 {
  int result;
  unsigned int i;
  
  result=0;
  if(str!=NULL)
   {
    for(i=0;i<strlen(str);i++)
      result=result+(i+1)*((int) str[i]);
    result=result%sizebigurlhash;
   }
  return(result);
 } 
 
unsigned int DownloadManager::InsertHashCache(const char *url)
 {
  int pos,hashvalue;
  bool found;
  unsigned int result,i;
  
  result=0;
  found=false;
  hashvalue=HashFunction(url);
  pos=hashvalue;
  if(pos==0)
    pos++;
  i=0;
  do
   {
    if(bigurlhash[pos]==NULL)
     { //Hemos dado con una posicion libre del vector asi que insertamos aqui
      bigurlhash[pos]=new char[1+strlen(url)];
      strcpy(bigurlhash[pos],url);
      n_urlsinhash++;
      found=true;
      result=pos;
     }
     else
      {
       if(strcmp(bigurlhash[pos],url)!=0)
        {//La posicion que nos ha tocado esta ocupada por otra url
         i++;
         pos=(hashvalue+i*kRedisperseHash)%sizebigurlhash;
         if(pos==hashvalue || i>sizebigurlhash)
          {//La hash estaba llena y la url no cabe
           found=true;
          }
          else
           {
            if(pos==0)
              pos++;
           } 
        }
        else
         {//La url ya estaba en la hash asi que devolvemos su posicion
          found=true;
          result=pos;
         }
      }
   } while(!found);  
    
  return(result);
 }

unsigned int DownloadManager::SearchHashCache(const char* url)
 {
  int pos,hashvalue;
  bool found;
  unsigned int result,i;

  result=0;
  found=false;
  hashvalue=HashFunction(url);
  pos=hashvalue;
  if(pos==0)
    pos++;
  i=0;
  do
   {
    if(bigurlhash[pos]!=NULL && strcmp(bigurlhash[pos],url)==0)
     { //Ya lo hemos encontrado
      result=pos;
      found=true;
     }
     else
      {
       if(bigurlhash[pos]==NULL)
         found=true; //Hemos llegado a una posicion vacia y no es la buscada, o sea que no se encuentra
        else
         {          
          if(i<sizebigurlhash)
           { //Buscamos en otra posicion de la hash
            i++;
            pos=(hashvalue+i)%sizebigurlhash;
            if(pos==0)
              pos++;
           }
           else
             found=true; //Hemos recorrido todo el vector sin encontrar la url
         }
      }
   } while(!found);
  
  return(result);
 }

bool DownloadManager::SaveHashCache(const char *filename)
 {
  bool result;
  unsigned int i;
  FILE *fout;
  
  if(filename!=NULL)
   {
    if((fout=fopen(filename,"w"))==NULL)
     {
      result=false;
      fprintf(stderr,"Error: The output long URL cache file \"%s\" could not be opened\n",filename);
     }
     else
      {
       for(i=1;i<sizebigurlhash;i++)
        {
         if(bigurlhash[i]!=NULL)
           fprintf(fout,"%d %s\n",i,bigurlhash[i]);
        }
       result=true;
       fclose(fout);
      }
   }
   else
    {
     result=false;
     fprintf(stderr,"Error: Filename to save the long URL cache was not specified\n");
    }  
  return(result);
 }
 
bool DownloadManager::LoadHashCache(const char *filename)
 {
  bool result;
  FILE *fin;
  char buffer[kSizeBufferUtils];
  unsigned int i,hashposition;
  
  result=false;
  if(filename!=NULL)
   {
    if((fin=fopen(filename,"r"))==NULL)
     {
      result=false;
      fprintf(stderr,"Warning: The input URL cache file \"%s\" could not be opened\n",filename);
     }
     else
      { //Reset of the hash table
       for(i=0;i<sizebigurlhash;i++)
        {
         if(bigurlhash[i]!=NULL)
          {
           delete bigurlhash[i];
           bigurlhash[i]=NULL;
          }
        }
       
       while(!feof(fin))
        {
         for(i=0,buffer[i]=fgetc(fin);!feof(fin) && buffer[i]!=' ';i++,buffer[i]=fgetc(fin));
         buffer[i]='\0';
         if(strlen(buffer)>0)
          {
           for(i=0,hashposition=0,result=true;result && i<strlen(buffer);i++)
            {
             if(!isdigit(buffer[i]))
              {
               result=false;
               fprintf(stderr,"Error: Invalid format while loading long URL cache file (hash position)\n");
              }
              else
                hashposition=hashposition*10+(buffer[i]-48);
            }
           for(i=0,buffer[i]=fgetc(fin);!feof(fin) && buffer[i]!='\n';i++,buffer[i]=fgetc(fin));
           buffer[i]='\0';
           if(strlen(buffer)>0)
            {
             if(bigurlhash[hashposition]!=NULL)
              {
               fprintf(stderr,"Warning: Two urls share the same hash position\n");
               delete bigurlhash[hashposition];
               n_urlsinhash--;
              }
             bigurlhash[hashposition]=new char[1+strlen(buffer)];
             strcpy(bigurlhash[hashposition],buffer);
             n_urlsinhash++;
            }
            else
             {
              result=false;
              fprintf(stderr,"Error: Invalid format while loading long URL cache file (url)\n");
             }
          }
        }
      }
   }
   else
     fprintf(stderr,"Error: Filename to does not exist when loading long URL cache\n");
  return(result);
 }
 
void DownloadManager::Reset() 
 {
  cachefaults=0;
  cachehits=0;
  downloadedpages=0;
  notfoundpages=0;
 }  

int DownloadManager::SizeURL(const char *origurl)
 {
  int result;
  int i,j;
  bool found;
  char buffer[kSizeBufferUtils];//Buffer para la lectura del fichero
  char sizefile[kSizeInt]; //Buffer para almacenar la longitud del fichero a convertir a entero
  unsigned int bytesleidos;//Bytes rellenados en el buffer
  int pidactual;//PID del programa actual para crear el fichero temporal
  char tempfilename[60];//Nombre que tendra el fichero temporal que se creara
  FILE *fent;
  char systemcall[1000];//Nombre de la llamada al sistema que se realiza al tidy
  char *url; //Auxiliar para tratar con la url
  
#ifdef TRAZANDO_DownloadManager
  if(origurl!=NULL)
    fprintf(stderr,"BeginSizeURL(%s)\n",origurl);
   else
     fprintf(stderr,"BeginSizeURL(NULL)\n");
#endif  
  
  url=RemoveAmpEntities(origurl);//Quitamos las entidades &amp;
  if(((url[0]=='j' || url[0]=='J') && //No se aceptan si empiezan por 'Java' 
      (url[1]=='a' || url[1]=='A') &&
      (url[2]=='v' || url[2]=='V') &&
      (url[3]=='a' || url[3]=='A')) ||
     ((url[0]=='m' || url[0]=='M') && //No se aceptan si empiezan por 'Mailto:'
      (url[1]=='a' || url[1]=='A') &&
      (url[2]=='i' || url[2]=='I') &&
      (url[3]=='l' || url[3]=='L') &&
      (url[4]=='t' || url[4]=='T') &&
      (url[5]=='o' || url[5]=='O') &&
       url[6]==':') ||
     ((url[0]=='h' || url[0]=='H') && //No se aceptan si empiezan por https
      (url[1]=='t' || url[1]=='T') &&
      (url[2]=='t' || url[2]=='T') &&
      (url[3]=='p' || url[3]=='P') &&
      (url[4]=='s' || url[4]=='S')))
    result=-1;
   else
    {
  
     pidactual=getpid();
     sprintf(tempfilename,"/tmp/genericutils%d",pidactual);
     sprintf(systemcall,"wget -nv --spider -T 15 -t 20 \"%s\" 2> %s",url,tempfilename);

#ifndef HIDEMESSAGES
     if(strlen(url)>50)
       fprintf(stderr,"Checking ...%s\n",url+strlen(url)-50);
      else
        fprintf(stderr,"Checking %s\n",url);
#endif  
  
     system(systemcall);
  
     if(!(fent=fopen(tempfilename,"r")))
      {
       result=-2;
       fprintf(stderr,"Error: Temporal file %s could not be created\n",tempfilename);
      }
      else
       {
        bytesleidos=fread(buffer,1,kSizeBufferUtils,fent);
        buffer[bytesleidos]='\0';
     
        //Recorremos el fichero hacia atras hasta encontrar un numero
        for(found=false,i=bytesleidos-1;i>0 && !found;i--)
         {
          if(buffer[i]>='0' && buffer[i]<='9')
            found=true;
         }
     
        //Ahora buscamos el inicio del numero
        for(found=false;i>1 && !found;i--)
         {
          if(!(buffer[i-1]>='0' && buffer[i-1]<='9'))
            found=true;
         }
        i++;
        if(found && buffer[i]=='2' && buffer[i+1]=='0' && buffer[i+2]=='0' &&
           buffer[i+3]==' ' && buffer[i+4]=='O' && buffer[i+5]=='K')
         {//Se ha accedido sin problemas a la URL asi que buscamos el tamaño
          for(found=false;i>9 && !found;i--)
           {
            if(buffer[i-9]=='L' && buffer[i-8]=='o' &&
               buffer[i-7]=='n' && buffer[i-6]=='g' &&
               buffer[i-5]=='i' && buffer[i-4]=='t' &&
               buffer[i-3]=='u' && buffer[i-2]=='d' &&
               buffer[i-1]==':' && buffer[i]==' ')
              found=true;
           }
          for(found=false,i=i+2,j=0;i<(int)strlen(buffer) && j<kSizeInt && !found;j++,i++)
           {
            if(buffer[i]>='0' && buffer[i]<='9')
              sizefile[j]=buffer[i];
             else
              {
               if(buffer[i]!=',')
                 found=true;
                else
                  j--;
              }
           }
          sizefile[j]='\0';
          
          //Nos aseguramos que el tipo del fichero sea [text/html]
          for(found=false,j=strlen(buffer);i<(j-11) && !found;i++)
           {
            if(buffer[i]=='[' && buffer[i+1]=='t' && buffer[i+2]=='e' &&
               buffer[i+3]=='x' && buffer[i+4]=='t' && buffer[i+5]=='/' &&
               (buffer[i+6]=='h' || buffer[i+6]=='p') &&
               (buffer[i+7]=='t' || buffer[i+7]=='l') &&
               (buffer[i+8]=='m' || buffer[i+8]=='a') &&
               (buffer[i+9]=='l' || buffer[i+9]=='i') &&
               (buffer[i+10]==']' || buffer[i+10]=='n'))
              found=true;
           }
          if(found)
            result=strtol(sizefile,NULL,10);
           else
            {
             result=-3;//La pagina no era del formato que queriamos

#ifndef HIDEMESSAGES
             if(strlen(url)>50)
               fprintf(stderr,"Discarding ...%s\n",url+strlen(url)-50);
              else
                fprintf(stderr,"Discarding %s\n",url);
#endif  
             
            }
         }
         else
          {//Hubo algun error, probablemente porque la pagina no existia
           result=-1;
          }
     
        fclose(fent);
        sprintf(systemcall,"rm /tmp/genericutils%d",pidactual);
        system(systemcall);
       }
    
    }
  delete url;
#ifdef TRAZANDO_DownloadManager
  if(origurl!=NULL)
    fprintf(stderr,"EndSizeURL(%s) -> %d\n",origurl,result);
   else
     fprintf(stderr,"EndSizeURL(NULL) -> %d\n",result);
#endif  
  
  return(result);
 }
 
bool DownloadManager::DownloadURL(const char *origurl,const char *localfile)
 {
  bool result;
  char systemcall[kSizeAuxStrings];//System call that is performed to use the tidy and other programs
  char localfilename[kSizeAuxStrings];//Filename already downloaded in hard disk
  char localdirname[kSizeAuxStrings];//Path of the filename already downloaded in hard disk
  char auxlocalfilename[kSizeAuxStrings];
  FILE *fent,*fent2;
  char *url,*url2; //Auxiliar for handling the url
  int status,i;
  struct itimerval t1; //Variable for the timing
  bool realdownload;
  int bytesleidos;
  bool longfilename; //Indicates that the file can not be saved directly in the cache
  
#ifdef TRAZANDO_DownloadManager
  if(origurl!=NULL)
   {
    if(localfile!=NULL)
      fprintf(stderr,"BeginDownloadURL(%s,%s)\n",origurl,localfile);
     else
       fprintf(stderr,"BeginDownloadURL(%s,NULL)\n",origurl);
   }
   else
    {
     if(localfile!=NULL)
      fprintf(stderr,"BeginDownloadURL(NULL,%s)\n",localfile);
     else
       fprintf(stderr,"BeginDownloadURL(NULL,NULL)\n");
    }
#endif  
  url2=RemoveAmpEntities(origurl);//Replacing the "&amp;" entities per "&"
  url=RemoveURLReference(url2);
  delete url2;
  result=true;
  if((fent=fopen(localfile,"r")))
   {//If the file would exist it would be erased
    fclose(fent);
    if(unlink(localfile)!=0)
     {
      if(unlink(localfile)!=0)
       {
        if(strlen(url)>50)
          fprintf(stderr,"Error: Erasing  ...%s\n",url+strlen(url)-50);
         else
           fprintf(stderr,"Error: Erasing %s\n",url);
        result=false;
       }
     }
   }
  
  if(strlen(url)<kMaxSizeCacheURL && url[strlen(url)-1]!='/')
    longfilename=false;
   else
     longfilename=true;
      
  if(result)
   {
    strcpy(localfilename,kLocalDirectory);
    if(localfilename[strlen(localfilename)-1]!='/')
      strcat(localfilename,"/");
    strcat(localfilename,url);
    if(cacheread)
     {
      if(!longfilename)
       {
        realdownload=true;
        if(!(fent2=fopen(localfilename,"r")))
         {//We try to concatenate the ".html" extension to the file, just in case it were a webpage with parameters
          sprintf(auxlocalfilename,"%s%s",localfilename,".html");
          if(!(fent2=fopen(auxlocalfilename,"r")))
            realdownload=true;
           else
            {
             strcat(localfilename,".html");
             realdownload=false;
            }
         }
         else
           realdownload=false;
       }
       else
        {//The filename is very long to be stored in the cache with the same name, so a search in
         //the hash of renamed files is perform
         i=SearchHashCache(url);
         if(i!=0)
          {//The filename was stored in the cache with a different name
           strcpy(localfilename,kLocalDirectory);
           if(localfilename[strlen(localfilename)-1]!='/')
             strcat(localfilename,"/");
           sprintf(localfilename,"%s%s/%s%d",localfilename,kURLRenamePrefix,kURLRenamePrefix,i);
           if(!(fent2=fopen(localfilename,"r")))
             realdownload=true;//The file was not found (error of the cache)
            else
              realdownload=false;
          }
          else
           {//The filename is not stored in the cache
            realdownload=true;
           }
        }
     }
     else
       realdownload=true;
     
    if(realdownload)
     {//If the file did not exist is that it could not be downloaded. So we try again and store it
      //in the cache
      if(!onlycache)
       {
#ifndef HIDEMESSAGES
        if(strlen(url)>50)
          fprintf(stderr,"Downloading ...%s\n",url+strlen(url)-50);
         else
           fprintf(stderr,"Downloading %s\n",url);
#endif
        //Split file and path where the webpage will be stored inside the cache
        strcpy(localdirname,localfilename);
        for(i=strlen(localdirname)-1;localdirname[i]!='/' && i>0;i--);
        localdirname[i]='\0';
        memmove(localfilename,localfilename+i+1,strlen(localfilename)-i);
      
        if(strlen(localfilename)>kMaxSizeNameFile || !cachegeneration || longfilename)
         {
          if(!longfilename)
            sprintf(systemcall,"%s/downloadpage.sh \"%s\" \"%s\"",scriptspath,url,localfile);//It will not be stored in the cache
           else
            {
             i=InsertHashCache(url);
             sprintf(systemcall,"%s/downloadandcachepage.sh \"%s\" \"%s\" \"%s\" \"%s/%s%d\"",scriptspath,url,localfile,kLocalDirectory,kURLRenamePrefix,kURLRenamePrefix,i);//It will be stored in the cache with a different name
             SaveHashCache(bigurlhashfilename);
            }
         }
         else
          {
           if(strlen(localfilename)>0)
             sprintf(systemcall,"%s/downloadandcachepage.sh \"%s\" \"%s\" \"%s\" \"%s\"",scriptspath,url,localfile,localdirname,localfilename);//It will be stored in the cache
            else
              sprintf(systemcall,"%s/downloadandcacheserverpage.sh \"%s\" \"%s\" \"%s\"",scriptspath,url,localfile,localdirname);//It will be stored in the cache with the name provided by the server
          }
        t1.it_interval.tv_sec=kTimeWaitingWGET;
        t1.it_interval.tv_usec=0;
        t1.it_value.tv_sec=kTimeWaitingWGET;
        t1.it_value.tv_usec=0;
        sonspin=fork();
        if(sonspin!=0)
         {//Father process
        
          //Turn on the wget timer
          signal(SIGALRM,KillAlarmTimer); //Assign the handler
          setitimer(ITIMER_REAL,&t1,0); //Program the alarm timer
          t1.it_interval.tv_sec=0;
          t1.it_interval.tv_usec=0;
          t1.it_value.tv_sec=0;
          t1.it_value.tv_usec=0;

          waitpid(-1,&status,0);
          setitimer(ITIMER_REAL,&t1,0);//Turn on the alarm timer
          if(status!=0)
            result=false;
           else
            {
             if(!(fent=fopen(localfile,"r")))
              {
               if(strlen(url)>50)
                 fprintf(stderr,"Error: Opening  ...%s\n",url+strlen(url)-50);
                else
                  fprintf(stderr,"Error: Opening %s\n",url);
               result=false;
               notfoundpages++;
              }
              else
               {              
                bytesleidos=fread(systemcall,1,10,fent);
                if(bytesleidos<10)//Check that the webpage has something
                 {
                  if(strlen(url)>50)
                    fprintf(stderr,"Error: Not found  ...%s\n",url+strlen(url)-50);
                   else
                     fprintf(stderr,"Error: Not found %s\n",url);
                  result=false;
                  notfoundpages++;
                 }
                 else
                  {
                   result=true;
                   cachefaults++;                   
                  }
                fclose(fent);
               }
            }
         }
         else
          {//Son Process
           status=system(systemcall);
           exit(status);
          }
       }
       else
        {
         if(strlen(url)>50)
           fprintf(stderr,"Error: Not found  ...%s\n",url+strlen(url)-50);
          else
            fprintf(stderr,"Error: Not found %s\n",url);
         result=false;
         notfoundpages++;
        }
     }
     else
      {//The file was found in the cache
       cachehits++;
       fclose(fent2);       
#ifndef HIDEMESSAGES
       if(strlen(url)>50)
         fprintf(stderr,"Copying ...%s\n",url+strlen(url)-50);
        else
          fprintf(stderr,"Copying %s\n",url);
#endif
       if(link(localfilename,localfile)!=0)
        {
         if(link(localfilename,localfile)!=0)
          {
           if(strlen(url)>50)
             fprintf(stderr,"Error: Copying ...%s\n",url+strlen(url)-50);
            else
              fprintf(stderr,"Error: Copying %s\n",url);
           result=false;
          }
        }
      }
    }
  delete url;
#ifdef TRAZANDO_DownloadManager
  if(origurl!=NULL)
   {
    if(localfile!=NULL)
      fprintf(stderr,"EndDownloadURL(%s,%s) -> %d\n",origurl,localfile,result);
     else
       fprintf(stderr,"EndDownloadURL(%s,NULL) -> %d\n",origurl,result);
   }
   else
    {
     if(localfile!=NULL)
      fprintf(stderr,"EndDownloadURL(NULL,%s) -> %d\n",localfile,result);
     else
       fprintf(stderr,"EndDownloadURL(NULL,NULL) -> %d\n",result);
    }
#endif    
  return(result);
 } 
 
bool DownloadManager::IsGoodExtension(const char* ext)
 {
  bool result;
  
#ifdef TRAZANDO_DownloadManager
  if(ext!=NULL)
    fprintf(stderr,"BeginIsGoodExtension(%s)\n",ext);
   else
     fprintf(stderr,"BeginIsGoodExtension(NULL)\n");
#endif    
  
  result=false;
  if(ext!=NULL)
   {
    if(strlen(ext)==0)
      result=true;
     else
      {
       switch(ext[0])
        {
         case 'a': if(ext[1]=='s')
                    {
                     if((ext[2]=='c' || ext[2]=='p') && ext[3]=='\0')
                       result=true;
                    }
                   break;
         case 'c': if(ext[1]=='g' && ext[2]=='i' && ext[3]=='\0')
                     result=true;
                   break;
         case 'h': if(ext[1]=='t' && ext[2]=='m' && 
                      (ext[3]=='\0' || (ext[3]=='l' && ext[4]=='\0')))
                     result=true;
                   break;
         case 'j': if(ext[1]=='s' && ext[2]=='p' && ext[3]=='\0')
                     result=true;
                   break;
         case 'p': if(ext[1]=='h' && ext[2]=='p' && ext[3]=='\0')
                     result=true;
                   break;
         case 't': if(ext[1]=='x' && ext[2]=='t' && ext[3]=='\0')
                     result=true;
                   break;
         case 'x': if(ext[1]=='m' && ext[2]=='l' && ext[3]=='\0')
                     result=true;
                   break;
        }
      }     
   }
  
#ifdef TRAZANDO_DownloadManager
  if(ext!=NULL)
    fprintf(stderr,"EndIsGoodExtension(%s) -> %d\n",ext,result);
   else
     fprintf(stderr,"EndIsGoodExtension(NULL) -> %d\n",result);
#endif    
  return(result);
 }

