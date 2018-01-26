/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_DICTIONARY_H
#define __HUNGLISH_ALIGNMENT_DICTIONARY_H

#include "words.h"

#include <string>
#include <vector>
#include <map>
#include <iosfwd>


namespace Hunglish
{

typedef std::pair<Phrase,Phrase> DictionaryItem;

class DictionaryItems : public std::vector<DictionaryItem>
{
public:
  void read( std::istream& is );
};

class HalfDictionary : public std::vector<WordList>
{
public:
  void read( std::istream& is );
};


// After reading, this dictionary cannot be altered.
// Also, this is a strictly one-directional dictionary.
// If the other direction is needed, reverse( const Dictionary& dic ) another dictionary.
class Dictionary
{
public:
  void read( const char* dictionaryFile );
  void reverse( const Dictionary& dic );
  void build( const DictionaryItems& dictionaryItems );

  bool lookupWord( const Word& word, DictionaryItems& results ) const;
  bool lookupWordSet( const WordList& words, DictionaryItems& results ) const;

private:
  void buildWordLookupTable();

private:
  DictionaryItems dictionaryItems;

  typedef std::map<Word,int> wordLookupTable;
};

class FrequencyMap : public std::map<Word,int>
{
public:
  void add( const Word& word );
  void remove( const Word& word );
  void build( const WordList& wordList );
  void remove( const WordList& wordList );
  void build( const SentenceList& sentenceList ); // Just for convenience.
  int  total() const;
  void dump( std::ostream& os, int itemNum ) const;
  void lowPassFilter( WordList& allowedWords, double ratio ) const;
  void highPassFilter( WordList& allowedWords, double ratio ) const;

private:
  typedef std::multimap<int,Word> ReFrequencyMap;
  void reverseMap( ReFrequencyMap& reFrequencyMap ) const;
};


void filterSentences( SentenceList& sentenceList, const WordList& words );

void removeHungarianStopwords( SentenceList& huSentenceList );
void removeEnglishStopwords  ( SentenceList& enSentenceList );
void removeStopwords  ( SentenceList& huSentenceList, SentenceList& enSentenceList );


typedef std::pair<Word,Word> WordPair;

class TransLex
{
public:

  typedef std::multimap<Word,Word> WordMultimap;
  typedef WordMultimap::const_iterator WordMultimapIt;
  typedef std::pair<WordMultimapIt,WordMultimapIt> DictInterval;

  void add( const Word& huWord, const Word& enWord );
  void build( const DictionaryItems& dictionaryItems );

  DictInterval lookupLeftWord ( const Word& huWord ) const;
  DictInterval lookupRightWord( const Word& enWord ) const;
  bool isPresent( const Word& huWord, const Word& enWord ) const;

private:
  WordMultimap forward;
  WordMultimap backward;
};

class IBMModelOne
{
public:
  double lookup( const Word& hu, const Word& en ) const;

  double distance( const Phrase& hu, const Phrase& en ) const;

  void build( const SentenceList& huSentenceList, const SentenceList& enSentenceList );

  void reestimate( const SentenceList& huSentenceList, const SentenceList& enSentenceList );

public:
  typedef std::pair<Word,Word> WordPair;
  typedef std::map<WordPair,double> TransProbs;

  TransProbs transProbs;
};

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_DICTIONARY_H
