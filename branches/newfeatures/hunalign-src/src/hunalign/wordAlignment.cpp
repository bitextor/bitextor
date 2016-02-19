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

#include "wordAlignment.h"
#include <serializeImpl.h>

#include <iostream>
#include <algorithm>

// Copypaste-elve. TODO Elhelyezni.
#define massert(e) if (!(e)) { std::cerr << #e << " failed" << std::endl; throw "assert"; }

namespace Hunglish
{

const WordRelations& WordAlignment::getWordRelations() const
{
  return wordRelations;
}

void WordAlignment::addWordRelation( const WordRelation& wordRelation )
{
  wordRelations.push_back(wordRelation);
}

WordSet WordAlignment::rightFriends( WordIndex wordIndex ) const
{
  WordSet res;

  for ( int i=0; i<wordRelations.size(); ++i )
  {
    if ( wordRelations[i].first == wordIndex )
      res.insert(wordRelations[i].second);
  }

  return res;
}

WordSet WordAlignment::leftFriends ( WordIndex wordIndex ) const
{
  WordSet res;

  for ( int i=0; i<wordRelations.size(); ++i )
  {
    if ( wordRelations[i].second == wordIndex )
      res.insert(wordRelations[i].first);
  }

  return res;
}

WordSet WordAlignment::relation( WordIndex wordIndex, bool leftSide ) const
{
  if (leftSide)
    return rightFriends( wordIndex );
  else
    return leftFriends ( wordIndex );
}

WordSet WordAlignment::group( WordIndex wordIndex, bool leftSide ) const
{
  WordSet rel = relation( wordIndex, leftSide );

  WordSet emptyGroup;

  if (rel.empty())
    return emptyGroup;

  const WordIndex& other = *(rel.begin());

  if (other==NullWord)
    return emptyGroup;

  return relation( other, !leftSide );
}


// Inconsistency can be caused by the following:
// - word connected to NIL and other.
// - two words connected twice.
  // - graph is not disjoint union of stars.( Or complete bigraphs, if many-to-many is supported.)
bool WordAlignment::isConsistent() const
{
  return false;
}

void WordAlignment::resort()
{
  std::sort( wordRelations.begin(), wordRelations.end() );
}

void WordAlignment::clear()
{
  wordRelations.clear();
}

void WordAlignedBisentence::markDictionaryItem( const DictionaryItem& dictionaryItem )
{
  if ( (dictionaryItem.first.size()!=1) || (dictionaryItem.second.size()!=1) )
  {
    std::cerr << "markDictionaryItem for many-to-any dictionaryItems is currently unimplemented." << std::endl;
    throw "unimplemented";
  }

  const Word& huWord = dictionaryItem.first .front();
  const Word& enWord = dictionaryItem.second.front();

  WordSet huSet,enSet;

  int i;
  for ( i=0; i<first.size(); ++i )
  {
    if (first[i]==huWord)
      huSet.insert(i);
  }
  for ( i=0; i<second.size(); ++i )
  {
    if (second[i]==enWord)
      enSet.insert(i);
  }

  if (huSet.size()!=enSet.size())
  {
    // std::cerr << "warning: differing instance numbers" << std::endl;
  }

  // Hack, but we cannot really do better at this point.
  int min = ( huSet.size()<enSet.size() ? huSet.size() : enSet.size() );

  WordSet::const_iterator hut,ent;
  int j;
  for ( j=0, hut=huSet.begin(), ent=enSet.begin(); j<min; ++j, ++hut, ent )
  {
    wordAlignment.addWordRelation( WordRelation( *hut, *ent ) );

    std::cerr << first[*hut] << " - " << second[*ent] << std::endl;
  }
}

DictionaryItem oneWordsDictionaryItem( const Word& hu, const Word& en )
{
  DictionaryItem dictionaryItem;
  dictionaryItem.first .push_back(hu);
  dictionaryItem.second.push_back(en);
  return dictionaryItem;
}

void WordAlignedBisentence::findDictionaryItemsByGaps( DictionaryItems& dictionaryItems ) // Not const because it resorts.
{
  // dictionaryItems.clear(); is deliberately avoided here!

  WordRelations newWordRelations;

  wordAlignment.resort();
  const WordRelations& wordRelations = wordAlignment.getWordRelations();

  int i;
  // Hack
  for ( i=0; i+1<wordRelations.size(); ++i ) // Fucking unsigned size_t.
  {
    const WordRelation& current = wordRelations[i];
    const WordRelation& next    = wordRelations[i+1];

    if (
      (current.first +2 == next.first +2)
         ||
      (current.second+2 == next.second+2)
       )
    {
      dictionaryItems.push_back( oneWordsDictionaryItem( first[current.first+1], second[current.second+1] ) );
      newWordRelations.push_back( WordRelation(current.first+1,current.second+1) );
    }
  }

  for ( i=0; i<newWordRelations.size(); ++i )
  {
    wordAlignment.addWordRelation(newWordRelations[i]);
  }
}

void WordAlignedBisentence::elimination()
{
  const WordRelations& wordRelations = wordAlignment.getWordRelations();

  WordSet huSet, enSet;
  for ( int i=0; i<wordRelations.size(); ++i )
  {
    huSet.insert(wordRelations[i].first );
    enSet.insert(wordRelations[i].second);
  }

  WordSet::reverse_iterator rit;
  for ( rit=huSet.rbegin(); rit!=huSet.rend(); ++rit )
  {
    first.erase( first.begin() + *rit );
  }
  for ( rit=enSet.rbegin(); rit!=enSet.rend(); ++rit )
  {
    second.erase( second.begin() + *rit );
  }

  wordAlignment.clear();
}

void WordAlignedBisentences::importBicorpus( SentenceList& huSentenceList, SentenceList& enSentenceList )
{
  massert(huSentenceList.size()==enSentenceList.size());

  for ( int i=0; i<huSentenceList.size(); ++i )
  {
    push_back(WordAlignedBisentence());
    back().first  = huSentenceList[i].words;
    back().second = enSentenceList[i].words;
  }
}

void WordAlignedBisentences::markDictionaryItem( const DictionaryItem& dictionaryItem )
{
  for ( int i=0; i<size(); ++i )
  {
    operator[](i).markDictionaryItem(dictionaryItem);
  }
}

void WordAlignedBisentences::findDictionaryItemsByGaps( DictionaryItems& dictionaryItems ) // Not const because it resorts.
{
  dictionaryItems.clear();

  for ( int i=0; i<size(); ++i )
  {
    operator[](i).findDictionaryItemsByGaps( dictionaryItems );
  }
}

void WordAlignedBisentences::elimination()
{
  for ( int i=0; i<size(); ++i )
  {
    operator[](i).elimination();
  }
}

} // namespace Hunglish


#include <fstream>
#include "dictionary.h"

namespace Hunglish
{

void main_wordAlignmentTest()
{
  DictionaryItems dictionary;

  /*
  std::ifstream dics("/hunglish/data/szotar/vonyo7.nojoker.txt");
  dictionary.read(dics);
  */

  DictionaryItem dictionaryItem = oneWordsDictionaryItem("te","you");
  dictionary.push_back(dictionaryItem);

  SentenceList huSentenceList;
  SentenceList enSentenceList;

  std::ifstream hus("/hunglish/data/experiments/cooccurrence/1984.huhalf");
  std::ifstream ens("/hunglish/data/experiments/cooccurrence/1984.enhalf");

  huSentenceList.readNoIds(hus);
  enSentenceList.readNoIds(ens);


  WordAlignedBisentences wordAlignedBisentences;
  wordAlignedBisentences.importBicorpus( huSentenceList, enSentenceList );

  wordAlignedBisentences.markDictionaryItem(dictionaryItem);

  // big brother be watch you the caption beneath it run
  // Nagy Testver szem tart hirdet az arc alatt a feliras

  wordAlignedBisentences.markDictionaryItem(oneWordsDictionaryItem("Nagy","big"));
  wordAlignedBisentences.markDictionaryItem(oneWordsDictionaryItem("szem","be"));

  DictionaryItems newDictionary;
  wordAlignedBisentences.findDictionaryItemsByGaps(newDictionary);

  for ( int i=0; i<newDictionary.size(); ++i )
  {
    std::cerr << newDictionary[i].first << " - " << newDictionary[i].second << std::endl;
  }
}

} // namespace Hunglish
