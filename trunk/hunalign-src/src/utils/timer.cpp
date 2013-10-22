/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#pragma warning ( disable : 4786 )

#include "timer.h"

#include <cassert>
#include <string.h>

//#include <iostream> // Just for testing.

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#ifndef WIN32
#include <sstream> // For itoa implementation.
#endif

namespace Hunglish
{

int Timer::getTick()
{
#ifdef WIN32

  return GetTickCount();

#else

  timeval tv;

  assert( 0 == gettimeofday( &tv, 0) );

//  std::cerr << "sec:"<< tv.tv_sec << " usec:"<< tv.tv_usec << std::endl;

  return (tv.tv_sec % (3600*24*49))*1000+tv.tv_usec/1000;

#endif
}

} // namespace Hunglish

// Ugly portability layer:

#ifndef WIN32

void itoa( int n, char* s, int radix )
{
  assert( radix==10 );
  std::ostringstream ss;
  ss << n;
  strcpy(s,ss.str().c_str());
}

#endif
