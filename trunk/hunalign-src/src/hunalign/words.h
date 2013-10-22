/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_WORDS_H
#define __HUNGLISH_ALIGNMENT_WORDS_H

#include <string>
#include <vector>
#include <iosfwd>

namespace Hunglish
{

typedef std::string String;

typedef String Word;

typedef std::vector<Word> WordList;

typedef WordList Phrase;

typedef std::vector<Phrase> Book;

struct Sentence
{
  WordList words;
  String   sentence;
  String   id;
};

// Implemented in dictionary.cpp
class SentenceList : public std::vector<Sentence>
{
public:
  void read ( std::istream& is );
  void readNoIds( std::istream& is );
  void write( std::ostream& os ) const;
  void writeNoIds( std::ostream& os ) const;
};

// Implemented in dictionary.cpp
void readBicorpus( std::istream& is, SentenceList& huSentenceList, SentenceList& enSentenceList);
void writeBicorpus( std::ostream& os, const SentenceList& huSentenceList, const SentenceList& enSentenceList);

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_WORDS_H
