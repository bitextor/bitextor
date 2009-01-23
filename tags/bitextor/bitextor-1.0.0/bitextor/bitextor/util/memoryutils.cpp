#include "memoryutils.h"

/*
void* mynew(size_t size)
 {
  void *result;
  
  result=malloc(size);
  if(errno==ENOMEM)
   {//Reintentamos tras 1 segundo
    sleep(1);
    result=malloc(size);
    if(errno==ENOMEM)
     {
      fprintf(stderr,"FATAL ERROR: Memory could not be reserved\n"); //No queda memoria
      result=NULL;
     }
     else
      {//Metemos en una tabla que se ha reservado la memoria
       //NewMemoryReserved(result,size);
      }
   }
   else
    {//Metemos en una tabla que se ha reservado la memoria
     //NewMemoryReserved(result,size);
    }
  return(result);
 }
*/
 
void* mynew(size_t size)
 {
  void *result;
  
  result=(void*)new char[size];

  return(result);
 }

void mydelete(void *ptr)
 {  
//  if(IsReservedMemory(ptr))
//   {
    free(ptr);
//   }
//   else
//    {//Se ha intentado liberar memoria no reservada
//     fprintf(stderr,"FATAL ERROR: Memory could not be deleted because it was not reserved\n");
//    }
 }

