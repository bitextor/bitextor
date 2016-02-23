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

#include "bookToMatrix.h"
#include "translate.h"

#include <histogram.h>
#include <serializeImpl.h>

#include <iostream>
#include <fstream>
#include <algorithm>

namespace Hunglish
{


void bisentenceListToBicorpus(
                              const SentenceList& huSentenceListC, const SentenceList& enSentenceListC,
                              const BisentenceList& bisentenceList,
                              SentenceList& huBisentenceHalves, SentenceList& enBisentenceHalves 
                              )
{
  huBisentenceHalves.clear();
  enBisentenceHalves.clear();

  for ( int i=0; i<bisentenceList.size(); ++i )
  {
    huBisentenceHalves.push_back( huSentenceListC[ bisentenceList[i].first  ] );
    enBisentenceHalves.push_back( enSentenceListC[ bisentenceList[i].second ] );
  }
}

class SimilarityScorer
{
public:
  virtual double operator()( const Phrase& hu, const Phrase& en ) const = 0;
};

class NullScorer : public SimilarityScorer
{
public:
  double operator()( const Phrase& hu, const Phrase& en ) const
  {
    return 0;
  }
};

class GaleScorer : public SimilarityScorer
{
public:
  double operator()( const Phrase& hu, const Phrase& en ) const
  {
    return closeness( characterLength(hu), characterLength(en) );
  }
};

class IdentityScorer : public SimilarityScorer
{
public:
  double operator()( const Phrase& hu, const Phrase& en ) const
  {
    return scoreByIdentity(hu,en);
  }
};

class CombinatorScorer : public SimilarityScorer
{
public:

  CombinatorScorer( const SimilarityScorer& s1_, const SimilarityScorer& s2_, double mul_ ) : s1(s1_), s2(s2_), mul(mul_) {}

  double operator()( const Phrase& hu, const Phrase& en ) const
  {
    return s1(hu,en) + mul*s2(hu,en);
  }
private:
  const SimilarityScorer& s1;
  const SimilarityScorer& s2;
  const double mul;

};

// The bigger the better.
double averageSimilarity( const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                          SimilarityScorer& similarityScorer,
                          DiscreteDoubleMap& distribution )
{
  double sum(0);
  distribution.clear();

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    const Phrase& hu = huSentenceList[i].words;
    const Phrase& en = enSentenceList[i].words;

    double value = similarityScorer( hu, en ) ;

    sum += value;
    ++distribution[value];
  }

  bool logBin = false;
  if (logBin)
  {
    SmoothDoubleMap binned;
    distribution.binning( true/*logBin*/, false/*dontShowZeros*/, 1.2/*step*/, binned );

    for ( SmoothDoubleMap::const_iterator it=binned.begin(); it!=binned.end(); ++it )
    {
      std::cerr << it->first << "\t" << it->second << std::endl;
    }
  }

  return ( sum / huSentenceList.size() );
}


void similarityEvaluator( const DictionaryItems& dictionary,
                          const SentenceList& huSentenceListPretty, const SentenceList& enSentenceListPretty )
{
  SentenceList huSentenceList, enSentenceList;

  normalizeTextsForIdentity( dictionary, huSentenceListPretty, enSentenceListPretty, huSentenceList, enSentenceList );

  for ( int i=0; i<5; ++i )
  {
    std::cout << huSentenceList[i].words << " --- " << enSentenceList[i].words << std::endl;
  }

  DiscreteDoubleMap distribution;

  IdentityScorer identityScorer;
  GaleScorer     galeScorer;

  CombinatorScorer similarityScorer( identityScorer, galeScorer, 1.0 );

  double realSimilarity = averageSimilarity( huSentenceList, enSentenceList, similarityScorer, distribution );

  std::cerr << "Real similarity " << realSimilarity << std::endl;

  SentenceList huSentenceListWarped(huSentenceList);
  SentenceList enSentenceListWarped(enSentenceList);

  huSentenceListWarped.insert( huSentenceListWarped.begin(), huSentenceListWarped.back() );
  huSentenceListWarped.resize( huSentenceListWarped.size()-1 );
  double warpedSimilarity1 = averageSimilarity( huSentenceListWarped, enSentenceListWarped, similarityScorer, distribution );

  std::cerr << "Placebo similarity #1 " << warpedSimilarity1 << std::endl;

//x   huSentenceListWarped.insert( huSentenceListWarped.begin(), huSentenceListWarped.back() );
//x   huSentenceListWarped.resize( huSentenceListWarped.size()-1 );
//x   double warpedSimilarity2 = averageSimilarity( huSentenceListWarped, enSentenceListWarped, similarityScorer, distribution );
//x 
//x   std::cerr << "Placebo similarity #2 " << warpedSimilarity2 << std::endl;
//x 
//x   huSentenceListWarped.insert( huSentenceListWarped.begin(), huSentenceListWarped.back() );
//x   huSentenceListWarped.resize( huSentenceListWarped.size()-1 );
//x   double warpedSimilarity3 = averageSimilarity( huSentenceListWarped, enSentenceListWarped, similarityScorer, distribution );
//x 
//x   std::cerr << "Placebo similarity #3 " << warpedSimilarity3 << std::endl;
//x 
//x   std::random_shuffle( huSentenceListWarped.begin(), huSentenceListWarped.end() );
//x   double randomSimilarity = averageSimilarity( huSentenceListWarped, enSentenceListWarped, similarityScorer, distribution );
//x 
//x   std::cerr << "Random  similarity    " << randomSimilarity << std::endl;
}

void main_similarityEvaluatorTool(int argC, char* argV[])
{
  if (argC!=4)
    throw "argument error";

  const char* dicFilename = argV[1];
  const char* huFilename  = argV[2];
  const char* enFilename  = argV[3];

  DictionaryItems dictionary;
  std::ifstream dis(dicFilename);
  dictionary.read(dis);

  SentenceList huSentenceList;
  SentenceList enSentenceList;

  std::ifstream hus(huFilename);
  huSentenceList.readNoIds(hus);
  std::ifstream ens(enFilename);
  enSentenceList.readNoIds(ens);

  if (huSentenceList.size()!=enSentenceList.size())
  {
    std::cerr << "Number of sentences not matching: " 
      << huSentenceList.size() << " versus " << enSentenceList.size() << "."
      << std::endl;
    throw "data error";
  }
  else
  {
    std::cerr << huSentenceList.size() << " bisentences read." << std::endl;
  }

  similarityEvaluator( dictionary, huSentenceList, enSentenceList );
}

} // namespace Hunglish
