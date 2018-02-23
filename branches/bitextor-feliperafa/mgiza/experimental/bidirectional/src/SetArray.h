/*
Array of set, for fast access of dictionary, and most important,
be threadsafe
*/


#ifndef __SET_ARRAY_H__
#define __SET_ARRAY_H__

#include <map>
#include <vector>
#include "defs.h"
#include "vocab.h"  
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include "syncObj.h"

template <class COUNT, class PROB>
class LpPair {
public:
    COUNT count ;
    PROB  prob ;
public: // constructor 
    LpPair():count(0), prob(0){} ;
    LpPair(COUNT c, PROB p):count(c), prob(p){};
} ;




template <class COUNT, class PROB>
class SetArray{
public:
         typedef LpPair<COUNT, PROB> CPPair;
protected:
    
    /*Information stores here*/
    std::vector<std::map<size_t,CPPair> > store;
    std::vector<Mutex> muts;
    size_t nEnglishWord;
    size_t nFrenchWord;
    void _init(){
        store.resize(nEnglishWord);
        muts.resize(nFrenchWord);
    }
    
public:
       
    /*
        Get reference, not creating
    */
    CPPair* find(size_t fi, size_t si){
        /*HERE: lock, unlock after we get the pointer*/
        muts[fi].lock();
        /* Sync-ed */
        std::map<size_t,CPPair>& w = store[fi];
        typename std::map<size_t,CPPair>::iterator it = w.find((size_t)si);
        CPPair* q = ( it!=store[fi].end() ? &(it->second) : 0);
//        for(it = w.begin(); it!=w.end();it++){
 //           cout << it->first << endl;
 //       }
        /* End Synced*/
        muts[fi].unlock();
        return q;
    };
    
    /*
        Get reference, creating it
    */
    inline CPPair& findRef(size_t fi, size_t si){
        std::map<size_t,CPPair> &x = store[fi];
        muts[fi].lock();
        /* Sync-ed */
        CPPair& ref= x[si];
        /* End Synced */
        muts[fi].unlock();
    };

    
    void insert(size_t fi, size_t si, COUNT count = 0, PROB prob = 0){
        muts[fi].lock();
        /*Syced*/
        std::map<size_t,CPPair> &x = store[fi];
        CPPair& v= x[si];
        v.count = count;
        v.prob = prob;
        muts[fi].unlock();
    }
    
    void incCount(size_t e, size_t f, COUNT inc) 
        // increments the count of the given word pair. if the pair does not exist, 
        // it creates it with the given value.
        {
            if( inc ){
                std::map<size_t,CPPair> &x = store[e];
                muts[e].lock();
                CPPair& ref= x[f];
                ref.count += inc;
                muts[e].unlock();
            }
        }

    PROB getProb(size_t e, size_t f) const
        // read probability value for P(fj/ei) from the hash table 
    // if pair does not exist, return floor value PROB_SMOOTH
    {
        muts[e].lock();
        typename std::map<size_t,CPPair >::const_iterator it = store[e].find(f);
        PROB b;
        if(it == store[e].end())  
            b = PROB_SMOOTH; 
        else
            b=max((it->second).prob, PROB_SMOOTH);
        muts[e].unlock();
        return b;
    }
    
    COUNT getCount(size_t e, size_t f) const
        /* read count value for entry pair (fj/ei) from the hash table */
    {
        muts[e].lock();
        typename std::map<size_t,CPPair >::const_iterator it = store[e].find(f);
        COUNT c;
        if(it == store[e].end())  
            c = 0; 
        else
            c = ((*it).second).count;
        muts[e].unlock();
    }
    
    void erase(size_t e, size_t f)
        // In: a source and a target token ids.
        // removes the entry with that pair from table
    {
        muts[e].lock();   
        store[e].erase(f);
        muts[e].unlock();
    };
    
    inline void setNumberOfEnlish(size_t e){nEnglishWord=e;_init();};
    inline void setNumberOfFrench(size_t f){nFrenchWord = f;};
    
    const std::map<size_t,CPPair>& getMap(size_t i) const{
        return store[i];
    }
    
    std::map<size_t,CPPair>& getMap1(size_t i){
        return store[i];
    }
    
    SetArray(size_t e, size_t f): nEnglishWord(e), nFrenchWord(f){
        _init();
    }
};



#endif
