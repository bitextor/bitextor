/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2005
*/

#include "orderedlist.h"

OrderedList2Levels::OrderedList2Levels()
 {  
  unsigned int i;
  OrderedList *aux;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::OrderedList2Levels()\n");
#endif  
  upperlistsize=kIniListNumber;
  growing=kGrowing;
  inimaxvalue=kIniMaxValue;
  orderedlist=new OrderedList;
  orderedlist->value=0;
  orderedlist->inside=NULL;
  orderedlist->sizeinside=0;
  orderedlist->next=NULL;
  for(i=1,aux=orderedlist;i<upperlistsize;i++)
   {
    aux->next=new OrderedList;
    aux=aux->next;
    aux->value=(unsigned int)(i*inimaxvalue/upperlistsize);
    aux->inside=NULL;
    aux->sizeinside=0;
    aux->next=NULL;
   }
  totalelements=0;
  searchresult=NULL;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::OrderedList2Levels()\n");
#endif    
 }

OrderedList2Levels::OrderedList2Levels(unsigned int balancegrowing,unsigned int sublistnumber,unsigned int initialmaxvalue)
 {  
  unsigned int i;
  OrderedList *aux;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::OrderedList2Levels(%d,%d,%d)\n",growing,sublistnumber,inimaxvalue);
#endif  
  upperlistsize=sublistnumber;
  growing=balancegrowing;
  inimaxvalue=initialmaxvalue;
  orderedlist=new OrderedList;
  orderedlist->value=0;
  orderedlist->inside=NULL;
  orderedlist->sizeinside=0;
  orderedlist->next=NULL;
  for(i=1,aux=orderedlist;i<upperlistsize;i++)
   {
    aux->next=new OrderedList;
    aux=aux->next;
    aux->value=(unsigned int)(i*inimaxvalue/upperlistsize);
    aux->inside=NULL;
    aux->sizeinside=0;
    aux->next=NULL;
   }
  totalelements=0;
  searchresult=NULL;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::OrderedList2Levels(%d,%d,%d)\n",growing,sublistnumber,inimaxvalue);  
#endif    
 }
 
 
OrderedList2Levels::~OrderedList2Levels()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::~OrderedList2Levels()\n");
#endif  
  
  Reset();  
  if(searchresult!=NULL)
    delete searchresult;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-URLOrderedList2Levels::~OrderedList2Levels()\n");
#endif   
 }
 
void OrderedList2Levels::Reset()
 {
  OrderedList *aux,*aux2;
  ItemList *aux3,*aux4;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::Reset()\n");
#endif   
  
  for(aux=orderedlist;aux!=NULL;aux=aux2)
   {
    if(aux->sizeinside>0)
     {//Deletion of the sublist
      for(aux3=aux->inside;aux3!=NULL;aux3=aux4)
       {
        aux4=aux3->next;
        delete aux3;
       }      
     }
    aux2=aux->next;
    delete aux;    
   }

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::Reset()\n");
#endif  
 }

void OrderedList2Levels::Insert(unsigned int value,unsigned int data)
 {
  OrderedList *aux,*ant;
  ItemList *aux2,*ant2,*newone;
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::Insert(%d,%d)\n",value,data);
#endif
  aux=orderedlist;
  for(ant=aux;aux!=NULL && aux->value<=value;ant=aux,aux=aux->next);
    
  //The insertion will take place in the sublist 'ant'    
  aux2=ant->inside;
  for(ant2=aux2;aux2!=NULL && aux2->value<=value;ant2=aux2,aux2=aux2->next);
  
  newone=new ItemList;
  newone->value=value;
  newone->data=data;    
  newone->next=aux2;
    
  if(ant2==aux2)
   {//Insert at the beginning
    ant->inside=newone;
   }
   else
    {//Insert in other place
     ant2->next=newone;
    }
  
  ant->sizeinside++;
  totalelements++;
  if(totalelements>=(pow(upperlistsize+growing,2)))
    Balance();
 
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::Insert(%d,%d) -> %d\n",value,data,result);
#endif    
 }

unsigned int OrderedList2Levels::Search(unsigned int value)
 {
  unsigned int result,counter,*auxsearchresult,sizesearchresult;
  OrderedList *aux,*ant,*ini,*last;
  ItemList *aux2,*first;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::Search(%d)\n",value);
#endif    

  result=0;
  if(searchresult!=NULL)
   {
    delete searchresult;
    searchresult=NULL;
   } 
  aux=orderedlist;
  ini=NULL;
  for(ant=aux;aux!=NULL && aux->value<=value;ant=aux,aux=aux->next)
   {
    if(aux->value!=value)
      ini=aux;
   }
  if(ini==ant)
   {//The set of occurences is inside the same sublist    
    for(aux2=ant->inside;aux2!=NULL && aux2->value<value;aux2=aux2->next);
    if(aux2!=NULL && aux2->value==value)
     {//The element was in the structure and its first occurence starts here
      first=aux2;
      result=1;
      for(aux2=aux2->next;aux2!=NULL && aux2->value==value;aux2=aux2->next,result++);
      searchresult=new unsigned int[result];
      searchresult[0]=first->data;
      for(aux2=first->next,counter=1;counter<result;counter++,aux2=aux2->next)
        searchresult[counter]=aux2->data;
     }
   }
   else
    {
     if(ini!=NULL)//The initial sublist must be existent
      {//The set of occurences is contained in several sublists
       last=ant;
       aux=ini;
       
       sizesearchresult=aux->sizeinside;
       for(aux2=aux->inside;aux2!=NULL && aux2->value<value;aux2=aux2->next,sizesearchresult--);
       if(aux2!=NULL && aux2->value==value)
        {//The element was in the structure and its first occurence starts here
         first=aux2;
         searchresult=new unsigned int[sizesearchresult];
         searchresult[0]=first->data;
         for(aux2=first->next,counter=1;counter<sizesearchresult;counter++,aux2=aux2->next)
           searchresult[counter]=aux2->data;
         result=sizesearchresult;
        }
        
       aux=aux->next;
       while(aux!=last)
        {//Intermediate sublists are included completely in the searchresult
         sizesearchresult=result+aux->sizeinside;
         auxsearchresult=new unsigned int[sizesearchresult];
         for(counter=0;counter<result;counter++)
           auxsearchresult[counter]=searchresult[counter];
         for(aux2=aux->inside;counter<sizesearchresult;counter++,aux2=aux2->next)
           auxsearchresult[counter]=aux2->data;
           
         delete searchresult;
         searchresult=auxsearchresult;
         result=sizesearchresult;
         auxsearchresult=NULL;
         aux=aux->next;       
        }
      
       //It only remains to add to the results the elements of the last sublist
       sizesearchresult=result;
       for(aux2=aux->inside;aux2!=NULL && aux2->value==value;aux2=aux2->next,sizesearchresult++);
       auxsearchresult=new unsigned int[sizesearchresult];
       for(counter=0;counter<result;counter++)
         auxsearchresult[counter]=searchresult[counter];     
       for(aux2=aux->inside,counter=result;counter<sizesearchresult;counter++,aux2=aux2->next)
         auxsearchresult[counter]=aux2->data;
              
       delete searchresult;
       searchresult=auxsearchresult;
       auxsearchresult=NULL;
       result=sizesearchresult;
      }
    }
  
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::Search(%d) -> %d\n",value,result);
#endif    
  
  return(result);
 }

const unsigned int* OrderedList2Levels::SearchResults()
 {
  return(searchresult);
 }
 
unsigned int OrderedList2Levels::Size()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::Size()\n");
  fprintf(stderr,"End-OrderedList2Levels::Size() -> %d\n",result);
#endif    
  
  return(totalelements);
 }

void OrderedList2Levels::Balance()
 {
  OrderedList *auxlist,*aux,*erasing;
  ItemList *aux2,*aux3;
  unsigned int i;

#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::Balance()\n");
#endif    
  
  auxlist=new OrderedList;
  auxlist->value=0;
  auxlist->inside=NULL;
  auxlist->sizeinside=0;
  auxlist->next=NULL;
  for(i=1,aux=auxlist;i<upperlistsize+growing;i++)
   {
    aux->next=new OrderedList;
    aux=aux->next;
    aux->value=0;
    aux->inside=NULL;
    aux->sizeinside=0;
    aux->next=NULL;
   }
  
  //Read through the whole structure to reorganize the lists
  aux=auxlist;
  aux->sizeinside=upperlistsize+growing;
  
  //Searching for the first element
  aux2=orderedlist->inside;
  while(aux2==NULL && orderedlist!=NULL)
   {
    erasing=orderedlist->next;
    delete orderedlist;
    orderedlist=erasing;
    if(orderedlist!=NULL)
      aux2=orderedlist->inside;
   }
  aux->inside=aux2;
  
  i=1;
  while(orderedlist!=NULL)
   {
    if(i%(upperlistsize+growing)==0)
     {//Start a new upper list
      aux3=aux2->next;
      aux2->next=NULL;      
      aux2=aux3;
      if(aux2==NULL)
       {
        while(aux2==NULL && orderedlist!=NULL)
         {
          erasing=orderedlist->next;
          delete orderedlist;
          orderedlist=erasing;
          if(orderedlist!=NULL)
             aux2=orderedlist->inside;
         }
       }
      aux=aux->next;
      if(aux!=NULL)
       {
        aux->sizeinside=upperlistsize+growing;
        aux->inside=aux2;
        if(aux2!=NULL)
          aux->value=aux2->value;
       }
     }
     else
      {       
       if(aux2->next==NULL)
        {
         while(aux2->next==NULL && orderedlist!=NULL)
          {
           erasing=orderedlist->next;
           delete orderedlist;
           orderedlist=erasing;
           if(orderedlist!=NULL)
             aux2->next=orderedlist->inside;
          }
         if(orderedlist!=NULL)
           aux2=aux2->next;
        }
        else
          aux2=aux2->next;        
      }
    i++;
   }
  orderedlist=auxlist;
  upperlistsize=upperlistsize+growing;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::Balance()\n");
#endif    
 
 }

void OrderedList2Levels::GoStart()
 {
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::GoStart()\n");
#endif
  actual=orderedlist;
  nextitem=NULL;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::GoStart()\n");
#endif    
 }

int OrderedList2Levels::GoNext()
 {
  int result;
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"Begin-OrderedList2Levels::GoNext()\n");
#endif
  if(actual==NULL)
   {//There is no more elements in the list
    result=-1;
   }
   else
    {
     if(nextitem==NULL)
      {//It is the first element of the list
       do
        {
         if(actual->sizeinside>0)
           nextitem=actual->inside;
         if(nextitem==NULL)
           actual=actual->next;
        } while(nextitem==NULL && actual!=NULL);
       if(actual==NULL)
         result=-1;
        else
         {
          result=nextitem->data;
          nextitem=nextitem->next;
          actual=actual->next;
         }
      }
      else
       {
        result=nextitem->data;
        nextitem=nextitem->next;
       }
    }
#ifdef TRAZANDO_URLHash
  fprintf(stderr,"End-OrderedList2Levels::GoNext() -> %d\n",result);
#endif
  return(result);
 }
