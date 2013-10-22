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

// Hajjaj. Ez komoly warning. De nemigen tudom kikerulni.
#pragma warning ( disable : 4503 )

#include "cooccurrence.h"

#include "networkFlow.h"

#include "dictionary.h"
#include "translate.h" // For DumbMultiDictionary

#include <serializeImpl.h>
#include <stringsAndStreams.h>

#include <timer.h>

#include <map>
#include <sstream>
#include <iostream>
#include <set>
#include <cassert>
#include <algorithm>
#include <iterator>

#include <portableHash.h>

namespace Hunglish
{

class BiWordHash
{
public:
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;
  bool operator()(const BiWord& key1, const BiWord& key2) const
  {
    return key1>key2;
  }
  inline size_t operator()( const BiWord& x ) const
  {
    unsigned long h = 0;
    int i;
    for ( i=0; i<x.first.size(); ++i )
    {
      h = 5*h + x.first[i];
    }
    for ( i=0; i<x.second.size(); ++i )
    {
      h = 5*h + x.second[i];
    }
    return size_t(h);
  }
};

typedef EXTNAMESPACE::hash_map< BiWord, int, BiWordHash > CooccurenceMap_HashVersion;
typedef std::map< BiWord, int > CooccurenceMap_MapVersion;
typedef CooccurenceMap_HashVersion CooccurenceMap;

class CorpusConstellation
{
public:
  void addBisentence( const Sentence& huSentence, const Sentence& enSentence );

  // Obsolete, much faster but much less meaningful. Multiply instead of minimize.
  // For "az A es a B es a C" - "the A and the B and the C"
  // Pietra coocc(es,the) is 6, Melamed is 2.
  void addBisentence_Pietra( const Sentence& huSent, const Sentence& enSent );

  void updateBisentence( const int& bisentenceId, const Sentence& huSentence, const Sentence& enSentence );

  void build( const SentenceList& huSentenceList, const SentenceList& enSentenceList );

  // These are generalized versions for dealing with multiple-word terms:
  void addBisentence( const Sentence& huSent, const Sentence& enSent,
                      int huTermLength, int enTermLength );
  void build( const SentenceList& huSentenceList, const SentenceList& enSentenceList,
              int huTermLength, int enTermLength );

  void removeDictionaryItem( const int& bisentenceId, const BiWord& biWord, int occ );
  void removeDictionaryItem( const BiWord& biWord );

  void filterForPromisingEntries( double minScore, int minCoocc );

  void getTopScorers( BiWords& biWords,
                      int itemsRequested, double minScore, int minCoocc );

  void getOccurences( const BiWord& biWord, int& huOcc, int& enOcc, int& coOcc ) const;

// These should be private, with const getters:
public:
  SentenceList huSentenceList;
  SentenceList enSentenceList;

  // These numbers must be consistent with the sentenceLists exactly:
  FrequencyMap huOccurenceMap;
  FrequencyMap enOccurenceMap;
  
  // This is just a LOWER bound on the real cooccurrence number. We may lie whenever we want to:
  CooccurenceMap cooccurenceMap;
};

void CorpusConstellation::getOccurences( const BiWord& biWord, int& huOcc, int& enOcc, int& coOcc ) const
{
  FrequencyMap::const_iterator ft;
  ft = huOccurenceMap.find(biWord.first);
  if (ft==huOccurenceMap.end())
  {
    huOcc=0;
  }
  else
  {
    huOcc = ft->second;
  }

  ft = enOccurenceMap.find(biWord.second);
  if (ft==enOccurenceMap.end())
  {
    enOcc=0;
  }
  else
  {
    enOcc = ft->second;
  }

  CooccurenceMap::const_iterator ct = cooccurenceMap.find(biWord);
  if (ct==cooccurenceMap.end())
  {
    coOcc=0;
  }
  else
  {
    coOcc = ct->second;
  }
}


void CorpusConstellation::addBisentence_Pietra( const Sentence& huSent, const Sentence& enSent )
{
  huSentenceList.push_back(huSent);
  enSentenceList.push_back(enSent);

  const WordList& huWords = huSent.words;
  const WordList& enWords = enSent.words;

  huOccurenceMap.build(huWords);
  enOccurenceMap.build(enWords);

  int j,k;
  for ( j=0; j<huWords.size(); ++j )
  {
    for ( k=0; k<enWords.size(); ++k )
    {
      ++ cooccurenceMap[ std::make_pair(huWords[j],enWords[k]) ];
    }
  }
}

void CorpusConstellation::addBisentence/*_Melamed*/( const Sentence& huSent, const Sentence& enSent )
{
  huSentenceList.push_back(huSent);
  enSentenceList.push_back(enSent);

  const WordList& huWords = huSent.words;
  const WordList& enWords = enSent.words;

  huOccurenceMap.build(huWords);
  enOccurenceMap.build(enWords);

  FrequencyMap huOccurenceMapLocal, enOccurenceMapLocal;
  huOccurenceMapLocal.build(huWords);
  enOccurenceMapLocal.build(enWords);

  for ( FrequencyMap::const_iterator it=huOccurenceMapLocal.begin(); it!=huOccurenceMapLocal.end(); ++it )
  {
    int huOcc = it->second;
    for ( FrequencyMap::const_iterator jt=enOccurenceMapLocal.begin(); jt!=enOccurenceMapLocal.end(); ++jt )
    {
      int enOcc = jt->second;
      int min = ( huOcc<enOcc ? huOcc : enOcc );

      cooccurenceMap[ std::make_pair(it->first,jt->first) ] += min;
    }
  }
}

const char termDelimiter = ' ';

inline std::string termToWord( const WordList& words, int pos, int termLength )
{
  assert( termLength>0 );
  assert( pos+termLength <= words.size() );

  Word term = words[pos];
  for ( int k=pos+1; k<pos+termLength; ++k )
  {
    term += termDelimiter;
    term += words[k];
  }

  return term;
}

inline WordList wordToTerm( const Word& composite )
{
  WordList term;
  split( composite, term, termDelimiter );
  return term;
}

void CorpusConstellation::addBisentence( const Sentence& huSent, const Sentence& enSent,
                                        int huTermLength, int enTermLength )
{
  assert(huTermLength  > 0);
  assert(huTermLength <= 3);
  assert(enTermLength  > 0);
  assert(enTermLength <= 3);

  huSentenceList.push_back(huSent);
  enSentenceList.push_back(enSent);

  const WordList& huWords = huSent.words;
  const WordList& enWords = enSent.words;

  int j,k;

  for ( j=0; j<=(int)huWords.size()-huTermLength; ++j )
  {
    huOccurenceMap.add( termToWord( huWords, j, huTermLength ) );
  }
  for ( j=0; j<=(int)enWords.size()-enTermLength; ++j )
  {
    enOccurenceMap.add( termToWord( enWords, j, enTermLength ) );
  }

  for ( j=0; j<=(int)huWords.size()-huTermLength; ++j )
  {
    for ( k=0; k<=(int)enWords.size()-enTermLength; ++k )
    {
      ++cooccurenceMap
        [ 
          std::make_pair(
          termToWord( huWords, j, huTermLength ),
          termToWord( enWords, k, enTermLength )
          ) 
        ];
    }
  }
}

void CorpusConstellation::build( const SentenceList& huSentenceList, const SentenceList& enSentenceList )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    addBisentence( huSentenceList[i], enSentenceList[i] );
  }
}

void CorpusConstellation::build( const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                                 int huTermLength, int enTermLength )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    addBisentence( huSentenceList[i], enSentenceList[i], huTermLength, enTermLength );
  }
}

void addOrSubtractSentencePair( 
                    FrequencyMap& huOccurenceMap, FrequencyMap& enOccurenceMap,
                    CooccurenceMap& cooccurenceMap,
                    const Sentence& huSentence, const Sentence& enSentence,
                    bool subtract )
{
  const WordList& huWords = huSentence.words;
  const WordList& enWords = enSentence.words;

  if (subtract)
  {
    huOccurenceMap.remove(huWords);
    enOccurenceMap.remove(enWords);
  }
  else
  {
    huOccurenceMap.build(huWords);
    enOccurenceMap.build(enWords);
  }

  int j,k;
  for ( j=0; j<huWords.size(); ++j )
  {
    for ( k=0; k<enWords.size(); ++k )
    {
      int& value = cooccurenceMap[ std::make_pair(huWords[j],enWords[k]) ];

      if (subtract)
      {
        --value;
        if (value<=0)
        {
          // Ez ketszeresere optimalizalhato.
          cooccurenceMap.erase( std::make_pair(huWords[j],enWords[k]) );
        }
      }
      else
      {
        ++value;
      }
    }
  }
  
}

/*
void addOrSubtractCooccurenceMap( CooccurenceMap& plusCooccurenceMap, const CooccurenceMap& minusCooccurenceMap,
                                  bool subtract )
{
  for ( CooccurenceMap::const_iterator ct=minusCooccurenceMap.begin(); ct!=minusCooccurenceMap.end(); ++ct )
  {
    int coocc = ct->second ;
    const std::pair<Word,Word>& pair = ct->first;

    CooccurenceMap::iterator ft = plusCooccurenceMap.find(pair);
    if (ft==plusCooccurenceMap.end())
    {
      std::cerr << "Value not found. Probable consistency error in CooccurenceMap" << std::endl;
    }
    else 
    {
      if (subtract)
      {
        ft->second -= coocc;
      }
      else
      {
        ft->second += coocc;
      }

      if (ft->second == 0)
      {
        plusCooccurenceMap.erase(ft);
      }
      else if (ft->second < 0)
      {
        std::cerr << "Negative value. Probable consistency error in CooccurenceMap" << std::endl;
        plusCooccurenceMap.erase(ft);
      }
    }
  }
}
*/

void CorpusConstellation::updateBisentence( const int& bisentenceId, 
                                            const Sentence& huSent, const Sentence& enSent )
{
  addOrSubtractSentencePair( 
                    huOccurenceMap, enOccurenceMap,
                    cooccurenceMap,
                    huSentenceList[bisentenceId], enSentenceList[bisentenceId],
                    true/*subtract*/ );

  addOrSubtractSentencePair( 
                    huOccurenceMap, enOccurenceMap,
                    cooccurenceMap,
                    huSent, enSent,
                    false/*subtract*/ );

  huSentenceList[bisentenceId] = huSent;
  enSentenceList[bisentenceId] = enSent;
}

int contains( const Word& word, const Sentence& sentence )
{
  int occ(0);
  for ( int i=0; i<sentence.words.size(); ++i )
  {
    if (sentence.words[i]==word)
      ++occ;
  }
  return occ;
}

// This is not on the performance-critical path.
int contains( const Phrase& phrase, const Sentence& sentence )
{
  int occ(0);
  for ( int i=0; i<(int)sentence.words.size()-(int)phrase.size()+1; )
  {
    bool found = true;
    for ( int j=0; j<phrase.size(); ++j )
    {
      if (sentence.words[i+j]!=phrase[j])
      {
        found = false;
        break;
      }
    }

    if (found)
    {
      ++occ;
      i += phrase.size();
    }
    else
    {
      ++i;
    }
  }
  return occ;
}

void removeDictionaryItem( Sentence& huSentence, Sentence& enSentence,
                           const BiWord& biWord, int occ = -1 ) /* -1 means remove all occurrences.*/
{
  {
    Sentence& sentence = huSentence;
    // std::cerr << "HB " << sentence.words << std::endl;
    int leftOcc=occ;
    const Word& word = biWord.first;

    for ( int i=0; i<sentence.words.size(); ++i )
    {
      if (sentence.words[i]==word)
      {
        sentence.words.erase(sentence.words.begin()+i);
        --i;
        --leftOcc;
        if (leftOcc==0)
          break;
      }
    }
    if (leftOcc>0)
    {
      std::cerr << "removeDictionaryItem inconsistency" << std::endl;
      throw "internal error";
    }
    // std::cerr << "HA " << sentence.words << std::endl;
  }

  {
    Sentence& sentence = enSentence;
    // std::cerr << "EB " << sentence.words << std::endl;
    int leftOcc=occ;
    const Word& word = biWord.second;

    for ( int i=0; i<sentence.words.size(); ++i )
    {
      if (sentence.words[i]==word)
      {
        sentence.words.erase(sentence.words.begin()+i);
        --i;
        --leftOcc;
        if (leftOcc==0)
          break;
      }
    }
    if (leftOcc>0)
    {
      std::cerr << "removeDictionaryItem inconsistency" << std::endl;
      throw "internal error";
    }
    // std::cerr << "EA " << sentence.words << std::endl;
  }
}

bool removePhrase( WordList& wordList, const Phrase& phrase )
{
  for ( int i=0; i<(int)wordList.size()-(int)phrase.size()+1; ++i )
  {
    bool found = true;
    for ( int j=0; j<phrase.size(); ++j )
    {
      if (wordList[i+j]!=phrase[j])
      {
        found = false;
        break;
      }
    }

    if (found)
    {
      wordList.erase( wordList.begin()+i, wordList.begin()+i+phrase.size() );
      return true;
    }
  }
  return false;
}

void removeComplexDictionaryItem( Sentence& huSentence, Sentence& enSentence,
                           const DictionaryItem& biTerm, int occ = -1 ) /* -1 means remove all occurrences.*/
{
  {
    Sentence& sentence = huSentence;
    // std::cerr << "HB " << sentence.words << std::endl;
    int localOcc=occ;
    const Phrase& phrase = biTerm.first;

    while (true)
    {
      bool found = removePhrase(sentence.words, phrase);
      if (found)
      {
        --localOcc;
      }
      if ( !found || (localOcc==0) )
        break;
    }

    if (localOcc>0)
    {
      std::cerr << "removeDictionaryItem inconsistency" << std::endl;
      throw "internal error";
    }
    // std::cerr << "HA " << sentence.words << std::endl;
  }

  {
    Sentence& sentence = enSentence;
    // std::cerr << "EB " << sentence.words << std::endl;
    int localOcc=occ;
    const Phrase& phrase = biTerm.second;

    while (true)
    {
      bool found = removePhrase(sentence.words, phrase);
      if (found)
      {
        --localOcc;
      }
      if ( !found || (localOcc==0) )
        break;
    }

    if (localOcc>0)
    {
      std::cerr << "removeDictionaryItem inconsistency" << std::endl;
      throw "internal error";
    }
    // std::cerr << "EA " << sentence.words << std::endl;
  }
}

void CorpusConstellation::removeDictionaryItem( const int& bisentenceId, const BiWord& biWord, int occ )
{
  Sentence huSentence(huSentenceList[bisentenceId]);
  Sentence enSentence(enSentenceList[bisentenceId]);

  Hunglish::removeDictionaryItem( huSentence, enSentence, biWord, occ );

  updateBisentence( bisentenceId, huSentence, enSentence );
}

void CorpusConstellation::removeDictionaryItem( const BiWord& biWord )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    Sentence& huSent = huSentenceList[i];
    Sentence& enSent = enSentenceList[i];

    int huOcc = contains(biWord.first , huSent);
    int enOcc = contains(biWord.second, enSent);
    int occ = ( huOcc<enOcc ? huOcc : enOcc );

    if ( occ>0 )
    {
      removeDictionaryItem( i, biWord, occ );
    }
  }
}

void removeDictionaryItem( SentenceList& huSentenceList, SentenceList& enSentenceList,
                           const BiWord& biWord )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    Sentence& huSent = huSentenceList[i];
    Sentence& enSent = enSentenceList[i];

    int huOcc = contains(biWord.first , huSent);
    int enOcc = contains(biWord.second, enSent);
    int occ = ( huOcc<enOcc ? huOcc : enOcc );

    if ( occ>0 )
    {
      removeDictionaryItem( huSent, enSent, biWord, occ );
    }
  }
}

void removeComplexDictionaryItem( SentenceList& huSentenceList, SentenceList& enSentenceList,
                           const DictionaryItem& biTerm )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    Sentence& huSent = huSentenceList[i];
    Sentence& enSent = enSentenceList[i];

    int huOcc = contains(biTerm.first , huSent);
    int enOcc = contains(biTerm.second, enSent);
    int occ = ( huOcc<enOcc ? huOcc : enOcc );

    if ( occ>0 )
    {
      removeComplexDictionaryItem( huSent, enSent, biTerm, occ );
    }
  }
}


// This should be done after removeStopwords, simply because of bilanguage words like
// "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
void removeIdenticals( SentenceList& huSentenceList, SentenceList& enSentenceList,
                       BiWords& idTranslations )
{
  assert(huSentenceList.size()==enSentenceList.size());

  idTranslations.clear();

  std::set<Word> words;

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    WordList& huWords = huSentenceList[i].words;
    WordList& enWords = enSentenceList[i].words;

    WordList huWordsSorted = huWords;
    std::sort( huWordsSorted.begin(), huWordsSorted.end() );
    WordList enWordsSorted = enWords;
    WordList commonWords;
    std::sort( enWordsSorted.begin(), enWordsSorted.end() );
    std::set_intersection( 
      huWordsSorted.begin(), huWordsSorted.end(),
      enWordsSorted.begin(), enWordsSorted.end(),
      std::inserter(commonWords, commonWords.begin())
      );

    for ( int j=0; j<commonWords.size(); ++j )
    {
      Word word = commonWords[j];

      words.insert(word);

      removeDictionaryItem( huSentenceList[i], enSentenceList[i],
        std::make_pair(word,word) );
    }
  }

  for ( std::set<Word>::const_iterator it=words.begin(); it!=words.end(); ++it )
  {
    const Word& word = *it;
    idTranslations.push_back(std::make_pair(word,word));
    std::cerr << word << " ";
  }

  std::cerr << std::endl;
}

void removeHapaxes( SentenceList& huSentenceList, SentenceList& enSentenceList,
                    BiWords& hapaxTranslations )
{
  FrequencyMap huOccurenceMap, enOccurenceMap;
  huOccurenceMap.build(huSentenceList);
  enOccurenceMap.build(enSentenceList);

  assert(huSentenceList.size()==enSentenceList.size());

  hapaxTranslations.clear();

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    WordList& huWords = huSentenceList[i].words;
    WordList& enWords = enSentenceList[i].words;

    int j;
    int huPaxNum(0);
    int enPaxNum(0);
    Word huPax,enPax;

    for ( j=0; j<huWords.size(); )
    {
      if (huOccurenceMap[huWords[j]]==1)
      {
        ++huPaxNum;
        huPax = huWords[j];
        huWords.erase(huWords.begin()+j);
      }
      else
      {
        ++j;
      }
    }

    for ( j=0; j<enWords.size(); )
    {
      if (enOccurenceMap[enWords[j]]==1)
      {
        ++enPaxNum;
        enPax = enWords[j];
        enWords.erase(enWords.begin()+j);
      }
      else
      {
        ++j;
      }
    }

    if ((huPaxNum==1)&&(enPaxNum==1))
    {
      hapaxTranslations.push_back( std::make_pair(huPax,enPax) );
    }
  }
}


/*
void buildCooccurenceMap( const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                          CooccurenceMap& cooccurenceMap )
{
  assert(huSentenceList.size()==enSentenceList.size());
  cooccurenceMap.clear();

  int total(0);

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    const Sentence& enSent = enSentenceList[i];
    const Sentence& huSent = huSentenceList[i];

    const WordList& huWords = huSent.words;
    const WordList& enWords = enSent.words;

    int j,k;
    for ( j=0; j<huWords.size(); ++j )
    {
      for ( k=0; k<enWords.size(); ++k )
      {
        ++ cooccurenceMap[ std::make_pair(huWords[j],enWords[k]) ];
        ++total;
      }
    }
  }

  std::cerr << total << " cooccurrence instances." << std::endl;
}
*/

// TODO Ez nagyon el fog avulni, lasd EMIM pontozas.
inline double scoreEntry( int huocc, int enocc, int coocc )
{

  // Ennyiszer kell lassuk a szot mindket oldalon, hogy egy kicsit is erdekeljen.
  const int quasiglobal_minimalMonoOccurence = 5; // Hosszu ideig itt 10 volt.

  coocc = (coocc<0 ? 0 : coocc);

  int max = (huocc>enocc ? huocc : enocc );
  int min = (huocc<enocc ? huocc : enocc );

  return ( (min<quasiglobal_minimalMonoOccurence) ? 0 : 1.0*coocc / max  );
}

void CorpusConstellation::filterForPromisingEntries( double minScore, int minCoocc )
{
  for ( CooccurenceMap::iterator ct=cooccurenceMap.begin(); ct!=cooccurenceMap.end(); )
  {
    int coocc = ct->second ;
    int huocc = huOccurenceMap[ct->first.first];
    int enocc = enOccurenceMap[ct->first.second];

    double score = scoreEntry( huocc, enocc, coocc );

    if ( (score>minScore) && (coocc>=minCoocc) )
    {
      ++ct;
    }
    else
    {
      std::cerr << "Le kell tesztelni, hogy ez jo-e" << std::endl;
      throw "maybe error";
      CooccurenceMap::iterator nt=ct;
      ++ct;
      cooccurenceMap.erase(nt);

      // Ez basszus nem fordul a hulye Linuxon. Eljen a Windows:
      // ct = cooccurenceMap.erase(ct);
    }
  }
}


void CorpusConstellation::getTopScorers( BiWords& biWords,
                                         int itemsRequested, double minScore, int minCoocc )
{
  biWords.clear();

  typedef std::multimap< double, std::pair<std::string,std::string> > ReverseCooccurenceMap;
  ReverseCooccurenceMap reverseCooccurenceMap;

  for ( CooccurenceMap::const_iterator ct=cooccurenceMap.begin(); ct!=cooccurenceMap.end(); ++ct )
  {
    int coocc = ct->second ;
    int huocc = huOccurenceMap[ct->first.first];
    int enocc = enOccurenceMap[ct->first.second];

    double score = scoreEntry( huocc, enocc, coocc );

    if ( (score>=minScore) && (coocc>=minCoocc) )
    {
      reverseCooccurenceMap.insert(ReverseCooccurenceMap::value_type( score, ct->first ));
    }
  }

  int n(0);
  bool conflict(false);
  std::set<Word> huWordsInRound, enWordsInRound;
  // Hihetetlen, de const_reverse_iterator -ral nem fordul le VC++ alatt.
  for ( ReverseCooccurenceMap::reverse_iterator rt=reverseCooccurenceMap.rbegin(); rt!=reverseCooccurenceMap.rend(); ++rt )
  {
    ++n;

    const Word& huWord = rt->second.first;
    const Word& enWord = rt->second.second;

    if (huWordsInRound.find(huWord)!=huWordsInRound.end())
    {
      std::cerr << "Conflict, ending round with (" << huWord << ")- " << enWord << " "
        << " after " << n << " items." << std::endl;
      conflict=true;
      break;
    }
    if (enWordsInRound.find(enWord)!=enWordsInRound.end())
    {
      std::cerr << "Conflict, ending round with  " << huWord << " -(" << enWord << ")"
        << " after " << n << " items." << std::endl;
      conflict=true;
      break;
    }
    huWordsInRound.insert(huWord);
    enWordsInRound.insert(enWord);

    std::cout
      << huWord << "\t" << enWord << "\t" << rt->first 
      << "\t" << huOccurenceMap[huWord]
      << "\t" << enOccurenceMap[enWord]
      << "\t" << cooccurenceMap[rt->second]
      << "\n";

    BiWord biWord(huWord,enWord);

    biWords.push_back(biWord);

    if (n==itemsRequested)
      break;
  }
  if ( !conflict && (n<itemsRequested) )
  {
    std::cerr << "Not enough elements left for getTopScorers." << std::endl;
    throw "internal error";
  }
}


/*
void eliminateDictionaryItem( 
                    const Word& huWord, const Word& enWord,
                    FrequencyMap& huOccurenceMap, FrequencyMap& enOccurenceMap,
                    CooccurenceMap& cooccurenceMap,
                    SentenceList& huSentenceList, SentenceList& enSentenceList
        )
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    Sentence& huSent = huSentenceList[i];
    Sentence& enSent = enSentenceList[i];

    int huOcc = contains(huWord,huSent);
    int enOcc = contains(enWord,enSent);

    if ( huOcc && enOcc )
    {
      int occ = ( huOcc<enOcc ? huOcc : enOcc );
      CooccurenceMap minusCooccurenceMap;
      addOrSubtractCooccurenceMap( cooccurenceMap, minusCooccurenceMap, true );
    }
  }
}
*/

void logBiWord( const CorpusConstellation& corpusConstellation, const BiWord& biWord )
{
  int huOcc, enOcc, coOcc;
  corpusConstellation.getOccurences( biWord, huOcc, enOcc, coOcc );
  std::cerr << biWord.first << " = " << biWord.second << "\t" << huOcc << "\t" << enOcc << "\t" << coOcc << std::endl;
}

void cooccurenceAnalysis_Old( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          double minScore, int minCoocc )
{
  bool reduceForTesting = false;
  if (reduceForTesting)
  {
    huSentenceList.resize(500);
    huSentenceList.resize(500);
  }

  assert(huSentenceList.size()==enSentenceList.size());

  CorpusConstellation corpusConstellation;

  Ticker ticker;

  corpusConstellation.build(huSentenceList, enSentenceList);

  std::cerr << "CorpusConstellation " << ticker.next() << " ms" << std::endl;

  BiWords automaticLexicon;

  bool consistencyTest = false;
  if (consistencyTest)
  {
    BiWord biWord((Word)"a",(Word)"the");
    logBiWord( corpusConstellation, biWord );
    corpusConstellation.removeDictionaryItem( biWord );
    logBiWord( corpusConstellation, biWord );

    CorpusConstellation corpusConstellationResync;
    corpusConstellationResync.build(corpusConstellation.huSentenceList, corpusConstellation.enSentenceList);
    logBiWord( corpusConstellationResync, biWord );

    return;
  }

  while (true)
  {
    ticker.start();

    // corpusConstellation.filterForPromisingEntries( minScore, minCoocc );
    // std::cerr << "filterForPromisingEntries " << ticker.next() << " ms" << std::endl;

    BiWords biWords;
    corpusConstellation.getTopScorers( biWords,
      10/*itemsRequested*/, minScore, minCoocc );

    std::cerr << "getTopScorers " << ticker.next() << " ms" << std::endl;

    for ( int i=0; i<biWords.size(); ++i )
    {
      automaticLexicon.push_back(biWords[i]);

      corpusConstellation.removeDictionaryItem( biWords[i] );

      const Word& huWord = biWords[i].first;
      const Word& enWord = biWords[i].second;
      // std::cout << huWord << "\t" << enWord << std::endl;
    }
    std::cerr << "removeDictionaryItems " << ticker.next() << " ms" << std::endl;
  }
}

void cooccurenceAnalysis( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          double minScore, int minCoocc )
{
  bool reduceForTesting = false;
  if (reduceForTesting)
  {
    std::cerr << "Sentence number reduced. Strictly for testing!" << std::endl;
    huSentenceList.resize(500);
    enSentenceList.resize(500);
  }

  assert(huSentenceList.size()==enSentenceList.size());
//x   int i;
//x   for ( i=0; i<huSentenceList.size(); ++i )
//x   {
//x     std::cout << huSentenceList[i].words << "\t" << enSentenceList[i].words << std::endl;
//x   }

  Ticker ticker;

  std::cerr << "Removing stopwords...";
  removeStopwords( huSentenceList, enSentenceList );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;
//x   std::cout << "removeStopwords" << std::endl;
//x   for ( i=0; i<huSentenceList.size(); ++i )
//x   {
//x     std::cout << huSentenceList[i].words << "\t" << enSentenceList[i].words << std::endl;
//x   }

  // This should be done after removeStopwords, simply because of bilanguage words like
  // "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
  std::cerr << "Removing identicals... ";
  BiWords idTranslations;
  // removeIdenticals( huSentenceList, enSentenceList, idTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;
  std::cerr << idTranslations.size() << " identical translations found." << std::endl;

  std::cerr << "Removing hapaxes...";
  BiWords hapaxTranslations;
  // removeHapaxes( huSentenceList, enSentenceList, hapaxTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;
//x   std::cout << "removeHapaxes" << std::endl;
//x   for ( i=0; i<huSentenceList.size(); ++i )
//x   {
//x     std::cout << huSentenceList[i].words << "\t" << enSentenceList[i].words << std::endl;
//x   }

  std::cerr << hapaxTranslations.size() << " hapax-based dictionary items found." << std::endl;

  BiWords automaticLexicon;

  int huTermLength = 1;
  int enTermLength = 1;

  while (true)
  {

//x     {
//x     BiWords biWords;
//x     biWords.push_back(BiWord("fehér köpenyes ember","man in white"));
//x     for ( int i=0; i<biWords.size(); ++i )
//x     {
//x       automaticLexicon.push_back(biWords[i]);
//x 
//x       if ((huTermLength==1)&&(enTermLength==1))
//x       {
//x         removeDictionaryItem( huSentenceList, enSentenceList, biWords[i] ); // , huTermLength, enTermLength );
//x       }
//x       else
//x       {
//x         DictionaryItem biTerm;
//x         biTerm.first  = wordToTerm( biWords[i].first );
//x         biTerm.second = wordToTerm( biWords[i].second );
//x         removeComplexDictionaryItem( huSentenceList, enSentenceList, biTerm );
//x       }
//x     }
//x     exit(-1);
//x     }


    CorpusConstellation corpusConstellation;

    ticker.start();
    std::cerr << "Building CorpusConstellation...";
    corpusConstellation.build( huSentenceList, enSentenceList, huTermLength, enTermLength );
    std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

    /*
    {
      typedef std::multimap< double, std::string > ResultMultiMap;
      ResultMultiMap resultMultiMap;

      for ( CooccurenceMap::const_iterator ct=corpusConstellation.cooccurenceMap.begin(); 
            ct!=corpusConstellation.cooccurenceMap.end(); ++ct )
      {
        int coocc = ct->second ;
        int huocc = corpusConstellation.huOccurenceMap[ct->first.first];
        int enocc = corpusConstellation.enOccurenceMap[ct->first.second];

        std::cout << ct->first.first << "\t" << ct->first.second << "\t" << coocc 
          << "\t" << huocc << "\t" << enocc << std::endl;
      }

      std::cerr << "Uristen, nehogy igy hagyd." << std::endl;
      exit(-1);
    }
    */

//x     corpusConstellation.huOccurenceMap.dump(std::cout,100);
//x     std::cout << std::endl;
//x     corpusConstellation.enOccurenceMap.dump(std::cout,100);

    BiWords biWords;

    std::cerr << "Finding TopScorers...";
    corpusConstellation.getTopScorers( biWords,
      -1/*unlimited itemsRequested*/, minScore, minCoocc );

    std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

    for ( int i=0; i<biWords.size(); ++i )
    {
      automaticLexicon.push_back(biWords[i]);

      if ((huTermLength==1)&&(enTermLength==1))
      {
        removeDictionaryItem( huSentenceList, enSentenceList, biWords[i] ); // , huTermLength, enTermLength );
      }
      else
      {
        DictionaryItem biTerm;
        biTerm.first  = wordToTerm( biWords[i].first );
        biTerm.second = wordToTerm( biWords[i].second );
        removeComplexDictionaryItem( huSentenceList, enSentenceList, biTerm );
      }
    }
  }
}


void preprocessAndBuildCooccurrenceData( SentenceList& huSentenceList, SentenceList& enSentenceList,
                               CorpusConstellation& corpusConstellation )
{
  assert(huSentenceList.size()==enSentenceList.size());

  std::cerr << "Removing stopwords...";
  removeStopwords( huSentenceList, enSentenceList );

  // This should be done after removeStopwords, simply because of bilanguage words like
  // "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
  std::cerr << "Removing identicals... ";
  BiWords idTranslations;
  removeIdenticals( huSentenceList, enSentenceList, idTranslations );
  std::cerr << idTranslations.size() << " identical translations found." << std::endl;

  std::cerr << "Removing hapaxes...";
  BiWords hapaxTranslations;
  removeHapaxes( huSentenceList, enSentenceList, hapaxTranslations );
  std::cerr << hapaxTranslations.size() << " hapax-based dictionary items found." << std::endl;

  BiWords automaticLexicon;

  int huTermLength = 1;
  int enTermLength = 1;

  std::cerr << "Building CorpusConstellation...";
  corpusConstellation.build( huSentenceList, enSentenceList, huTermLength, enTermLength );
  std::cerr << " Done." << std::endl;
}

// Adds to the dictionary it recieves as input.
void autoDictionaryForRealign( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          DictionaryItems& dictionary,
                          double minScore, int minCoocc )
{
  CorpusConstellation corpusConstellation;

  preprocessAndBuildCooccurrenceData( huSentenceList, enSentenceList, corpusConstellation );

  for ( CooccurenceMap::const_iterator ct=corpusConstellation.cooccurenceMap.begin(); ct!=corpusConstellation.cooccurenceMap.end(); ++ct )
  {
    int coocc = ct->second ;
    int huocc = corpusConstellation.huOccurenceMap[ct->first.first];
    int enocc = corpusConstellation.enOccurenceMap[ct->first.second];

    double score = scoreEntry( huocc, enocc, coocc );

    if ( (score>=minScore) && (coocc>=minCoocc) )
    {
      const BiWord& biWord = ct->first;

      // WARNING WARNING WARNING TODO: For historical reasons,
      // in the dictionaryItem, first means the English phrase.
      // Everywhere else in the code, first means the Hungarian phrase.

      DictionaryItem dictionaryItem;
      dictionaryItem.first .push_back(biWord.second); // !!!
      dictionaryItem.second.push_back(biWord.first);  // !!!

      // std::cout << biWord.first << "\t" << biWord.second << std::endl;

      dictionary.push_back(dictionaryItem);
    }
  }
}

// Removes dictionary items for which it doesn't find cooccurrences in the bicorpus.
// Typically, bicorpus is built from a primary alignment.
void filterDictionaryForRealign( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          DictionaryItems& dictionary )
{
  CorpusConstellation corpusConstellation;

  preprocessAndBuildCooccurrenceData( huSentenceList, enSentenceList, corpusConstellation );

  DictionaryItems newDictionary;

  for ( DictionaryItems::const_iterator dt=dictionary.begin(); dt!=dictionary.end(); ++dt )
  {
    bool good = false ;

    const DictionaryItem& item = *dt;

    // WARNING WARNING WARNING TODO: For historical reasons,
    // in the dictionaryItem, first means the English phrase.
    // Everywhere else in the code, first means the Hungarian phrase.
    const Phrase& hu = item.second ;
    const Phrase& en = item.first ;

    if ( (hu.size()!=1) || (en.size()!=1) )
    {
      // We throw away every not-one-to-one dictionary item.
      good = false;
    }
    else
    {
      if ( corpusConstellation.cooccurenceMap[ std::make_pair(hu.front(),en.front()) ] > 0 )
      {
        good = true;
      }
    }

    if (good)
    {
      newDictionary.push_back(item);
    }
  }

  std::swap( dictionary, newDictionary );
}

//x typedef Cooccurrence Postoccurrence;
//x 
//x 
//x class HalfConstellation
//x {
//x public:
//x   void addSentence( const Sentence& sentence );
//x   void build( const SentenceList& sentenceList );
//x 
//x   void getTopScorers( BiWords& biWords,
//x                       int itemsRequested, double minScore, int minCoocc );
//x 
//x   void getOccurences( const BiWord& biWord, int& huOcc, int& enOcc, int& coOcc ) const;
//x 
//x public:
//x   SentenceList sentenceList;
//x   FrequencyMap occurenceMap;
//x 
//x   Postoccurrence postoccurrenceMap;
//x };
//x 
//x void HalfConstellation::addSentence( const Sentence& sentence )
//x {
//x   sentenceList.push_back(sentence);
//x 
//x   const WordList& words = sentence.words;
//x 
//x   occurenceMap.build(words);
//x 
//x   int j,k;
//x   for ( j=0; j<huWords.size(); ++j )
//x   {
//x     for ( k=0; k<enWords.size(); ++k )
//x     {
//x       ++ cooccurenceMap[ std::make_pair(huWords[j],enWords[k]) ];
//x     }
//x   }
//x }
//x 
//x void HalfConstellation::build( const SentenceList& sentenceList )
//x {
//x   for ( int i=0; i<huSentenceList.size(); ++i )
//x   {
//x     addSentence( sentenceList[i] );
//x   }
//x }

void combinatoricaXmlHeader( std::ostream& os )
{
  os << "<?xml version='1.0'?>\n";
  os << "<!DOCTYPE Expression SYSTEM 'http://www.wolfram.com/XML/notebookml1.dtd'>\n";
  os << "<Expression xmlns:mathematica='http://www.wolfram.com/XML/'\n";
  os << "    xmlns='http://www.wolfram.com/XML/'>\n";
  os << " <Function>\n";
  os << "  <Symbol>Graph</Symbol>\n";
  os << "  <Function>\n";
  os << "   <Symbol>List</Symbol>\n";
}

void combinatoricaXmlFooter( std::ostream& os )
{
  os << "  </Function>\n";

  os << "  <Function>\n";
  os << "   <Symbol>Rule</Symbol>\n";
  os << "   <Symbol>EdgeDirection</Symbol>\n";
  os << "   <Symbol>True</Symbol>\n";
  os << "  </Function>\n";

  os << " </Function>\n";
  os << "</Expression>" << std::endl;
}

void putEdge( int a, int b, double capacity, std::ostream& os )
{
  // !!!
  if (capacity<2)
  {
    return;
  }

  const char sFs[] = "<Function>";
  const char sFe[] = "</Function>";

  const char sSs[] = "<Symbol>";
  const char sSe[] = "</Symbol>";

  const char sEndl[] = "\n";

  // TEMP TODO
  // os << a << "\t" << b << "\t" << capacity << std::endl;

  os << "   " << sFs << sEndl;

  os << "    "   << sSs << "List" << sSe << sEndl;

  os << "    "   << sFs << sEndl;
  os << "     "  << sSs << "List" << sSe << sEndl;
  os << "     "  << "<Number>" << a << "</Number>" << sEndl;
  os << "     "  << "<Number>" << b << "</Number>" << sEndl;
  os << "    "   << sFe << sEndl;

  os << "    "   << sFs << sEndl;
  os << "     "  << sSs << "Rule" << sSe << sEndl;
  os << "     "  << sSs << "EdgeWeight" << sSe << sEndl;
  os << "     "  << "<Number>" << capacity << "</Number>" << sEndl;
  os << "    "   << sFe << sEndl;

  os << "   " << sFe << sEndl;
}

void putDummyNode( std::ostream& os )
{

  os <<
"   <Function>\n\
    <Symbol>List</Symbol>\n\
    <Function>\n\
     <Symbol>List</Symbol>\n";
  
  os << "     <Number>" << 0.01*(rand()%100) << "</Number>\n";
  os << "     <Number>" << 0.01*(rand()%100) << "</Number>\n";

  os <<
"    </Function>\n\
   </Function>\n";

}

void flowBuilderXml( const SentenceList& huSentenceListC, const SentenceList& enSentenceListC,
                  std::ostream& flowStream )
{
  // Small test:
  bool smallTest = false;
  if (smallTest)
  {
    combinatoricaXmlHeader(flowStream);

    putEdge( 1, 2, 5, flowStream );
    putEdge( 1, 3, 6, flowStream );
    putEdge( 2, 4, 5, flowStream );
    putEdge( 2, 5, 6, flowStream );
    putEdge( 3, 4, 5, flowStream );
    putEdge( 3, 5, 6, flowStream );
    putEdge( 4, 6, 5, flowStream );
    putEdge( 5, 6, 6, flowStream );

    flowStream << "  </Function>\n";
    flowStream << "  <Function>\n";
    flowStream << "   <Symbol>List</Symbol>\n";

    for ( int i=0; i<6; ++i )
      putDummyNode( flowStream );

    combinatoricaXmlFooter(flowStream);
    return;
  }

  SentenceList huSentenceList(huSentenceListC);
  SentenceList enSentenceList(enSentenceListC);

  bool reduceForTesting = true;
  if (reduceForTesting)
  {
    huSentenceList.resize(100);
    enSentenceList.resize(100);
  }

  assert(huSentenceList.size()==enSentenceList.size());

  Ticker ticker;

  std::cerr << "Removing stopwords...";
  removeStopwords( huSentenceList, enSentenceList );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  // This should be done after removeStopwords, simply because of bilanguage words like
  // "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
  std::cerr << "Removing identicals... ";
  BiWords idTranslations;
  // removeIdenticals( huSentenceList, enSentenceList, idTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;
  std::cerr << idTranslations.size() << " identical translations found." << std::endl;

  std::cerr << "Removing hapaxes...";
  BiWords hapaxTranslations;
  removeHapaxes( huSentenceList, enSentenceList, hapaxTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  std::cerr << hapaxTranslations.size() << " hapax-based dictionary items found." << std::endl;

  CorpusConstellation corpusConstellation;

  ticker.start();
  std::cerr << "Building CorpusConstellation...";
  corpusConstellation.build( huSentenceList, enSentenceList );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  combinatoricaXmlHeader(flowStream);

  int id = 1;
  const int sourceNode = id;
  ++id;
  const int targetNode = id;
  ++id;

  FrequencyMap::const_iterator it;
  std::map<Word,int> wordsToIds;
  {
    const FrequencyMap& freqMap = corpusConstellation.huOccurenceMap;
    for ( it=freqMap.begin(); it!=freqMap.end(); ++it, ++id )
    {
      wordsToIds[it->first] = id;
      putEdge( sourceNode, id, it->second, flowStream );
    }
  }
  {
    const FrequencyMap& freqMap = corpusConstellation.enOccurenceMap;
    for ( it=freqMap.begin(); it!=freqMap.end(); ++it, ++id )
    {
      wordsToIds[it->first] = id;
      putEdge( id, targetNode, it->second, flowStream );
    }
  }
  
  for ( CooccurenceMap::const_iterator ct=corpusConstellation.cooccurenceMap.begin(); ct!=corpusConstellation.cooccurenceMap.end(); ++ct )
  {
    int coocc = ct->second ;
    int huId = wordsToIds[ct->first.first];
    int enId = wordsToIds[ct->first.second];

    putEdge( huId, enId, coocc, flowStream );
  }

  flowStream << "  </Function>\n";
  flowStream << "  <Function>\n";
  flowStream << "   <Symbol>List</Symbol>\n";

  for ( int i=1; i<id; ++i )
  {
    putDummyNode( flowStream );
  }

  combinatoricaXmlFooter(flowStream);
}

void flowBuilder( const SentenceList& huSentenceListC, const SentenceList& enSentenceListC,
                  NetworkWithFlow& nw, std::map<int,Word>& idsToWords )
{
  SentenceList huSentenceList(huSentenceListC);
  SentenceList enSentenceList(enSentenceListC);

  bool reduceForTesting = true;
  if (reduceForTesting)
  {
    huSentenceList.resize(300);
    enSentenceList.resize(300);
  }

  assert(huSentenceList.size()==enSentenceList.size());

  Ticker ticker;

  std::cerr << "Removing stopwords...";
  removeStopwords( huSentenceList, enSentenceList );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  // This should be done after removeStopwords, simply because of bilanguage words like
  // "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
  std::cerr << "Removing identicals... ";
  BiWords idTranslations;
  removeIdenticals( huSentenceList, enSentenceList, idTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;
  std::cerr << idTranslations.size() << " identical translations found." << std::endl;

  std::cerr << "Removing hapaxes...";
  BiWords hapaxTranslations;
  removeHapaxes( huSentenceList, enSentenceList, hapaxTranslations );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  std::cerr << hapaxTranslations.size() << " hapax-based dictionary items found." << std::endl;

  CorpusConstellation corpusConstellation;

  ticker.start();
  std::cerr << "Building CorpusConstellation...";
  corpusConstellation.build( huSentenceList, enSentenceList );
  std::cerr << " Done in " << ticker.next() << " ms." << std::endl;

  ticker.start();
  std::cerr << "Building Network...\n";
  
  int id = 1;
  const int sourceNode = id;
  ++id;
  const int targetNode = id;
  ++id;

  std::map<Word,int> huWordsToIds, enWordsToIds;
  FrequencyMap::const_iterator it;
  {
    double totalCapacity(0);
    const FrequencyMap& freqMap = corpusConstellation.huOccurenceMap;
    for ( it=freqMap.begin(); it!=freqMap.end(); ++it, ++id )
    {
      huWordsToIds[it->first] = id;
      idsToWords[id] = it->first;
      std::cout << id << "\t" << it->first << "\t" << it->second << std::endl;
      nw.addEdge( sourceNode, id, 1/*it->second*/ );
      totalCapacity += it->second;
    }
    std::cout << "Hungarian total possible capacity: " << totalCapacity << std::endl;
  }
  {
    double totalCapacity(0);
    const FrequencyMap& freqMap = corpusConstellation.enOccurenceMap;
    for ( it=freqMap.begin(); it!=freqMap.end(); ++it, ++id )
    {
      enWordsToIds[it->first] = id;
      idsToWords[id] = it->first;
      std::cout << id << "\t" << it->first << "\t" << it->second << std::endl;
      nw.addEdge( id, targetNode, 1/*it->second*/ );
      totalCapacity += it->second;
    }
    std::cout << "English total possible capacity: " << totalCapacity << std::endl;
  }

  for ( CooccurenceMap::const_iterator ct=corpusConstellation.cooccurenceMap.begin(); ct!=corpusConstellation.cooccurenceMap.end(); ++ct )
  {
    int coocc = ct->second ;
    int huId = huWordsToIds[ct->first.first];
    int enId = enWordsToIds[ct->first.second];

    double augmentedCoocc = (double)coocc 
      / corpusConstellation.huOccurenceMap[ct->first.first]
      / corpusConstellation.enOccurenceMap[ct->first.second] ;

    nw.addEdge( huId, enId, augmentedCoocc );
  }
  std::cerr << "Done building network in " << ticker.next() << " ms." << std::endl;
}

void lexiconByEdmondsKarp( const SentenceList& huSentenceListC, const SentenceList& enSentenceListC )
{
  NetworkWithFlow nw;
  std::map<int,Word> idsToWords;
  flowBuilder( huSentenceListC, enSentenceListC, nw, idsToWords );
  nw.edmondsKarp(1,2);

  const NetworkWithFlow::Valuation& flow = nw.getFlow();
  const NetworkWithFlow::Valuation& capacity = nw.getCapacity();

  for ( NetworkWithFlow::Valuation::const_iterator it=capacity.begin(); it!=capacity.end(); ++it )
  {
    assert(flow.find(it->first)!=flow.end());

    int hu = it->first.first;
    int en = it->first.second;

    double flowValue = flow.find(it->first)->second;

    if (flowValue == 0)
      continue;

    std::cout << idsToWords[hu] << "\t" << idsToWords[en]
      << "\t" << it->second;

    std::cout << "\t" << flowValue ;
    

    if ((hu!=1)&&(en!=2))
    {
      double huPotentialFlowValue
        = capacity.find(NetworkWithFlow::Edge( 1, hu ))->second ;

      std::cout << "\t" << 100*flowValue/huPotentialFlowValue ;
    }

    std::cout << std::endl;
  }
}

// Latvanyosan figyelmen kivul hagyja a tobbszavas kifejezeseket.
// Nagyon lassu n^2*k algoritmus.
void bisentenceToBipartite( const WordList& huSentence, const WordList& enSentence,
                            const DumbMultiDictionary& huSourceDict,
                            DiGraph& graph )
{
  graph.clear();

  const int bigEnglishConst = 1000000;

  for ( int huPos=0; huPos<huSentence.size(); ++huPos )
  {
    const Word& huWord = huSentence[huPos];

    std::pair<DumbMultiDictionary::const_iterator, DumbMultiDictionary::const_iterator> range =
      huSourceDict.equal_range(huWord);

    // Balrol jobbra haladva az elso match-elhetot match-eli az angol mondatbol.
    for ( int enPos=0; enPos<enSentence.size(); ++enPos )
    {
      bool foundItsPair=false;

      // A kiveteles eset: identikus szavakat leforditunk.
      // A kivetel aloli kivetel: a-t a-ra nem forditjuk.
      if (huSentence[huPos]==enSentence[enPos])
      {
        if (huSentence[huPos]!="a")
        {
          foundItsPair = true;
        }
      }
      else
      {
        for ( DumbMultiDictionary::const_iterator it=range.first; it!=range.second; ++it )
        {
          const Phrase& enPhrase = it->second;
          if (enPhrase.size()!=1)
            continue;
          const Word& enWord = enPhrase[0];

          if (enWord==enSentence[enPos])
          {
            foundItsPair = true;
            break;
          }
        }
      }
      if (foundItsPair)
      {
        graph.addEdge(huPos, bigEnglishConst+enPos);
        std::cout << huSentence[huPos] << " - " << enSentence[enPos] << std::endl;
      }
    }
  }
}

void filterBisentenceByLexicon( WordList& huSentence, WordList& enSentence,
                                const DumbMultiDictionary& huSourceDict )
{
  for ( int huPos=0; huPos<huSentence.size(); ++huPos )
  {
    const Word& huWord = huSentence[huPos];

    std::pair<DumbMultiDictionary::const_iterator, DumbMultiDictionary::const_iterator> range =
      huSourceDict.equal_range(huWord);

    bool foundItsPair=false;

    // Balrol jobbra haladva az elso match-elhetot match-eli az angol mondatbol.
    for ( int enPos=0; enPos<enSentence.size(); ++enPos )
    {
      // A kiveteles eset: identikus szavakat leforditunk.
      // A kivetel aloli kivetel: a-t a-ra nem forditjuk.

      if (huSentence[huPos]==enSentence[enPos])
      {
        if (huSentence[huPos]!="a")
        {
          foundItsPair = true;
        }
      }
      else
      {
        for ( DumbMultiDictionary::const_iterator it=range.first; it!=range.second; ++it )
        {
          const Phrase& enPhrase = it->second;
          if (enPhrase.size()!=1)
            continue;
          const Word& enWord = enPhrase[0];

          if (enWord==enSentence[enPos])
          {
            foundItsPair = true;
            break;
          }
        }
      }

      if (foundItsPair)
      {
        huSentence.erase(huSentence.begin()+huPos);
        enSentence.erase(enSentence.begin()+enPos);
        --huPos;
        break;
      }
    }
  }
}

// Ez ideiglenes kiserlet-kod, ezert nem zavar, hogy teljesen morbid es ertelmetlen
// modon nem a cooccurrenceTool-agrol van hivva, hanem az alignerTool agrol.
void logLexiconCoverageOfBicorpus
       ( SentenceList& huSentenceList, SentenceList& enSentenceList,
       const DictionaryItems& dictionaryItems )
{
  DumbMultiDictionary huSourceDict;
  buildDumbMultiDictionary( dictionaryItems, huSourceDict, false/*reverse*/ );
//  DumbMultiDictionary enSourceDict;
//  buildDumbMultiDictionary( dictionaryItems, enSourceDict, true /*reverse*/ );

  for ( int huPos=830; huPos<846; ++huPos ) // Az 1984 egy szakasza: Ohu.1.6.7.1 - Ohu.1.6.12.1
  {
    int enPos = huPos-4; // Empirikus igazsag. :)

    WordList& huSentence = huSentenceList[huPos].words;
    WordList& enSentence = enSentenceList[enPos].words;

    DiGraph graph;

    std::cout << "--------------" << std::endl;
    std::cout << huSentence << " - " << enSentence << std::endl;

    bisentenceToBipartite( huSentence, enSentence, huSourceDict, graph );
    filterBisentenceByLexicon( huSentence, enSentence, huSourceDict );

    std::cout << huSentence << " - " << enSentence << std::endl;
  }
}

void filterBicorpusByLexicon
       ( SentenceList& huSentenceList, SentenceList& enSentenceList,
       const DictionaryItems& dictionaryItems )
{
  assert(huSentenceList.size()==enSentenceList.size());

  DumbMultiDictionary huSourceDict;
  buildDumbMultiDictionary( dictionaryItems, huSourceDict, false/*reverse*/ );

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    filterBisentenceByLexicon( huSentenceList[i].words, enSentenceList[i].words,
                               huSourceDict );
  }
}

} // namespace Hunglish
