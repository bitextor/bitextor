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
#include "TTables.h"
#include "Parameter.h"
#include<iostream>
#include <fstream>

GLOBAL_PARAMETER(float,PROB_CUTOFF,"PROB CUTOFF","Probability cutoff threshold for lexicon probabilities",PARLEV_OPTHEUR,1e-7);
GLOBAL_PARAMETER2(float, COUNTINCREASE_CUTOFF,"COUNTINCREASE CUTOFF","countCutoff","Counts increment cutoff threshold",PARLEV_OPTHEUR,1e-6);


/* ------------------ Method Definiotns for Class tmodel --------------------*/


// To output to STDOUT, submit filename as NULL
template <class COUNT, class PROB>
void tmodel<COUNT, PROB>::printCountTable(const char *filename, 
					 const Vector<WordEntry>& evlist, 
					 const Vector<WordEntry>& fvlist,
					 const bool actual) const
{
	ostream *tof;
	
	if(filename)
		tof = new ofstream(filename);
	else
		tof = & cout;
	
	ostream &of = *tof;
	/*  for(unsigned int i=0;i<es.size()-1;++i)
	for(unsigned int j=es[i];j<es[i+1];++j)
	{
	const CPPair&x=fs[j].second;
	WordIndex e=i,f=fs[j].first;
	if( actual )
	of << evlist[e].word << ' ' << fvlist[f].word << ' ' << x.prob << '\n';
	else
	of << e << ' ' << f << ' ' << x.prob << '\n';
	}*/
	for(unsigned int i=0;i<lexmat.size();++i){
		if( lexmat[i] ){
			for(unsigned int j=0;j<lexmat[i]->size();++j)
			{                        
				const CPPair&x=(*lexmat[i])[j].second;
				WordIndex e=i,f=(*lexmat[i])[j].first;
				if( x.prob>MINCOUNTINCREASE ){
					if( actual ){
						of << evlist[e].word << ' ' << fvlist[f].word << ' ' << x.count << '\n';
					}else{
						of << e << ' ' << f << ' ' << x.count << '\n';
					}
				}
			}
		}
	}
	
	if(filename){
		((ofstream*)tof)->close();
		delete tof;
	}
}

template <class COUNT, class PROB>
void tmodel<COUNT, PROB>::printProbTable(const char *filename, 
					 const Vector<WordEntry>& evlist, 
					 const Vector<WordEntry>& fvlist,
					 const bool actual) const
{
	ofstream of(filename);
	/*  for(unsigned int i=0;i<es.size()-1;++i)
	for(unsigned int j=es[i];j<es[i+1];++j)
	{
	const CPPair&x=fs[j].second;
	WordIndex e=i,f=fs[j].first;
	if( actual )
	of << evlist[e].word << ' ' << fvlist[f].word << ' ' << x.prob << '\n';
	else
	of << e << ' ' << f << ' ' << x.prob << '\n';
	}*/
	for(unsigned int i=0;i<lexmat.size();++i){
		if( lexmat[i] ){
			for(unsigned int j=0;j<lexmat[i]->size();++j)
			{                        
				const CPPair&x=(*lexmat[i])[j].second;
				WordIndex e=i,f=(*lexmat[i])[j].first;
				if( x.prob>PROB_SMOOTH ){
					if( actual ){
						of << evlist[e].word << ' ' << fvlist[f].word << ' ' << x.prob << '\n';
					}else{
						of << e << ' ' << f << ' ' << x.prob << '\n';
					}
				}
			}
		}
	}
}

template <class COUNT, class PROB>
void tmodel<COUNT, PROB>::printProbTableInverse(const char *, 
				   const Vector<WordEntry>&, 
				   const Vector<WordEntry>&, 
				   const double, 
				   const double, 
				   const bool ) const
{
}
template <class COUNT, class PROB>
void tmodel<COUNT, PROB>::normalizeTable(const vcbList&, const vcbList&, int)
{
    for(unsigned int i=0;i<lexmat.size();++i){
        double c=0.0;
        if( lexmat[i] ){
            unsigned int lSize=lexmat[i]->size();
            for(unsigned int j=0;j<lSize;++j)
                c+=(*lexmat[i])[j].second.count;
            for(unsigned int j=0;j<lSize;++j)  {
                if( c==0 )
                    (*lexmat[i])[j].second.prob=1.0/(lSize);
                else
                    (*lexmat[i])[j].second.prob=(*lexmat[i])[j].second.count/c;
                (*lexmat[i])[j].second.count=0;
            }
        }
    }
}

template <class COUNT, class PROB>
bool tmodel<COUNT, PROB>::readProbTable(const char *filename){
	/* This function reads the t table from a file.
	 Each line is of the format:  source_word_id target_word_id p(target_word|source_word)
	 This is the inverse operation of the printTable function.
	 NAS, 7/11/99
	 */
	ifstream inf(filename);
	cerr << "Reading t prob. table from " << filename << "\n";
	if (!inf) {
		cerr << "\nERROR: Cannot open " << filename << "\n";
		return false;
	}
	WordIndex src_id, trg_id;
	PROB prob;
	int nEntry=0;
	while (inf >> src_id >> trg_id >> prob) {
		insert(src_id, trg_id, 0.0, prob);
		nEntry++;
	}
	cerr << "Read " << nEntry << " entries in prob. table.\n";
	return true;
}



template class tmodel<COUNT,PROB> ; 

/* ---------------- End of Method Definitions of class tmodel ---------------*/



