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
#ifndef _aligntables_h
#define _aligntables_h 1

#include "defs.h"


#include <cassert>

#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
//#include <vector>
#include "Vector.h"
#include <utility>
#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif
#include <cmath>
#include <fstream>
#include "transpair_model1.h"


/* ----------------- Class Defintions for hashmyalignment --------------------
   Objective: This class is used to define a hash mapping function to map
   an alignment (defined as a vector of integers) into a hash key
 ----------------------------------------------------------------------------*/

class hashmyalignment : public unary_function< Vector<WordIndex>, size_t >
{
public:
  size_t operator() (const Vector<WordIndex>& key) const
  // to define the mapping function. it takes an alignment (a vector of
  // integers) and it returns an integer value (hash key).
  {
    WordIndex j ;
    size_t s  ;
    size_t key_sum = 0 ;
    //      logmsg << "For alignment:" ;
    for (j = 1 ; j < key.size() ; j++) {
      //	logmsg << " " << key[j] ;
      key_sum += (size_t) (int) pow(double(key[j]), double((j % 6)+1));
    }
    //      logmsg << " , Key value was : " <<  key_sum;
    s = key_sum % 1000000 ;
    //      logmsg << " h(k) = " << s << endl ;
    return(s);
  }
#ifdef WIN32
  enum {
    // parameters for hash table
    bucket_size = 1		// 0 < bucket_size
  };

  bool operator()(const Vector<WordIndex> t1,
                  const Vector<WordIndex> t2) const {
    WordIndex j ;
    if (t1.size() != t2.size())
      return(false);
    for (j = 1 ; j < t1.size() ; j++)
      if (t1[j] != t2[j])
        return(false);
    return(true);
  }
#endif
};

#ifndef WIN32
class equal_to_myalignment
{
  // returns true if two alignments are the same (two vectors have same enties)
public:
  bool operator()(const Vector<WordIndex> t1,
                  const Vector<WordIndex> t2) const {
    WordIndex j ;
    if (t1.size() != t2.size())
      return(false);
    for (j = 1 ; j < t1.size() ; j++)
      if (t1[j] != t2[j])
        return(false);
    return(true);
  }

};
#endif

/* ---------------- End of Class Defnition for hashmyalignment --------------*/


/* ------------------ Class Defintions for alignmodel -----------------------
 Class Name: alignmodel
 Objective: Alignments neighborhhoods (collection of alignments) are stored in
 a hash table (for easy lookup). Each alignment vector is mapped into a hash
 key using the operator defined above.
 *--------------------------------------------------------------------------*/

class alignmodel
{
private:
#ifdef WIN32
  typedef hash_map<Vector<WordIndex>, LogProb, hashmyalignment > alignment_hash;

#else
  typedef hash_map<Vector<WordIndex>, LogProb, hashmyalignment, equal_to_myalignment > alignment_hash;

#endif
  alignment_hash a;
private:
  //  void erase(Vector<WordIndex>&);
public:

  // methods;

  inline alignment_hash::iterator begin(void) {
    return a.begin(); // begining of hash
  }
  inline alignment_hash::iterator end(void) {
    return a.end(); // end of hash
  }
  inline const alignment_hash& getHash() const {
    return a;
  }; // reference to hash table
  bool insert(Vector<WordIndex>&, LogProb val=0.0); // add a alignmnet
//  void setValue(Vector<WordIndex>&, LogProb val); // not needed
  LogProb getValue(Vector<WordIndex>&)const; // retrieve prob. of alignment
  inline void clear(void) {
    a.clear();
  }; // clear hash table
  //  void printTable(const char* filename);
  inline void resize(WordIndex n) {
#ifndef WIN32
    a.resize(n);
#endif
  }; // resize table

};

/* -------------- End of alignmode Class Definitions ------------------------*/
#endif
