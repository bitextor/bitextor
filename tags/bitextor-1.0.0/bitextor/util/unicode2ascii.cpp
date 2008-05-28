/*

  Codigo fuente de la aplicacion que preprocesa los ficheros XML
  de la biblioteca para corregir los caracteres que no son ascii,
  sino que se encuentran en otra codificacion distinta, transformandolos
  a entidades html
  Implementado por Enrique Sanchez Villamil, Leví Tejedor Rico
  Año 2003
  Revisado por Enrique Sánchez Villamil en 2005

*/

#include <iostream>
#include <string>
#include <stdio.h>
#include <signal.h>

#define cerr std::cerr
#define cout std::cout
#define endl std::endl

#define kMAXNUMENT	10000	// Maximo nº que se le puede asignar a una entidad
#define kMAXTAMENT	50	// Maximo tamaño del nombre de una entidad

char ent[kMAXNUMENT][kMAXTAMENT];

bool salir; //Variable que indica si tenemos que abortar el proceso a la primera oportunidad

//Esta funcion se llama cuando se interrumpe el proceso, y permite que el fichero
//actual acabe, pero luego aborta
void Salir(int sig)
 {
  salir=true;
 }

//Funcion que actualiza los cambios del fichero para indicar que lo hemos cambiado
void ActualizaCambios(FILE *fent,FILE *fsal)
 {
  char cadenaleida; //Cadena que va almacenando la palabra que se lee

  //Guardamos el resto del fichero
  cadenaleida=fgetc(fent);
  while(!feof(fent))
   {
    fputc(cadenaleida,fsal);
    cadenaleida=fgetc(fent);
   }

 }

//Levi
// Carga el fichero de las entidades
void cargarEntidades() {
  FILE *f;
  if ((f = fopen ("entidades.txt", "r")) != NULL) {
    for (int i = 0; i < kMAXNUMENT; i++) ent[i][0] = '\0';

    do {
      char cad[kMAXTAMENT];
      int n;
      if (fscanf (f, "%s %d", cad, &n) == 2) {
        if (n >= 0 && n < kMAXNUMENT)
          strcpy (ent[n], cad);
        else
          printf ("Entidad %d no cargada\n", n);
      }
    } while (!feof(f));
  }
}

//Levi
//Funcion que realiza la sustitución de los caracteres erroneos de un fichero
//recibe la ruta del fichero de entrada y la del fichero de salida
bool PreprocesaFichero (FILE *fent, FILE *fsal) {
  unsigned char c; //Cadena que va almacenando la palabra que se lee
  bool modificado = false;

  bool encontrado; //Nos indica si hemos encontrado ya o no el </teiHeader>

  encontrado = false;

  c=0;

  if(!feof(fent)) 
    c=fgetc(fent);

  while(!feof(fent))
   {
    long int ud = -1;
    if(c=='<') 
     {//Do not check inside tags
      do
       {
        fputc(c, fsal);
        c=fgetc(fent);
       } while(!feof(fent) && c!='>');
      if(!feof(fent)) 
        fputc(c,fsal);
     }
     else
      {
       if(c==126)
        {
         modificado=true;
         fprintf(fsal,"&tilde;");
        }
        else
         {
          if(c>=192 && c<=223)
           {
            unsigned char c1=fgetc(fent);
            if(!feof(fent) && c1>=128)
              ud=(c-192)*64+c1-128;
             else
              {
               fputc(c,fsal);
               if(!feof(fent)) 
                 fputc(c1,fsal);
              }
           }
           else 
            {
             if(c>=224 && c<=239)
              {
               unsigned char c1=fgetc(fent);
               if(!feof(fent) && c1>=128)
                {
                 unsigned char c2=fgetc(fent);
                 if(!feof(fent) && c2>=128)
                   ud=(c-224)*4096+(c1-128)*64+c2-128;
                  else
                   {
                    fputc(c,fsal);
                    fputc(c1,fsal);
                    if(!feof(fent)) 
                      fputc(c2,fsal);
                   }
                }
                else
                 {
                  fputc(c,fsal);
                  if(!feof(fent))
                    fputc(c1,fsal);
                 }
              }
              else
               {
                if(c>=240 && c<=247)
                 {
                  unsigned char c1=fgetc(fent);
                  if(!feof(fent) && c1>=128)
                   {
                    unsigned char c2=fgetc(fent);
                    if(!feof(fent) && c2>=128)
                     {
                      unsigned char c3=fgetc(fent);
                      if(!feof(fent) && c3>=128)
                        ud=(c-240)*26144+(c1-128)*4096+(c2-128)*64+c3-128;
                       else
                        {
                         fputc(c,fsal);
                         fputc(c1,fsal);
                         fputc(c2,fsal);
                         if(!feof(fent))
                           fputc(c3,fsal);
                        }
                     }
                     else
                      {
                       fputc(c,fsal);
                       fputc(c1,fsal);
                       if(!feof(fent))
                         fputc(c2,fsal);
                      }
                   }
                   else
                    {
                     fputc(c,fsal);
                     if(!feof(fent)) 
                       fputc(c1,fsal);
                    }
                 }
                 else 
                  {
                   if(c>=248 && c<=251)
                    {
                     unsigned char c1=fgetc(fent);
                     if(!feof(fent) && c1>=128)
                      {
                       unsigned char c2=fgetc(fent);
                       if(!feof(fent) && c2>=128)
                        {
                         unsigned char c3=fgetc(fent);
                         if(!feof(fent) && c3>=128)
                          {
                           unsigned char c4=fgetc(fent);
                           if(!feof (fent) && c4>=128)
                             ud=(c-248)*16177216+(c1-128)*262144+(c2-128)*4096+(c3-128)*64+c4-128;
                            else
                             {
                              fputc(c,fsal);
                              fputc(c1,fsal);
                              fputc(c2,fsal);
                              fputc(c3,fsal);
                              if(!feof(fent))
                                fputc(c4,fsal);
                             }
                          }
                          else
                           {
                            fputc(c,fsal);
                            fputc(c1,fsal);
                            fputc(c2,fsal);
                            if(!feof(fent)) 
                              fputc(c3,fsal);
                           }
                        }
                        else
                         {
                          fputc(c,fsal);
                          fputc(c1,fsal);
                          if(!feof(fent))
                            fputc(c2,fsal);
                         }
                      }
                      else
                       {
                        fputc(c,fsal);
                        if(!feof(fent)) 
                          fputc(c1,fsal);
                       }
                    }
                    else
                     {
                      if(c>=252 && c<=253)
                       {
                        unsigned char c1=fgetc(fent);
                        if(!feof(fent) && c1>=128)
                         {
                          unsigned char c2=fgetc(fent);
                          if(!feof(fent) && c2>=128)
                           {
                            unsigned char c3=fgetc(fent);
                            if(!feof(fent) && c3>=128)
                             {
                              unsigned char c4=fgetc(fent);
                              if(!feof(fent) && c4>=128)
                               {
                                unsigned char c5=fgetc(fent);
                                if(!feof(fent) && c5>=128)
                                  ud=(c-252)*1073741824+(c1-128)*16777216+(c2-128)*262144+(c3-128)*4096+(c4-128)*64+c5-128;
                                 else
                                  {
                                   fputc(c,fsal);
                                   fputc(c1,fsal);
                                   fputc(c2,fsal);
                                   fputc(c3,fsal);
                                   fputc(c4,fsal);
                                   if(!feof(fent))
                                     fputc(c5,fsal);
                                  }
                               }
                               else
                                {
                                 fputc(c,fsal);
                                 fputc(c1,fsal);
                                 fputc(c2,fsal);
                                 fputc(c3,fsal);
                                 if(!feof(fent))
                                   fputc(c4,fsal);
                                }
                             }
                             else
                              {
                               fputc(c,fsal);
                               fputc(c1,fsal);
                               fputc(c2,fsal);
                               if(!feof(fent))
                                 fputc(c3,fsal);
                              }
                           }
                           else
                            {
                             fputc(c,fsal);
                             fputc(c1,fsal);
                             if(!feof(fent))
                               fputc(c2,fsal);
                            }
                         }
                         else
                          {
                           fputc(c,fsal);
                           if(!feof(fent))
                             fputc(c1,fsal);
                          }
                       }
                       else
                        {
                         fputc(c,fsal);
                        }
                     }
                  }
               }
            }
         }
      }
    if(ud != -1)
     {
      modificado = true;
      // Si existe un nombre de entidad para ud, lo utilizamos
      if (ud <= 10000 && ud >= 0 && ent[ud][0] != '\0')
        fprintf (fsal, "&%s;", ent[ud]);
      else
        fprintf (fsal, "&#%d;", (int)ud);
     }
    c = fgetc (fent);
   }
  return modificado;
 }

int main(int argc,char *argv[])
 {
  FILE *fent, *fsal;
  int i,j;
  salir = false;

  cargarEntidades();

  signal (SIGINT, Salir);
  j=0;
  if(argc<2)
   {
#ifndef NOMENSAJES
    cerr<<"Uso: " << argv[0] << " <fich1> <fich2> ..."<<endl;
#endif
   }
   else
    {
     for(i=1;i<argc && !salir;i++)
      {
       if((fent=fopen(argv[i],"r"))==NULL)
        {
#ifndef NOMENSAJES
         cerr<<"Error al abrir el fichero "<<argv[i]<<endl;
#endif
        }
        else
         {
          if((fsal=tmpfile())==NULL) //Creamos un fichero temporal
            cerr<<"Error al abrir fichero temporal"<<endl;
           else
            {//Procesamos el fichero
             if(PreprocesaFichero(fent, fsal))
              {
               fclose(fent);
               rewind(fsal);
               if((fent=fsal)==NULL)
                {
                 cerr<<"Error al abrir el fichero temporal para actualizar los changes"<<endl;
                 i=argc;
                }
                else
                 {
                  if((fsal=fopen(argv[i],"w"))==NULL)
                   {
                    cerr<<"Error al grabar el fichero modificado"<<endl;
                    i=argc;
                   }
                   else
                    {
                     j++;
                     ActualizaCambios(fent,fsal);
#ifndef NOMENSAJES
                     cout<<"Modificado el fichero "<<argv[i]<<endl;
#endif
                     fclose(fsal);
                    }
                 }
              }
            }
          fclose(fent);
         }
      }
#ifndef NOMENSAJES
     cout<<"Procesados "<< i - 1 <<" archivos"<<endl;
     cout<<"Cambiados "<<j<<" archivos"<<endl;
#endif
    }
  return(0);
 }
