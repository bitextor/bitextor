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
/* --------------------------------------------------------------------------*
 *                                                                           *
 * Module : TTables                                                          *
 *                                                                           *
 * Prototypes File: TTables.h                                               *
 *                                                                           *
 * Objective: Defines clases and methods for handling I/O for Probability &  *
 *            Count tables and also alignment tables                         *
 *****************************************************************************/

#ifndef _ttables_h
#define _ttables_h 1


#include "defs.h"
#include "vocab.h"

#include <cassert>

#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include "Vector.h"
#include <utility>
#include "syncObj.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

#include <fstream>

#include "Globals.h"


/* The tables defined in the following classes are defined as hash tables. For
   example. the t-table is a hash function of a word pair; an alignment is
   a hash function of a vector of integer numbers (sentence positions) and so
   on   */


/*----------- Defnition of Hash Function for class tmodel ------- -----------*/

typedef pair<WordIndex, WordIndex> wordPairIds;



class hashpair : public unary_function< pair<WordIndex, WordIndex>, size_t >
{
public:
  size_t operator() (const pair<WordIndex, WordIndex>& key) const {
    return (size_t) MAX_W*key.first + key.second; /* hash function and it
						       is guarnteed to have
						       unique id for each
						       unique pair */
  }
#ifdef WIN32
  inline bool operator() (const pair<WordIndex, WordIndex>& key, const pair<WordIndex, WordIndex>& key2) {
    return key.first==key2.first && key.second==key2.second;
  }
  enum {
    // parameters for hash table
    bucket_size = 1		// 0 < bucket_size
  };
#endif
};



/* ------------------ Class Prototype Definitions ---------------------------*
  Class Name: tmodel
  Objective: This defines the underlying data structur for t Tables and t
  Count Tables. They are defined as a hash table. Each entry in the hash table
  is the probability (P(fj/ei) ) or count collected for ( C(fj/ei)). The
  probability and the count are represented as log integer probability as
  defined by the class LogProb .

  This class is used to represents t Tables (probabiliity) and n (fertility
  Tables and also their corresponding count tables .

 *---------------------------------------------------------------------------*/

//typedef float COUNT ;
//typedef LogProb PROB ;
template <class COUNT, class PROB>
class LpPair
{
public:
  COUNT count ;
  PROB  prob ;
public: // constructor
  LpPair():count(0), prob(0) {} ;
  LpPair(COUNT c, PROB p):count(c), prob(p) {};
} ;

template<class T>
T*mbinary_search(T*x,T*y,unsigned int val)
{
  if( y-x==0 )
    return 0;
  if( x->first==val)
    return x;
  if( y-x<2 )
    return 0;
  T*mid=x+(y-x)/2;
  if( val < mid->first )
    return mbinary_search(x,mid,val);
  else
    return mbinary_search(mid,y,val);

}

template<class T>
const T*mbinary_search(const T*x,const T*y,unsigned int val)
{
  if( y-x==0 )
    return 0;
  if( x->first==val)
    return x;
  if( y-x<2 )
    return 0;
  const T*mid=x+(y-x)/2;
  if( val < mid->first )
    return mbinary_search(x,mid,val);
  else
    return mbinary_search(mid,y,val);

}

template <class COUNT, class PROB>
class tmodel
{
  typedef LpPair<COUNT, PROB> CPPair;
public:
  bool recordDiff;

public:
  int noEnglishWords;  // total number of unique source words
  int noFrenchWords;   // total number of unique target words
  //vector<pair<unsigned int,CPPair> > fs;
  //vector<unsigned int> es;
  vector< vector<pair<unsigned int,CPPair> >* > lexmat;
  vector< Mutex* > mutex;

  void erase(WordIndex e, WordIndex f) {
    CPPair *p=find(e,f);
    if(p)
      *p=CPPair(0,0);
  };

  CPPair*find(int e,int f) {
    //pair<unsigned int,CPPair> *be=&(fs[0])+es[e];
    //pair<unsigned int,CPPair> *en=&(fs[0])+es[e+1];
    if(e>=lexmat.size()||lexmat[e]==NULL) {
      return NULL;
    }
    pair<unsigned int,CPPair> *be=&(*lexmat[e])[0];
    pair<unsigned int,CPPair> *en=&(*lexmat[e])[0]+(*lexmat[e]).size();
    pair<unsigned int,CPPair> *x= mbinary_search(be,en,f);
    if( x==0 ) {
      //cerr << "A:DID NOT FIND ENTRY: " << e << " " << f << '\n';
      //abort();
      return 0;
    }
    return &(x->second);
  }

  const CPPair*find(int e,int f)const {
    if(e>=lexmat.size()||lexmat[e]==NULL) {
      return NULL;
    }
    const pair<unsigned int,CPPair> *be=&(*lexmat[e])[0];
    const pair<unsigned int,CPPair> *en=&(*lexmat[e])[0]+(*lexmat[e]).size();
    //const pair<unsigned int,CPPair> *be=&(fs[0])+es[e];
    //const pair<unsigned int,CPPair> *en=&(fs[0])+es[e+1];
    const pair<unsigned int,CPPair> *x= mbinary_search(be,en,f);
    if( x==0 ) {
      //cerr << "B:DID NOT FIND ENTRY: " << e << " " << f << '\n';
      //abort();
      return 0;
    }

    return &(x->second);
  }
public:
  void insert(WordIndex e, WordIndex f, COUNT cval=0.0, PROB pval = 0.0) {
    CPPair* found = find(e,f);
    if(found)
      *found=CPPair(cval,pval);
  }

  CPPair*getPtr(int e,int f) {
    return find(e,f);
  }

  tmodel() {};
  tmodel(const string&fn)	{
    recordDiff = false;
    int count=0,count2=0;
    ifstream infile2(fn.c_str());
    cerr << "Inputfile in " << fn << endl;
    int e,f,olde=-1,oldf=-1;
    pair<unsigned int,CPPair> cp;
    vector< pair<unsigned int,CPPair> > cps;
    while(infile2>>e>>f) {
      cp.first=f;
      assert(e>=olde);
      assert(e>olde ||f>oldf);
      if( e!=olde&&olde>=0 ) {
        int oldsize=lexmat.size();
        lexmat.resize(olde+1);
        for(unsigned int i=oldsize; i<lexmat.size(); ++i)
          lexmat[i]=0;
        lexmat[olde]=new vector< pair<unsigned int,CPPair> > (cps);
        cps.clear();
        if( !((*lexmat[olde]).size()==(*lexmat[olde]).capacity()) )
          cerr << "eRROR: waste of memory: " << (*lexmat[olde]).size() << " " << (*lexmat[olde]).capacity() << endl;
        count2+=lexmat[olde]->capacity();
      }
      cps.push_back(cp);
      olde=e;
      oldf=f;
      count++;
    }
    lexmat.resize(olde+1);
    lexmat[olde]=new vector< pair<unsigned int,CPPair> > (cps);
    count2+=lexmat[olde]->capacity();
    cout << "There are " << count << " " << count2 << " entries in table" << '\n';
    mutex.resize(lexmat.size());
    for(int _i = 0; _i< lexmat.size(); _i++) {
      mutex[_i] = new Mutex();
    }
    /* Create mutex */
  }

  ~tmodel() {
    for(int _i = 0; _i< lexmat.size(); _i++) {
      delete mutex[_i];
    }

  }


  /*  tmodel(const string&fn)
    {
      size_t count=0;
      {
  ifstream infile1(fn.c_str());
  if( !infile1 )
    {
      cerr << "ERROR: can't read coocurrence file " << fn << '\n';
      abort();
    }
  int e,f;
  while(infile1>>e>>f)
    count++;
      }
      cout << "There are " << count << " entries in table" << '\n';
      ifstream infile2(fn.c_str());
      fs.resize(count);
      int e,f,olde=-1,oldf=-1;
      pair<unsigned int,CPPair> cp;
      count=0;
      while(infile2>>e>>f)
  {
    assert(e>=olde);
    assert(e>olde ||f>oldf);
    if( e!=olde )
      {
        es.resize(e+1);
        for(unsigned int i=olde+1;int(i)<=e;++i)
  	es[i]=count;
      }
    cp.first=f;
    assert(count<fs.size());
    fs[count]=cp;
    //fs.push_back(cp);
    olde=e;
    oldf=f;
    count++;
  }
      assert(count==fs.size());
      es.push_back(fs.size());
      cout << fs.size() << " " << count << " coocurrences read" << '\n';
      }*/

  void incCount(WordIndex e, WordIndex f, COUNT inc) {
    if( inc ) {
      CPPair *p=find(e,f);
      if( p ) {
        mutex[e]->lock();
        p->count += inc ;
        mutex[e]->unlock();
      }
    }
  }

  PROB getProb(WordIndex e, WordIndex f) const {
    const CPPair *p=find(e,f);
    if( p )
      return max(p->prob, PROB_SMOOTH);
    else
      return PROB_SMOOTH;
  }

  COUNT getCount(WordIndex e, WordIndex f) const {
    const CPPair *p=find(e,f);
    if( p )
      return p->count;
    else
      return 0.0;
  }

  void printProbTable(const char* filename, const Vector<WordEntry>&, const Vector<WordEntry>&,bool actual) const;
  void printCountTable(const char* filename, const Vector<WordEntry>&, const Vector<WordEntry>&,bool actual) const;
  void printProbTableInverse(const char *filename,
                             const Vector<WordEntry>& evlist,
                             const Vector<WordEntry>& fvlist,
                             const double eTotal,
                             const double fTotal,
                             const bool actual = false ) const;
  void normalizeTable(const vcbList&engl, const vcbList&french, int iter=2);
  bool readProbTable(const char *filename);
  bool readSubSampledProbTable(const char* filename, std::set<WordIndex> &e, std::set<WordIndex> &f);
};

#endif
