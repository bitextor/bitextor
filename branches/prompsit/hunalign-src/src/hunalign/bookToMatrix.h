/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_BOOKTOMATRIX_H
#define __HUNGLISH_ALIGNMENT_BOOKTOMATRIX_H

#include "words.h"
#include "alignment.h"

namespace Hunglish
{

const double scoreOfParagraphMatch = 0.31;

const double scoreOfParagraphMisMatch = -1.0;

bool isParagraph( const Phrase& phrase );

// (!!!) We assert that sx and sy are ordered sets of Word-s!
int intersectionSize( const WordList& sx, const WordList& sy );

void sentenceListsToAlignMatrixIdentity( const SentenceList& huSentenceList, const SentenceList& enSentenceList, AlignMatrix& alignMatrix );

class TransLex;

double scoreByIdentity( const Phrase& hu, const Phrase& en );

double scoreByTranslation( const Phrase& hu, const Phrase& en, const TransLex& transLex );

// This is much-much slower, but instead of identity, uses a many-to-many dictionary.
// For performance reasons, by convention does not calculate the similarity if the 
// alignMatrix element contains outsideOfRadiusValue, a big negative number.
void sentenceListsToAlignMatrixTranslation(
                                           const SentenceList& huSentenceListPretty, const SentenceList& enSentenceList,
                                           const TransLex& transLex,
                                           AlignMatrix& alignMatrixDetailed );

class IBMModelOne;

void sentenceListsToAlignMatrixIBMModelOne(
                                           const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                                           const IBMModelOne& modelOne,
                                           AlignMatrix& alignMatrix );

int characterLength( const Word& words, bool utfCharCountingMode );

double characterLength( const Phrase& words, bool utfCharCountingMode );

double characterLength( int start, int end, const SentenceList& sentenceList, bool utfCharCountingMode );

void setSentenceValues( const SentenceList& sentences, SentenceValues& lengths, bool utfCharCountingMode );

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_BOOKTOMATRIX_H
