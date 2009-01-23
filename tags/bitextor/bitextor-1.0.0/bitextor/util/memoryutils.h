/*
   Implementado por Enrique Sánchez Villamil
   Email:   esvillamil@dlsi.ua.es
   Año 2004
   
   Este codigo fuente contiene la recopilacion de funciones
   especificas no incluibles dentro de una clase determinada.
*/

#ifndef MEMORYUTILS_H
#define MEMORYUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>

/** Funcion destinada a reemplazar a la funcion new y que comprueba que no haya errores y que
 *  la memoria se haya reservado correctamente
 *
 * @param size el tamaño de memoria a reservar
 * @returns el puntero asociado a la nueva memoria reservada
 */
void* mynew(size_t size);

/** Funcion destinada a reemplazar a la funcion delete y que comprueba que no haya errores y
 *  que la memoria se haya liberado correctamente
 *
 */
void mydelete(void *ptr);

#endif
