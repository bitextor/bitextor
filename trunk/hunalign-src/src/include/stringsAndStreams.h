/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_INCLUDE_STRINGSANDSTREAMS_H
#define __HUNGLISH_INCLUDE_STRINGSANDSTREAMS_H

#include <string>
#include <vector>

namespace Hunglish
{

void split( const std::string line, std::vector<std::string>& words, char delim='\t' );

} // namespace Hunglish

#endif // #define __HUNGLISH_INCLUDE_STRINGSANDSTREAMS_H
