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
#include "alignment.h"
#include "dictionary.h" // For IBMModelOne

#include <iostream>
#include <cmath>

#include <fstream> // Just for similarityEvaluator, which should go anyway. TODO.

namespace Hunglish
{


// (!!!) We assert that sx and sy are ordered sets of Word-s!
int intersectionSize( const WordList& sx, const WordList& sy )
{
  int inter=0;
  WordList::const_iterator sxt = sx.begin();
  WordList::const_iterator syt = sy.begin();
  WordList::const_iterator sxe = sx.end();
  WordList::const_iterator sye = sy.end();
  for ( ; sxt!=sxe && syt!=sye ; )
  {
    if ( *sxt < *syt )
      ++sxt;
    else if ( *sxt > *syt )
      ++syt;
    else
    {
      ++inter;
      ++sxt;
      ++syt;
    }
  }
  return inter;
}

bool isNumber( const std::string& s )
{
  int n = s.size();
  for ( int i=0; i<n; ++i )
  {
    if ( (s[i]<'0') || (s[i]>'9') )
    {
      return false;
    }
  }
  return true;
}

// (!!!) We assert that sx and sy are ordered sets of Word-s!
int specializedIntersectionSize( const WordList& sx, const WordList& sy )
{
  int inter=0;
  WordList::const_iterator sxt = sx.begin();
  WordList::const_iterator syt = sy.begin();
  WordList::const_iterator sxe = sx.end();
  WordList::const_iterator sye = sy.end();

  int numberOfDifferingNumbers = 0;
  int numberOfSameNumbers = 0;

  for ( ; sxt!=sxe && syt!=sye ; )
  {
    if ( *sxt < *syt )
    {
      if (isNumber(*sxt))
      {
        ++numberOfDifferingNumbers;
      }
      ++sxt;
    }
    else if ( *sxt > *syt )
    {
      if (isNumber(*syt))
      {
        ++numberOfDifferingNumbers;
      }
      ++syt;
    }
    else
    {
      if (isNumber(*syt))
      {
        ++numberOfSameNumbers;
      }
      ++inter;
      ++sxt;
      ++syt;
    }
  }

  // TODO miert pont.
  if ( (numberOfSameNumbers>0) && ( numberOfDifferingNumbers <= numberOfSameNumbers/5 ) )
  {
    inter += 10;
  }

  return inter;
}

const std::string paragraphString = "<p>";

bool isParagraph( const Phrase& phrase )
{
  return ( (phrase.size()==1) && (phrase[0]==paragraphString) );
}

// Lenyegeben arra jo, hogy paragrafus csak paragrafussal legyen match-elve.
// Amiknek nem jut par kozuluk, azok gyakorlatilag mindenkeppen skippelve lesznek, mert
// scoreOfParagraphMisMatch << -skipPenalty.
// 
// Az, hogy a scoreOfParagraphMatch nagy, csak olyankor tud gondot okozni, amikor a
// magyar paragrafushatarolok halmaza(nak kepe) sem nem tartalmazza, sem nem tartalmazottja
// az angol paragrafushatarolok halmazanak. Peldaul
// a,<p>,b,<p>,c,d ellen a,b,<p>,c,<p>,d kikenyszeritheti a rossz megoldast a mondat-hasonlosagi
// pontszamok tiltakozasanak dacara is.
bool exceptionalScoring( const Phrase& hu, const Phrase& en, double& score )
{
  bool huIsParagraph = isParagraph(hu);
  bool enIsParagraph = isParagraph(en);

  // We like it if both are paragraph delimiters
  if ( huIsParagraph && enIsParagraph )
  {
    score = scoreOfParagraphMatch;
    return true;
  }

  if ( huIsParagraph || enIsParagraph )
  {
    score = scoreOfParagraphMisMatch;
    return true;
  }

  return false;
}


const double maximumScore = 3.0;

double scoreByIdentity( const Phrase& hu, const Phrase& en )
{
  double score = 0;
  if ( ! exceptionalScoring( hu, en, score ) )
  {
    score = specializedIntersectionSize( hu, en );

    // If we divide with max here, we are better at avoiding global mistakes.
    // If we divide with min here, we are better at avoiding local mistakes.
    // I think. This is just a theory. :)
    // What is fact? If we divide with min, we give higher scores to valid 2-to-1 segments.
    // But we make silly mistakes because we give higher scores to some invalid 1-to-1 segments like this:
    // Kocogtam. -Like I said, I was out jogging-- -ObviousIy, you weren't jogging.
    // Emlékszel? Remember the day that they threw you out?
    // 
    // Hopefully Gale-Church scoring compensates for this. Sometimes does not compensate enough.
    score /= ( (hu.size()<en.size() ? hu.size() : en.size() ) + 1 ) ;
    score *= maximumScore ;
  }

  return score;
}

// Itt egy ujabb valtozat, de egyelore rosszabb szamokat ad, talan mert rosszabbul kezeli az osszeolvadast.
//x 
//x // Ezt akkor csereltem ki 3.0-rol 5.0-re, amikor a minimumot maximumra csreltem alabb.
//x const double maximumScore = 5.0;
//x 
//x double scoreByIdentity( const Phrase& hu, const Phrase& en )
//x {
//x   double score = 0;
//x   if ( ! exceptionalScoring( hu, en, score ) )
//x   {
//x     score = specializedIntersectionSize( hu, en );
//x 
//x     // Itt honapokig minimum volt maximum helyett. A maximum kicsit rontja a jo minusegu handalignolt 
//x     // szovegeken elert pontossagot, A fene tudja. Vacakokon ez biztosan jobb.
//x     score /= ( (hu.size()>en.size() ? hu.size() : en.size() ) + 1 ) ;
//x     score *= maximumScore ;
//x   }
//x 
//x   return score;
//x }

void sentenceListsToAlignMatrixIdentity( const SentenceList& huSentenceList, const SentenceList& enSentenceList, AlignMatrix& alignMatrix )
{
  int huPos,enPos;

  int huBookSize = huSentenceList.size();
  int enBookSize = enSentenceList.size();

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    int rowStart = alignMatrix.rowStart(huPos);
    int rowEnd   = alignMatrix.rowEnd(huPos);
    for ( enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      const Phrase& hu = huSentenceList[huPos].words;
      const Phrase& en = enSentenceList[enPos].words;

      alignMatrix.setCell( huPos, enPos, scoreByIdentity(hu,en) );
    }

    bool rarelyLogging = true;

    if (!rarelyLogging || (huPos%100==0))
    {
      std::cerr << huPos << " ";
    }
  }
}

double scoreByTranslation( const Phrase& hu, const Phrase& en, const TransLex& transLex )
{
  double score = 0;
  if ( ! exceptionalScoring( hu, en, score ) )
  {
    for ( int huPos=0; huPos<hu.size(); ++huPos )
    {
      const Word& huWord = hu[huPos];
      // TODO Ezt lookupLeftWord es intersection_size kombinaciojaval kell:
      for ( int enPos=0; enPos<en.size(); ++enPos )
      {
        const Word& enWord = en[enPos];
        if ( (huWord==enWord) && (huWord!="is") && (huWord!="a") )
        {
          ++score;
        }
        else if (transLex.isPresent(huWord,enWord))
        {
          ++score;
        }
      }
    }
  }

  return score;
}

// This is much-much slower, but instead of identity, uses a many-to-many dictionary.
// For performance reasons, by convention does not calculate the similarity if the 
// alignMatrix element contains outsideOfRadiusValue, a big negative number.
void sentenceListsToAlignMatrixTranslation(
                                           const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                                           const TransLex& transLex,
                                           AlignMatrix& alignMatrix )
{

  int huPos,enPos;

  int huBookSize = huSentenceList.size();
  int enBookSize = enSentenceList.size();

  int numberOfEvaluatedItems(0);

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    int rowStart = alignMatrix.rowStart(huPos);
    int rowEnd   = alignMatrix.rowEnd(huPos);
    for ( enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      if (alignMatrix[huPos][enPos]==outsideOfRadiusValue)
      {
        continue;
      }

      ++numberOfEvaluatedItems;

      const Phrase& hu = huSentenceList[huPos].words;
      const Phrase& en = enSentenceList[enPos].words;

      alignMatrix.setCell( huPos, enPos, scoreByTranslation( hu, en, transLex ) );
    }

    bool rarelyLogging = true;

    if (!rarelyLogging || (huPos%100==0))
    {
      std::cerr << huPos << " (" << numberOfEvaluatedItems << ") ";
    }
  }
}


// typedef std::map< Word, std::set<std::pair<double,Word> > > IBMModelOne;

double scoreByModelOne( const Phrase& hu, const Phrase& en, const IBMModelOne& modelOne )
{
  double score = 0;
  if ( ! exceptionalScoring( hu, en, score ) )
  {
    score = - modelOne.distance(hu,en);
  }

  return score;
}

void sentenceListsToAlignMatrixIBMModelOne(
                                           const SentenceList& huSentenceList, const SentenceList& enSentenceList,
                                           const IBMModelOne& modelOne,
                                           AlignMatrix& alignMatrix )
{
  int huPos,enPos;

  int huBookSize = huSentenceList.size();
  int enBookSize = enSentenceList.size();

  for ( huPos=0; huPos<huBookSize; ++huPos )
  {
    int rowStart = alignMatrix.rowStart(huPos);
    int rowEnd   = alignMatrix.rowEnd(huPos);
    for ( enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      if (alignMatrix[huPos][enPos]==outsideOfRadiusValue)
      {
        continue;
      }

      const Phrase& hu = huSentenceList[huPos].words;
      const Phrase& en = enSentenceList[enPos].words;

      alignMatrix.setCell( huPos, enPos, scoreByModelOne( hu, en, modelOne ) );
    }

    bool rarelyLogging = true;

    if (!rarelyLogging || (huPos%100==0))
    {
      std::cerr << huPos << " ";
    }
  }
}


const double paragraphDelimiterFictiveLength = 0.1973;


int characterLength( const Word& word, bool utfCharCountingMode )
{
  if (utfCharCountingMode)
  {
    int length = 0;
    for ( int i=0; i<word.size(); ++i )
    {
      // A code is the start of an utf-8 byte-sequence describing a character
      // iff it is not in the [128,192) range. 
      if (((unsigned char)word[i]<(unsigned char)128)||((unsigned char)word[i]>=(unsigned char)192))
      {
        ++length;
      }
    }
    return length;
  }
  else
  {
    return word.size();
  }
}

double characterLength( const Phrase& words, bool utfCharCountingMode )
{
  // A space ennyi betut er:
  const double spaceValue = 0; // 1.5;


  if (isParagraph(words))
  {
    return paragraphDelimiterFictiveLength;
  }

  double sum(0);
  for ( int i=0; i<words.size(); ++i )
  {
    sum += characterLength( words[i], utfCharCountingMode ) + spaceValue ;
  }
  return sum;
}

double characterLength( int start, int end,
                 const SentenceList& sentenceList, bool utfCharCountingMode )
{
  // A mondat vege ennyi betut er:
  const double sentenceValue = 3 ;

  double sum(0);
  for ( int i=start; i<end; ++i )
  {
    double len = characterLength( sentenceList[i].words, utfCharCountingMode );

    // A paragrafushatarolo nullaval noveli a hosszt, mert ha tenyleg benne van a szegmentumban,
    // az ugyis huntoken- vagy szoveg-inkompatibilitasi hiba.
    if ( len != paragraphDelimiterFictiveLength )
    {
      sum += len + sentenceValue ;
    }
  }
  return sum;
}

void setSentenceValues( const SentenceList& sentences, SentenceValues& lengths, bool utfCharCountingMode )
{
  lengths.clear();

  for ( int i=0; i<sentences.size(); ++i )
  {
    lengths.push_back( characterLength(sentences[i].words,utfCharCountingMode) );
  }
}


} // namespace Hunglish
