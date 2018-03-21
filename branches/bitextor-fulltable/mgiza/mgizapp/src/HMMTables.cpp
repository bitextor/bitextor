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
#include "HMMTables.h"
#include <fstream>
#include <sstream>
#include "Globals.h"
#include "Parameter.h"

template<class CLS, class MAPPERCLASSTOSTRING> void HMMTables<CLS,
         MAPPERCLASSTOSTRING>::writeJumps(ostream&out) const
{
  double ssum=0.0;
  for (typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator i=
  alProb.begin(); i!=alProb.end(); ++i) {
    double sum=0.0;
    out << "\n\nDistribution for: ";
    printAlDeps(out, i->first, *mapper1, *mapper2);
    out << ' ';
    for (int a=i->second.low(); a<=i->second.high(); ++a)
      if (i->second[a]) {
        out << a << ':' << i->second[a] << ';' << ' ';
        sum+=i->second[a];
      }
    out << '\n' << '\n';
    out << "SUM: " << sum << '\n';
    ssum+=sum;
  }
  out << "FULL-SUM: " << ssum << '\n';
}
template<class CLS, class MAPPERCLASSTOSTRING> void HMMTables<CLS,
         MAPPERCLASSTOSTRING>::readJumps(istream&)
{
}
template<class CLS, class MAPPERCLASSTOSTRING> double HMMTables<CLS,
         MAPPERCLASSTOSTRING>::getAlProb(int istrich, int k, int sentLength,
             int J, CLS w1, CLS w2, int j, int iter) const
{
  massert(k<sentLength&&k>=0);
  massert(istrich<sentLength&&istrich>=-1);
  int pos=istrich-k;
  switch (PredictionInAlignments) {
  case 0:
    pos=istrich-k;
    break;
  case 1:
    pos=k;
    break;
  case 2:
    pos=(k*J-j*sentLength);
    if (pos>0)
      pos+=J/2;
    else
      pos-=J/2;
    pos/=J;
    break;
  default:
    abort();
  }
  lock->lock();
  typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator p=
    alProb.find(AlDeps<CLS>(sentLength, istrich, j, w1, w2));
  if (p!=alProb.end() ) {
    lock->unlock();
    return (p->second)[pos];
  } else {
    if (iter>0&&iter<5000)
      cout << "WARNING: Not found: " << ' ' << J << ' ' << sentLength
      << '\n';;
    lock->unlock();
    return 1.0/(2*sentLength-1);
  }
  lock->unlock();
}

template<class CLS, class MAPPERCLASSTOSTRING> void HMMTables<CLS,
         MAPPERCLASSTOSTRING>::addAlCount(int istrich, int k, int sentLength,
             int J, CLS w1, CLS w2, int j, double value, double valuePredicted)
{


  int pos=istrich-k;
  switch (PredictionInAlignments) {
  case 0:
    pos=istrich-k;
    break;
  case 1:
    pos=k;
    break;
  case 2:
    pos=(k*J-j*sentLength);
    if (pos>0)
      pos+=J/2;
    else
      pos-=J/2;
    pos/=J;
    break;
  default:
    abort();
  }


  AlDeps<CLS> deps(AlDeps<CLS>(sentLength, istrich, j, w1, w2));

  {
    lock->lock();
    typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
      alProb.find(deps);
    if (p==alProb.end() ) {
      if ( (CompareAlDeps&1)==0)
        p=alProb.insert(make_pair(deps,FlexArray<double> (-MAX_SENTENCE_LENGTH,MAX_SENTENCE_LENGTH,0.0))).first;
      else
        p=alProb.insert(make_pair(deps,FlexArray<double> (-sentLength,sentLength,0.0))).first;
    }
    p->second[pos]+=value;
    lock->unlock();
  }

  if (valuePredicted) {
    lock->lock();
    typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
      alProbPredicted.find(deps);
    if (p==alProbPredicted.end() ) {
      if ( (CompareAlDeps&1)==0)
        p
        =alProbPredicted.insert(make_pair(deps,FlexArray<double> (-MAX_SENTENCE_LENGTH,MAX_SENTENCE_LENGTH,0.0))).first;
      else
        p=alProbPredicted.insert(make_pair(deps,FlexArray<double> (-sentLength,sentLength,0.0))).first;
    }
    p->second[pos]+=valuePredicted;
    lock->unlock();
  }

}

template<class CLS, class MAPPERCLASSTOSTRING>
hmmentry_type& HMMTables<CLS,MAPPERCLASSTOSTRING>::doGetAlphaInit(int I)
{
  alphalock->lock();
  if( !init_alpha.count(I) ) {
#ifdef WIN32
    init_alpha[I]=hmmentry_type(Array<double>(I,0),new Mutex());
#else
    init_alpha[I]=hmmentry_type(Array<double>(I,0),Mutex());
#endif
  }
  hmmentry_type& ret  = init_alpha[I];
  alphalock->unlock();
  return ret;
}
template<class CLS, class MAPPERCLASSTOSTRING>
hmmentry_type& HMMTables<CLS,MAPPERCLASSTOSTRING>::doGetBetaInit(int I)
{
  betalock->lock();
  if( !init_beta.count(I) ) {
#ifdef WIN32
    init_beta[I]=hmmentry_type(Array<double>(I,0),new Mutex());
#else
    init_beta[I]=pair<Array<double>,Mutex>(Array<double>(I,0),Mutex());
#endif
  }
  hmmentry_type& ret = init_beta[I];
  betalock->unlock();
  return ret;
}

template<class CLS, class MAPPERCLASSTOSTRING> bool HMMTables<CLS,
         MAPPERCLASSTOSTRING>::getAlphaInit(int I, Array<double>&x) const
{
  alphalock->lock();
  hash_map<int,hmmentry_type >::const_iterator i=init_alpha.find(I);
  if (i==init_alpha.end() ) {
    alphalock->unlock();
    return 0;
  } else {
    x=i->second.first;
    alphalock->unlock();
    for (unsigned int j=x.size()/2+1; j<x.size(); ++j)
      // only first empty word can be chosen
      x[j]=0;
    return 1;
  }
  alphalock->unlock();
}
template<class CLS, class MAPPERCLASSTOSTRING> bool HMMTables<CLS,
         MAPPERCLASSTOSTRING>::getBetaInit(int I, Array<double>&x) const
{
  betalock->lock();
  hash_map<int,hmmentry_type >::const_iterator i=init_beta.find(I);
  if (i==init_beta.end() ) {
    betalock->unlock();
    return 0;
  } else {
    x=i->second.first;
    betalock->unlock();
    return 1;
  }
  betalock->unlock();
}

/***********************************
 By Edward Gao
 ************************************/

template<class CLS, class MAPPERCLASSTOSTRING> bool HMMTables<CLS,
         MAPPERCLASSTOSTRING>::writeJumps(const char* alprob,
             const char* alpredict, const char* alpha, const char* beta) const
{
  if (alprob) {
    ofstream ofs(alprob);
    if (!ofs.is_open()) {
      cerr << "Cannot open file for HMM output " << alprob << endl;
      return false;
    }
    cerr << "Dumping HMM table to " << alprob << endl;

    for (typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator i=
    alProb.begin(); i!=alProb.end(); ++i) {
      double sum=0.0;
      ofs <<i->first.englishSentenceLength << " "
      << i->first.classPrevious << " " << i->first.previous
      << " " << i->first.j << " " << i->first.Cj <<" "
      << i->second.low() <<" " << i->second.high()<< " ";
      for (int a=i->second.low(); a<=i->second.high(); ++a)
        if (i->second[a]) {
          ofs << a << ' ' << i->second[a] << ' ';
          sum+=i->second[a];
        }
      ofs << endl;
    }
    ofs.close();
  }
  if (alpredict) {
    ofstream ofs(alpredict);
    if (!ofs.is_open()) {
      cerr << "Cannot open file for HMM output " << alpredict << endl;
      return false;
    }
    cerr << "Dumping HMM table to " << alpredict << endl;
    for (typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator i=
    alProbPredicted.begin(); i!=alProbPredicted.end(); ++i) {
      double sum=0.0;
      ofs << i->first.englishSentenceLength << " "
      << i->first.classPrevious << " " << i->first.previous
      << " " << i->first.j << " " << i->first.Cj <<" "
      << i->second.low() <<" " << i->second.high()<< " ";
      for (int a=i->second.low(); a<=i->second.high(); ++a)
        if (i->second[a]) {
          ofs << a << ' ' << i->second[a] << ' ';
          sum+=i->second[a];
        }
      ofs << endl;
    }
    ofs.close();
  }
  if (alpha) {
    ofstream ofs(alpha);

    if (!ofs.is_open()) {
      cerr << "Cannot open file for HMM output " << alpha << endl;
      return false;
    }
    cerr << "Dumping HMM table to " << alpha << endl;
    for (typename hash_map<int,hmmentry_type>::const_iterator i=
    init_alpha.begin(); i!=init_alpha.end(); i++) {
      ofs << i->first << " " << i->second.first.size() <<" ";
      int j;
      for (j=0; j<i->second.first.size(); j++) {
        ofs << i->second.first[j] << " ";
      }
      ofs<<endl;
    }
    ofs.close();
  }
  if (beta) {
    ofstream ofs(beta);
    if (!ofs.is_open()) {
      cerr << "Cannot open file for HMM output " << beta << endl;
      return false;
    }
    cerr << "Dumping HMM table to " << beta << endl;
    for (typename hash_map<int,hmmentry_type>::const_iterator i=
    init_beta.begin(); i!=init_beta.end(); i++) {
      ofs << i->first << " " << i->second.first.size() << " ";
      int j;
      for (j=0; j<i->second.first.size(); j++) {
        ofs << i->second.first[j] << " ";
      }
      ofs << endl;
    }
    ofs.close();
  }
  return true;
}

template<class CLS, class MAPPERCLASSTOSTRING> bool HMMTables<CLS,
         MAPPERCLASSTOSTRING>::readJumps(const char* alprob,
             const char* alpredict, const char* alpha, const char* beta)
{
  if (alprob) {
    ifstream ifs(alprob);
    if (!ifs.is_open()) {
      cerr << "Cannot open file for HMM input " << alprob << endl;
      return false;
    }
    cerr << "Reading HMM table from " << alprob << endl;
    string strLine="";
    while (!ifs.eof()) {
      strLine = "";
      getline(ifs, strLine);
      if (strLine.length()) {
        stringstream ss(strLine.c_str());
        AlDeps<CLS> dep;
        int low, high;
        ss >> dep.englishSentenceLength >> dep.classPrevious
        >> dep.previous >> dep.j >> dep.Cj >> low >> high;
        typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
          alProb.find(dep);
        if (p==alProb.end() ) {
          p=alProb.insert(make_pair(dep,FlexArray<double> (low,high,0.0))).first;
        }
        int pos;
        double val;
        while (!ss.eof()) {
          pos = low-1;
          val = 0;
          ss >> pos >> val;
          if (pos>low-1) {
            p->second[pos]+=val;
          }
        }
      }
    }
  }
  if (alpredict) {
    ifstream ifs(alpredict);
    if (!ifs.is_open()) {
      cerr << "Cannot open file for HMM input " << alpredict << endl;
      return false;
    }
    cerr << "Reading HMM table from " << alpredict << endl;
    string strLine="";
    while (!ifs.eof()) {
      strLine = "";
      getline(ifs, strLine);
      if (strLine.length()) {
        stringstream ss(strLine.c_str());
        AlDeps<CLS> dep;
        int low, high;
        ss >> dep.englishSentenceLength >> dep.classPrevious
        >> dep.previous >> dep.j >> dep.Cj >> low >> high;
        typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
          alProbPredicted.find(dep);
        if (p==alProbPredicted.end() ) {
          p=alProbPredicted.insert(make_pair(dep,FlexArray<double> (low,high,0.0))).first;
        }
        int pos;
        double val;

        while (!ss.eof()) {
          pos = low-1;
          val = 0;
          ss >> pos >> val;
          if (pos>low-1) {
            p->second[pos]+=val;
          }
        }
      }
    }
  }

  if (alpha) {
    ifstream ifs(alpha);

    if (!ifs.is_open()) {
      cerr << "Cannot open file for HMM input " << alpha << endl;
      return false;
    }
    string strLine="";
    while (!ifs.eof()) {
      strLine = "";
      getline(ifs, strLine);
      if (strLine.length()) {
        stringstream ss(strLine.c_str());
        int id = -1, size = -1;
        ss >> id >> size;
        if (id<0||size<0||id!=size) {
          cerr << "Mismatch in alpha init table!" << endl;
          return false;
        }
        hmmentry_type&alp = doGetAlphaInit(id);
        Array<double>& gk = alp.first;
        int j;
        double v;
#ifdef WIN32
        alp.second->lock();
#else
        alp.second.lock();
#endif
        for (j=0; j<gk.size(); j++) {
          ss >> v;
          gk[j]+=v;
        }
#ifdef WIN32
        alp.second->unlock();
#else
        alp.second.unlock();
#endif

      }
    }
  }

  if (beta) {
    ifstream ifs(beta);

    if (!ifs.is_open()) {
      cerr << "Cannot open file for HMM input " << beta << endl;
      return false;
    }
    string strLine="";
    while (!ifs.eof()) {
      strLine = "";
      getline(ifs, strLine);
      if (strLine.length()) {
        stringstream ss(strLine.c_str());
        int id = -1, size = -1;
        ss >> id >> size;
        if (id<0||size<0||id!=size) {
          cerr << "Mismatch in alpha init table!" << endl;
          return false;
        }
        hmmentry_type&bet1 = doGetBetaInit(id);
        Array<double>&bet = bet1.first;

        int j;
        double v;
#ifdef WIN32
        bet1.second->lock();
#else
        bet1.second.lock();
#endif
        for (j=0; j<bet.size(); j++) {
          ss >> v;
          bet[j]+=v;
        }
#ifdef WIN32
        bet1.second->unlock();
#else
        bet1.second.unlock();
#endif
      }
    }
  }

  return true;
}

template<class CLS, class MAPPERCLASSTOSTRING> bool HMMTables<CLS,
         MAPPERCLASSTOSTRING>::merge(HMMTables<CLS,MAPPERCLASSTOSTRING> & ht)
{

  for (typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator i=
  ht.alProb.begin(); i!=ht.alProb.end(); ++i) {
    typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
      alProb.find(i->first);
    if (p==alProb.end() ) {
      p=alProb.insert(make_pair(i->first,FlexArray<double> (i->second.low(),i->second.high(),0.0))).first;
    }
    for (int a=i->second.low(); a<=i->second.high(); ++a)
      if (i->second[a]) {
        p->second[a] += i->second[a];
      }

  }

  for (typename map<AlDeps<CLS>,FlexArray<double> >::const_iterator i=
  ht.alProbPredicted.begin(); i!=ht.alProbPredicted.end(); ++i) {
    typename map<AlDeps<CLS>,FlexArray<double> >::iterator p=
      alProbPredicted.find(i->first);
    if (p==alProbPredicted.end() ) {
      p=alProbPredicted.insert(make_pair(i->first,FlexArray<double> (i->second.low(),i->second.high(),0.0))).first;
    }
    for (int a=i->second.low(); a<=i->second.high(); ++a)
      if (i->second[a]) {
        p->second[a] += i->second[a];
      }

  }

  for (typename hash_map<int,hmmentry_type>::iterator i=
  ht.init_alpha.begin(); i!=ht.init_alpha.end(); i++) {
    hmmentry_type& alp = doGetAlphaInit(i->first);
    int j;
    for (j=0; j<alp.first.size(); j++) {
      alp.first[j]+=i->second.first[j];
    }
  }
  for (typename hash_map<int,hmmentry_type>::iterator i=
  ht.init_beta.begin(); i!=ht.init_beta.end(); i++) {
    hmmentry_type&alp = doGetBetaInit(i->first);
    int j;
    for (j=0; j<alp.first.size(); j++) {
      alp.first[j]+=i->second.first[j];
    }
  }

  return true;

}

//////////////////////////////////////
template<class CLS, class MAPPERCLASSTOSTRING> HMMTables<CLS,
         MAPPERCLASSTOSTRING>::HMMTables(double _probForEmpty,
             const MAPPERCLASSTOSTRING&m1, const MAPPERCLASSTOSTRING&m2) :
           probabilityForEmpty(mfabs(_probForEmpty)),
           updateProbabilityForEmpty(_probForEmpty<0.0), mapper1(&m1),
           mapper2(&m2)
{
  lock = new Mutex();
  alphalock = new Mutex();
  betalock = new Mutex();
}

template<class CLS, class MAPPERCLASSTOSTRING> HMMTables<CLS,
         MAPPERCLASSTOSTRING>::HMMTables(const HMMTables& ref):
           mapper1(ref.mapper1), mapper2(ref.mapper2)
{
  probabilityForEmpty=ref.probabilityForEmpty;
  updateProbabilityForEmpty=ref.updateProbabilityForEmpty;
  init_alpha=ref.init_alpha;
  init_beta=ref.init_beta;
  alProb=ref.alProb;
  alProbPredicted=ref.alProbPredicted;
  globalCounter=ref.globalCounter;
  divSum=ref.divSum;
  p0_count=ref.p0_count;
  np0_count=ref.np0_count;
}
template<class CLS, class MAPPERCLASSTOSTRING> void HMMTables<CLS,
         MAPPERCLASSTOSTRING>::operator=(const HMMTables& ref)
{
  probabilityForEmpty=ref.probabilityForEmpty;
  updateProbabilityForEmpty=ref.updateProbabilityForEmpty;
  init_alpha=ref.init_alpha;
  init_beta=ref.init_beta;
  alProb=ref.alProb;
  alProbPredicted=ref.alProbPredicted;
  globalCounter=ref.globalCounter;
  divSum=ref.divSum;
  p0_count=ref.p0_count;
  np0_count=ref.np0_count;
}


template<class CLS, class MAPPERCLASSTOSTRING> HMMTables<CLS,
         MAPPERCLASSTOSTRING>::~HMMTables()
{
#if 0
  for (typename hash_map<int,hmmentry_type>::iterator i=
  init_alpha.begin(); i!=init_alpha.end(); i++) {
    i->second.second->unlock();
  }
  for (typename hash_map<int,hmmentry_type>::iterator i=
  init_beta.begin(); i!=init_beta.end(); i++) {
    i->second.second->unlock();
  }


  delete lock;
  delete alphalock;
  delete betalock;

  for (typename hash_map<int,hmmentry_type>::iterator i=
  init_alpha.begin(); i!=init_alpha.end(); i++) {
    delete i->second.second;
  }
  for (typename hash_map<int,hmmentry_type>::iterator i=
  init_beta.begin(); i!=init_beta.end(); i++) {
    delete i->second.second;
  }
#endif
}
