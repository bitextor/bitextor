/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_INCLUDE_TIMER_H
#define __HUNGLISH_INCLUDE_TIMER_H

// Don't use it for anything important:
// The windows version overflows at 2^32=4294967296 milliseconds (49.71 days) after boot time.
// The unix version overflows in every seven weeks (3600*24*49*1000=4233600000 ms).

namespace Hunglish
{

// In microseconds.
class Timer
{
public:
  static int getTick();
};

class Ticker
{
public:
  Ticker() { start(); }
  
  void start() { time = Timer::getTick(); }
  int get() { return Timer::getTick()-time; }
  int next() { int t=get(); start(); return t; }

private:
  int time;
};

} // namespace Hunglish

#ifndef WIN32

void itoa( int n, char* s, int radix );

#endif

#endif // #define __HUNGLISH_INCLUDE_TIMER_H
