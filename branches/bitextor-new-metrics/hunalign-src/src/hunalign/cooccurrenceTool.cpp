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
#pragma warning ( disable : 4018 )

#include "cooccurrence.h"
#include "words.h"
#include "networkFlow.h"
#include <serializeImpl.h>

#include "dictionary.h"

#include <argumentsParser.h>
#include <timer.h>

#include <iostream>
#include <fstream>
#include <cassert>

namespace Hunglish
{

// Quick and very-very dirty.
void cooccurrenceTool( const char* huFilename, const char* enFilename, double minScore, int minCoocc )
{
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

  cooccurenceAnalysis( huSentenceList, enSentenceList, minScore, minCoocc );
}

void flowBuildTool( const char* huFilename, const char* enFilename, const std::string& flowFilename )
{
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

  std::ofstream flowStream(flowFilename.c_str());

  if (!flowStream.good())
  {
    throw "file error";
  }

  lexiconByEdmondsKarp( huSentenceList, enSentenceList );

//  flowBuilderXml( huSentenceList, enSentenceList, flowStream );
}

void main_cooccurrenceToolUsage()
{
  std::cerr << std::endl;
  std::cerr << "Usage : coocc [-mcNum ] [ -msPercent ] hunFile engFile" << std::endl;
  std::cerr << "          or" << std::endl;
  std::cerr << "        coocc -flow=combinatorica_filename hunFile engFile" << std::endl;
  std::cerr << "          -mcNum means only biwords with cooccurrence number Num are dumped." << std::endl;
  std::cerr << "          -msPercent means only biwords with score at least Percent/100 are dumped." << std::endl;
  std::cerr << "          -mc5 and -mc=5 is accepted syntax, -mc 5 is NOT." << std::endl;
}

int main_cooccurrenceTool(int argC, char* argV[])
{
#ifndef _DEBUG
  try
#endif
  {
    if (argC<3)
    {
      main_cooccurrenceToolUsage();
      throw "";
    }

    Arguments args;
    args.read(argC-2,argV);

    int minCoocc = -1;
    int minScore100 = -1;
    args.getNumericParam("ms",minScore100);
    args.getNumericParam("mc",minCoocc);
    double minScore = 0.01 * minScore100 ;

    const std::string flowArgName = "flow";
    bool buildFlow = ( args.find(flowArgName) != args.end() );

    std::string flowFilename;
    if (buildFlow)
    {
      flowFilename = args[flowArgName].dString;
      args.erase(flowArgName);

      if (flowFilename.empty())
      {
        std::cerr << "combinatorica_filename must be supported with the -flow argument."
          << std::endl;
        throw "argument error";
      }
    }

    try
    {
      args.checkEmptyArgs();
    }
    catch (...)
    {
      main_cooccurrenceToolUsage();
      throw "argument error";
    }

    if (!buildFlow)
    {
      cooccurrenceTool( argV[argC-2]/*huFilename*/, argV[argC-1]/*enFilename*/, minScore, minCoocc );
    }
    else
    {
      flowBuildTool( argV[argC-2]/*huFilename*/, argV[argC-1]/*enFilename*/, flowFilename );
    }
  }
#ifndef _DEBUG
  catch ( const char* errorType )
  {
    std::cerr << errorType << std::endl;
    return -1;
  }
  catch ( std::exception& e )
  {
    std::cerr << "some failed assertion:" << e.what() << std::endl;
    return -1;
  }
  catch ( ... )
  {
    std::cerr << "some unknown failed assertion..." << std::endl;
    return -1;
  }
#endif
  return 0;
}


void main_bicorpusProcessorUsage()
{
  std::cerr << std::endl;
  std::cerr << "Usage : bicorpus -dic=dicfile in.bicorpus [ out.bicorpus ]" << std::endl;
  std::cerr << "          or" << std::endl;
  std::cerr << "        bicorpus -dic=dicfile -split in.hun in.eng [ out.hun out.eng ]" << std::endl;
}

int main_bicorpusProcessor(int argC, char* argV[])
{
#ifndef _DEBUG
  try
#endif
  {
    Arguments args;
    std::vector<const char*> remains;
    args.read( argC, argV, remains );

    const std::string dicArgName = "dic";
    bool dicPresent = ( args.find(dicArgName) != args.end() );

    DictionaryItems dictionaryItems;

    if (dicPresent)
    {
      std::string dicFilename;
      dicFilename = args[dicArgName].dString;
      args.erase(dicArgName);

      if (dicFilename.empty())
      {
        std::cerr << "dictionary_filename must be supported with the -dic argument."
          << std::endl;
        throw "argument error";
      }

      std::cerr << "Reading dictionary..." << std::endl;
      std::ifstream dis(dicFilename.c_str());
      dictionaryItems.read(dis);
    }

    SentenceList huSentenceList;
    SentenceList enSentenceList;

    bool splitFileFormat = args.getSwitchCompact("split");

    if (splitFileFormat)
    {
      assert(remains.size()>=2);
      std::string huFilename = remains[0];
      std::string enFilename = remains[1];
      remains.erase(remains.begin(),remains.begin()+2);

      std::ifstream hus(huFilename.c_str());
      huSentenceList.readNoIds(hus);
      std::ifstream ens(enFilename.c_str());
      enSentenceList.readNoIds(ens);

      if (huSentenceList.size()!=enSentenceList.size())
      {
        std::cerr << "Number of sentences not matching: " 
          << huSentenceList.size() << " versus " << enSentenceList.size() << "."
          << std::endl;
        throw "data error";
      }
    }
    else
    {
      assert(remains.size()>=1);
      std::string huenFilename = remains[0];
      remains.erase(remains.begin(),remains.begin()+1);

      std::ifstream huens(huenFilename.c_str());
      readBicorpus(huens, huSentenceList, enSentenceList);
    }

    std::cerr << huSentenceList.size() << " bisentences read." << std::endl;

    try
    {
      args.checkEmptyArgs();
    }
    catch (...)
    {
      main_bicorpusProcessorUsage();
      throw "argument error";
    }

    Ticker ticker;

    // Itt vegezzuk a tenyleges munkat.
    std::cerr << "Starting filterBicorpusByLexicon... " << std::endl;
    filterBicorpusByLexicon( huSentenceList, enSentenceList, dictionaryItems );
    std::cerr << "Done in " << ticker.next() << " ms." << std::endl;

    if (!remains.empty())
    {
      // assert( remains.size() == ( splitFileFormat ? 2 : 1 ) );

      if (splitFileFormat)
      {
        assert(remains.size()==2);
        std::string huFilename = remains[0];
        std::string enFilename = remains[1];
        remains.erase(remains.begin(),remains.begin()+2);

        std::ofstream hus(huFilename.c_str());
        huSentenceList.writeNoIds(hus);
        std::ofstream ens(enFilename.c_str());
        enSentenceList.writeNoIds(ens);
      }
      else
      {
        assert(remains.size()==1);
        std::string huenFilename = remains[0];
        remains.erase(remains.begin(),remains.begin()+1);

        std::ofstream huens(huenFilename.c_str());
        writeBicorpus(huens, huSentenceList, enSentenceList);
      }
    }
    else
    {
      std::cerr << "No output written." << std::endl;
    }
  }

#ifndef _DEBUG
  catch ( const char* errorType )
  {
    std::cerr << errorType << std::endl;
    return -1;
  }
  catch ( std::exception& e )
  {
    std::cerr << "some failed assertion:" << e.what() << std::endl;
    return -1;
  }
  catch ( ... )
  {
    std::cerr << "some unknown failed assertion..." << std::endl;
    return -1;
  }
#endif
  return 0;
}


} // namespace Hunglish
