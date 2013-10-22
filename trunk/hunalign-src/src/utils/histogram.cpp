/*************************************************************************
*                                                                        *
*  (C) Copyright 2002.  Daniel Varga                                     *
*  All rights reserved by the author.                                    *
*                                                                        *
*************************************************************************/

#pragma warning ( disable : 4786 )

#include <histogram.h>

#include <iostream>

void DiscreteDoubleMap::binning( bool logBin, bool dontShowZeros, double step, SmoothDoubleMap& binned ) const
{
  const DiscreteDoubleMap& m = *this;

  binned.clear();

  if (m.empty())
    return;

  double leftestValue = m.begin()->first;

  double leftFloat = 0.0;
  if (logBin)
  {
    if ( leftestValue < 0 )
    {
      std::cerr << "Logbinning currently does not work for values smaller than 0." << std::endl;
      throw "data error";
    }
    else if ( leftestValue < 1 )
    {
      // A very primitive, basically incorrect way to get something for sub-one values. Not a real logbin.
      double left = 0.0;
      double right = 1.0;

      DoubleMap::const_iterator leftit  = m.lower_bound(left);
      DoubleMap::const_iterator rightit = m.lower_bound(right);

      if (leftit!=m.end())
      {
        double sum=0;

        for ( ; leftit!=rightit; ++leftit )
        {
          sum += leftit->second;
        }

        // Nem vilagos, hogy x meghatarozasara mi a jo politika. Itt van ket primitiv:
        // double adHocCenter = left;
        double adHocCenter = (left+right-1)/2;

        if ( (!dontShowZeros) || (sum>0) )
        {
          binned[adHocCenter] = sum/(right-left);
        }
      }
    }

    leftFloat = 1.0;
  }
  else
  {
    leftFloat = 0.0;
    if ( leftestValue < 0 )
    {
      std::cerr << "Binning currently does not work for values smaller than 0." << std::endl;
      throw "data error";
    }
  }

  while (true)
  {
    double rightFloat;
    if (logBin)
      rightFloat = leftFloat * step;
    else
      rightFloat = leftFloat + step;

    // Nulla hosszu bin intervallum.
    if ((int)leftFloat==(int)rightFloat)
    {
      leftFloat = rightFloat;
      continue;
    }

    double left  = (int)leftFloat;
    double right = (int)rightFloat;

    DoubleMap::const_iterator leftit  = m.lower_bound(left);
    DoubleMap::const_iterator rightit = m.lower_bound(right);

    if (leftit==m.end())
      break;

    double sum=0;

    for ( ; leftit!=rightit; ++leftit )
    {
      sum += leftit->second;
    }

    // Nem vilagos, hogy x meghatarozasara mi a jo politika. Itt van ket primitiv:
    // double adHocCenter = left;
    double adHocCenter = (left+right-1)/2;

    if ( (!dontShowZeros) || (sum>0) )
    {
      binned[adHocCenter] = sum/(right-left);
    }
    leftFloat = rightFloat;
  }
}

void DoubleMap::read( std::istream& is )
{
  clear();

  while ( !is.eof() && (is.good()) )
  {
    double x(-1024),y(-1024);
    is >> x >> y;
    is.ignore(); // New line.

    if (!is.good())
      break;

    operator[](x) = y;
  }
}

void DoubleMap::write( std::ostream& os ) const
{
  for ( DoubleMap::const_iterator it=begin(); it!=end(); ++it )
  {
    os << it->first << "\t" << it->second << std::endl;
  }
}

// A nulla implicit (azaz nem) jelenik meg a DiscreteDoubleMap-ben!
void DiscreteDoubleMap::from( const Histogram& h )
{
  clear();
  for ( int i=0; i<h.size(); ++i )
  {
    if (h[i] != 0)
    {
      operator[](i) = h[i];
    }
  }
}

// Bezzeg a SmoothDoubleMap-ben!
void SmoothDoubleMap::from( const Histogram& h )
{
  clear();
  for ( int i=0; i<h.size(); ++i )
  {
    operator[](i) = h[i];
  }
}

void Histogram::add( int x, double val/* = 1 */ )
{
  if (x>=size())
  {
    resize(x+1);
  }
  operator[](x) += val;
}

void Histogram::write( std::ostream& os ) const
{
  for ( int i=0; i<size(); ++i )
  {
    os << i << "\t" << operator[](i) << std::endl;
  }
}

void Histogram::write_othernonull( std::ostream& os ) const
{
  for ( int i=1; i<size(); ++i )
  {
    if (operator[](i) != 0)
      os << i << "\t" << operator[](i) << std::endl;
  }
}

void Histogram::read( std::istream& is )
{
  while ( !is.eof() && (is.good()) )
  {
    int x(-1024);
    double y(-1024);
    is >> x >> y;
    is.ignore(); // New line.

    if (!is.good())
      break;

    if (x>=size())
    {
      resize(x+1);
    }
    operator[](x) = y;
  }
}

double Histogram::sumFromOne() const
{
  double n=0;
  for ( int i=1; i<size(); ++i )
  {
    n += operator[](i);
  }
  return n;
}

void Histogram::setZeroByTotal( double total )
{
  double thereis = sumFromOne();
  operator[](0) = total-thereis;
}
