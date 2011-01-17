/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_INCLUDE_SERIALIZEIMPL_H
#define __HUNGLISH_INCLUDE_SERIALIZEIMPL_H

#include <iostream>
#include <vector>
#include <set>

template <class T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& v )
{
  for ( typename std::vector<T>::const_iterator it=v.begin(); it!=v.end(); ++it )
  {
    os << *it ;
    if (it+1!=v.end())
      os << " ";
  }
  return os;
}

template <class T>
std::ostream& operator<<( std::ostream& os, const std::set<T>& v )
{
  typename std::set<T>::const_iterator it=v.begin();
  while (true)
  {
    os << *it ;

    typename std::set<T>::const_iterator itplus = it;
    ++itplus;

    if (itplus == v.end())
      break;
    else
      os << " ";

    it = itplus;
  }
  return os;
}

#endif // #define __HUNGLISH_INCLUDE_SERIALIZEIMPL_H
