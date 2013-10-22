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

#include "argumentsParser.h"

#include <iostream>
#include <stdlib.h>

// Could be better.
bool alphabetic( char c)
{
  return ((c>='a')&&(c<='z')) || ((c>='A')&&(c<='Z')) || (c=='_');
}

bool Arguments::read( int argc, char **argv )
{
  for ( int i=1; i<argc; ++i )
  {
    std::string p = argv[i];
    if (p.empty() || p[0]!='-')
    {
      std::cerr << p << ": unable to parse argument\n";
      throw "argument error";
      return false;
    }
    p.erase(0,1);

    if (p.empty())
    {
      std::cerr << "Empty argument\n";
      throw "argument error";
      return false;
    }

    int j(0);
    for ( ; j<p.size(); ++j )
    {
      if (! alphabetic(p[j]) )
      {
        if (p[j]=='=')
          p.erase(j,1);
        break;
      }
    }

    ArgName name = p.substr(0,j);
    std::string val = p.substr(j, p.size()-j);
    int num = atoi(val.c_str());

    AnyData anyData(val);
    if ( (num!=0) || (val=="0") )
    {
      anyData.dInt = num;
      anyData.kind = AnyData::Int;
    }
    operator[](name) = anyData;

  }

  return true;
}

bool Arguments::read( int argc, char **argv, std::vector<const char*>& remains )
{
  remains.clear();

  for ( int i=1; i<argc; ++i )
  {
    std::string p = argv[i];
    if (p.empty() || p[0]!='-')
    {
      remains.push_back(argv[i]);
      continue;
    }

    p.erase(0,1);

    if (p.empty())
    {
      std::cerr << "Empty argument\n";
      throw "argument error";
      return false;
    }

    int j(0);
    for ( ; j<p.size(); ++j )
    {
      if (! alphabetic(p[j]) )
      {
        if (p[j]=='=')
          p.erase(j,1);
        break;
      }
    }

    ArgName name = p.substr(0,j);
    std::string val = p.substr(j, p.size()-j);
    int num = atoi(val.c_str());

    AnyData anyData(val);
    if ( (num!=0) || (val=="0") )
    {
      anyData.dInt = num;
      anyData.kind = AnyData::Int;
    }
    operator[](name) = anyData;

  }

  return true;
}

bool Arguments::getNumericParam( const std::string& name, int& num )
{
  const_iterator it=find(name);
  if (it==end())
  {
    // std::cerr << "Argument -" << name << " missing.\n";
    return false;
  }

  if (it->second.kind != AnyData::Int)
  {
    std::cerr << "Argument -" << name << ": integer expected.\n";
    throw "argument error";
  }

  num = it->second.dInt;
  erase(name);
  return true;
}

bool Arguments::getSwitchConst( const ArgName& name, bool& sw ) const
{
  const_iterator it=find(name);
  if (it==end())
  {
    sw = false;
    return true;
  }
  else if (! it->second.dString.empty())
  {
    std::cerr << "Argument -" << name << ": value is not allowed.\n";
    return false;
  }
  else
  {
    sw = true;
    return true;
  }
}

bool Arguments::getSwitch( const ArgName& name, bool& sw )
{
  bool ok = getSwitchConst(name, sw);
  if (ok)
    erase(name);

  return ok;
}

bool Arguments::getSwitchCompact( const ArgName& name )
{
  bool sw(false);
  bool ok = getSwitchConst(name, sw);
  if (ok)
  {
    erase(name);
    return sw;
  }
  else
  {
    std::cerr << "No value is allowed for argument -" << name << ".\n";
    throw "argument error";
  }
}

void Arguments::checkEmptyArgs() const
{
  if (!empty())
  {
    std::cerr << "Invalid argument: ";

    for ( Arguments::const_iterator it=begin(); it!=end(); ++it )
    {
      std::cerr << "-" << it->first;
      if (!it->second.dString.empty())
        std::cerr << "=" << it->second.dString;
      std::cerr << " ";
    }
    std::cerr << std::endl;

    throw "argument error";
  }
}
