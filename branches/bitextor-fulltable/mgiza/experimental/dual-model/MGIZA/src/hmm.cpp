/*

Copyright (C) 1998,1999,2000,2001  Franz Josef Och (RWTH Aachen - Lehrstuhl fuer Informatik VI)

This file is part of GIZA++ ( extension of GIZA ).

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
#include "hmm.h" 
#include "Globals.h"
#include "utility.h"
#include "HMMTables.h" 
#include "ForwardBackward.h"
#include "Parameter.h"
#include <iostream>
#include "syncObj.h"
using namespace std;
#define CLASSIFY(i,empty,ianf) bool empty=(i>=l); unsigned int ianf=(i%l);
#define CLASSIFY2(i,ianf) unsigned int ianf=(i%l);


short PredictionInAlignments=0;
short UniformEntryExit=3;
short HMMTrainingSpecialFlags=0;

GLOBAL_PARAMETER2(int,ModelH_Dump_Freq,
                "HMM DUMP FREQUENCY","th",
                "dump frequency of HMM",
                PARLEV_OUTPUT,0);

GLOBAL_PARAMETER(short,CompareAlDeps,"emAlignmentDependencies",
		 "lextrain: dependencies in the HMM alignment model. "
		 " &1: sentence length; &2: previous class; &4: previous position; "
		 " &8: French position; &16: French class"
		 ,PARLEV_MODELS,2);

GLOBAL_PARAMETER(double,GLOBALProbabilityForEmpty,
                "emProbForEmpty","f-b-trn: probability for empty word",
                PARLEV_MODELS,0.4);

GLOBAL_PARAMETER(short,SmoothHMM,"emSmoothHMM",
		"f-b-trn: smooth HMM model &1: modified counts; &2:perform smoothing with -emAlSmooth",
        PARLEV_SPECIAL,2);

GLOBAL_PARAMETER(double,HMMAlignmentModelSmoothFactor,"emAlSmooth",
        "f-b-trn: smoothing factor for HMM alignment model (can be ignored by -emSmoothHMM)",
        PARLEV_SMOOTH,0.2);


/*template<class T>
void smooth_standard(T*a,T*b,double p)
{
  int n=b-a;
  if( n==0 ) 
    return;
  double pp=p/n;
  for(T*i=a;i!=b;++i)
    *i = (1.0-p)*(*i)+pp;
}*/


hmm::hmm(model2&m2,WordClasses &e, WordClasses& f)
:   ewordclasses(e), fwordclasses(f),model2(m2),counts(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses),
probs(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses)
{  }


void hmm::initialize_table_uniformly(sentenceHandler&){}

struct hmm_em_loop_t{
    hmm *m;
    int done;
    int valid;
    string alignfile;
    int it;
    bool dump_files;
	bool resume;
    pthread_t thread;
    hmm_em_loop_t():m(0),done(0),valid(0){};
};
    
void* hmm_exe_emloop(void *arg){
	hmm_em_loop_t* em =(hmm_em_loop_t *) arg;
    em->m->em_thread(em->it,em->alignfile,em->dump_files,em->resume);
    em->done = -1; 
    return arg;
} 

void hmm::em_thread(int it,string alignfile,bool dump_files,bool resume){
	em_loop(perp, sHandler1,  dump_files , alignfile.c_str(), trainViterbiPerp, false,it==1 && (!resume),it);
}
extern short NCPUS;
int hmm::em_with_tricks(int noIterations,bool dumpCount, 
	    const char* dumpCountName, bool useString ,bool resume){
    double minErrors=1.0;int minIter=0;
    string modelName="Hmm",shortModelName="hmm";
    int dumpFreq=ModelH_Dump_Freq;
    time_t it_st, st, it_fn, fn;
    string tfile, afile,afileh, number, alignfile, test_alignfile;
    int pair_no = 0;
    bool dump_files = false ;
    ofstream of2 ;
    st = time(NULL) ;
    sHandler1.rewind();
    cout << "\n==========================================================\n";
    cout << modelName << " Training Started at: " << ctime(&st);
    vector<hmm_em_loop_t> th;
    th.resize(NCPUS);
    for(int it=1; it <= noIterations ; it++){
        pair_no = 0;
        it_st = time(NULL) ;
        cout << endl << "-----------\n" << modelName << ": Iteration " << it << '\n';
        dump_files = (dumpFreq != 0) && ((it % dumpFreq) == 0) && !NODUMPS;
        //dump_files = true;
        number = "";
        int n = it;
        do{
            number.insert((size_t)0, 1, (char)(n % 10 + '0'));
        } while((n /= 10) > 0);
        tfile = Prefix + ".t" + shortModelName + "." + number ;
        afile = Prefix + ".a" + shortModelName + "." + number ;
       // acfile = Prefix + ".ac" + shortModelName + "." + number ;
        afileh = Prefix + ".h" + shortModelName + "." + number ;
        alignfile = Prefix + ".A" + shortModelName + "." + number ;
        test_alignfile = Prefix + ".tst.A" + shortModelName + "." + number ;
        counts=HMMTables<int,WordClasses>(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses);
        aCountTable.clear();
        initAL();
        sHandler1.rewind();
        int k;
        char node[2] ;
        node[1] = '\0';
        for (k=1 ; k< NCPUS ; k++){
        	th[k].m = this;
        	th[k].done = 0;
        	th[k].valid = 0;
        	th[k].it = it;
			th[k].resume = resume;
        	th[k].alignfile = alignfile + ".part";
        	node[0] = '0' + k;
        	th[k].alignfile += node;
        	th[k].dump_files = dump_files;
            th[k].valid = pthread_create(&(th[k].thread),NULL,hmm_exe_emloop,&(th[k]));
            if(th[k].valid){
                cerr << "Error starting thread " << k << endl;
            }
        }
        node[0] = '0';
        alignfile += ".part";
        alignfile += node;
        em_loop(perp, sHandler1,  dump_files , alignfile.c_str(), trainViterbiPerp, false,it==1 && (!resume),it);
        for (k=1;k<NCPUS;k++){
            pthread_join((th[k].thread),NULL);
            cerr << "Thread " << k << "done" << endl;
        }
        perp.record("HMM");
        trainViterbiPerp.record("HMM");
        errorReportAL(cout,"HMM");
        
        sHandler1.rewind();
        if( errorsAL()<minErrors ){
            minErrors=errorsAL();
            minIter=it;
        }
        if (testPerp && testHandler){
        	testHandler->rewind();
            em_loop(*testPerp, *testHandler, dump_files, test_alignfile.c_str(), *testViterbiPerp,  true,it==1 && (!resume),it);
            testHandler->rewind();
        }
        if (dump_files&&OutputInAachenFormat==1)
            tTable.printCountTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),1);

		if(dumpCount && it == noIterations){
			string realTableName = dumpCountName;
			realTableName += ".t.count";
			tTable.printCountTable(realTableName.c_str(),Elist.getVocabList(),Flist.getVocabList(),useString);
			string realATableName = dumpCountName;
			realATableName += ".a.count";
			aCountTable.printRealTable(realATableName.c_str());
			string realHTableName = dumpCountName;
			realHTableName += ".h.count";
			string fnamealpha = realHTableName;
			string fnamebeta = realHTableName;
			fnamealpha += ".alpha";
			fnamebeta += ".beta";
			counts.writeJumps(realHTableName.c_str(),NULL,fnamealpha.c_str(),fnamebeta.c_str());      

		}
        tTable.normalizeTable(Elist, Flist);
        aCountTable.normalize(aTable);
        probs=counts;
        cout << modelName << ": ("<<it<<") TRAIN CROSS-ENTROPY " << perp.cross_entropy()
            << " PERPLEXITY " << perp.perplexity() << '\n';
        if (testPerp && testHandler)
            cout << modelName << ": ("<<it<<") TEST CROSS-ENTROPY " << (*testPerp).cross_entropy()
            << " PERPLEXITY " << (*testPerp).perplexity() 
            << '\n';
        cout << modelName << ": ("<<it<<") VITERBI TRAIN CROSS-ENTROPY " << trainViterbiPerp.cross_entropy()
            << " PERPLEXITY " << trainViterbiPerp.perplexity() << '\n';
        if (testPerp && testHandler)
            cout << modelName << ": ("<<it<<") VITERBI TEST CROSS-ENTROPY " << testViterbiPerp->cross_entropy()
            << " PERPLEXITY " << testViterbiPerp->perplexity() 
            << '\n';
        if (dump_files){
            if( OutputInAachenFormat==0)
                tTable.printProbTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),OutputInAachenFormat);
           // ofstream afilestream(afileh.c_str());    
			string fnamealpha = afileh;
			string fnamebeta = afileh;
			fnamealpha += ".alpha";
			fnamebeta += ".beta";
            probs.writeJumps(afileh.c_str(),NULL,fnamealpha.c_str(),fnamebeta.c_str());
//            aCountTable.printTable(acfile.c_str());
            aTable.printTable(afile.c_str());
        }
        it_fn = time(NULL) ;
        cout << "\n" << modelName << " Iteration: " << it<< " took: " <<
            difftime(it_fn, it_st) << " seconds\n";
    } // end of iterations 
    fn = time(NULL) ;
    cout << endl << "Entire " << modelName << " Training took: " << difftime(fn, st) << " seconds\n";
    //cout << "tTable contains " << tTable.getHash().bucket_count() 
    //     << " buckets and  " << tTable.getHash().size() << " entries." ;
    cout << "==========================================================\n";
    return minIter;
}

/*template<class T>
T normalize_if_possible_with_increment(T*a,T*b,int increment)
{
  T sum=0;
  for(T*i=a;i!=b;i+=increment)
    sum+=*i;
  if( sum )
    for(T*i=a;i!=b;i+=increment)
      *i/=sum;
  else
    {
      T factor=increment/(b-a);
      for(T*i=a;i!=b;i+=increment)
	*i=factor;
    }
  return sum;
}*/

void hmm::load_table(const char* aname){
    cout << "Hmm: loading a table not implemented.\n";
    abort();
    ifstream anamefile(aname);
    probs.readJumps(anamefile);
}

HMMNetwork *hmm::makeHMMNetwork(const Vector<WordIndex>& es,const Vector<WordIndex>&fs,bool doInit)const
{
    unsigned int i,j;
    unsigned int l = es.size() - 1;
    unsigned int m = fs.size() - 1;
    unsigned int I=2*l,J=m;
    int IJ=I*J;
    bool DependencyOfJ=(CompareAlDeps&(16|8))||(PredictionInAlignments==2);
    bool DependencyOfPrevAJ=(CompareAlDeps&(2|4))||(PredictionInAlignments==0);
    HMMNetwork *net = new HMMNetwork(I,J);
    fill(net->alphainit.begin(),net->alphainit.end(),0.0);
    fill(net->betainit.begin(),net->betainit.end(),0.0);
    for(j=1;j<=m;j++){
        for(i=1;i<=l;i++){
           // cout << es[i] <<" " << fs[j] <<" " << tTable.getProb(es[i], fs[j]) << endl;
            net->n(i-1,j-1)=tTable.getProb(es[i], fs[j]) ;
        }
        double emptyContribution=0;  
        emptyContribution=tTable.getProb(es[0],fs[j]) ;
        for(i=1;i<=l;i++)
            net->n(i+l-1,j-1)=emptyContribution;
        net->finalMultiply*=max(normalize_if_possible_with_increment(&net->n(0,j-1),&net->n(0,j-1)+IJ,J),double(1e-12));
    }
    if( DependencyOfJ )
        net->e.resize(m-1);
    else
        net->e.resize(J>1);
    for(j=0;j<net->e.size();j++){
        int frenchClass=fwordclasses.getClass(fs[1+min(int(m)-1,int(j)+1)]);
        net->e[j].resize(I,I,0);
        for(unsigned int i1=0;i1<I;++i1) {
            Array<double> al(l);
            CLASSIFY2(i1,i1real);
            for(unsigned int i2=0;i2<l;i2++)
                al[i2]=probs.getAlProb(i1real,i2,l,m,ewordclasses.getClass(es[1+i1real]),frenchClass
                                       ,j+1);
            normalize_if_possible(conv<double>(al.begin()),conv<double>(al.end()));
            if( SmoothHMM&2 )
                smooth_standard(conv<double>(al.begin()),conv<double>(al.end()),HMMAlignmentModelSmoothFactor);
            for(unsigned int i2=0;i2<I;i2++) {
                CLASSIFY(i2,empty_i2,i2real);
                net->e[j](i1,i2)	    = al[i2real];
                
                if( empty_i2 )
                    if(i1real!=i2real) {
                        net->e[j](i1,i2)=0;
                    } else{   
                    net->e[j](i1,i2)=doInit?al[0]:(probs.getProbabilityForEmpty()); // make first HMM iteration like IBM-1
                    }
            }
            normalize_if_possible(&net->e[j](i1,0),&net->e[j](i1,0)+I);
        }
    }
    if( doInit ){
        for(unsigned int i=0;i<I;++i)
        {
            net->alphainit[i]=net->betainit[i]=(i<I/2)?1:(2.0/I);
            net->betainit[i]=1.0;
        }
    }else{
        if( DependencyOfPrevAJ==0 ){
            for(i=0;i<I;i++){
                CLASSIFY2(i,ireal);
                net->alphainit[i]=probs.getAlProb(-1,ireal,l,m,0,fwordclasses.getClass(fs[1+0]),0);
            }
        }else{
            if( UniformEntryExit&2 )probs.getBetaInit(I,net->betainit);
            if( UniformEntryExit&1 )probs.getAlphaInit(I,net->alphainit);
        }
    }
    massert( net->alphainit.size()==I );massert( net->betainit.size()==I );
    normalize_if_possible(conv<double>(net->alphainit.begin()),conv<double>(net->alphainit.end()));
    normalize_if_possible(conv<double>(net->betainit.begin()),conv<double>(net->betainit.end()));
    transform(net->betainit.begin(),net->betainit.end(),net->betainit.begin(),bind1st(multiplies<double>(),2*l));
    return net;
}
extern float MINCOUNTINCREASE;

void hmm::em_loop(Perplexity& perp, sentenceHandler& sHandler1, 
		  bool dump_alignment, const char* alignfile, Perplexity& viterbi_perp, 
		     bool test,bool doInit,int 
){
    WordIndex i, j, l, m ;
    double cross_entropy;
    int pair_no=0 ;
    perp.clear();
    viterbi_perp.clear();
    ofstream of2;
    // for each sentence pair in the corpus
    if (dump_alignment||FEWDUMPS )
        of2.open(alignfile);
    sentPair sent ;
    
    while(sHandler1.getNextSentence(sent)){
        const Vector<WordIndex>& es = sent.get_eSent();// #
        const Vector<WordIndex>& fs = sent.get_fSent();
        const float so  = sent.getCount();
        l = es.size() - 1;
        m = fs.size() - 1;
        cross_entropy = log(1.0);
        Vector<WordIndex> viterbi_alignment(fs.size());// #
        
        unsigned int I=2*l,J=m;
        bool DependencyOfJ=(CompareAlDeps&(16|8))||(PredictionInAlignments==2);
        bool DependencyOfPrevAJ=(CompareAlDeps&(2|4))||(PredictionInAlignments==0);
        HMMNetwork *net=makeHMMNetwork(es,fs,doInit);
        Array<double> gamma;
        Array<Array2<double> > epsilon(DependencyOfJ?(m-1):1);
        double trainProb;
        trainProb=ForwardBackwardTraining(*net,gamma,epsilon);
        if( !test ){
            double *gp=conv<double>(gamma.begin());
            for(unsigned int i2=0;i2<J;i2++)for(unsigned int i1=0;i1<I;++i1,++gp){
                if( *gp>MINCOUNTINCREASE ) {
                    COUNT add= *gp*so;
                    if( i1>=l ){
                        tTable.incCount(es[0],fs[1+i2],add);
                        aCountTable.addValue(0,i2+1,l,m,add);
                        //aCountTable.getRef(0,i2+1,l,m)+=add;
                    } else	{
                        tTable.incCount(es[1+i1],fs[1+i2],add);
                        aCountTable.addValue(1+i1,1+i2,l,m,add);
                        //aCountTable.getRef(1+i1,1+i2,l,m)+=add;
                    }
                }
            }
            double p0c=0.0,np0c=0.0;
            for(unsigned int jj=0;jj<epsilon.size();jj++){
                int frenchClass=fwordclasses.getClass(fs[1+min(int(m)-1,int(jj)+1)]);
                double *ep=epsilon[jj].begin();
                if( ep ){
                    //for(i=0;i<I;i++)
                    //  normalize_if_possible_with_increment(ep+i,ep+i+I*I,I);
                    //		for(i=0;i<I*I;++i)
                    //  ep[i] *= I;
                    //if( DependencyOfJ )
                    //  if( J-1 )
                    //    for(i=0;i<I*I;++i)
                    //      ep[i] /= (J-1);
                    double mult=1.0;
                    mult*=l;
                    //if( DependencyOfJ && J-1)
                    //  mult/=(J-1);
                    for(i=0;i<I;i++){
                        for(unsigned int i_bef=0;i_bef<I;i_bef++,ep++){
                            CLASSIFY(i,i_empty,ireal);
                            CLASSIFY2(i_bef,i_befreal);
                            if( i_empty )
                                p0c+=*ep * mult;
                            else{
                                counts.addAlCount(i_befreal,ireal,l,m,ewordclasses.getClass(es[1+i_befreal]),
                                                  frenchClass ,jj+1,*ep * mult,0.0);
                                np0c+=*ep * mult; 
                            }
                            massert( &epsilon[jj](i,i_bef)== ep);
                        }
                    }
                }
            }
            double *gp1=conv<double>(gamma.begin()),*gp2=conv<double>(gamma.end())-I;
            pair<Array<double>,Mutex >&ai0=counts.doGetAlphaInit(I);
            Array<double>&ai = ai0.first;
            pair<Array<double>,Mutex >&bi0=counts.doGetBetaInit(I);
            Array<double>&bi = bi0.first;
            int firstFrenchClass=(fs.size()>1)?(fwordclasses.getClass(fs[1+0])):0;
            ai0.second.lock();
            for(i=0;i<I;i++,gp1++){
                CLASSIFY(i,i_empty,ireal);
                ai[i]+= *gp1;
                //bi[i]+= *gp2;
                if( DependencyOfPrevAJ==0 ){
                    if( i_empty )
                        p0c+=*gp1;
                    else{
                        counts.addAlCount(-1,ireal,l,m,0,firstFrenchClass,0,*gp1,0.0);
                        np0c+=*gp1;
                    }
                }
            }
            ai0.second.unlock();
            bi0.second.lock();
            for(i=0;i<I;i++,gp2++){
                CLASSIFY(i,i_empty,ireal);
                bi[i]+= *gp2;
            }
            bi0.second.unlock();
            if( Verbose )
                cout << "l: " << l << "m: " << m << " p0c: " << p0c << " np0c: " << np0c << endl;
        }
        cross_entropy+=log(max(trainProb,1e-100))+log(max(net->finalMultiply,1e-100));
        Array<int>vit;
        double viterbi_score=1.0;
        if( (HMMTrainingSpecialFlags&1) )
            HMMViterbi(*net,gamma,vit);
        else
            viterbi_score=HMMRealViterbi(*net,vit);
        for(j=1;j<=m;j++){
            viterbi_alignment[j]=vit[j-1]+1;
            if( viterbi_alignment[j]>l)
                viterbi_alignment[j]=0;
        }
        sHandler1.setProbOfSentence(sent,cross_entropy);
        perp.addFactor(cross_entropy, so, l, m,1);
        viterbi_perp.addFactor(log(viterbi_score)+log(max(net->finalMultiply,1e-100)), so, l, m,1);
        if( Verbose )
            cout << "Viterbi-perp: " << log(viterbi_score) << ' ' << log(max(net->finalMultiply,1e-100)) << ' ' << viterbi_score << ' ' << net->finalMultiply << ' ' << *net << "gamma: " << gamma << endl;
        delete net;net=0;
        if (dump_alignment||(FEWDUMPS&&sent.getSentenceNo()<1000) )
            printAlignToFile(es, fs, Elist.getVocabList(), Flist.getVocabList(), of2, viterbi_alignment, sent.getSentenceNo(), viterbi_score);
        addAL(viterbi_alignment,sent.getSentenceNo(),l);    
        pair_no++;
    } /* of while */
    
}

void hmm::clearCountTable(){counts=HMMTables<int,WordClasses>(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses);}

#if 0
CTTableDiff<COUNT,PROB>* hmm::em_loop_1(Perplexity& perp, sentenceHandler& sHandler1, 
		  bool dump_alignment, const char* alignfile, Perplexity& viterbi_perp, 
		     bool test,bool doInit,int 
){
    CTTableDiff<COUNT,PROB> *diff = new CTTableDiff<COUNT,PROB>();
    //diff->incCount(1,1,0);
    WordIndex i, j, l, m ;
    double cross_entropy;
    int pair_no=0 ;
    perp.clear();
    viterbi_perp.clear();
    ofstream of2;
    // for each sentence pair in the corpus
    if (dump_alignment||FEWDUMPS )
        of2.open(alignfile);
    sentPair sent ;
    sHandler1.rewind();
    int nnn = 0;
    while(sHandler1.getNextSentence(sent)){
	    nnn ++;
	    cout << nnn << endl;
	cout << 1 << endl;
        const Vector<WordIndex>& es = sent.get_eSent();
        const Vector<WordIndex>& fs = sent.get_fSent();
        const float so  = sent.getCount();
        l = es.size() - 1;
        m = fs.size() - 1;
        cross_entropy = log(1.0);
        Vector<WordIndex> viterbi_alignment(fs.size());
        
        unsigned int I=2*l,J=m;
        bool DependencyOfJ=(CompareAlDeps&(16|8))||(PredictionInAlignments==2);
        bool DependencyOfPrevAJ=(CompareAlDeps&(2|4))||(PredictionInAlignments==0);
	cout << 2 << endl;
        HMMNetwork *net=makeHMMNetwork(es,fs,doInit);
        Array<double> gamma;
        Array<Array2<double> > epsilon(DependencyOfJ?(m-1):1);
        double trainProb;
	cout << 2.5 << endl;
        trainProb=ForwardBackwardTraining(*net,gamma,epsilon);
	cout << 3 << endl;
        if( !test ){
            double *gp=conv<double>(gamma.begin());
	    cout << 4 << endl;
            for(unsigned int i2=0;i2<J;i2++)for(unsigned int i1=0;i1<I;++i1,++gp){
                if( *gp>MINCOUNTINCREASE ) {
                    COUNT add= *gp*so;
                    if( i1>=l ){
                        diff->incCount(es[0],fs[1+i2],add);
                        //tTable.incCount(es[0],fs[1+i2],add);
                        aCountTable.getRef(0,i2+1,l,m)+=add;
                    } else	{
                        diff->incCount(es[1+i1],fs[1+i2],add);
                        //tTable.incCount(es[1+i1],fs[1+i2],add);
                        aCountTable.getRef(1+i1,1+i2,l,m)+=add;
                    }
                }
            }
	    cout << 5 << endl;
            double p0c=0.0,np0c=0.0;
            for(unsigned int jj=0;jj<epsilon.size();jj++){
		   if (nnn==7779) cout << 1 << endl;
                int frenchClass=fwordclasses.getClass(fs[1+min(int(m)-1,int(jj)+1)]);
		   if (nnn==7779) cout << 2 << endl;
                double *ep=epsilon[jj].begin();
		if (nnn==7779) cout << 3 << endl;
                if( ep ){
                    //for(i=0;i<I;i++)
                    //  normalize_if_possible_with_increment(ep+i,ep+i+I*I,I);
                    //		for(i=0;i<I*I;++i)
                    //  ep[i] *= I;
                    //if( DependencyOfJ )
                    //  if( J-1 )
                    //    for(i=0;i<I*I;++i)
                    //      ep[i] /= (J-1);
                    double mult=1.0;
                    mult*=l;
                    //if( DependencyOfJ && J-1)
                    //  mult/=(J-1);
		    if (nnn==7779) cout << 4 << ":" << I  << endl;
                    for(i=0;i<I;i++){
			    if (nnn==7779) cout << "i:" << i << endl;
                        for(unsigned int i_bef=0;i_bef<I;i_bef++,ep++){
				if (nnn==7779) cout << "   CL 1"   << endl;
                            CLASSIFY(i,i_empty,ireal);
		if (nnn==7779) cout << "   CL 2 : "   << i_bef << " " <<  (size_t)ep << endl;
                            CLASSIFY2(i_bef,i_befreal);
			    if((i+1)*(i_bef+1)>epsilon[jj].getLen1()*epsilon[jj].getLen2()){
				    continue;
			    }
                            if( i_empty )
                                p0c+=epsilon[jj](i,i_bef)*mult;// p0c+=*ep * mult;
                            else{
				if (nnn==7779) cout << "ELSE" << endl;
                                if (nnn==7779){
					cout << i_befreal<<" " <<ireal<<" " << l<<" " << m<<" "<< jj<<" "<<epsilon.size()<< " " << epsilon[jj].getLen1() <<" " << epsilon[jj].getLen2()<< endl;
					np0c+=epsilon[jj](i,i_bef)*mult;
					cout <<"..."<<endl;
					cout <<"......"<<ewordclasses.getClass(es[1+i_befreal]) << endl;
					cout <<"......"<<endl;
                                      counts.addAlCount(i_befreal,ireal,l,m,ewordclasses.getClass(es[1+i_befreal]),
						      frenchClass ,jj+1,0,0.0);
				      np0c+=epsilon[jj](i,i_bef)*mult;
				}
				else{
					counts.addAlCount(i_befreal,ireal,l,m,ewordclasses.getClass(es[1+i_befreal]),
                                                  frenchClass ,jj+1,epsilon[jj](i,i_bef)*mult,0.0);
					np0c+=epsilon[jj](i,i_bef)*mult; 
				}
                            }
			    if (nnn==7779) cout << "FI" << endl;
                            massert( &epsilon[jj](i,i_bef)== ep);
                        }
                    }
		    if (nnn==7779) cout << 5 << endl;
                }
            }
	//    cout << 6 << endl;
            double *gp1=conv<double>(gamma.begin()),*gp2=conv<double>(gamma.end())-I;
            Array<double>&ai=counts.doGetAlphaInit(I);/*If it is not get yet, init it, all operation envolved is add*/
            Array<double>&bi=counts.doGetBetaInit(I);
            int firstFrenchClass=(fs.size()>1)?(fwordclasses.getClass(fs[1+0])):0;
            for(i=0;i<I;i++,gp1++,gp2++){
                CLASSIFY(i,i_empty,ireal);
                ai[i]+= *gp1;
                bi[i]+= *gp2;
                if( DependencyOfPrevAJ==0 ){
                    if( i_empty )
                        p0c+=*gp1;
                    else{
                        counts.addAlCount(-1,ireal,l,m,0,firstFrenchClass,0,*gp1,0.0);
                        np0c+=*gp1;
                    }
                }
            }
	  //  cout << 7 << endl;
            if( Verbose )
                cout << "l: " << l << "m: " << m << " p0c: " << p0c << " np0c: " << np0c << endl;
        }
	//cout << 8 << endl;
        cross_entropy+=log(max(trainProb,1e-100))+log(max(net->finalMultiply,1e-100));
        Array<int>vit;
        double viterbi_score=1.0;
	//cout << 9 << endl;
        if( (HMMTrainingSpecialFlags&1) )
            HMMViterbi(*net,gamma,vit);
        else
            viterbi_score=HMMRealViterbi(*net,vit);
	//cout << 10 << endl;
        for(j=1;j<=m;j++){
            viterbi_alignment[j]=vit[j-1]+1;
            if( viterbi_alignment[j]>l)
                viterbi_alignment[j]=0;
        }
	//cout << 11 << endl;
        sHandler1.setProbOfSentence(sent,cross_entropy);
	//cout << 12 << endl;
        perp.addFactor(cross_entropy, so, l, m,1);
        viterbi_perp.addFactor(log(viterbi_score)+log(max(net->finalMultiply,1e-100)), so, l, m,1);
        if( Verbose )
            cout << "Viterbi-perp: " << log(viterbi_score) << ' ' << log(max(net->finalMultiply,1e-100)) << ' ' << viterbi_score << ' ' << net->finalMultiply << ' ' << *net << "gamma: " << gamma << endl;
        delete net;net=0;
	//cout << 13 << endl;
        if (dump_alignment||(FEWDUMPS&&sent.getSentenceNo()<1000) )
            printAlignToFile(es, fs, Elist.getVocabList(), Flist.getVocabList(), of2, viterbi_alignment, sent.getSentenceNo(), viterbi_score);
	//cout << 14 << endl;
        addAL(viterbi_alignment,sent.getSentenceNo(),l);    
        pair_no++;
    } /* of while */
    sHandler1.rewind();
    perp.record("HMM");
    viterbi_perp.record("HMM");
    errorReportAL(cout,"HMM");
    return diff;
}

#endif
Mutex mu;

#if 0
void hmm::em_loop_2(Perplexity& perp, sentenceHandler& sHandler1, 
		  bool dump_alignment, const char* alignfile, Perplexity& viterbi_perp, 
		     bool test,bool doInit,int part
){
    WordIndex i, j, l, m ;
    double cross_entropy;
    int pair_no=0 ;
    perp.clear();
    viterbi_perp.clear();
    ofstream of2;
    // for each sentence pair in the corpus
    if (dump_alignment||FEWDUMPS )
        of2.open(alignfile);
    sentPair sent ;
    //sHandler1.rewind();
    int nnn = 0;
    while(sHandler1.getNextSentence(sent)){
	    //nnn ++;
	    //cout << nnn << endl;
        //cout << 1 << endl;
        const Vector<WordIndex>& es = sent.get_eSent();
        const Vector<WordIndex>& fs = sent.get_fSent();
        const float so  = sent.getCount();
        l = es.size() - 1;
        m = fs.size() - 1;
        cross_entropy = log(1.0);
        Vector<WordIndex> viterbi_alignment(fs.size());
        
        unsigned int I=2*l,J=m;
        bool DependencyOfJ=(CompareAlDeps&(16|8))||(PredictionInAlignments==2);
        bool DependencyOfPrevAJ=(CompareAlDeps&(2|4))||(PredictionInAlignments==0);

        HMMNetwork *net=makeHMMNetwork(es,fs,doInit);
        Array<double> gamma;
        Array<Array2<double> > epsilon(DependencyOfJ?(m-1):1);
        double trainProb;
        trainProb=ForwardBackwardTraining(*net,gamma,epsilon);
        if( !test ){
            double *gp=conv<double>(gamma.begin());
            for(unsigned int i2=0;i2<J;i2++)for(unsigned int i1=0;i1<I;++i1,++gp){
                if( *gp>MINCOUNTINCREASE ) {
                    COUNT add= *gp*so;
                    if( i1>=l ){
                        //diff->incCount(es[0],fs[1+i2],add);
                        tTable.incCount(es[0],fs[1+i2],add);
                        aCountTable.getRef(0,i2+1,l,m)+=add;
                    } else	{
                        //diff->incCount(es[1+i1],fs[1+i2],add);
                        tTable.incCount(es[1+i1],fs[1+i2],add);
                        aCountTable.getRef(1+i1,1+i2,l,m)+=add;
                    }
                }
            }
            double p0c=0.0,np0c=0.0;
            for(unsigned int jj=0;jj<epsilon.size();jj++){
                int frenchClass=fwordclasses.getClass(fs[1+min(int(m)-1,int(jj)+1)]);
                double *ep=epsilon[jj].begin();
                if( ep ){
                    double mult=1.0;
                    mult*=l;
                    //if( DependencyOfJ && J-1)
                    //  mult/=(J-1);
                    for(i=0;i<I;i++){
		                for(unsigned int i_bef=0;i_bef<I;i_bef++,ep++){
		                    CLASSIFY(i,i_empty,ireal);
                            CLASSIFY2(i_bef,i_befreal);
                            if( i_empty ){
                                p0c+=*ep * mult;
							}else{
								//mu.lock(); 
								//cout<<"\rP "<<part<<" ";
								//cout<<epsilon.size()<<" "<<jj<<" ";
								//cout<<epsilon[jj].h1<<" " << epsilon[jj].h2<<" ";
								//cout<<i<<" "<<i_bef<<" ";
								//cout<<I<<" "<<J<<"             ";
								
								cout.flush();
								counts.addAlCount(i_befreal,ireal,l,m,ewordclasses.getClass(es[1+i_befreal]),
												  frenchClass ,jj+1,*ep * mult,0.0);
								np0c+=*ep * mult; 
								//mu.unlock();
							}
                            massert( &epsilon[jj](i,i_bef)== ep);
                        }
                    }
                }
            }
            double *gp1=conv<double>(gamma.begin()),*gp2=conv<double>(gamma.end())-I;
            Array<double>&ai=counts.doGetAlphaInit(I);/*If it is not get yet, init it, all operation envolved is add*/
            Array<double>&bi=counts.doGetBetaInit(I);
            int firstFrenchClass=(fs.size()>1)?(fwordclasses.getClass(fs[1+0])):0;
            for(i=0;i<I;i++,gp1++,gp2++){
                CLASSIFY(i,i_empty,ireal);
                ai[i]+= *gp1;
                bi[i]+= *gp2;
                if( DependencyOfPrevAJ==0 ){
                    if( i_empty )
                        p0c+=*gp1;
                    else{
                        counts.addAlCount(-1,ireal,l,m,0,firstFrenchClass,0,*gp1,0.0);
                        np0c+=*gp1;
                    }
                }
            }
	  //  cout << 7 << endl;
            if( Verbose )
                cout << "l: " << l << "m: " << m << " p0c: " << p0c << " np0c: " << np0c << endl;
        }
	//cout << 8 << endl;
        cross_entropy+=log(max(trainProb,1e-100))+log(max(net->finalMultiply,1e-100));
        Array<int>vit;
        double viterbi_score=1.0;
	//cout << 9 << endl;
        if( (HMMTrainingSpecialFlags&1) )
            HMMViterbi(*net,gamma,vit);
        else
            viterbi_score=HMMRealViterbi(*net,vit);
	//cout << 10 << endl;
        for(j=1;j<=m;j++){
            viterbi_alignment[j]=vit[j-1]+1;
            if( viterbi_alignment[j]>l)
                viterbi_alignment[j]=0;
        }
	//cout << 11 << endl;
        sHandler1.setProbOfSentence(sent,cross_entropy);
	//cout << 12 << endl;
        perp.addFactor(cross_entropy, so, l, m,1);
        viterbi_perp.addFactor(log(viterbi_score)+log(max(net->finalMultiply,1e-100)), so, l, m,1);
        if( Verbose )
            cout << "Viterbi-perp: " << log(viterbi_score) << ' ' << log(max(net->finalMultiply,1e-100)) << ' ' << viterbi_score << ' ' << net->finalMultiply << ' ' << *net << "gamma: " << gamma << endl;
        delete net;net=0;
	//cout << 13 << endl;
        if (dump_alignment||(FEWDUMPS&&sent.getSentenceNo()<1000) )
            printAlignToFile(es, fs, Elist.getVocabList(), Flist.getVocabList(), of2, viterbi_alignment, sent.getSentenceNo(), viterbi_score);
	//cout << 14 << endl;
        addAL(viterbi_alignment,sent.getSentenceNo(),l);    
        pair_no++;
    } /* of while */
    

    return ;
}


CTTableDiff<COUNT,PROB>* hmm::em_one_step(int it){
    double minErrors=1.0;int minIter=0;
    string modelName="Hmm",shortModelName="hmm";
    int dumpFreq=ModelH_Dump_Freq;
    time_t it_st, st, it_fn, fn;
    string tfile, afile,afileh, number, alignfile, test_alignfile;
    int pair_no = 0;
    bool dump_files = false ;
    ofstream of2 ;
    st = time(NULL) ;
    sHandler1.rewind();
    cout << "\n==========================================================\n";
    cout << modelName << " Training Started at: " << ctime(&st);
    pair_no = 0;
    
    cout << endl << "-----------\n" << modelName << ": Iteration " << it << '\n';
    dump_files = true ;//(dumpFreq != 0) && ((it % dumpFreq) == 0) && !NODUMPS;
    number = "";
    int n = it;
    do{
        number.insert((size_t)0, 1, (char)(n % 10 + '0'));
    } while((n /= 10) > 0);
    tfile = Prefix + ".t" + shortModelName + "." + number ;
    afile = Prefix + ".a" + shortModelName + "." + number ;
    afileh = Prefix + ".h" + shortModelName + "." + number ;
    alignfile = Prefix + ".AH" ;
    test_alignfile = Prefix + ".tst.A" + shortModelName + "." + number ;
    counts=HMMTables<int,WordClasses>(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses);
    aCountTable.clear();
    initAL();
    CTTableDiff<COUNT,PROB>* diff =em_loop_1(perp, sHandler1,  dump_files , alignfile.c_str(), trainViterbiPerp, false,it==1,it);
    
    if( errorsAL()<minErrors ){
        minErrors=errorsAL();
        minIter=it;
    }
    //        if (testPerp && testHandler)
//            em_loop(*testPerp, *testHandler, dump_files, test_alignfile.c_str(), *testViterbiPerp,  true,it==1,it); 
//        if (dump_files&&OutputInAachenFormat==1)
//            tTable.printCountTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),1);
//        tTable.normalizeTable(Elist, Flist);
//        aCountTable.normalize(aTable);
//        probs=counts;
//        cout << modelName << ": ("<<it<<") TRAIN CROSS-ENTROPY " << perp.cross_entropy()
//            << " PERPLEXITY " << perp.perplexity() << '\n';
//        if (testPerp && testHandler)
//            cout << modelName << ": ("<<it<<") TEST CROSS-ENTROPY " << (*testPerp).cross_entropy()
//            << " PERPLEXITY " << (*testPerp).perplexity() 
//            << '\n';
//        cout << modelName << ": ("<<it<<") VITERBI TRAIN CROSS-ENTROPY " << trainViterbiPerp.cross_entropy()
//            << " PERPLEXITY " << trainViterbiPerp.perplexity() << '\n';
//        if (testPerp && testHandler)
//            cout << modelName << ": ("<<it<<") VITERBI TEST CROSS-ENTROPY " << testViterbiPerp->cross_entropy()
//            << " PERPLEXITY " << testViterbiPerp->perplexity() 
//            << '\n';
//        if (dump_files){
//            if( OutputInAachenFormat==0)
///                tTable.printProbTable(tfile.c_str(),Elist.getVocabList(),Flist.getVocabList(),OutputInAachenFormat);
 //           ofstream afilestream(afileh.c_str());      
 //           probs.writeJumps(afilestream);
 //           aCountTable.printTable(afile.c_str());
        
    fn = time(NULL) ;
    cout << endl << "Entire " << modelName << " Training took: " << difftime(fn, st) << " seconds\n";
    //cout << "tTable contains " << tTable.getHash().bucket_count() 
    //     << " buckets and  " << tTable.getHash().size() << " entries." ;
    cout << "==========================================================\n";
    return diff;
}


void hmm::em_one_step_2(int it,int part){
    double minErrors=1.0;int minIter=0;
    string modelName="Hmm",shortModelName="hmm";
    int dumpFreq=ModelH_Dump_Freq;
    time_t it_st, st, it_fn, fn;
    string tfile, afile,afileh, number, alignfile, test_alignfile;
    int pair_no = 0;
    bool dump_files = false ;
    ofstream of2 ;
    
    pair_no = 0;


    dump_files = true ;//(dumpFreq != 0) && ((it % dumpFreq) == 0) && !NODUMPS;
    number = "";
    int n = it;
    do{
        number.insert((size_t)0, 1, (char)(n % 10 + '0'));
    } while((n /= 10) > 0);
    tfile = Prefix + ".t" + shortModelName + "." + number ;
    afile = Prefix + ".a" + shortModelName + "." + number ;
    afileh = Prefix + ".h" + shortModelName + "." + number ;
    alignfile = Prefix + ".Ahmm." ;
    char v[2];
    v[1] = 0;
    v[0] = '0' + it;
    alignfile += v;
    alignfile += ".part";
    v[0] = '0' + part;
    alignfile += v;
    
    counts=HMMTables<int,WordClasses>(GLOBALProbabilityForEmpty,ewordclasses,fwordclasses);
    aCountTable.clear();
    initAL();
    em_loop_2(perp, sHandler1,  dump_files , alignfile.c_str(), trainViterbiPerp, false,it==1,part);
    
    if( errorsAL()<minErrors ){
        minErrors=errorsAL();
        minIter=it;
    }
    return ;
}

struct hmm_align_struct{
    hmm *h;
    int part;
    int iter;
    int valid;
    pthread_t thread;
    int done;
};

void* em_thread(void *arg){
    hmm_align_struct * hm = (hmm_align_struct*) arg;
    hm->h->em_one_step_2(hm->iter,hm->part);
    hm->done = 1;
    return hm;
}


int multi_thread_em(int noIter, int noThread, hmm* base){
    // First, do one-step EM
    int i;
    int j;
    time_t it_st, st, it_fn, fn;
    fn = time(NULL);
    int dumpFreq=ModelH_Dump_Freq;
    bool dump_files = false ;
    string modelName = "HMM",shortModelName="hmm";
    string tfile, afile,acfile,afileh, number, alignfile, test_alignfile;
    vector<amodel<COUNT> > counts;
    vector<model2 *> m2;
    counts.resize(noThread);
    m2.resize(noThread);
    for(j=1;j<noThread;j++){
        m2[j] = new model2(*((model1*)base),base->aTable,counts[j]);
    }
    st  = time(NULL);
    cout << "\n==========================================================\n";
    cout << modelName << " Training Started at: " << ctime(&st);

    for(i=1;i<=noIter;i++){
        base->perp.clear();
        base->trainViterbiPerp.clear();
        if (base->testPerp && base->testHandler){
            base->testHandler->rewind();
            base->testPerp->clear();
            base->testViterbiPerp->clear();
        }
        
        it_st = time(NULL) ;
        
        cout << endl << "-----------\n" << modelName << ": Iteration " << i << '\n';
        dump_files = (dumpFreq != 0) && ((i % dumpFreq) == 0) && !NODUMPS;
        dump_files = true;
        string number = "";
        int n = i;
        do{
            number.insert((size_t)0, 1, (char)(n % 10 + '0'));
        } while((n /= 10) > 0);
        tfile = Prefix + ".t" + shortModelName + "." + number ;
        afile = Prefix + ".a" + shortModelName + "." + number ;
        acfile = Prefix + ".ac" + shortModelName + "." + number ;
        afileh = Prefix + ".h" + shortModelName + "." + number ;
        
        alignfile = Prefix + ".A" + shortModelName + "." + number ;
        test_alignfile = Prefix + ".tst.A" + shortModelName + "." + number ;
        base->initAL();
        // except the current thread
        vector<hmm_align_struct> args;
        base->sHandler1.rewind();
        args.resize(noThread);
        for(j=1;j<noThread;j++){
            args[j].iter = i;
            args[j].part = j;
            args[j].done = 0;
            counts[j].clear();
            args[j].h = new hmm(*m2[j],base->ewordclasses,base->fwordclasses);
            args[j].h->probs = base->probs;
            args[j].valid = pthread_create(&(args[j].thread),NULL,em_thread,&(args[j]));
            if(args[j].valid){
                cerr << "Error starting thread " << j << endl;
            }
        }
        base->em_one_step_2(i,0);
        //ofstream afilestream(afileh.c_str());                
        while(1){
            bool done = true;
            for (j=1;j<noThread;j++){
                //pthread_join((args[j].thread),NULL);
                // Start normalization as soon as possible
                if(args[j].done==1){
                    args[j].done = 2;
                    base->aCountTable.merge(args[j].h->aCountTable);                
                    //afilestream << "BEFORE MERGE"<<endl;
                    //base->counts.writeJumps(afilestream);
                    //afilestream << "MERGING"<<endl;
                    //args[j].h->counts.writeJumps(afilestream);
                    //afilestream << "MERGED"<<endl; 
                    base->counts.merge(args[j].h->counts);
                    //base->counts.writeJumps(afilestream);
                    delete args[j].h;
                    args[j].h = 0;
                }else if(args[j].done==2){
                    // Nothing
                }else if(args[j].done==0){
                    done = false;
                }
            }
            if(done) break;
        }
        base->perp.record("HMM");
        base->trainViterbiPerp.record("HMM");
        base->errorReportAL(cout,"HMM");
        
        // Normalize
//        cout <<" Writing " << afileh  <<"\n";
        base->probs = base->counts;
//        cout <<" Writing " << afileh  <<"\n";
//        ofstream afilestream(afileh.c_str());
//        base->probs.writeJumps(afilestream);
        base->tTable.normalizeTable(base->Elist, base->Flist);
        base->aCountTable.normalize(base->aTable);
        base->aCountTable.clear(); 
        if (base->testPerp && base->testHandler)
            base->em_loop(*base->testPerp, *base->testHandler, dump_files, test_alignfile.c_str(), *base->testViterbiPerp,  true,i==1,i); 
        if (dump_files&&OutputInAachenFormat==1)
            base->tTable.printCountTable(tfile.c_str(),base->Elist.getVocabList(),base->Flist.getVocabList(),1);
        cout << modelName << ": ("<<i<<") TRAIN CROSS-ENTROPY " << base->perp.cross_entropy()
            << " PERPLEXITY " << base->perp.perplexity() << '\n';
        if (base->testPerp && base->testHandler)
            cout << modelName << ": ("<<i<<") TEST CROSS-ENTROPY " << base->testPerp->cross_entropy()
            << " PERPLEXITY " << base->testPerp->perplexity() 
            << '\n';
        cout << modelName << ": ("<<i<<") VITERBI TRAIN CROSS-ENTROPY " << base->trainViterbiPerp.cross_entropy()
            << " PERPLEXITY " << base->trainViterbiPerp.perplexity() << '\n';
        if (base->testPerp && base->testHandler)
            cout << modelName << ": ("<<i<<") VITERBI TEST CROSS-ENTROPY " << base->testViterbiPerp->cross_entropy()
            << " PERPLEXITY " << base->testViterbiPerp->perplexity() 
            << '\n';
        dump_files = true;
        if (dump_files){
            if( OutputInAachenFormat==0)
                base->tTable.printProbTable(tfile.c_str(),base->Elist.getVocabList(),base->Flist.getVocabList(),OutputInAachenFormat);
            ofstream afilestream(afileh.c_str());
            base->counts.writeJumps(afilestream);
            //base->counts.clear();
            base->aCountTable.printTable(acfile.c_str());
            base->aTable.printTable(afile.c_str());
        }
        it_fn = time(NULL) ;
        
        cout << "\n" << modelName << " Iteration: " << i<< " took: " <<
            difftime(it_fn, it_st) << " seconds\n";
    
    }
    for(j=1;j<noThread;j++){
        delete m2[j];
    }
    cout << endl << "Entire " << modelName << " Training took: " << difftime(fn, st) << " seconds\n";
    return 1;
}



#endif
#include "HMMTables.cpp"
template class HMMTables<int,WordClasses>;
 
