/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Applied Logic Laboratory, Ltd.                    *
*  All rights reserved.                                                  *
*                                                                        *
*  Developed by Daniel Varga                                             *
*                                                                        *
*************************************************************************/

#ifndef __PRIVATE_DANIEL_NEIGHBOURS_HISTOGRAM_H
#define __PRIVATE_DANIEL_NEIGHBOURS_HISTOGRAM_H

#include <map>
#include <vector>

class Histogram : public std::vector<double>
{
public:
  void add( int x, double val = 1 );

  void write( std::ostream& os ) const;
  void write_othernonull( std::ostream& os ) const;
  void read( std::istream& is );

  double sumFromOne() const;
  void setZeroByTotal( double total );
};

// Szemantikus kaosz:
// Ket szemantikusan radikalisan kulonbozo strukturat tarolhatunk DoubleMap-ben.
// Az egyik egy olyan dolog, ami mindenhol nulla, ahol nem mondtuk meg, mennyi.
// A hisztogrammok ilyenek.
// A masik, amelyiket interpolalnank az explicit adott ertekek kozott.
// A binning eredmenyek ilyenek.

// TODO Szarmaztassunk le egy Interpolable osztalyt.
class DoubleMap : public std::map<double,double>
{
public:

  void read ( std::istream& is );
  void write( std::ostream& os ) const;

};

class SmoothDoubleMap : public DoubleMap
{
public:
  void from( const Histogram& h );

  // Ide majd interpolalo metodusok jonnek.
};

class DiscreteDoubleMap : public DoubleMap
{
public:
  // A nulla implicit (azaz nem) jelenik meg a DoubleMap-ben!
  void from( const Histogram& h );

  void binning( bool logBin, bool dontShowZeros, double step, SmoothDoubleMap& binned ) const;
};

#endif // #define __PRIVATE_DANIEL_NEIGHBOURS_HISTOGRAM_H
