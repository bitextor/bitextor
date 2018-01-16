/*

Copyright (C) 1997,1998,1999,2000,2001  Franz Josef Och

mkcls - a program for making word classes .

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.

*/





#include <cstdlib>
#include <cstdio>


extern "C" {

#ifndef WIN32
#include <sys/time.h>
#include <sys/resource.h>
#else
#define srand48 srand
#define drand48() (rand()/RAND_MAX)
#endif

}

#include "general.h"

extern "C" {
#ifndef __linux__
  int getrusage(int who, struct rusage *rusage);
#endif
};
int verboseMode=0;

#ifdef aNeXT
#define NO_TEMPLATES
#endif


void myerror(int line,const char *file,const char *expression)
{
  cerr << "(general.h):Assertion failed: '" << expression <<  "' ::: b "
       << file << ":" << line << endl;
}


void imyerror(int line,const char *file,const char *expression)
{
  cerr << "Error: '" << expression <<  "' ::: in Source " << file
       << ":" << line << endl;
#ifndef DEBUG

#endif
}



void zufallSeed(int z)
{
#ifdef NeXT
  srandom(z);
#else
  srand48(z);
#endif
}



double zufall01()
{
#ifdef NeXT
  return (double)(random()%65536)/65536.0;
#else
  return drand48();
#endif
}



double zufall(double min,double max)
{
  double z;
  // Get a random double from 0 inclusive to 1 exclusive, and scale to fit
  // the [min, max) interval.  In theory this always produces a number in the
  // interval, but due to rounding errors it's possible occasionally to get
  // something just outside it, especially z == max.  No point trying to
  // optimize for that; we can just retry if it happens.
  do {
    z = zufall01()*(max-min)+min;
  } while (z < min || z>= max);
  assert(z>=min&&z<max);
  return z;
}



int randomInt(int exclusive)
{
  int i=(int)zufall(0,exclusive);
  assert(i>=0);
  assert(i<exclusive);
  return i;
}

double clockSec()
{
#ifdef WIN32
  return 0;
#else
#ifdef linux
  enum __rusage_who who=RUSAGE_SELF;
#else
  int who=RUSAGE_SELF;
#endif
  struct rusage rusage;
  getrusage(who, &rusage);
  return rusage.ru_utime.tv_sec+rusage.ru_utime.tv_usec/1000000.0;
#endif
}
