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
#ifndef _hmm_h
#define _hmm_h 1

#include <cassert>
 
#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include "Vector.h"
#include <utility>

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif
#include <fstream>
#include <cmath>
#include <ctime>

#include "TTables.h"
#include "ATables.h" 
#include "getSentence.h"
#include "defs.h"
#include "model2.h"
#include "Perplexity.h"
#include "vocab.h"
#include "WordClasses.h"
#include "HMMTables.h"
#include "ForwardBackward.h"
#include "ttableDiff.hpp"

class hmm : public model2{
public:
    WordClasses& ewordclasses;
    WordClasses& fwordclasses;
public:    
    HMMTables<int,WordClasses> counts,probs;
public:
    template<class MAPPER>
    void makeWordClasses(const MAPPER&m1,const MAPPER&m2,string efile,string ffile){
        ifstream estrm(efile.c_str()),fstrm(ffile.c_str());
        if( !estrm ) {
            cerr << "ERROR: can not read " << efile << endl;
        }else
            ewordclasses.read(estrm,m1,Elist);
        if( !fstrm )
            cerr << "ERROR: can not read " << ffile << endl;
        else
            fwordclasses.read(fstrm,m2,Flist);
    }
    hmm(model2&m2,WordClasses &e, WordClasses& f);
    void initialize_table_uniformly(sentenceHandler&);
    int em_with_tricks(int iterations, bool dumpCount = false, 
	    const char* dumpCountName = NULL, bool useString = false,bool resume=false);
    CTTableDiff<COUNT,PROB>* em_one_step(int it);
   // void em_one_step_2(int it,int part);
    void load_table(const char* aname);

   // void em_loop(Perplexity& perp, sentenceHandler& sHandler1, bool dump_files, 
     //            const char* alignfile, Perplexity&, bool test,bool doInit,int iter);
   /* CTTableDiff<COUNT,PROB>* em_loop_1(Perplexity& perp, sentenceHandler& sHandler1, bool dump_files, 
                 const char* alignfile, Perplexity&, bool test,bool doInit,int iter);*/
   /* void em_loop_2(    Perplexity& perp, sentenceHandler& sHandler1, 
		  bool dump_alignment, const char* alignfile, Perplexity& viterbi_perp, 
	     bool test,bool doInit,int part);*/
    void em_loop(Perplexity& perp, sentenceHandler& sHandler1, 
                 bool dump_alignment, const char* alignfile, Perplexity& viterbi_perp, 
                 bool test,bool doInit,int 
                 );
    void em_thread(int it,string alignfile,bool dump_files,bool resume=false);
    HMMNetwork *makeHMMNetwork(const Vector<WordIndex>& es,const Vector<WordIndex>&fs,bool doInit)const;
	void clearCountTable();
    friend class model3;
};
//int multi_thread_em(int noIter, int noThread, hmm* base);


#endif
