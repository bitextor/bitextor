/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_INCLUDE_PORTABLEHASH_H
#define __HUNGLISH_INCLUDE_PORTABLEHASH_H


#ifdef WIN32

#define EXTNAMESPACE std
#include <hash_map>

#else

#define EXTNAMESPACE __gnu_cxx
#include <ext/hash_map>

#endif

#endif // #define __HUNGLISH_INCLUDE_PORTABLEHASH_H
