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

#include <stringsAndStreams.h>

namespace Hunglish
{

void split( const std::string line, std::vector<std::string>& words, char delim /*='\t'*/ )
{
  words.clear();

  std::string current;
  int i;
  for ( i=0; i<line.size(); ++i )
  {
    if (line[i]==delim)
    {
      words.push_back(current);
      current = "";
    }
    else
    {
      current += line[i];
    }
  }
  words.push_back(current);
}

} // namespace Hunglish
