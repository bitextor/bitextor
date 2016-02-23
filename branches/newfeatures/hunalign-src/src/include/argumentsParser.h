/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __ARGUMENTSPARSER_H
#define __ARGUMENTSPARSER_H

#include <string>
#include <map>
#include <vector>

// Current usage and limitations:
// Every argument starts with a '-'.
// It is a key/value pair. The delimiter
// is either the first '=' (erased), or the
// first nonalphabetic character (not erased).

class AnyData
{
public:
  enum Kind { Int, String, Float, Set };

public:
  AnyData() : kind(String), dInt(-1) {}
  AnyData( const int& d ) : kind(Int), dInt(d) {}
  AnyData( const std::string& d ) : kind(String), dInt(-1), dString(d) {}
//  AnyData( const float& d ) : kind(Float), dFloat(d) {}
//  AnyData( const std::set<int>& d ) : kind(Set), dSet(d), dInt(-1) {}

public:
  Kind kind;
  int dInt;
  std::string dString;
//  float dFloat;
//  std::set<int> dSet;
};

typedef std::string ArgName;
typedef std::map< ArgName, AnyData > ArgumentMap;

class Arguments : public ArgumentMap
{
public:
  // Very important note: When read finds a numeric/set argument,
  // it sets anyData.kind to Int. But STILL, it fills anyData.dString,
  // just in case. So if the ArgumentMap was built by Arguments::read,
  // the dString fields are all filled.
  bool read( int argc, char **argv );

  // remains is filled with the arguments not starting with '-'.
  bool read( int argc, char **argv, std::vector<const char*>& remains );

  // const if fails, erases arg if succeeds.
  bool getNumericParam( const ArgName& name, int& num );

  // sw is true if the switch is present. The function
  // returns false if the argument value is not empty.
  bool getSwitch( const ArgName& name, bool& sw );

  bool getSwitchConst( const ArgName& name, bool& sw ) const;

  // Returns true if the switch is present. Throws an error message if
  // if the argument value is not empty.
  bool getSwitchCompact( const ArgName& name );

  void checkEmptyArgs() const;
};

#endif // #define __ARGUMENTSPARSER_H
