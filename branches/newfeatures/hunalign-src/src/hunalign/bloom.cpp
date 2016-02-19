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

#include "bloom.h"

namespace Hunglish
{

int BloomFilter::hash( const Word& w )
{
  unsigned int v=0;

  for ( Word::const_iterator it=w.begin(); it!=w.end(); ++it )
  {
      unsigned int top = v >> 24;
      // Ez ugy lett tervezve, hogy (unsigned int)(*it) valodi ketbajtos.
      // A helyesebb megoldas az lenne, ha ketbajtosaval mennenk vegig rajta.
      // De ez overkill, mert ha a xerces unicode-ot hash-el, akkor pont
      // ugyanazt csinalja, mint most en.
      v += (v * 37) + top + (unsigned int)(*it);
  }

  // Divide by modulus
  return v % bloomSize;
}

void BloomFilter::set( const Word& w )
{
  ((std::bitset<bloomSize>*)this)->set( hash(w) % bloomSize );

}

bool BloomFilter::test ( const Word& w ) const
{
  return ((std::bitset<bloomSize>*)this)->test( hash(w) % bloomSize );
}

int BloomFilter::count() const
{
  return ((std::bitset<bloomSize>*)this)->count();
}

std::bitset<bloomSize>& BloomFilter::getBitset()
{
  return * (std::bitset<bloomSize>*)this ;
}

const std::bitset<bloomSize>& BloomFilter::getBitset() const
{
  return * (const std::bitset<bloomSize>*)this ;
}

int intersectionSize( const BloomFilter& b1, const BloomFilter& b2 )
{
  int count(0);
  for ( int i=0; i<bloomSize; ++i )
  {
    if (b1[i]&&b2[i])
    {
      ++count;
    }
  }
  return count;
}

} // namespace Hunglish
