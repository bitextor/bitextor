/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_QUASIDIAGONAL_H
#define __HUNGLISH_ALIGNMENT_QUASIDIAGONAL_H

#include <vector>

namespace Hunglish
{

template <class T>
class QuasiDiagonal
{
public:

  // Quite slow, because of the many bounds checks.
  class QuasiDiagonalRow
  {
  public:

    // QuasiDiagonalRow is similar to a vector of size size_. The difference is
    // that only the [offset_,offset_+thickness) subinterval can be written.
    // Reading from outside this interval yields the default T().
    // Reading from outside the [0,size) interval yields a throw.
    // It is NOT asserted that [offset_,offset_+thickness)
    // should be a subset of [0,size).
    //
    QuasiDiagonalRow( int size_=0, int offset_=0, int thickness=0, T outsideDefault_=T() )
      : offset(offset_), size(size_), data(thickness,T()), outsideDefault(outsideDefault_) {}

//x     T operator[](int k) const
//x     {
//x       if ( ! ((k>=0) && (k<size)) )
//x       {
//x         throw "out of bounds";
//x       }
//x       int d = k-offset;
//x       if ( (d>=0) && (d<(int)data.size()) )
//x       {
//x         return data[k-offset];
//x       }
//x       else
//x       {
//x         return outsideDefault;
//x       }
//x     }

    enum ZoneType
    {
      DiagZone    = 1,
      MatrixZone  = 2,
      OutsideZone = 3
    };

    ZoneType zone(int k) const
    {
      if ( ! ((k>=0) && (k<size)) )
      {
        return OutsideZone;
      }
      int d = k-offset;
      if ( (d>=0) && (d<(int)data.size()) )
      {
        return DiagZone;
      }
      else
      {
        return MatrixZone;
      }
    }

    const T& operator[](int k) const
    {
      if ( ! ((k>=0) && (k<size)) )
      {
        throw "out of matrix";
      }
      int d = k-offset;
      if ( (d>=0) && (d<(int)data.size()) )
      {
        return data[k-offset];
      }
      else
      {
        return outsideDefault;
      }
    }

    T& cell(int k)
    {
      if ( ! ((k>=0) && (k<size)) )
      {
        throw "out of matrix";
      }
      int d = k-offset;
      if ( (d>=0) && (d<(int)data.size()) )
      {
        return data[k-offset];
      }
      else
      {
        throw "out of quasidiagonal";
      }
    }

  private:
    std::vector<T> data;
    int offset;
    int size;
    T   outsideDefault;
  };

  QuasiDiagonal( int height_, int width_, int thickness_, T outsideDefault_=T() )
    : height(height_), width(width_), thicknes(thickness_)
  {
    for ( int i=0; i<height; ++i )
    {
      // Too much copying, but we don't care.
      QuasiDiagonalRow row( width, offset(i), thicknes, outsideDefault_ );
      rows.push_back(row);
    }
  }

  int offset( int row ) const
  {
    return (row*width/height-thicknes/2);
  }

  int rowStart( int row ) const
  {
    int s=offset(row);
    return ( s>0 ? s : 0 );
  }

  int rowEnd( int row ) const
  {
    int e=offset(row)+thicknes;
    return ( e<width ? e : width );
  }

  // The first coordinate is (somewhat atypically) the row.
  const QuasiDiagonalRow& operator[]( int y ) const
  {
    return rows[y];
  }
  
  T& cell( int y, int x )
  {
    if ((y<0)||(y>=height))
    {
      throw "out of matrix";
    }

    return rows[y].cell(x);
  }

  bool setCell( int y, int x, const T& t )
  {
    cell(y,x) = t;
    return true;
  }

  int size() const { return height; }
  // Yes, I know it's a stupid name. The reason is, I don't want to
  // put width/height on the interface, because usually
  // the first coord is the columns, but not here.
  // This could lead to confusion.
  int otherSize() const { return width; }

  int thickness() const { return thicknes; }

private:
  std::vector<QuasiDiagonalRow> rows;
  int height,width,thicknes;
};

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_QUASIDIAGONAL_H
