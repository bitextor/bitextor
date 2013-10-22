/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_TRAILPOSTPROCESSORS_H
#define __HUNGLISH_ALIGNMENT_TRAILPOSTPROCESSORS_H

#include "alignment.h"

namespace Hunglish
{

// Helper class that calculates scores of holes.
class TrailScores
{
public:
  TrailScores( const Trail& trail_, const AlignMatrix& dynMatrix_ );
  // The score of the jth segmentum. The bigger the better.
  double operator()( int j ) const;

private:
  const Trail& trail;
  const AlignMatrix& dynMatrix;
};


class SentenceList;


// Helper class that calculates scores of segmentums.
class TrailScoresInterval
{
public:
  TrailScoresInterval( const Trail& trail_, const AlignMatrix& dynMatrix_,
    const SentenceList& huSentenceList_, const SentenceList& enSentenceList_ );

  // The average score of the jth segmentum. The bigger the better.
  // Division is by the maximum of the Hungarian and English intervals.
  // This is a somewhat arbritary decision, and goes very badly with the
  // scoring of the knight's moves. But we really have no better choice.
  // 
  // Also, the method applies some very ugly hacks to avoid the effect of
  // paragraph-delimiters. It strips both intervals of <p>s, and
  // modifies the dynMatrix-based score assuming that all <p>s got paired.
  // except surplus <p>s.
  double scoreSegmentum( const Rundle& start, const Rundle& end ) const;

  // The score of a segment identified by its index.
  double operator()( int j ) const;
  // The score of a union of segments identified by its start and end rundles' index.
  // Both these methods rely on scoreSegmentum():
  // This means an important thing: the score only depends
  // on the start and end rundle, not the rundles in between.
  double operator()( int j, int k ) const;

private:
  const Trail& trail;
  const AlignMatrix& dynMatrix;
  const SentenceList& huSentenceList;
  const SentenceList& enSentenceList;
};

// Helper class that calculates scores of one-to-one holes.
class BisentenceListScores
{
public:
  BisentenceListScores( const BisentenceList& bisentenceList_, const AlignMatrix& dynMatrix_ );
  // The score of the jth bisentence. The bigger the better.
  double operator()( int j ) const;

private:
  const BisentenceList& bisentenceList;
  const AlignMatrix& dynMatrix;
};

void removeRundles( Trail& trail, const std::set<int>& rundlesToKill );

// In cautious mode, auto-aligned rundles are thrown away if
// their left or right neighbour holes are not one-to-one.
// From the point of view of the resultant bisentences:
// In cautious mode, one-to-one bisentences are thrown away if
// they have left or right neighbours which are not one-to-one.
// This of course dramatically improves precision while slightly degrading recall.
void cautiouslyFilterTrail( Trail& bestTrail );

void spaceOutBySentenceLength( Trail& bestTrail, 
                 const SentenceList& huSentenceListPretty,
                 const SentenceList& enSentenceList,
		 bool utfCharCountingMode );

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrailStart( Trail& bestTrail,
                            const TrailScoresInterval& trailScoresInterval,
                            const double& qualityThreshold );

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrailStartAndEnd( Trail& bestTrail,
                                  const TrailScoresInterval& trailScoresInterval,
                                  double qualityThreshold );

// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void postprocessTrail( Trail& bestTrail, 
                       const TrailScoresInterval& trailScoresInterval, 
                       double qualityThreshold );


// Throws away rundles which are predominantly surrounded by not-one-to-one holes.
void postprocessTrailByTopology( Trail& bestTrail, double qualityThreshold );


// Only collect bisentences with score at least qualityThreshold.
void trailToBisentenceList( const Trail& bestTrail, const TrailScores& trailScores, double qualityThreshold,
                            BisentenceList& bisentenceList );

// This is basically incorrect.
// Here we use the score of the right-hand segment to decide about the rundle.
//
// The function gets a nonconst reference to bestTrail.
// On the other hand, it gets a const reference to bestTrail, through trailScoresInterval.
// Therefore, the function may only modify bestTrail after it finished reading trailScoresInterval.
void filterTrailByQuality( Trail& trail, const TrailScoresInterval& trailScoresInterval,
                           const double& qualityThreshold );

void filterBisentenceListByQuality( BisentenceList& bisentenceList, const AlignMatrix& dynMatrix,
                                    const double& qualityThreshold );

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_TRAILPOSTPROCESSORS_H
