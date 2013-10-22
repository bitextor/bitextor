/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_COOCCURRENCE_H
#define __HUNGLISH_ALIGNMENT_COOCCURRENCE_H

#include "words.h"

namespace Hunglish
{

void cooccurenceAnalysis( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          double minScore, int minCoocc );

void flowBuilderXml( const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                  std::ostream& flowStream );

void lexiconByEdmondsKarp( const SentenceList& huSentenceListC, const SentenceList& enSentenceListC );


typedef std::pair<Word,Word> BiWord;
typedef std::vector<BiWord> BiWords;

// This should be done after removeStopwords, simply because of bilanguage words like
// "a","is","be". We absolutely don't care about rare bilanguage words like "petty".
void removeIdenticals( SentenceList& huSentenceList, SentenceList& enSentenceList,
                       BiWords& idTranslations );

void removeHapaxes( SentenceList& huSentenceList, SentenceList& enSentenceList,
                    BiWords& hapaxTranslations );

class DictionaryItems;

void filterBicorpusByLexicon
       ( SentenceList& huSentenceList, SentenceList& enSentenceList,
       const DictionaryItems& dictionaryItems );

// Adds plausible items to the dictionary it recieves as input.
void autoDictionaryForRealign( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          DictionaryItems& dictionary,
                          double minScore, int minCoocc );

// Removes dictionary items for which it doesn't find cooccurrences in the bicorpus.
// Typically, bicorpus is built from a primary alignment.
void filterDictionaryForRealign( SentenceList& huSentenceList, SentenceList& enSentenceList,
                          DictionaryItems& dictionary );

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_COOCCURRENCE_H
