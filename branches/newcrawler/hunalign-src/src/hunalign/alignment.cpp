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

#include "alignment.h"

#include "words.h" // For SentenceList
#include "dictionary.h" // For FrequencyMap

#include <iostream>
#include <map>
#include <set>
#include <algorithm>

// Copypaste-elve. TODO Elhelyezni.
#define massert(e) if (!(e)) { std::cerr << #e << " failed" << std::endl; throw "assert"; }

std::ostream& operator<<( std::ostream& os, std::pair<int,int> p )
{
  os << p.first << "," << p.second;
  return os;
}

namespace Hunglish
{


// Attention, the two-sentence length is the first argument. Usually the Hungarian is, but not here.
// The bigger the better. closeness is always smaller than bestScore.
double closeness( double twoSentenceLength, double oneSentenceLength )
{
  double closeness(0);

  const double bestScore = 0.3;
  const double quasiglobal_closenessMultiplier = 0.3;

  double ratio;

  if (twoSentenceLength>oneSentenceLength)
  {
    ratio = (twoSentenceLength+1)/(oneSentenceLength+1);
  }
  else
  {
    ratio = (oneSentenceLength+1)/(twoSentenceLength+1);
  }

  ratio -= 1.0;

  // assert(ratio>=0);
  return bestScore - quasiglobal_closenessMultiplier * ratio;
}

const unsigned char Diag = 1;
const unsigned char HuSkip = 2;
const unsigned char EnSkip = 3;
const unsigned char HuHuEnSkip = 4;
const unsigned char HuEnEnSkip = 5;
const unsigned char Dead = 6;

void buildDynProgMatrix( const AlignMatrix& w, const SentenceValues& huLength, const SentenceValues& enLength,
                         QuasiDiagonal<double>& v, TrelliMatrix& trellis )
{
  const int huBookSize = w.size();
  const int enBookSize = w.otherSize();

  int huPos,enPos;

  // v[huPos][enPos] gives the similarity of the [0,huPos) and [0,enPos) intervals.
  // The smaller value, the better similarity. (Unlike in the original similarity matrix w, where bigger is better.)

  double infinity = 1e6;

  for ( huPos=0; huPos<=huBookSize; ++huPos )
  {
    int rowStart = v.rowStart(huPos);
    int rowEnd   = v.rowEnd(huPos);
    for ( enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      double& val = v.cell(huPos,enPos);
      unsigned char& trail = trellis.cell(huPos,enPos);

      bool quasiglobal_knightsMoveAllowed = true;
      if (quasiglobal_knightsMoveAllowed)
      {
        double lengthFitness(0);

        bool quasiglobal_lengthFitnessApplied = true;

        // The array is indexed by the step directions. The smaller value, the better.
        double values[Dead];
        int i;
        for ( i=1; i<Dead; ++i )
          values[i] = infinity;

        if (huPos>0)
        {
          values[HuSkip] = v[huPos-1][enPos]   - skipScore;
        }

        if (enPos>0)
        {
          values[EnSkip] = v[huPos][enPos-1]   - skipScore;
        }

        if ((huPos>0) && (enPos>0))
        {
          if (quasiglobal_lengthFitnessApplied)
          {
            lengthFitness = closeness(huLength[huPos-1], enLength[enPos-1]);
          }
          else
          {
            lengthFitness = 0;
          }

          values[Diag] = v[huPos-1][enPos-1] - w[huPos-1][enPos-1] - lengthFitness ;
        }

        const double dotLength = 2.0 ;

        if ((huPos>1) && (enPos>0))
        {
          if (quasiglobal_lengthFitnessApplied)
          {
            lengthFitness = closeness(huLength[huPos-2]+huLength[huPos-1]+dotLength, enLength[enPos-1]);
          }
          else
          {
            lengthFitness = 0;
          }

          const double& a = w[huPos-1][enPos-1] ;
          const double& b = w[huPos-2][enPos-1] ;
          double lengthSimilarity =
          values[HuHuEnSkip] = v[huPos-2][enPos-1] - ( a<b ? a : b ) - skipScore - lengthFitness ; // The worse of the two crossed square.
        }

        if ((huPos>0) && (enPos>1))
        {
          if (quasiglobal_lengthFitnessApplied)
          {
            // Attention, the two-sentence length is the first argument. Usually the Hungarian is the first argument, but not here.
            lengthFitness = closeness(enLength[enPos-2]+enLength[enPos-1]+dotLength, huLength[huPos-1]);
          }
          else
          {
            lengthFitness = 0;
          }

          const double& a = w[huPos-1][enPos-1] ;
          const double& b = w[huPos-1][enPos-2] ;
          values[HuEnEnSkip] = v[huPos-1][enPos-2] - ( a<b ? a : b ) - skipScore - lengthFitness ; // The worse of the two crossed square.
        }

        unsigned char direction = Dead;
        double bestValue = infinity;
        for ( i=1; i<Dead; ++i )
        {
          if (values[i]<bestValue)
          {
            bestValue = values[i];
            direction = i;
          }
        }

        trail = direction;
        if (direction==Dead)
        {
          val = 0;
        }
        else
        {
          val = bestValue;
        }
      }
      else // (!quasiglobal_knightsMoveAllowed)
      {
        int borderCase = ( (huPos==0) ? 0 : 2 ) + ( (enPos==0) ? 0 : 1 ) ;

        switch (borderCase)
        {
        case 0:
          {
            val = 0;
            trail = Dead;
            break;
          }
        case 1: // huPos==0
          {
            val = v[0][enPos-1] - skipScore ;
            trail = EnSkip;
            break;
          }
        case 2: // enPos==0
          {
            val = v[huPos-1][0] - skipScore ;
            trail = HuSkip;
            break;
          }
        case 3:
          {
            double x  = v[huPos-1][enPos]   - skipScore ;
            double y  = v[huPos]  [enPos-1] - skipScore ;
            double xy = v[huPos-1][enPos-1] - w[huPos-1][enPos-1] ;

            double best = xy;
            trail = Diag;
            if (x<best)
            {
              best = x;
              trail = HuSkip;
            }
            if (y<best)
            {
              best = y;
              trail = EnSkip;
            }
            val = best;
            break;
          }
        }
      }
    }
  }
}

void trelliToLadder( const TrelliMatrix& trellis, Trail& bestTrail )
{
  bestTrail.clear();

  // The -1 is needed because the trellis matrix is one larger than the similarity matrix.
  // This points to its downmost rightmost element.
  const int huBookSize = trellis.size()-1;
  const int enBookSize = trellis.otherSize()-1;

  int huPos=huBookSize;
  int enPos=enBookSize;

  bool logging = false;

  if (logging) std::cerr << std::endl;

  bool over = false;
  bool hopelesslyBadTrail = false;
  bestTrail.push_back(std::make_pair(huPos,enPos));

  while (true)
  {
    unsigned char trelli = trellis[huPos][enPos];

    // std::cerr << huPos << "," << enPos << "," << (int)trelli << std::endl;

    if ((huPos==0) || (enPos==0))
      break;

    switch (trelli)
    {
    case Diag :
    {
      --huPos;
      --enPos;
      break;
    }
    case HuSkip :
    {
      --huPos;
      break;
    }
    case EnSkip :
    {
      --enPos;
      break;
    }
    case HuHuEnSkip :
    {
      huPos -= 2;
      --enPos;
      break;
    }
    case HuEnEnSkip :
    {
      --huPos;
      enPos -= 2;
      break;
    }
    case Dead :
    {
      over = true;
      break;
    }
    default:
    {
      hopelesslyBadTrail = true;
      over = true;
      break;
    }
    }

    if (over)
      break;

    bestTrail.push_back(std::make_pair(huPos,enPos));

    if (logging)
    {
      std::cerr << huPos << " \t" << enPos << std::endl;
    }

  }

  if (hopelesslyBadTrail)
  {
    bestTrail.clear();
    bestTrail.push_back(std::make_pair(huBookSize,enBookSize));
    bestTrail.push_back(std::make_pair(0,0));
    std::cerr << "Error: hopelessly bad trail." << std::endl;
  }

  std::reverse(bestTrail.begin(),  bestTrail.end()  );
}


void align( const AlignMatrix& w, const SentenceValues& huLength, const SentenceValues& enLength,
            Trail& bestTrail, AlignMatrix& v )
{
  const int huBookSize = w.size();
  const int enBookSize = w.otherSize();
  const int thickness  = w.thickness();

  massert(w.size()+1 == v.size());
  massert(w.otherSize()+1 == v.otherSize());

  TrelliMatrix trellis( huBookSize+1,enBookSize+1,thickness, Dead );

  buildDynProgMatrix( w, huLength, enLength, v, trellis );

//x   std::cout << std::endl;
//x   dumpAlignMatrix(v);
//x   std::cout << std::endl;
//x   dumpTrelliMatrix(trellis);
//x   exit(-1);

  std::cerr << "Matrix built." << std::endl;

  trelliToLadder( trellis, bestTrail );

  std::cerr << "Trail found." << std::endl;
}


bool oneToOne( const Trail& bestTrail, int pos )
{
  return (
      ( bestTrail[pos+1].first -bestTrail[pos].first  == 1 )
        &&
      ( bestTrail[pos+1].second-bestTrail[pos].second == 1 )
     );
}


int countIntersectionOfTrails( const Trail& sx, const Trail& sy )
{
  int inter(0);

  Trail::const_iterator sxt = sx.begin();
  Trail::const_iterator syt = sy.begin();
  Trail::const_iterator sxe = sx.end();
  Trail::const_iterator sye = sy.end();
  for ( ; sxt!=sxe && syt!=sye ; )
  {
    if ( *sxt < *syt )
      ++sxt;
    else if ( *sxt > *syt )
      ++syt;
    else
    {
      ++inter;
      ++sxt;
      ++syt;
    }
  }
  return inter;
}


// A bit of an abuse of the fact that Trail and BisentenceList are typedef'd to the same structure.
double scoreTrailOrBisentenceList( const Trail& trailAuto, const Trail& trailHand )
{
  int score = countIntersectionOfTrails( trailAuto, trailHand );

  std::cerr << trailAuto.size()-score << " misaligned out of " << trailHand.size() << " correct items, "
    << trailAuto.size() << " bets." << std::endl;

  std::cerr << "Precision: " << 1.0*score/trailAuto.size() 
    << ", Recall: " << 1.0*score/trailHand.size() << std::endl;

  double ratio = 1.0*(trailAuto.size()-score)/trailAuto.size();
  return ratio;
}


void trailToBisentenceList( const Trail& bestTrail,
                            BisentenceList& bisentenceList )
{
  bisentenceList.clear();

  int trailSize = bestTrail.size();

  for ( int pos=0; pos<trailSize-1; ++pos )
  {
    if ( oneToOne(bestTrail,pos) )
    {
      bisentenceList.push_back(bestTrail  [pos]);
    }
  }
}


double scoreBisentenceList( const BisentenceList& bisentenceListAuto, const Trail& trailHand )
{
  BisentenceList bisentenceListHand;
  trailToBisentenceList( trailHand, bisentenceListHand );

  double score = scoreTrailOrBisentenceList( bisentenceListAuto, bisentenceListHand ) ;

  return score;
}

double scoreTrail( const Trail& trailAuto, const Trail& trailHand )
{
  return ( scoreTrailOrBisentenceList( trailAuto, trailHand ) );
}


void setBox( AlignMatrix& m, int huPos, int enPos, int radius, int insideOfRadiusValue )
{
  for ( int x=huPos-radius; x<=huPos+radius; ++x )
  {
    for ( int y=enPos-radius; y<=enPos+radius; ++y )
    {
      if ( (x>=0) && (x<m.size()) && (y>=0) && (y<m.otherSize()) )
      {
        m.cell(x,y) = insideOfRadiusValue ;
      }
    }
  }
}

// Fills the complement of the radius of the trail with minus infties.
// The return value true means success. Failure means that during the fill,
// we intersected the outside of the quasidiagonal area.
// In this case, the operation is not finished.
bool borderDetailedAlignMatrix( AlignMatrix& alignMatrix, const Trail& trail, int radius )
{
  int huBookSize = alignMatrix.size();
  int enBookSize = alignMatrix.otherSize();

  int huPos, enPos;
  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    int rowStart = alignMatrix.rowStart(huPos);
    int rowEnd   = alignMatrix.rowEnd(huPos);
    for ( enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      alignMatrix.cell(huPos,enPos) = outsideOfRadiusValue;
    }
  }

  // We seriously use the fact that many-to-zero segments are subdivided into one-to-zero segments.
  // Inside setBox, an exception is thrown if we try to write outside the quasidiagonal.
  // If we catch such an exception, it means that the quasidiagonal is not thick enough.
  // In this case, we abandon the whole align, just to be sure.
  try
  {
    for ( int i=0; i<trail.size(); ++i )
    {
      setBox( alignMatrix, trail[i].first, trail[i].second, radius, insideOfRadiusValue );
    }
  }
  catch ( const char* errorType )
  {
    massert( std::string(errorType) == "out of quasidiagonal" )
    return false;
  }

  bool verify = true;
  if (verify)
  {
    int numberOfEvaluatedItems(0);
    for ( huPos=0; huPos<huBookSize; ++huPos )
    {
      int rowStart = alignMatrix.rowStart(huPos);
      int rowEnd   = alignMatrix.rowEnd(huPos);
      for ( enPos=rowStart; enPos<rowEnd; ++enPos )
      {
        if (alignMatrix[huPos][enPos]==insideOfRadiusValue)
        {
          ++numberOfEvaluatedItems;
        }
      }
    }

    std::cerr << numberOfEvaluatedItems << " items inside the border." << std::endl;
  }

  return true;
}

template <class T>
void dumpAlignMatrix( const QuasiDiagonal<T>& alignMatrix )
{
  int huPos,enPos;

  int huBookSize = alignMatrix.size();
  int enBookSize = alignMatrix.otherSize();

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    for ( enPos=0; enPos<enBookSize; ++enPos )
    {
      int start = alignMatrix.rowStart(huPos);
      int end   = alignMatrix.rowEnd  (huPos);

      if ( (enPos<start) || (enPos>=end) )
      {
        std::cout << "-1\t";
        continue;
      }

      std::cout << alignMatrix[huPos][enPos] << "\t";
    }
    std::cout << std::endl;
  }
}

void dumpAlignMatrix( const QuasiDiagonal<int>& alignMatrix, bool graphical )
{
  int huPos,enPos;

  int huBookSize = alignMatrix.size();
  int enBookSize = alignMatrix.otherSize();

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    for ( enPos=0; enPos<enBookSize; ++enPos )
    {
      int start = alignMatrix.rowStart(huPos);
      int end   = alignMatrix.rowEnd  (huPos);

      if ( (enPos<start) || (enPos>=end) )
      {
        if (graphical)
        {
          std::cout << "   ";
        }
        else
        {
          std::cout << "-1\t";
        }
        continue;
      }

      if (graphical)
      {
        char c(' ');
        switch (alignMatrix[huPos][enPos])
        {
          case 0: c=' '; break;
          case 1: c='.'; break;
          case 2: c=':'; break;
          case 3: c='|'; break;
          case 4: c='+'; break;
          default: c='X'; break;
        }
        std::cout << c << " ";
      }
      else
      {
        std::cout << alignMatrix[huPos][enPos] << "\t";
      }
    }
    std::cout << std::endl;
  }
}

void dumpTrelliMatrix( const TrelliMatrix& trellis )
{
  std::map<int, std::string> directions;

  directions[Diag] = "HuEn";
  directions[HuSkip] = "Hu";
  directions[EnSkip] = "En";
  directions[HuHuEnSkip] = "HuHuEn";
  directions[HuEnEnSkip] = "HuEnEn";
  directions[Dead] = "Dead";

  int huPos,enPos;

  int huBookSize = trellis.size();
  int enBookSize = trellis.otherSize();

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    for ( enPos=0; enPos<enBookSize; ++enPos )
    {
      int start = trellis.rowStart(huPos);
      int end   = trellis.rowEnd  (huPos);

      if ( (enPos<start) || (enPos>=end) )
      {
        std::cout << "-1\t";
        continue;
      }

      std::cout << directions[trellis[huPos][enPos]] << "\t";
    }
    std::cout << std::endl;
  }
}

} // namespace Hunglish
