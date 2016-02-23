/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_WORDALIGNMENT_H
#define __HUNGLISH_ALIGNMENT_WORDALIGNMENT_H

#include "words.h"
#include "dictionary.h"

#include <set>
#include <vector>


namespace Hunglish
{

const int NullWord = -3 ;

typedef int WordIndex;

typedef std::pair<WordIndex,WordIndex> WordRelation;

typedef std::vector<WordRelation> WordRelations;

typedef std::set<WordIndex> WordSet;

// Describes the word-to-word structure of a bisentence. Many-to-one and one-to-NIL relationships are allowed.
// Many-to-many is currently supported but not encouraged: disjoint complete bigraphs are allowed.
// The bisentence itself is not stored. It is referred into by integer word indices.
// Word-to-NIL relations must be made expicit. An initial empty WordAlignment means no knowledge, not knowledge of NIL.
// TODO confidence values may be incorporated later.
// TODO maybe even more importantly, flags to denote kinds of relations:
//   - suitable as dictionary item
//   - the result of ellipsis, not suitable as dictionary item
//   - ?
//
class WordAlignment
{
public:
  const WordRelations& getWordRelations() const ;
  void addWordRelation( const WordRelation& wordRelation ) ;

  // Under the current, unindexed implementation this is an O(n) operation.
  // leftSide refers to the argument being on the leftside, not the result! Major f.ck up possibility!
  WordSet relation( WordIndex wordIndex, bool leftSide ) const ;

  // Under the current, unindexed implementation this is implemented by two *Friends operations, so it is very very slow.
  // leftSide refers to the argument being on the leftside, not the result! Major f.ck up possibility!
  WordSet group   ( WordIndex wordIndex, bool leftSide ) const ;

  // Inconsistency can be caused by the following:
  // - word connected to NIL and other.
  // - two words connected twice.
  // - graph is not disjoint union of stars.( Or complete bigraphs, if many-to-many is supported.)
  bool isConsistent() const;

  // Reorders the data lexicographically, without changing its semantics in any way.
  void resort();

  void clear();

private:
  WordSet rightFriends( WordIndex wordIndex ) const;
  WordSet leftFriends ( WordIndex wordIndex ) const;

private:
  WordRelations wordRelations;
};

// BiSentence::first is the source (Hungarian) sentence.
typedef std::pair<Phrase,Phrase> BiSentence;

class WordAlignedBisentence : public BiSentence // Inheritance from nonvirtual. It sounds so strange but it feels so good.
{
public:
  void markDictionaryItem( const DictionaryItem& dictionaryItem );

  void findDictionaryItemsByGaps( DictionaryItems& dictionaryItems ); // Not const because it resorts.

  // Removes all words that the align can account for.
  void elimination();

public:
  WordAlignment wordAlignment;
};

class WordAlignedBisentences : public std::vector<WordAlignedBisentence> // Inheritance from nonvirtual. It sounds so strange but it feels so good.
{
public:

  void markDictionaryItem( const DictionaryItem& dictionaryItem );

  void importBicorpus( SentenceList& huSentenceList, SentenceList& enSentenceList );

  void findDictionaryItemsByGaps( DictionaryItems& dictionaryItems ); // Not const because it resorts.

  // Removes all words that the align can account for.
  void elimination();
};

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_WORDALIGNMENT_H
