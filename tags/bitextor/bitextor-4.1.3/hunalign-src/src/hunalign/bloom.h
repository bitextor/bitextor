/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_BLOOM_H
#define __HUNGLISH_ALIGNMENT_BLOOM_H

#include <bitset>

// TODO
#include "words.h"

namespace Hunglish
{

const int bloomSize=512;

class BloomFilter : private std::bitset<bloomSize>
{
public:
  void set( const Word& w );
  bool test ( const Word& w ) const;
  int count() const;

public:
  std::bitset<bloomSize>& getBitset();
  const std::bitset<bloomSize>& getBitset() const;

public:
  friend int intersectionSize( const BloomFilter& b1, const BloomFilter& b2 );

public:
  static int hash( const Word& w );
};

int intersectSize( const BloomFilter& b1, const BloomFilter& b2 );

typedef std::vector<BloomFilter> BloomBook;

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_BLOOM_H
