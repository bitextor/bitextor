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

#include "trailPostprocessors.h"

#include "words.h"
#include "bookToMatrix.h"

#include <iostream>
#include <cmath>

namespace Hunglish
{

const bool global_postprocessLogging = false;

TrailScores::TrailScores( const Trail& trail_, const AlignMatrix& dynMatrix_ ) : trail(trail_), dynMatrix(dynMatrix_) {}

double TrailScores::operator()( int j ) const
{
  return
    dynMatrix[trail[j  ].first][trail[j  ].second]
    -
    dynMatrix[trail[j+1].first][trail[j+1].second] ;
}

BisentenceListScores::BisentenceListScores( const BisentenceList& bisentenceList_, const AlignMatrix& dynMatrix_ ) 
  : bisentenceList(bisentenceList_), dynMatrix(dynMatrix_) {}

double BisentenceListScores::operator()( int j ) const
{
  return
    dynMatrix[bisentenceList[j].first  ][bisentenceList[j].second]
    -
    dynMatrix[bisentenceList[j].first+1][bisentenceList[j].second+1] ;
}


TrailScoresInterval::TrailScoresInterval( const Trail& trail_, const AlignMatrix& dynMatrix_,
    const SentenceList& huSentenceList_, const SentenceList& enSentenceList_ )
    : trail(trail_), dynMatrix(dynMatrix_), huSentenceList(huSentenceList_), enSentenceList(enSentenceList_) {}

// The average score of the jth segmentum. The bigger the better.
// Division is by the maximum of the Hungarian and English intervals.
// This is a somewhat arbritary decision, and goes very badly with the
// scoring of the knight's moves. But we really have no better choice.
// 
// Also, the method applies some very ugly hacks to avoid the effect of
// paragraph-delimiters. It strips both intervals of <p>s, and
// modifies the dynMatrix-based score assuming that all <p>s got paired.
// except surplus <p>s.
double TrailScoresInterval::scoreSegmentum( const Rundle& start, const Rundle& end ) const
{
  int huDiff = end.first  - start.first  ;
  int enDiff = end.second - start.second ;

  double score = 
    dynMatrix[start.first][start.second]
    -
    dynMatrix[end.  first][end.  second] ;

  int i;
  int huParagraphNum(0), enParagraphNum(0) ;
  for ( i=start.first; i<end.first; ++i )
  {
    if (isParagraph(huSentenceList[i].words))
    {
      ++huParagraphNum;
    }
  }
  for ( i=start.second; i<end.second; ++i )
  {
    if (isParagraph(enSentenceList[i].words))
    {
      ++enParagraphNum;
    }
  }

  int estimatedParagraphMatches = huParagraphNum<enParagraphNum ? huParagraphNum : enParagraphNum ;

  int estimatedParagraphMismatches = ( huParagraphNum>enParagraphNum ? huParagraphNum : enParagraphNum ) - estimatedParagraphMatches ;

  double scoreDeviationBecauseOfThoseStupidParagraphs =
    scoreOfParagraphMatch * estimatedParagraphMatches + skipScore * estimatedParagraphMismatches;

  int huDiffParagraphAdjusted = huDiff - huParagraphNum ;
  int enDiffParagraphAdjusted = enDiff - enParagraphNum ;

  int maxDiffParagraphAdjusted = huDiffParagraphAdjusted>enDiffParagraphAdjusted ? huDiffParagraphAdjusted : enDiffParagraphAdjusted ;

  if (maxDiffParagraphAdjusted==0)
  {
    return 0;
  }
  else
  {
    return ( score - scoreDeviationBecauseOfThoseStupidParagraphs ) / maxDiffParagraphAdjusted ;
  }
}

// The score of the jth segmentum. The bigger the better.
double TrailScoresInterval::operator()( int j ) const
{
  Rundle start = trail[j];
  Rundle end   = trail[j+1];

  return scoreSegmentum( start, end );
}

double TrailScoresInterval::operator()( int j, int k ) const
{
  Rundle start = trail[j];
  Rundle end   = trail[k];

  return scoreSegmentum( start, end );
}


void removeRundles( Trail& trail, const std::set<int>& rundlesToKill )
{
  // Not a speed bottleneck.
  Trail newTrail;
  for ( int i=0; i<trail.size(); ++i )
  {
    if (rundlesToKill.find(i)==rundlesToKill.end())
    {
      newTrail.push_back(trail[i]);
    }
  }

  trail = newTrail;
}


// In cautious mode, auto-aligned rundles are thrown away if
// their left or right neighbour holes are not one-to-one.
// From the -bisent point of view:
// In cautious mode, auto-aligned one-to-one bisentences are thrown away if
// they have left or right neighbours which are not one-to-one.
// This of course dramatically improves precision while slightly degrading recall.
void cautiouslyFilterTrail( Trail& bestTrail )
{
  Trail bestTrailNew;

  int trailSize = bestTrail.size();

  for ( int pos=0; pos<trailSize-1; ++pos )
  {
    if (
         (pos==0)
           || 
         ( oneToOne(bestTrail,pos-1) && oneToOne(bestTrail,pos) )
       )
    {
      bestTrailNew.push_back(bestTrail  [pos]);
    }
  }

  bestTrail = bestTrailNew ;
}


// O, hogy szomorodna meg a C++.
inline double absValue( double x )
{
  return ( x>0 ? x : -x );
}

// Egy zero-to-nonzero hole valamelyik oldalan levo rundle-t kiirtom, ha a
// rundle torlese kozeliti az uj hezagban a magyar karakterszam / angol karakterszam
// hanyadost egyhez. A bal es a jobb kozul azt valasztom, amelyik tobbet javit.
// 
// Meg akkor is olvasztok, ha ezzel kicsit rontok, mivel a valodi zero-to-one eleg ritka.
// Legalabbis regenyekben. Az improvementSlack konstansnak domainfuggonek kellene lennie.
void spaceOutBySentenceLength( Trail& bestTrail, 
                 const SentenceList& huSentenceListPretty,
                 const SentenceList& enSentenceList,
		 bool utfCharCountingMode )
{
  // i most egy hole es nem egy rundle indexe.
  for ( int i=1; i<bestTrail.size()-2; ) // Figyelem, direkt nincs ++i.
  {
    bool huZero = (bestTrail[i].first == bestTrail[i+1].first);
    bool enZero = (bestTrail[i].second== bestTrail[i+1].second);

    bool huParagraph = ( (bestTrail[i].first +1 == bestTrail[i+1].first) && isParagraph(huSentenceListPretty[bestTrail[i].first].words) );
    bool enParagraph = ( (bestTrail[i].second+1 == bestTrail[i+1].second) && isParagraph(enSentenceList[bestTrail[i].second].words) );

    if (
         ( huZero && enParagraph )
         ||
         ( enZero && huParagraph )
       )
    {
      ++i;
      continue;
    }

    // It is a zero-to-any, and the "any" is not a lonely paragraph-delimiter.
    if (
         ( huZero && !enParagraph )
         ||
         ( enZero && !huParagraph )
       )
//x     // It is a zero-to-any.
//x     if ( huZero || enZero )
    {
      // continue is not allowed here, because of the ++i in the else branch.

      double huRightBlock  = characterLength( bestTrail[i+1].first, bestTrail[i+2].first, huSentenceListPretty, utfCharCountingMode );
      double huMiddleBlock = characterLength( bestTrail[i].first,   bestTrail[i+1].first, huSentenceListPretty, utfCharCountingMode );
      double huLeftBlock   = characterLength( bestTrail[i-1].first, bestTrail[i].first,   huSentenceListPretty, utfCharCountingMode );
    
      double enRightBlock  = characterLength( bestTrail[i+1].second, bestTrail[i+2].second, enSentenceList, utfCharCountingMode );
      double enMiddleBlock = characterLength( bestTrail[i].second,   bestTrail[i+1].second, enSentenceList, utfCharCountingMode );
      double enLeftBlock   = characterLength( bestTrail[i-1].second, bestTrail[i].second,   enSentenceList, utfCharCountingMode );

      // A middleBlock-ok kozul az egyik nulla.
      double oldLeftRatio = (huLeftBlock+1)/(enLeftBlock+1);
      double newLeftRatio = (huLeftBlock+huMiddleBlock+1)/(enLeftBlock+enMiddleBlock+1);

      double oldRightRatio = (huRightBlock+1)/(enRightBlock+1);
      double newRightRatio = (huRightBlock+huMiddleBlock+1)/(enRightBlock+enMiddleBlock+1);

      double improvesLeft  = absValue(log(oldLeftRatio )) - absValue(log(newLeftRatio )) ;
      double improvesRight = absValue(log(oldRightRatio)) - absValue(log(newRightRatio)) ;

//x       std::cerr << "Rundle " << bestTrail[i].first << "," << bestTrail[i].second ; // << " \t" << absValue(log(oldRatio)) << "\t" << absValue(log(newRatio)) ;
//x       std::cerr << "\t" << huLeftBlock << "," << huRightBlock  << "\t" << enLeftBlock << "," << enRightBlock ;

      // Meg olyankor is megszuntetjuk, ha kicsit ront a megszuntetese, mert regenyekben nagyon ritka a zero-to-nonzero.
      const double improvementSlack = log(0.8);
      if ( (improvesLeft>improvementSlack) || (improvesRight>improvementSlack) )
      {
        bool eraseLeft = (improvesLeft>improvesRight);

        if (eraseLeft)
        {
          bestTrail.erase(bestTrail.begin()+i);
        }
        else
        {
          bestTrail.erase(bestTrail.begin()+i+1);
        }

//x         std::cerr << ", erasing." ;
      }
      else
      {
        ++i;
      }
//x       std::cerr << std::endl;
    }
    else
    {
      ++i;
    }
  }
}


// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrailStart( Trail& bestTrail,
                            const TrailScoresInterval& trailScoresInterval,
                            const double& qualityThreshold )
{
  const int window = 10;

  std::set<int> rundlesToKill;

  int trailSize = bestTrail.size();

  for ( int pos=1; pos<trailSize-1-window; ++pos )
  {
    double avg = trailScoresInterval( pos, pos+window );

    if (avg<qualityThreshold)
    {
      if (global_postprocessLogging)
      {
        std::cerr << "Thrown away at position " << pos
          << ", avarage " << avg << ", threshold " << qualityThreshold << std::endl;
      }

      for ( int j=pos; (j<pos+window) && (j<bestTrail.size()-1); ++j )
      {
        rundlesToKill.insert(j);
      }
    }
    else
    {
      // !!! Az elso olyan alkalommal, amikor az atlag magas, abba is hagyjuk a
      // gyilkolast, mert ez csak a fejlecek detektalasara van, a belso skipekkel
      // nem foglalkozik.
      break;
    }
  }

  removeRundles( bestTrail, rundlesToKill );
}

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrailEnd( Trail& bestTrail,
                            const TrailScoresInterval& trailScoresInterval,
                            const double& qualityThreshold )
{
  const int window = 10;

  std::set<int> rundlesToKill;

  int trailSize = bestTrail.size();

  for ( int pos=trailSize-1-window-1; pos>0; --pos )
  {
    double avg = trailScoresInterval( pos, pos+window );

    if (avg<qualityThreshold)
    {
      if (global_postprocessLogging)
      {
        std::cerr << "Thrown away at position " << pos
          << ", avarage " << avg << ", threshold " << qualityThreshold << std::endl;
      }

      for ( int j=pos; (j<pos+window) && (j<bestTrail.size()-1); ++j )
      {
        rundlesToKill.insert(j);
      }
    }
    else
    {
      // !!! Az elso olyan alkalommal, amikor az atlag magas, abba is hagyjuk a
      // gyilkolast, mert ez csak a fejlecek detektalasara van, a belso skipekkel
      // nem foglalkozik.
      break;
    }
  }

  removeRundles( bestTrail, rundlesToKill );
}

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrailStartAndEnd( Trail& bestTrail, const TrailScoresInterval& trailScoresInterval, double qualityThreshold )
{
  postprocessTrailStart( bestTrail, trailScoresInterval, qualityThreshold );
  postprocessTrailEnd  ( bestTrail, trailScoresInterval, qualityThreshold );
}

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrail( Trail& bestTrail, const TrailScoresInterval& trailScoresInterval, double qualityThreshold )
{
  const int window = 10;

  std::set<int> rundlesToKill;

  int trailSize = bestTrail.size();

  for ( int pos=1; pos<trailSize-1-window; ++pos )
  {
    double avg = trailScoresInterval( pos, pos+window );

    if (avg<qualityThreshold)
    {
      if (global_postprocessLogging)
      {
        std::cerr << "Thrown away at position " << pos
          << ", avarage " << avg << ", threshold " << qualityThreshold << std::endl;
      }

      for ( int j=pos; (j<pos+window) && (j<bestTrail.size()-1); ++j )
      {
        rundlesToKill.insert(j);
      }
    }
  }

  removeRundles( bestTrail, rundlesToKill );
}


// Erases rundles which are predominantly surrounded by not-one-to-one holes.
void postprocessTrailByTopology( Trail& bestTrail, double qualityThreshold )
{
  const int window = 10;

  std::set<int> rundlesToKill;

  int trailSize = bestTrail.size();

  for ( int pos=1; pos<trailSize-1-window; ++pos )
  {
    int huStart = bestTrail[pos].first;
    int enStart = bestTrail[pos].second;

    int huEnd   = bestTrail[pos+window].first;
    int enEnd   = bestTrail[pos+window].second;

    double sum=0;

    for ( int j=pos; j<pos+window; ++j )
    {
      sum += ( oneToOne(bestTrail,j) ? 1 : 0 ) ;
    }

    double avg = sum / window ;

    if (avg<qualityThreshold)
    {
      if (global_postprocessLogging)
      {
        std::cerr << "Thrown away at position " << pos
          << ", avarage " << avg << std::endl;
      }

      for ( int j=pos; (j<pos+window) && (j<bestTrail.size()-1); ++j )
      {
        rundlesToKill.insert(j);
      }
    }
  }

  removeRundles( bestTrail, rundlesToKill );
}


void trailToBisentenceList( const Trail& bestTrail, const TrailScores& trailScores, double qualityThreshold,
                            BisentenceList& bisentenceList )
{
  bisentenceList.clear();

  int trailSize = bestTrail.size();

  for ( int pos=0; pos<trailSize-1; ++pos )
  {
    if ( oneToOne(bestTrail,pos) && (trailScores(pos)>=qualityThreshold) )
    {
      bisentenceList.push_back(bestTrail[pos]);
    }
  }
}


// This is basically incorrect.
// Here we use the score of the right-hand segment to decide about the rundle.
//
// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void filterTrailByQuality( Trail& trail, const TrailScoresInterval& trailScoresInterval,
                           const double& qualityThreshold )
{
  Trail newTrail;

  newTrail.push_back(trail.front());
  for ( int i=1; i<trail.size()-1; ++i )
  {
    if ( trailScoresInterval(i) >= qualityThreshold )
    {
      newTrail.push_back(trail[i]);
    }
  }
  newTrail.push_back(trail.back());

  trail = newTrail;
}

void filterBisentenceListByQuality( BisentenceList& bisentenceList, const AlignMatrix& dynMatrix,
                                    const double& qualityThreshold )
{
  BisentenceList newBisentenceList;

  BisentenceListScores bisentenceListScores(bisentenceList,dynMatrix);

  for ( int i=0; i<bisentenceList.size(); ++i )
  {
    if ( bisentenceListScores(i) >= qualityThreshold )
    {
      newBisentenceList.push_back(bisentenceList[i]);
    }
  }

  bisentenceList = newBisentenceList;
}

//x typedef int RundlePosition;
//x 
//x // The set of rundles as a bipartite graph is always the disjoint union of stars.
//x // This function finds the leftmost and rightmost rundle of this star.
//x // Attention: NOT bisentence, rundle.
//x void detectBush( const Trail& bestTrail, const RundlePosition& inside, RundlePosition& left, RundlePosition& right )
//x {
//x   int huPos = bestTrail[inside].first;
//x   int enPos = bestTrail[inside].second;
//x   int cursor;
//x 
//x   cursor=inside;
//x   while (cursor!=0)
//x   {
//x     --cursor;
//x     if (bestTrail[cursor].first==huPos)
//x     {
//x     }
//x     else if (bestTrail[cursor].second==enPos)
//x     {
//x     }
//x     else
//x     {
//x       ++cursor;
//x       break;
//x     }
//x   }
//x   left=cursor;
//x 
//x   cursor=inside;
//x   while (cursor!=bestTrail.size())
//x   {
//x     ++cursor;
//x     if (bestTrail[cursor].first==huPos)
//x     {
//x     }
//x     else if (bestTrail[cursor].second==enPos)
//x     {
//x     }
//x     else
//x     {
//x       --cursor;
//x       break;
//x     }
//x   }
//x   right=cursor;
//x }


} // namespace Hunglish
