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

#include "dictionary.h"
#include "bloom.h"
#include "translate.h"
#include "alignment.h"
#include "bookToMatrix.h"

#include "dicTree.h" // Just for main_SmallSubsetLookupTest()

#include <serializeImpl.h>

#include <argumentsParser.h>

#include <timer.h> // For itoa. Sorry for the misleading header filename.

#include <iostream>
#include <fstream>
#include <set>

// Copypaste-elve. TODO Elhelyezni.
#define massert(e) if (!(e)) { std::cerr << #e << " failed" << std::endl; throw "assert"; }

// Az ebben a namespace bracketben levo kod mind teljesen obsolete.
// Sehol mashol nincsenek meghivva, mint a szinten obsolete main_alignTest()-ben.
namespace Hunglish
{

extern std::string hunglishHome;
extern std::string hunglishDictionaryHome;
extern std::string hunglishExperimentsHome;

typedef std::map<std::string,std::string> AlignmentMap;

typedef std::string SentenceId;
typedef std::vector<SentenceId> Sentences;
typedef std::pair<Sentences,Sentences> BisentenceId;

class Alignment : public std::vector<BisentenceId>
{
public:
  void read( std::istream& is );
  void write( std::ostream& os ) const;
};

void Alignment::read( std::istream& is )
{
  clear();

  while ( is.peek() != -1 )
  {
    bool leftHand(true);
    BisentenceId bisentence;

    Sentences& left = bisentence.first;
    Sentences& right = bisentence.second;

    while (true)
    {
      std::string s;
      is >> s;

      if (s==";")
      {
        leftHand = false;
      }
      else
      {
        if (leftHand)
        {
          left.push_back(s);
        }
        else
        {
          right.push_back(s);
        }
      }

      if (is.peek()=='\n')
      {
        is.ignore();
        if (leftHand)
        {
          std::cerr << "no ; sign in line" << std::endl;
          throw "data error";
        }

        push_back(bisentence);
        break;
      }
    }
  }
}

void Alignment::write( std::ostream& os ) const
{
  for ( int i=0; i<size(); ++i )
  {
    const BisentenceId& bisentenceId=operator[](i);
    const Sentences& hu=bisentenceId.first;
    const Sentences& en=bisentenceId.second;
    int j;
    for ( j=0; j<hu.size(); ++j )
    {
      os << hu[j] << " " ;
    }
    os << ";" ;
    for ( j=0; j<en.size(); ++j )
    {
      os << " " << en[j] ;
    }
    os << std::endl;
  }
}

// Ez annyira obsolete, hogy meg az itteni obsolete kodok altal sincs hivva.
void alignmentToBisentences( const SentenceList& huSentenceList, const SentenceList& enSentenceList, const Alignment& alignmentHand,
                             SentenceList& huAlignedSentenceList, SentenceList& enAlignedSentenceList )
{
  huAlignedSentenceList.clear();
  enAlignedSentenceList.clear();

  int enCursor(0), huCursor(0);

  for ( int i=0; i<alignmentHand.size(); ++i )
  {
    if ( ( alignmentHand[i].first.size()==1 ) && ( alignmentHand[i].second.size()==1 ) )
    {
      const Sentence& enSent = enSentenceList[enCursor];
      const Sentence& huSent = huSentenceList[huCursor];

      if ( huSent.id != alignmentHand[i].first[0] )
      {
        std::cerr << "Mismatch: " << huSent.id << " " << alignmentHand[i].first[0] << std::endl;
        throw "data error";
      }
      if ( enSent.id != alignmentHand[i].second[0] )
      {
        std::cerr << "Mismatch: " << enSent.id << " " << alignmentHand[i].second[0] << std::endl;
        throw "data error";
      }

      huAlignedSentenceList.push_back(huSent);
      enAlignedSentenceList.push_back(enSent);
      // std::cerr << huSent.words << "\t" << enSent.words << std::endl;

    }

    huCursor += alignmentHand[i].first.size();
    enCursor += alignmentHand[i].second.size();
  }

  std::cerr << "bisentence number: " << huAlignedSentenceList.size() << std::endl;
}

// Ez annyira obsolete, hogy meg az itteni obsolete kodok altal sincs hivva.
void trailToAlignment( const Trail& bestTrail, Alignment& alignmentAuto )
{
  alignmentAuto.clear();

  // massert( bestTrail[0].first  == 0 );
  // massert( bestTrail[0].second == 0 );

  for ( int i=1; i<bestTrail.size(); ++i )
  {
    // Aki beszol a sok masolasra, azt kinevetjuk.

    Sentences hu,en;
    int j;
    for ( j=bestTrail[i-1].first;  j<bestTrail[i].first;  ++j )
    {
      char s[10];
      itoa(j,s,10);
      hu.push_back(s);
    }
    for ( j=bestTrail[i-1].second; j<bestTrail[i].second; ++j )
    {
      char s[10];
      itoa(j,s,10);
      en.push_back(s);
    }

    BisentenceId bisentenceId;
    bisentenceId.first  = hu;
    bisentenceId.second = en;

    alignmentAuto.push_back(bisentenceId);
  }

  /*
  std::ofstream os("autoalign.temp");
  alignmentAuto.write(os);
  */
}

// Pick one-to-one bisentences.
void turnAlignmentToMap( const Alignment& alignment, AlignmentMap& alignmentMap )
{
  alignmentMap.clear();

  Alignment::const_iterator it;
  for ( it=alignment.begin(); it!=alignment.end(); ++it )
  {
    const BisentenceId& bis = *it;
    if ( (bis.first.size()!=1) || (bis.second.size()!=1) )
    {
    }
    else
    {
      alignmentMap[ bis.first[0] ] = bis.second[0] ;
    }

  }
}

// In cautious mode, auto-aligned one-to-one bisentences are thrown away if
// the have left or right neighbours which are not one-to-one.
// This of course dramatically improves precision while slightly degrading recall.
void turnAlignmentToMapCautiously( const Alignment& alignment, AlignmentMap& alignmentMap )
{
  Alignment::const_iterator it;
  for ( it=alignment.begin(); it!=alignment.end(); ++it )
  {
    Alignment::const_iterator ct = it;
    if (ct==alignment.end())
      continue;
    bool previousIsOneToOne = ( (ct->first.size()==1) && (ct->second.size()==1) );

    ++ct;
    Alignment::const_iterator currentT = ct;
    if (ct==alignment.end())
      continue;
    bool thisIsOneToOne = ( (ct->first.size()==1) && (ct->second.size()==1) );

    ++ct;
    if (ct==alignment.end())
      continue;
    bool nextIsOneToOne = ( (ct->first.size()==1) && (ct->second.size()==1) );

    if ( previousIsOneToOne && thisIsOneToOne && nextIsOneToOne )
    {
      alignmentMap[ currentT->first[0] ] = currentT->second[0] ;
    }
  }
}

// Related to the evaluation of alignments. Calculates precision and recall between two sets:
// The hand-aligned and the auto-aligned one-to-one bisentence sets.
// Many-to-x and zero-to-x alignments are ignored.
//
// In cautious mode, auto-aligned one-to-one bisentences are thrown away if
// the have left or right neighbours which are not one-to-one.
// This of course dramatically improves precision while slightly degrading recall.
double scoreByHandAlign( const Alignment& alignmentAuto, const Alignment& alignmentHand,
                        bool cautiously )
{
  double score(0);

  AlignmentMap alignmentAutoMap;
  if (cautiously)
  {
    turnAlignmentToMapCautiously( alignmentAuto, alignmentAutoMap);
  }
  else
  {
    turnAlignmentToMap( alignmentAuto, alignmentAutoMap);
  }

  AlignmentMap alignmentHandMap;
  // Itt semmi ertelme a cautiously-nek, mert csak tevesen redukalja a lehetseges talalatok teret.
  turnAlignmentToMap( alignmentHand, alignmentHandMap);

  AlignmentMap::const_iterator alit;
  for ( alit=alignmentHandMap.begin(); alit!=alignmentHandMap.end(); ++alit )
  {
    AlignmentMap::const_iterator alfit = alignmentAutoMap.find(alit->first);
    if ( ( alfit != alignmentAutoMap.end() ) && ( alfit->second == alit->second ) )
      ++score;
  }

  std::cerr << alignmentAutoMap.size()-score << " misaligned out of " << alignmentHandMap.size() << " correct items, "
    << alignmentAutoMap.size() << " bets." << std::endl;

  std::cerr << "Precision: " << 1.0*score/alignmentAutoMap.size() 
    << ", Recall: " << 1.0*score/alignmentHandMap.size() << std::endl;

  /*
  // Ez rossz, csak emlekbe van itt. Rossz emlekbe.
  double ratio = ((double)alignmentHandMap.size()-score) / alignmentHandMap.size();
  std::cerr << "Score: " << ratio << " out of " << alignmentHandMap.size() << " correct items, "
    << alignmentAutoMap.size() << " bets." << std::endl;
    */

  double ratio = 1.0*(alignmentAutoMap.size()-score)/alignmentAutoMap.size();
  return ratio;
}

void bloomize( const SentenceList& sentenceList, BloomBook& bloomBook )
{
  massert(sentenceList.size()==bloomBook.size());

  for ( int i=0; i<sentenceList.size(); ++i )
  {
    const Phrase& sentence = sentenceList[i].words;

    for ( int j=0; j<sentence.size(); ++j )
    {
      bloomBook[i].set(sentence[j]);
    }
  }
}

// Ez kicsit monstrozusra sikeredett. Egy 340 soros fuggveny. :)
void main_alignTest()
{
  SentenceList huSentenceListPretty;
  {
    std::string xmlFile = hunglishExperimentsHome+"1984.hu.lemmas";
    std::ifstream is(xmlFile.c_str());
    huSentenceListPretty.read( is );

    std::cerr << huSentenceListPretty.size() << " hungarian sentences read." << std::endl;
  }

  SentenceList enSentenceList;
  {
    std::string xmlFile = hunglishExperimentsHome+"1984.en.lemmas";
    std::ifstream is(xmlFile.c_str());
    enSentenceList.read( is );

    std::cerr << enSentenceList.size() << " english sentences read." << std::endl;
  }

  SentenceList enSentenceListPretty = enSentenceList;

  SentenceList huSentenceList;

  bool hunglishFromFile = false; // If true, that makes the alignment a two-step process.
  // First main_translationTest()'s output should be directed to 1984.enFromHu.lemmas, then
  // main_alignTest() must be called.

  if (hunglishFromFile)
  {
    std::string xmlFile = hunglishExperimentsHome+"1984.enFromHu.lemmas";
    std::ifstream is(xmlFile.c_str());
    huSentenceList.read( is );

    std::cerr << huSentenceList.size() << " hunglish sentences read." << std::endl;
  }
  else
  {
    DumbDictionary dumbDictionary;

    std::string dictionaryName = 
      "vonyo7.nojoker.txt" ;
      // "vonyokornai.nojoker.txt" ;
      // "vonyo7plusvonyokornai.nojoker.txt" ;

    dictionaryName = hunglishDictionaryHome+dictionaryName;

    buildDumbDictionary( dumbDictionary, dictionaryName, enSentenceList );

    trivialTranslateSentenceList( dumbDictionary, huSentenceListPretty, huSentenceList );

    std::string lemmasFile = hunglishExperimentsHome+"1984.enFromHuGenerated.lemmas";
    std::ofstream is(lemmasFile.c_str());
    huSentenceList.write( is );
  }

  int huBookSize = huSentenceList.size();
  int enBookSize = enSentenceList.size();

  bool reduceForTesting = false;
  if (reduceForTesting)
  {
    std::cerr << "Reduced booksizes." << std::endl; // 1.[1 2].*
    // Ez meg igen   : Ohu.1.2.45.6	feláll és levert elindul az ajtó felé
    // De ez mar nem : Ohu.1.3.1.1	mikor az ajtó gomb tesz a kéz akkor vesz észre hogy a napló nyitva hagy az asztal
    huBookSize = 313;
    // Ez meg igen   : Oen.1.1.45.6	he get up and move heavily towards the door
    // De ez mar nem : Oen.1.2.1.1	as he put his hand to the door-knob Winston see that he have leave the diary open on the table
    enBookSize = 312;

    huSentenceList.resize(huBookSize);
    huSentenceListPretty.resize(huBookSize);

    enSentenceList.resize(enBookSize);
    enSentenceListPretty.resize(enBookSize);
  }

  bool filterByFrequency = false;
  if (filterByFrequency)
  {
    FrequencyMap huFreq, enFreq;

    huFreq.build(huSentenceList);
    enFreq.build(enSentenceList);

    std::cout << "---\nHungarian top ten:\n";
    huFreq.dump(std::cout, 20);
    std::cout << "---\nEnglish top ten:\n";
    enFreq.dump(std::cout, 20);

    WordList huAllowedWords, enAllowedWords;

    huFreq.lowPassFilter(huAllowedWords,0.99);
    enFreq.lowPassFilter(enAllowedWords,0.99);

    filterSentences(huSentenceList,huAllowedWords);
    filterSentences(enSentenceList,enAllowedWords);

    {
      FrequencyMap huFreq, enFreq;

      huFreq.build(huSentenceList);
      enFreq.build(enSentenceList);

      std::cout << "---\nHungarian top ten after lowpass filter:\n";
      huFreq.dump(std::cout, 20);
      std::cout << "---\nEnglish top ten after lowpass filter:\n";
      enFreq.dump(std::cout, 20);
    }
  }

  BloomBook huBloomBook(huBookSize);
  BloomBook enBloomBook(enBookSize);

  bloomize(huSentenceList,huBloomBook);
  bloomize(enSentenceList,enBloomBook);

  /*
  for ( int i=0; i<huBloomBook.size(); ++i )
  {
    for ( int j=0; j<bloomSize; ++j )
    {
      std::cout << huBloomBook[i].getBitset()[j] ? "x" : "." ;
    }
    std::cout << std::endl;
  }
  */

  // Normalizaljuk a Sentence-eket, hogy abecerendben legyenek a szavak.
  {
    sortNormalizeSentences(huSentenceList);
    sortNormalizeSentences(enSentenceList);
  }

  const int thickness = 100;

  AlignMatrix alignMatrix( huBookSize, enBookSize, thickness );

  sentenceListsToAlignMatrixIdentity( huSentenceList, enSentenceList, alignMatrix );

  bool visualize = false;
  bool graphical = false;

  SentenceValues huLength,enLength;
  setSentenceValues( huSentenceListPretty, huLength, false/*utfCharCountingMode*/ ); // Here we use the most originalest Hungarian text.
  setSentenceValues( enSentenceList,       enLength, false/*utfCharCountingMode*/ );

  Trail bestTrail;
  AlignMatrix dynMatrix( huBookSize+1, enBookSize+1, thickness );
  align( alignMatrix, huLength, enLength, bestTrail, dynMatrix );

  std::cerr << "Align ready." << std::endl;

  bool compareToHandAlign = true;
  if (compareToHandAlign)
  {
    Alignment alignmentHand;
    std::ifstream is1( (hunglishExperimentsHome+"1984.align").c_str() );

    alignmentHand.read(is1);

    Alignment alignmentAuto;
    for ( int i=0; i<bestTrail.size()-1 /*!!!*/; ++i )
    {
      // The [huPos, nexthuPos) interval corresponds to the [enPos, nextenPos) interval.
      int huPos = bestTrail[i].first;
      int enPos = bestTrail[i].second;
      int nexthuPos = bestTrail[i+1].first;
      int nextenPos = bestTrail[i+1].second;

      bool logTrail(false);
      if (logTrail)
      {
        bool justOneToOne(true);
        if ( !justOneToOne || ((nexthuPos-huPos==1)&&(nextenPos-enPos==1)) )
        {
          std::cout << nexthuPos << " " << nextenPos << std::endl;
        }
      }

      BisentenceId bisentence;
      Sentences& left = bisentence.first;
      Sentences& right = bisentence.second;

      int j;
      for ( j=huPos; j<nexthuPos; ++j )
      {
        left.push_back(huSentenceListPretty[j].id);
      }
      for ( j=enPos; j<nextenPos; ++j )
      {
        right.push_back(enSentenceListPretty[j].id);
      }
      alignmentAuto.push_back(bisentence);
    }

    std::cerr << "Cautious mode:\n";
    double cscore = scoreByHandAlign( alignmentAuto, alignmentHand, true/*cautiously*/ );
    std::cout << "Score: " << cscore << std::endl;
    std::cerr << "Score: " << cscore << std::endl;

    std::cerr << "\nUncautious mode:\n";
    double uscore = scoreByHandAlign( alignmentAuto, alignmentHand, false/*cautiously*/ );
    std::cout << "Score: " << uscore << std::endl;
    std::cerr << "Score: " << uscore << std::endl;

    return;
  }

  bool conciseOutput = true;
  if (conciseOutput)
  {
    for ( int i=0; i<bestTrail.size()-1 /*!!!*/; ++i )
    {
      // The [huPos, nexthuPos) interval corresponds to the [enPos, nextenPos) interval.
      int huPos = bestTrail[i].first;
      int enPos = bestTrail[i].second;
      int nexthuPos = bestTrail[i+1].first;
      int nextenPos = bestTrail[i+1].second;

      int j;
      for ( j=huPos; j<nexthuPos; ++j )
      {
        std::cout << huSentenceListPretty[j].id << " ";
      }
      std::cout << ";" ;
      for ( j=enPos; j<nextenPos; ++j )
      {
        std::cout << " " << enSentenceListPretty[j].id;
      }
      std::cout << std::endl;
    }
  }
  else
  {
    for ( int i=0; i<bestTrail.size()-1 /*!!!*/; ++i )
    {
      // The [huPos, nexthuPos) interval corresponds to the [enPos, nextenPos) interval.
      int huPos = bestTrail[i].first;
      int enPos = bestTrail[i].second;
      int nexthuPos = bestTrail[i+1].first;
      int nextenPos = bestTrail[i+1].second;

      bool justTheInteresting = true;

      if (!justTheInteresting || ((nexthuPos-huPos)!=1) || ((nextenPos-enPos)!=1) )
      {
        int j;
        for ( j=huPos; j<nexthuPos; ++j )
        {
          std::cout << huSentenceListPretty[j].id << "\t" << huSentenceListPretty[j].words << std::endl;
        }
        for ( j=enPos; j<nextenPos; ++j )
        {
          std::cout << enSentenceListPretty[j].id << "\t" << enSentenceListPretty[j].words << std::endl;
        }
        std::cout << std::endl;
      }
    }
  }
}

void main_scoreByHandAlign()
{
  Alignment alignmentHand;
  std::ifstream is1( (hunglishExperimentsHome+"hand.bisentences").c_str() );

  alignmentHand.read(is1);

  Alignment alignmentAuto;
  std::ifstream is2( (hunglishHome+"auto.oldindexes.rebound").c_str() );

  alignmentAuto.read(is2);

  std::cout << "Score: " << scoreByHandAlign( alignmentAuto, alignmentHand, false ) << std::endl;
}

void main_SmallSubsetLookupTest()
{
  SubsetLookup<Word,int> subsetLookup;
  WordList wl;
  wl.push_back("a");
  subsetLookup.add(wl,1); // "a" = 1
  wl.push_back("b");
  wl.push_back("c1");
  subsetLookup.add(wl,2); // "a b c1" = 2
  wl[2]="c2";
  subsetLookup.add(wl,3); // "a b c2" = 3

  subsetLookup.dump(std::cerr);

  std::set<int> result;
  WordList wr;
 
  wr.push_back("a");
  subsetLookup.lookup(wr,result);
  std::cerr << wr << " : " << result << std::endl;

  wr.push_back("b");
  wr.push_back("c2");
  wr.push_back("d");
  subsetLookup.lookup(wr,result);
  std::cerr << wr << " : " << result << std::endl;
}


void main_HunHalfTest()
{
  HalfDictionary halfDictionary;

  std::ifstream is( (hunglishExperimentsHome+"nojoker.txt").c_str() );
  // std::ifstream is( (hunglishExperimentsHome+"nojoker.test").c_str());
  halfDictionary.read( is );

  std::cerr << halfDictionary.size() << " items read." << std::endl;

  SubsetLookup<Word,int> subsetLookup;

  int i;
  for ( i=0; i<halfDictionary.size(); ++i )
  {
    subsetLookup.add( halfDictionary[i], i+1 ); // !!! i+1
  }

  std::cerr << "Index tree built." << std::endl;

  HalfDictionary testSentences;

  std::ifstream sens( (hunglishExperimentsHome+"s1.txt").c_str() );
  testSentences.read( sens );

  for ( i=0; i<testSentences.size(); ++i )
  {
    std::cout << testSentences[i] << " :" << std::endl;

    std::set<int> results;
    subsetLookup.lookup( testSentences[i], results );

    for ( std::set<int>::const_iterator it=results.begin(); it!=results.end(); ++it )
    {
      std::cout << "    " << halfDictionary[*it-1] << std::endl; // !!! i-1
    }

    std::cout << std::endl;
  }

  std::cerr << "Analysis ready." << std::endl;
}


void main_translationTest()
{
  DictionaryItems dictionary;
  {
    std::ifstream is( (hunglishDictionaryHome+"vonyokornai.nojoker.txt").c_str() );
    dictionary.read( is );

    std::cerr << dictionary.size() << " dictionary items read." << std::endl;
  }

  SentenceList sentenceList;
  {
    std::string xmlFile = hunglishExperimentsHome+"1984.hu.lemmas";
    std::ifstream is(xmlFile.c_str());
    sentenceList.read( is );

    std::cerr << sentenceList.size() << " sentences read." << std::endl;
  }

  SentenceList translatedSentenceList;

  naiveTranslate( dictionary, sentenceList, translatedSentenceList );

  translatedSentenceList.write(std::cout);
}

} // namespace Hunglish
