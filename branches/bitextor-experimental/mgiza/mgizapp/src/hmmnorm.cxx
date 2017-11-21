// HMM Normalization executable

#include <iostream>
#include <strstream>
#include <string>
#include "hmm.h"
#include "Parameter.h"
#define ITER_M2 0
#define ITER_MH 5
GLOBAL_PARAMETER3(int,Model1_Iterations,"Model1_Iterations","NO. ITERATIONS MODEL 1","m1","number of iterations for Model 1",PARLEV_ITER,5);
GLOBAL_PARAMETER3(int,Model2_Iterations,"Model2_Iterations","NO. ITERATIONS MODEL 2","m2","number of iterations for Model 2",PARLEV_ITER,ITER_M2);
GLOBAL_PARAMETER3(int,HMM_Iterations,"HMM_Iterations","mh","number of iterations for HMM alignment model","mh", PARLEV_ITER,ITER_MH);
GLOBAL_PARAMETER3(int,Model3_Iterations,"Model3_Iterations","NO. ITERATIONS MODEL 3","m3","number of iterations for Model 3",PARLEV_ITER,5);
GLOBAL_PARAMETER3(int,Model4_Iterations,"Model4_Iterations","NO. ITERATIONS MODEL 4","m4","number of iterations for Model 4",PARLEV_ITER,5);
GLOBAL_PARAMETER3(int,Model5_Iterations,"Model5_Iterations","NO. ITERATIONS MODEL 5","m5","number of iterations for Model 5",PARLEV_ITER,0);
GLOBAL_PARAMETER3(int,Model6_Iterations,"Model6_Iterations","NO. ITERATIONS MODEL 6","m6","number of iterations for Model 6",PARLEV_ITER,0);

GLOBAL_PARAMETER(float, PROB_SMOOTH,"probSmooth","probability smoothing (floor) value ",PARLEV_OPTHEUR,1e-7);
GLOBAL_PARAMETER(float, MINCOUNTINCREASE,"minCountIncrease","minimal count increase",PARLEV_OPTHEUR,1e-7);

GLOBAL_PARAMETER2(int,Transfer_Dump_Freq,"TRANSFER DUMP FREQUENCY","t2to3","output: dump of transfer from Model 2 to 3",PARLEV_OUTPUT,0);
GLOBAL_PARAMETER2(bool,Verbose,"verbose","v","0: not verbose; 1: verbose",PARLEV_OUTPUT,0);
GLOBAL_PARAMETER(bool,Log,"log","0: no logfile; 1: logfile",PARLEV_OUTPUT,0);

GLOBAL_PARAMETER(double,P0,"p0","fixed value for parameter p_0 in IBM-3/4 (if negative then it is determined in training)",PARLEV_EM,-1.0);
GLOBAL_PARAMETER(double,M5P0,"m5p0","fixed value for parameter p_0 in IBM-5 (if negative then it is determined in training)",PARLEV_EM,-1.0);
GLOBAL_PARAMETER3(bool,Peg,"pegging","p","DO PEGGING? (Y/N)","0: no pegging; 1: do pegging",PARLEV_EM,0);

GLOBAL_PARAMETER(short,OldADBACKOFF,"adbackoff","",-1,0);
GLOBAL_PARAMETER2(unsigned int,MAX_SENTENCE_LENGTH,"ml","MAX SENTENCE LENGTH","maximum sentence length",0,MAX_SENTENCE_LENGTH_ALLOWED);

GLOBAL_PARAMETER(short, DeficientDistortionForEmptyWord,"DeficientDistortionForEmptyWord","0: IBM-3/IBM-4 as described in (Brown et al. 1993); 1: distortion model of empty word is deficient; 2: distoriton model of empty word is deficient (differently); setting this parameter also helps to avoid that during IBM-3 and IBM-4 training too many words are aligned with the empty word",PARLEV_MODELS,0);

/**
Here are parameters to support Load models and dump models
*/

GLOBAL_PARAMETER(int,restart,"restart","Restart training from a level,0: Normal restart, from model 1, 1: Model 1, 2: Model 2 Init (Using Model 1 model input and train model 2), 3: Model 2, (using model 2 input and train model 2), 4 : HMM Init (Using Model 1 model and train HMM), 5: HMM (Using Model 2 model and train HMM) 6 : HMM (Using HMM Model and train HMM), 7: Model 3 Init (Use HMM model and train model 3) 8: Model 3 Init (Use Model 2 model and train model 3) 9: Model 3, 10: Model 4 Init (Use Model 3 model and train Model 4) 11: Model 4 and on, ",PARLEV_INPUT,0);
GLOBAL_PARAMETER(bool,dumpCount,"dumpcount","Whether we are going to dump count (in addition to) final output?",PARLEV_OUTPUT,false);
GLOBAL_PARAMETER(bool,dumpCountUsingWordString,"dumpcountusingwordstring","In count table, should actual word appears or just the id? default is id",PARLEV_OUTPUT,false);
/// END
short OutputInAachenFormat=0;
bool Transfer=TRANSFER;
bool Transfer2to3=0;
short NoEmptyWord=0;
bool FEWDUMPS=0;
GLOBAL_PARAMETER(bool,ONLYALDUMPS,"ONLYALDUMPS","1: do not write any files",PARLEV_OUTPUT,0);
GLOBAL_PARAMETER(short,NCPUS,"NCPUS","Number of CPUS",PARLEV_EM,2);
GLOBAL_PARAMETER(short,CompactAlignmentFormat,"CompactAlignmentFormat","0: detailled alignment format, 1: compact alignment format ",PARLEV_OUTPUT,0);
GLOBAL_PARAMETER2(bool,NODUMPS,"NODUMPS","NO FILE DUMPS? (Y/N)","1: do not write any files",PARLEV_OUTPUT,0);

GLOBAL_PARAMETER(WordIndex, MAX_FERTILITY, "MAX_FERTILITY",
                 "maximal fertility for fertility models", PARLEV_EM, 10);

using namespace std;
string Prefix, LogFilename, OPath, Usage, SourceVocabFilename,
       TargetVocabFilename, CorpusFilename, TestCorpusFilename, t_Filename,
       SourceVocabClassesFilename, TargetVocabClassesFilename,
       a_Filename, p0_Filename, d_Filename, n_Filename, dictionary_Filename;


int main(int argc, char* argv[])
{
  if(argc < 5) {
    cerr << "Usage: " << argv[0] << " vcb1 vcb2 outputFile baseFile [additional1 ]..." << endl;
    return 1;
  }
  Vector<WordEntry> evlist,fvlist;
  vcbList eTrainVcbList(evlist), fTrainVcbList(fvlist);
  TargetVocabFilename = argv[2];
  SourceVocabFilename = argv[1];
  eTrainVcbList.setName(argv[1]);
  fTrainVcbList.setName(argv[2]);
  eTrainVcbList.readVocabList();
  fTrainVcbList.readVocabList();
  Perplexity trainPerp, testPerp, trainViterbiPerp, testViterbiPerp;
  tmodel<float, float> tTable;
  sentenceHandler *corpus = new sentenceHandler();


  model1 m1(CorpusFilename.c_str(), eTrainVcbList, fTrainVcbList, tTable,
            trainPerp, *corpus, &testPerp, corpus, trainViterbiPerp,
            &testViterbiPerp);
  amodel<float> aTable(false);
  amodel<float> aCountTable(false);
  model2 m2(m1, aTable, aCountTable);
  WordClasses french,english;
  hmm h(m2,english,french);
  SourceVocabClassesFilename = argv[1];
  TargetVocabClassesFilename = argv[2];
  SourceVocabClassesFilename += ".classes";
  TargetVocabClassesFilename += ".classes";
  h.makeWordClasses(m1.Elist, m1.Flist, SourceVocabClassesFilename.c_str(), TargetVocabClassesFilename.c_str());
  string base = argv[4];
  string baseA = base+".alpha";
  string baseB = base+".beta";
  string output = argv[3];
  string outputA = output+".alpha";
  string  outputB = output+".beta";
  h.probs.readJumps(base.c_str(),NULL,baseA.c_str(), baseB.c_str());
  // Start iteration:
  for(int i = 5; i< argc ; i++) {
    string name = argv[i];
    string nameA = name + ".alpha";
    string nameB = name + ".beta";
    if(h.counts.readJumps(name.c_str(),NULL,nameA.c_str(), nameB.c_str()))
      h.probs.merge(h.counts);
    else
      cerr << "Error, cannot load name.c_str()";
    h.clearCountTable();
  }
  h.probs.writeJumps(output.c_str(),NULL,outputA.c_str(), outputB.c_str());
  delete corpus;
}

// Some utility functions to get it compile..

ofstream logmsg;
double LAMBDA=1.09;

Vector<map< pair<int,int>,char > > ReferenceAlignment;

double ErrorsInAlignment(const map< pair<int,int>,char >&reference,
                         const Vector<WordIndex>&test, int l, int&missing, int&toomuch,
                         int&eventsMissing, int&eventsToomuch, int pair_no)
{
  return 0;
}

void printGIZAPars(ostream&out)
{
}

