/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_ALIGNMENT_H
#define __HUNGLISH_ALIGNMENT_ALIGNMENT_H

#include "quasiDiagonal.h"

#include <string>
#include <set>

namespace Hunglish
{

// Simply double values for each sentence. Right now we store sentence lengths in them.
typedef std::vector<double> SentenceValues;

// See quasiDiagonal.h
typedef QuasiDiagonal<double> AlignMatrix;

// Contains directions, a bit like a force field.
typedef QuasiDiagonal<unsigned char> TrelliMatrix;

// A Rundle (x,y) cuts the bitext into two sub-bitexts:
// [0,x)+[0,y) and [x,huSize)+[y,enSize).
typedef std::pair<int,int> Rundle;

// A Trail is a strictly ordered list of Rundles.
// It cuts the bitext into small bitexts.
// Such a small bitext is called a hole or segmentum.
// A hole can contion zero Hungarian sentence,
// it can contain zero English sentences, but not both.
// A Trail is sometimes referred to as a Ladder.
typedef std::vector<Rundle> Trail;

// A BisentenceList is formally identical to a Trail, but semantically very different.
// It represents an ordered list of bisentences.
// There are some functions which utilize the formal identity,
// manipulating both structures.
typedef std::vector< std::pair<int,int> > BisentenceList;

// OBSOLETE:
// TrailValues gives scores to the Rundles of a Trail (of the same size).
// Conceptually TrailValues should be attached to Trails.
// A TrailValues structure always accompanies a Trails list,
// but their consistency must be maintained by hand, pre-OO-style. (TODO)
// typedef std::vector<double> TrailValues;

// OBSOLETE:
// Has the exactly same relation to BisentenceList as
// a TrailValues has to a Trail. But note that these 
// scores mark the confidence in a bisentence. This is
// very different from the confidence in a rundle.
// typedef std::vector<double> BisentenceValues;

double closeness( double twoSentenceLength, double oneSentenceLength );

const double skipScore = -0.3;


// The main align function,
// Gets a confidence value for every sentence-pair,
// and sentence lengths for each sentence (for a a Gale-Church-like scoring).
// Returns a trail with the best total score, and the computed dynMatrix matrix:
// dynMatrix[huPos][enPos] gives the similarity of the [0,huPos) and [0,enPos) intervals.
void align( const AlignMatrix& w, const SentenceValues& huLength, const SentenceValues& enLength,
            Trail& bestTrail, AlignMatrix& dynMatrix );


bool oneToOne( const Trail& bestTrail, int pos );

// Collect bisentences.
void trailToBisentenceList( const Trail& bestTrail,
                            BisentenceList& bisentenceList );

// Score precision-recall of a BisentenceList according to a hand-aligned bicorpus.
// For best results, zero-to-many holes of the hand-alignment should be subdivided to zero-to-ones.
// Builds the manual bisentencelist. The compared sets consist of Bisentences.
double scoreBisentenceList( const BisentenceList& bisentenceList, const Trail& trailHand );

// The same precision-recall calculation for Trails. The compared sets consist of Rundles.
double scoreTrail         ( const Trail&          trailAuto,      const Trail& trailHand );


const int outsideOfRadiusValue = -1000000;
const int insideOfRadiusValue  = 0;

// Fills the complement of the radius of the trail with minus infties.
// The return value true means success. Failure means that during the fill,
// we intersected the outside of the quasidiagonal area.
// In this case, the operation is not finished.
bool borderDetailedAlignMatrix( AlignMatrix& m, const Trail& trail, int radius );

// What the name implies.
void dumpAlignMatrix( const AlignMatrix& m, bool graphical );

template <class T>
void dumpAlignMatrix( const QuasiDiagonal<T>& alignMatrix );

void dumpAlignMatrix( const QuasiDiagonal<int>& alignMatrix, bool graphical );

void dumpTrelliMatrix( const TrelliMatrix& trellis );


} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_ALIGNMENT_H
