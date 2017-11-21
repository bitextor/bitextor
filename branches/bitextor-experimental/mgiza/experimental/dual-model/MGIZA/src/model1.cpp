/*

EGYPT Toolkit for Statistical Machine Translation
Written by Yaser Al-Onaizan, Jan Curin, Michael Jahr, Kevin Knight, John Lafferty, Dan Melamed, David Purdy, Franz Och, Noah Smith, and David Yarowsky.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
USA.

*/
#include "model1.h"
#include "Globals.h"
#include "utility.h"
#include "Parameter.h"

extern short NoEmptyWord;
extern int VerboseSentence;

extern short NCPUS;

GLOBAL_PARAMETER2(int,Model1_Dump_Freq,"MODEL 1 DUMP FREQUENCY","t1","dump frequency of Model 1",PARLEV_OUTPUT,0);
int NumberOfVALIalignments=100;

model1::model1(const char* efname, vcbList& evcblist, vcbList& fvcblist,tmodel<COUNT, PROB>&_tTable,Perplexity& _perp,
	      sentenceHandler& _sHandler1,
	      Perplexity* _testPerp,
	      sentenceHandler* _testHandler,
	      Perplexity& _trainViterbiPerp,
	      Perplexity* _testViterbiPerp):
  report_info(_perp,_sHandler1,_testPerp,_testHandler,_trainViterbiPerp,_testViterbiPerp),
  efFilename(efname), Elist(evcblist), Flist(fvcblist), 
  eTotalWCount(Elist.totalVocab()), fTotalWCount(Flist.totalVocab()), 
  noEnglishWords(Elist.size()), noFrenchWords(Flist.size()), tTable(_tTable),
  evlist(Elist.getVocabList()), fvlist(Flist.getVocabList())
{tTable2 = NULL;}

model1::model1 (const model1& m1, int _threadID):
report_info(m1),efFilename(m1.efFilename),
Elist(m1.Elist),Flist(m1.Flist),eTotalWCount(m1.eTotalWCount),fTotalWCount(m1.fTotalWCount),
noEnglishWords(m1.noEnglishWords),noFrenchWords(m1.noFrenchWords),tTable(m1.tTable),
evlist(m1.evlist),fvlist(m1.fvlist)
{tTable2 = NULL;}

void model1::initialize_table_uniformly(sentenceHandler& sHandler1){
  WordIndex i, j;

  cout << "Initialize tTable\n";

  sentPair sent ;
  sHandler1.rewind();
  while(sHandler1.getNextSentence(sent)){
    Vector<WordIndex>& es = sent.eSent;
    Vector<WordIndex>& fs = sent.fSent;
    PROB uniform = 1.0/es.size() ;
    for( i=0; i < es.size(); i++)
      for(j=1; j < fs.size(); j++)
      tTable.insert(es[i],fs[j],0,uniform);
	
  }
}
void model1::mle_with_links(sentenceHandler& sHandler1){
	if(tTable2){
		tTable2->init_constant(1e-10);
	}else{
		return;
	}
	WordIndex i, j;

  cout << "Initialize tTable (cost)\n";

  sentPair sent ;
  sHandler1.rewind();
  while(sHandler1.getNextSentence(sent)){
    Vector<WordIndex>& es = sent.eSent;
    Vector<WordIndex>& fs = sent.fSent;
	for(int k = 0 ; k < sent.eAnchor.size(); k++){
		i = sent.eAnchor[k];
		j = sent.fAnchor[k];
		if(i > es.size()||j > fs.size()){
			continue;
		}else{
			tTable2->incCount(i,j,1);
		}
	}
  }
  tTable2->normalizeTable(Elist,Flist);
  cerr << "Outputting MLE TTable file " << endl;
  string rs = Prefix + ".tmle"  ;
  cerr << "Before " << Elist.getVocabList().size() << " " << Flist.getVocabList().size()<<endl;
  tTable2->printProbTable(rs.c_str(),Elist.getVocabList(),Flist.getVocabList(),false);

}


struct em_loop_t{
    model1 *m1;
    int it;
    int nthread;
    Dictionary *dict;
    bool useDict;
    int result;
    pthread_t thread;
    int valid ;
};
    
void* exe_emloop(void *arg){
    em_loop_t* em =(em_loop_t *) arg;
    em->result = em->m1->em_thread(em->it,em->nthread,*em->dict,em->useDict);
    return arg;
}

int model1::em_thread(int noIterations, int nthread, /*Perplexity& perp, sentenceHandler& sHandler1, */
			     Dictionary& dictionary, bool useDict /*Perplexity* testPerp, sentenceHandler* testHandler, 
										     Perplexity& trainViterbiPerp, Perplexity* testViterbiPerp */ )
{
    double minErrors=1.0;int minIter=0;
    string modelName="Model1",shortModelName="1";
    char b[2];
    b[1] = '\0';
    b[0] = '0' + nthread;
    time_t st, it_st, fn, it_fn;
    string tfile, number, alignfile, test_alignfile;
    int pair_no;
    bool dump_files = false ;
    cout << "==========================================================\n";
    cout << modelName << " Training Started at: "<< ctime(&st) << "\n";  
    int it = noIterations;
    pair_no = 0 ;
    it_st = time(NULL);
    cout <<  "-----------\n" << modelName << ": Iteration " << it << '\n';
    dump_files = (Model1_Dump_Freq != 0) &&  ((it % Model1_Dump_Freq)  == 0) && !NODUMPS ;
//    dump_files = true;
    number = "";
    int n = it;
    do{
        number.insert((size_t)0, 1, (char)(n % 10 + '0'));
    } while((n /= 10) > 0);
    alignfile = Prefix + ".A" + shortModelName + "." + number + ".part" ;
    alignfile = alignfile + b;

    em_loop(it,perp, sHandler1, false, dump_files, alignfile.c_str(), dictionary, useDict, trainViterbiPerp); 
    return minIter;
}

int model1::em_with_tricks(int noIterations, /*Perplexity& perp, sentenceHandler& sHandler1, */
			    bool seedModel1, Dictionary& dictionary, bool useDict /*Perplexity* testPerp, sentenceHandler* testHandler, 
										     Perplexity& trainViterbiPerp, Perplexity* testViterbiPerp */ 
, bool dumpCount ,  const char* dumpCountName, bool useString)  // If specified, then will dump files before last iteration
{
    double minErrors=1.0;int minIter=0;
    string modelName="Model1",shortModelName="1";
    time_t st, it_st, fn, it_fn;
    string tfile, number, alignfile, test_alignfile;
    int pair_no;
    bool dump_files = false ;
    st = time(NULL);
    sHandler1.rewind();
    cout << "==========================================================\n";
    cout << modelName << " Training Started at: "<< ctime(&st) << "\n";  
    for(int it = 1; it <= noIterations; it++){
        pair_no = 0 ;
        it_st = time(NULL);
        cout <<  "-----------\n" << modelName << ": Iteration " << it << '\n';
        dump_files = (Model1_Dump_Freq != 0) &&  ((it % Model1_Dump_Freq)  == 0) && !NODUMPS ;
	//dump_files = true;
        number = "";
        int n = it;
        do{
            number.insert((size_t)0, 1, (char)(n % 10 + '0'));
        } while((n /= 10) > 0);
        tfile = Prefix + ".t" + shortModelName + "." + number ;
        alignfile = Prefix + ".A" + shortModelName + "." + number+".part0" ;
        test_alignfile = Prefix +".tst.A" + shortModelName + "." + number ;
        initAL();
        threadID = 0;
        int th;
        vector<em_loop_t> ths;
        ths.resize(NCPUS);
        sHandler1.rewind();
        for (th=1;th<NCPUS;th++){
            ths[th].m1=this;
            ths[th].it = it;
            ths[th].nthread = th;
            ths[th].dict = & dictionary;
            ths[th].useDict = useDict;
            ths[th].result = 0;
            ths[th].valid = pthread_create(&(ths[th].thread),NULL,exe_emloop,&(ths[th]));
            if(ths[th].valid){
                cerr << "Error starting thread " << th << endl;
            }
        }
        em_loop(it,perp, sHandler1, seedModel1, dump_files, alignfile.c_str(), dictionary, useDict, trainViterbiPerp); 
        perp.record("Model1");
        trainViterbiPerp.record("Model1");
        errorReportAL(cout, "IBM-1");
        
        cerr << "Main thread done, waiting" << endl;;
        for (th=1;th<NCPUS;th++){
            pthread_join((ths[th].thread),NULL);
            cerr << "Thread " << th << "done" << endl;
        }
        if (testPerp && testHandler) // calculate test perplexity
            em_loop(it,*testPerp, *testHandler, seedModel1, dump_files, test_alignfile.c_str(), dictionary, useDict, *testViterbiPerp, true); 
        if( errorsAL()<minErrors ) {
            minErrors=errorsAL();
            minIter=it;
        }
        //if (dump_files){
        //    if( OutputInAachenFormat==1 )
        //        tTable.printCountTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),1);
        //}
        cerr << "Normalizing T " << endl;

		/**
		 If asked for dumping count table, just dump it.
		 */
		if(dumpCount && it == noIterations){
			string realTableName = dumpCountName;
			realTableName += ".t.count";
			tTable.printCountTable(realTableName.c_str(),Elist.getVocabList(),Flist.getVocabList(),useString);
		}
		
        tTable.normalizeTable(Elist, Flist);
        //cout << tTable.getProb(2,2) << endl;
        cerr << " DONE Normalizing " << endl;
        cout << modelName << ": ("<<it<<") TRAIN CROSS-ENTROPY " << perp.cross_entropy()
            << " PERPLEXITY " << perp.perplexity() << '\n';
        if (testPerp && testHandler)
            cout << modelName << ": ("<<it<<") TEST CROSS-ENTROPY " << (*testPerp).cross_entropy()
            << " PERPLEXITY " << (*testPerp).perplexity() 
            << '\n';
        cout << modelName << ": ("<<it<<") VITERBI TRAIN CROSS-ENTROPY " << trainViterbiPerp.cross_entropy()
            << " PERPLEXITY " << trainViterbiPerp.perplexity() << '\n';
        if (testPerp && testHandler)
            cout << modelName << ": ("<<
            it<<") VITERBI TEST CROSS-ENTROPY " 
            << (*testViterbiPerp).cross_entropy()
            << " PERPLEXITY " << (*testViterbiPerp).perplexity() 
            << '\n';
        if (dump_files){
            if( OutputInAachenFormat==0 )
                tTable.printProbTable(tfile.c_str(),Elist.getVocabList(),
                                      Flist.getVocabList(),OutputInAachenFormat);
        }
        it_fn = time(NULL);
        cout << "Model 1 Iteration: " << it<< " took: " << difftime(it_fn, it_st) << " seconds\n";
        
        
    }
    fn = time(NULL) ;
    cout <<  "Entire " << modelName << " Training took: " << difftime(fn, st) << " seconds\n";
    return minIter;
}

bool model1::load_table(const char* tname){
  /* This function loads the t table from the given file; use it
     when you want to load results from previous t training
     without doing any new training.
     NAS, 7/11/99
  */
    cout << "Model1: loading t table \n" ;
    return tTable.readProbTable(tname);
}

  
extern float MINCOUNTINCREASE;
void model1::em_loop(int it,Perplexity& perp, sentenceHandler& sHandler1, bool seedModel1, 
		     bool dump_alignment, const char* alignfile, Dictionary& dict, bool useDict, Perplexity& viterbi_perp, bool test)
{
    WordIndex i, j, l, m ;
    double cross_entropy;
    int pair_no=0 ;
    perp.clear();
    viterbi_perp.clear();
    ofstream of2;
    // for each sentence pair in the corpus
    if (dump_alignment||FEWDUMPS)
        of2.open(alignfile);
    PROB uniform = 1.0/noFrenchWords ;
    sentPair sent ;
    
    while(sHandler1.getNextSentence(sent)){
        Vector<WordIndex>& es = sent.eSent;
        Vector<WordIndex>& fs = sent.fSent;
        const float so  = sent.getCount();
        l = es.size() - 1;
        m = fs.size() - 1;
        cross_entropy = log(1.0);
        Vector<WordIndex> viterbi_alignment(fs.size());
        double viterbi_score = 1 ;
        
        bool eindict[l + 1];
        bool findict[m + 1];
        bool indict[m + 1][l + 1];
        if(it == 1 && useDict){
            for(unsigned int dummy = 0; dummy <= l; dummy++) eindict[dummy] = false;
            for(unsigned int dummy = 0; dummy <= m; dummy++){
                findict[dummy] = false;
                for(unsigned int dummy2 = 0; dummy2 <= l; dummy2++) 
                    indict[dummy][dummy2] = false;
            }
            for(j = 0; j <= m; j++)
                for(i = 0; i <= l; i++)
                if(dict.indict(fs[j], es[i])){
                    eindict[i] = findict[j] = indict[j][i] = true;
                }
        }
        
        for(j=1; j <= m; j++){
            // entries  that map fs to all possible ei in this sentence.
            Vector<LpPair<COUNT,PROB> *> sPtrCache(es.size(),0); // cache pointers to table 
            LpPair<COUNT,PROB> **sPtrCachePtr;
            
            PROB denom = 0.0;
            WordIndex best_i = 0 ; // i for which fj is best maped to ei
            PROB word_best_score = 0 ;  // score for the best mapping of fj
            if (it == 1 && !seedModel1){
                denom = uniform  * es.size() ;
                word_best_score = uniform ;
            }
            else 
                for((i=0),(sPtrCachePtr=&sPtrCache[0]); i <= l; i++,sPtrCachePtr++){
                    PROB e(0.0) ;
                    (*sPtrCachePtr) = tTable.getPtr(es[i], fs[j]) ;
                    if ((*sPtrCachePtr) != 0 && (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                        e = (*((*sPtrCachePtr))).prob;
                    else e = PROB_SMOOTH ;
                    denom += e  ;
                    if (e > word_best_score){
                        word_best_score = e ;
                        best_i = i ;
                    }	
                }
            viterbi_alignment[j] = best_i ;
            viterbi_score *= word_best_score ; /// denom ;
            if (denom == 0){
                if (test)
                    cerr << "WARNING: denom is zero (TEST)\n";
                else 
                    cerr << "WARNING: denom is zero (TRAIN)\n";
            }
            cross_entropy += log(denom) ;
            if (!test){
                if(denom > 0){	  
                    COUNT val = COUNT(so) / (COUNT) double(denom) ;
                    /* this if loop implements a constraint on counting:
                    count(es[i], fs[j]) is implemented if and only if
                    es[i] and fs[j] occur together in the dictionary, 
                    OR
                    es[i] does not occur in the dictionary with any fs[x] and
                    fs[j] does not occur in the dictionary with any es[y]
                    */
                    if(it == 1 && useDict){
                        for((i=0),(sPtrCachePtr=&sPtrCache[0]); i <= l; i++,sPtrCachePtr++){
                            if(indict[j][i] || (!findict[j] && !eindict[i])){
                                PROB e(0.0) ;
                                if (it == 1 && !seedModel1)
                                    e =  uniform  ;
                                else if ((*sPtrCachePtr) != 0 &&  (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                                    e = (*((*sPtrCachePtr))).prob;
                                else e = PROB_SMOOTH ;
                                COUNT x=e*val;
                                if( (it==1 && !seedModel1)||x>MINCOUNTINCREASE )
                                /*    if ((*sPtrCachePtr) != 0)
                                    (*((*sPtrCachePtr))).count += x;
                                else 	      */
									tTable.incCount(es[i], fs[j], x);
                            } /* end of if */
                        } /* end of for i */
                    } /* end of it == 1 */
                    // Old code:
                    else{
                        for((i=0),(sPtrCachePtr=&sPtrCache[0]); i <= l; i++,sPtrCachePtr++){
                            //for(i=0; i <= l; i++) {	    
                            PROB e(0.0) ;
                            if (it == 1 && !seedModel1)
                                e =  uniform  ;
                            else if ((*sPtrCachePtr) != 0 &&  (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                                e = (*((*sPtrCachePtr))).prob;
                            else e = PROB_SMOOTH ;
                            //if( !(i==0) )
                            //cout << "COUNT(e): " << e << " " << MINCOUNTINCREASE << endl;
                            COUNT x=e*val;
                            if( pair_no==VerboseSentence )
                                cout << i << "(" << evlist[es[i]].word << ")," << j << "(" << fvlist[fs[j]].word << ")=" << x << endl;
                            if( (it==1 && !seedModel1)||x>MINCOUNTINCREASE ){
                                /*if( NoEmptyWord==0 || i!=0 )
								if ((*sPtrCachePtr) != 0) 
								(*((*sPtrCachePtr))).count += x;
							else */	      
								//cerr << i << " " << j << " (+) " << endl;
								//cerr.flush();
								//cerr << es[i] << " " << fs[j] << " (=) "<< endl;
								//cerr.flush();
								tTable.incCount(es[i], fs[j], x);
								//cerr << es[i] << " " << fs[j] << " (-) "<< endl;
								//cerr.flush();
							}
                        } /* end of for i */
                    } // end of else
                } // end of if (denom > 0)
            }// if (!test)
        } // end of for (j) ;
        sHandler1.setProbOfSentence(sent,cross_entropy);
        //cerr << sent << "CE: " << cross_entropy << " " << so << endl;
        perp.addFactor(cross_entropy-m*log(l+1.0), so, l, m,1);
        viterbi_perp.addFactor(log(viterbi_score)-m*log(l+1.0), so, l, m,1);
        if (dump_alignment||(FEWDUMPS&&sent.sentenceNo<1000))
            printAlignToFile(es, fs, evlist, fvlist, of2, viterbi_alignment, sent.sentenceNo, viterbi_score);
        addAL(viterbi_alignment,sent.sentenceNo,l);
        pair_no++;
    } /* of while */
}

CTTableDiff<COUNT,PROB>* model1::one_step_em(int it, bool seedModel1, 
    Dictionary& dictionary, bool useDict){
        CTTableDiff<COUNT,PROB> *diff = new CTTableDiff<COUNT,PROB>();
        double minErrors=1.0;int minIter=0;
        string modelName="Model1",shortModelName="1";
        time_t st, it_st, fn, it_fn;
        string tfile, number, alignfile, test_alignfile;
        int pair_no;
        bool dump_files = false ;
        st = time(NULL);
        sHandler1.rewind();
        cout << "==========================================================\n";
        cout << modelName << " Training Started at: "<< ctime(&st) << "\n";  
        pair_no = 0 ;
        it_st = time(NULL);
        cout <<  "-----------\n" << modelName << ": Iteration " << it << '\n';
        dump_files = (Model1_Dump_Freq != 0) &&  ((it % Model1_Dump_Freq)  == 0) && !NODUMPS ;
        number = "";
        int n = it;
        do{
            number.insert((size_t)0, 1, (char)(n % 10 + '0'));
        } while((n /= 10) > 0);
        tfile = Prefix + ".t" + shortModelName + "." + number ;
        alignfile = Prefix + ".A1" ;
        test_alignfile = Prefix +".tst.A" + shortModelName + "." + number ;
        initAL();
        em_loop_1(diff,it,perp, sHandler1, seedModel1, 
                  dump_files, alignfile.c_str(), dictionary, useDict, trainViterbiPerp); 
        //if (testPerp && testHandler) // calculate test perplexity
        //    em_loop(it,*testPerp, *testHandler, seedModel1, dump_files, test_alignfile.c_str(), dictionary, useDict, *testViterbiPerp, true); 
        if( errorsAL()<minErrors ){
            minErrors=errorsAL();
            minIter=it;
        }
        fn = time(NULL) ;
        cout <<  "Partial " << modelName << " Training took: " << difftime(fn, it_st) << " seconds\n";
        return diff;        
    }

    void model1::combine_one(CTTableDiff<COUNT,PROB>* cb){
        cb->AugmentTTable(tTable);
    }
    
    void model1::recombine(){
        tTable.normalizeTable(Elist, Flist);
    }
    
    void save_table(const char* tname){
/*         if (dump_files){
 *             if( OutputInAachenFormat==0 )
 *                 tTable.printProbTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),OutputInAachenFormat);
 */

    }

        
void model1::em_loop_1(CTTableDiff<COUNT,PROB> *diff,int it,Perplexity& perp, sentenceHandler& sHandler1, bool seedModel1, 
    bool dump_alignment, const char* alignfile, Dictionary& dict, bool useDict, Perplexity& viterbi_perp, bool test)    {
        WordIndex i, j, l, m ;
        double cross_entropy;
        int pair_no=0 ;
        perp.clear();
        viterbi_perp.clear();
        ofstream of2;
        // for each sentence pair in the corpus
        if (dump_alignment||FEWDUMPS)
            of2.open(alignfile);
        PROB uniform = 1.0/noFrenchWords ;
        sentPair sent ;
        sHandler1.rewind();
        while(sHandler1.getNextSentence(sent)){
            Vector<WordIndex>& es = sent.eSent;
            Vector<WordIndex>& fs = sent.fSent;
            const float so  = sent.getCount();
            l = es.size() - 1;
            m = fs.size() - 1;
            cross_entropy = log(1.0);
            Vector<WordIndex> viterbi_alignment(fs.size());
            double viterbi_score = 1 ;
            
            bool eindict[l + 1];
            bool findict[m + 1];
            bool indict[m + 1][l + 1];
            if(it == 1 && useDict){
                for(unsigned int dummy = 0; dummy <= l; dummy++) eindict[dummy] = false;
                for(unsigned int dummy = 0; dummy <= m; dummy++){
                    findict[dummy] = false;
                    for(unsigned int dummy2 = 0; dummy2 <= l; dummy2++) 
                        indict[dummy][dummy2] = false;
                }
                for(j = 0; j <= m; j++)
                    for(i = 0; i <= l; i++)
                    if(dict.indict(fs[j], es[i])){
                        eindict[i] = findict[j] = indict[j][i] = true;
                    }
            }
            
            for(j=1; j <= m; j++){
                // entries  that map fs to all possible ei in this sentence.
                Vector<LpPair<COUNT,PROB> *> sPtrCache(es.size(),0); // cache pointers to table 
                //Vector<COUNT *> sPtrCacheDif(es.size(),0); // cache pointers to table 
                LpPair<COUNT,PROB> **sPtrCachePtr;
                //COUNT **sPtrCachePtrDif;
                
                PROB denom = 0.0;
                WordIndex best_i = 0 ; // i for which fj is best maped to ei
                PROB word_best_score = 0 ;  // score for the best mapping of fj
                if (it == 1 && !seedModel1){
                    denom = uniform  * es.size() ;
                    word_best_score = uniform ;
                }
                else {
                    for((i=0),(sPtrCachePtr=&sPtrCache[0]); i <= l; i++,sPtrCachePtr++){
                        PROB e(0.0) ;
                        (*sPtrCachePtr) = tTable.getPtr(es[i], fs[j]) ;
                        //(*sPtrCachePtrDif) = diff->GetPtr(es[i], fs[j]) ;
                        if ((*sPtrCachePtr) != 0 && (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                            e = (*((*sPtrCachePtr))).prob;
                        else e = PROB_SMOOTH ;
                        denom += e  ;
                        if (e > word_best_score){
                            word_best_score = e ;
                            best_i = i ;
                        }	
                    }
                }
                viterbi_alignment[j] = best_i ;
                viterbi_score *= word_best_score ; /// denom ;
                if (denom == 0){
                    if (test)
                        cerr << "WARNING: denom is zero (TEST)\n";
                    else 
                        cerr << "WARNING: denom is zero (TRAIN)\n";
                }
                cross_entropy += log(denom) ;
                if (!test){
                    if(denom > 0){	  
                        COUNT val = COUNT(so) / (COUNT) double(denom) ;
                        /* this if loop implements a constraint on counting:
                        count(es[i], fs[j]) is implemented if and only if
                        es[i] and fs[j] occur together in the dictionary, 
                        OR
                        es[i] does not occur in the dictionary with any fs[x] and
                        fs[j] does not occur in the dictionary with any es[y]
                        */
                        if(it == 1 && useDict){
                            for((i=0),(sPtrCachePtr=&sPtrCache[0]);
                                i <= l; i++,sPtrCachePtr++){
                                if(indict[j][i] || (!findict[j] && !eindict[i])){
                                    PROB e(0.0) ;
                                    if (it == 1 && !seedModel1)
                                        e =  uniform  ;
                                    else if ((*sPtrCachePtr) != 0 &&  (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                                        e = (*((*sPtrCachePtr))).prob;
                                    else e = PROB_SMOOTH ;
                                    COUNT x=e*val;
                                    if( it==1||x>MINCOUNTINCREASE ){
                                        /*if ((*sPtrCachePtr) != 0){
                                            (*((*sPtrCachePtr))).count += x;
                                        } else {*/
                                            tTable.incCount(es[i], fs[j], x);
                                        //}
                                        diff->incCount(es[i], fs[j], x);
                                    }
                                } /* end of if */
                            } /* end of for i */
                        } /* end of it == 1 */
                        // Old code:
                        else{
                            for((i=0),(sPtrCachePtr=&sPtrCache[0]); i <= l; i++,sPtrCachePtr++){
                                //for(i=0; i <= l; i++) {	    
                                PROB e(0.0) ;
                                if (it == 1 && !seedModel1)
                                    e =  uniform  ;
                                else if ((*sPtrCachePtr) != 0 &&  (*((*sPtrCachePtr))).prob > PROB_SMOOTH) 
                                    e = (*((*sPtrCachePtr))).prob;
                                else e = PROB_SMOOTH ;
                                //if( !(i==0) )
                                //cout << "COUNT(e): " << e << " " << MINCOUNTINCREASE << endl;
                                COUNT x=e*val;
                                if( pair_no==VerboseSentence )
                                    cout << i << "(" << evlist[es[i]].word << "),"
                                    << j << "(" << fvlist[fs[j]].word << ")=" << x << endl;
                                if( it==1||x>MINCOUNTINCREASE )
                                    if( NoEmptyWord==0 || ( NoEmptyWord==0 || i!=0 )){
                                        /*if ((*sPtrCachePtr) != 0){
                                            (*((*sPtrCachePtr))).count += x;
                                        } else 	      */
                                            tTable.incCount(es[i], fs[j], x);
                                        diff->incCount(es[i], fs[j], x);
                                    }
                            } /* end of for i */
                        } // end of else
                    } // end of if (denom > 0)
                }// if (!test)
            } // end of for (j) ;
            sHandler1.setProbOfSentence(sent,cross_entropy);
            //cerr << sent << "CE: " << cross_entropy << " " << so << endl;
            perp.addFactor(cross_entropy-m*log(l+1.0), so, l, m,1);
            viterbi_perp.addFactor(log(viterbi_score)-m*log(l+1.0), so, l, m,1);
            if (dump_alignment||(FEWDUMPS&&sent.sentenceNo<1000))
                printAlignToFile(es, fs, evlist, fvlist, of2, viterbi_alignment, sent.sentenceNo, viterbi_score);
            addAL(viterbi_alignment,sent.sentenceNo,l);
            pair_no++;
        } /* of while */
        sHandler1.rewind();
        perp.record("Model1");
        viterbi_perp.record("Model1");
        errorReportAL(cout, "IBM-1");
    }
