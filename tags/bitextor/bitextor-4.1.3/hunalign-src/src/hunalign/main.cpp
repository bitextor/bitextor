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


#include <string>


namespace Hunglish
{

#ifdef WIN32
const std::string globalHome = "/";
#else
const std::string globalHome = "/home/daniel/";
#endif

std::string hunglishHome = globalHome + "hunglish/";
std::string hunglishExperimentsHome = hunglishHome + "data/experiments/";
std::string hunglishDictionaryHome  = hunglishHome + "data/szotar/";

} // namespace Hunglish


namespace Hunglish
{
  ///////////////////////////////////
  // Entry points of important tools:

  // Implemented in alignerTool.cpp
  int main_alignerTool(int argC, char* argV[]);

  // Implemented in cooccurrenceTool.cpp
  int main_cooccurrenceTool(int argC, char* argV[]);

  // Implemented in cooccurrenceTool.cpp
  int main_bicorpusProcessor(int argC, char* argV[]);

  ///////////////////////////////////
  // Just Tests:

  // We don't want to include DOM just for this function.
  // On the other hand, we don't want to create a header file just for this function. :)
  // Implemented in TEIReader.cpp
  int main_TEIReader( int argC, char* argV[] );

  // Implemented in networkFlow.cpp
  void main_edmondsKarpTest();

  // Implemented in oldAlignTest.cpp
  void main_alignTest();

  // Implemented in oldAlignTest.cpp
  void main_scoreByHandAlign();

  // Implemented in oldAlignTest.cpp
  void main_SmallSubsetLookupTest();

  // Implemented in oldAlignTest.cpp
  void main_HunHalfTest();

  // Implemented in oldAlignTest.cpp
  void main_translationTest();

  // Implemented in wordAlignment.cpp
  void main_wordAlignmentTest();

  // Implemented in bookToMatrix.cpp
  void main_similarityEvaluatorTool(int argC, char* argV[]);

} // namespace Hunglish

#include <timer.h>
#include <iostream>

void rectangleCacheTest()
{
  int xmax = 5000;

  const int ymaxmax=10000;
  const int step=100;

  char* a = new char[xmax*ymaxmax];

  {
    for ( int ymax=step; ymax<=ymaxmax; ymax+=step )
    {
      Hunglish::Ticker ticker;
      for ( int i=0; i<xmax; ++i )
      {
        for ( int j=0; j<ymax; ++j )
        {
          // a[i*ymax+j]= 97;
          // a[j*xmax+i]= 97; // slow
        }
      }

      std::cout << ymax << "\t" << (double)ticker.get()/xmax/ymax*1000 << std::endl;
    }
  }

  delete [] a;
}

void compilerOptimizationTest()
{
  int xmax=2000;
  int ymax=2000;

  int a;

  Hunglish::Ticker ticker;
  for ( int i=0; i<xmax; ++i )
  {
    for ( int j=0; j<ymax; ++j )
    {
      a = i*i*xmax+j ;
      if (a<0)
        return;
    }
  }

  std::cerr << ticker.get() << std::endl;
}


int main(int argC, char* argV[])
{
  // Hunglish::main_similarityEvaluatorTool(argC,argV); return 0;

  // compilerOptimizationTest(); return 0;
  // rectangleCacheTest(); return 0;

  return ( Hunglish::main_alignerTool(argC,argV) );

  Hunglish::main_wordAlignmentTest(); return 0;

  return ( Hunglish::main_cooccurrenceTool(argC,argV) );

  return ( Hunglish::main_bicorpusProcessor(argC,argV) );

  Hunglish::main_scoreByHandAlign(); return 0;

  Hunglish::main_edmondsKarpTest(); return 0;

  Hunglish::main_alignTest(); return 0;

  Hunglish::main_translationTest(); return 0;

  // return ( Hunglish::main_TEIReader(argC,argV) );

  Hunglish::main_HunHalfTest(); return 0;

  Hunglish::main_SmallSubsetLookupTest(); return 0;

  return 0;
}
