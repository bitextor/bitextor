#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <cstdio>
#include <cstdlib>

using namespace std;

void readVoc(istream&in,map<string,string>&voc)
{
  string line,s1,s2;
  voc["1"]="UNK";
  if( !in )cerr <<"Vocabulary does not exist.\n";
  while(getline(in,line)) {
    istrstream eingabe(line.c_str());
    if( !(eingabe>>s1>>s2))
      cerr << "ERROR in vocabulary '" << line << "'\n";
    voc[s1]=s2;
  }
}

int maxElems=0;
int main(int argc,char **argv)
{
  if( argc!=4&&argc!=5 ) {
    cerr << "Usage: " << argv[0] << " output vcb1 vcb2 snt12 \n";
    cerr << "Converts GIZA++ snt-format into plain text.\n";
    exit(1);
  }
  if( argc==6 ) {
    if(string(argv[4])!="-counts")
      cerr << "ERROR: wrong option " << argv[5] << endl;
    maxElems=10000000;
  }
  ifstream v1(argv[1]),v2(argv[2]),t(argv[3]);
  map<string,string>voc1,voc2;
  readVoc(v1,voc1);
  readVoc(v2,voc2);
  string line1,line2,line3;
  int nLine=0;
  while(getline(t,line1)&&getline(t,line2)&&getline(t,line3)) {
    istrstream eingabe1(line1.c_str()),eingabe2(line2.c_str()),eingabe3(line3.c_str());
    double count;
    string word;
    eingabe1>>count;
    vector<int>l1,l2;
    while(eingabe2>>word)
      l1.push_back(atoi(word.c_str()));
    while(eingabe3>>word)
      l2.push_back(atoi(word.c_str()));
    if( ((++nLine)%1000)==0 )
      cerr << "line " << nLine << '\n';
    for(unsigned int j=0; j<l2.size(); ++j) {
      cout << 0 << " " << l2[j] << endl;
    }
    for(unsigned int i=0; i<l1.size(); ++i) {
      for(unsigned int j=0; j<l2.size(); ++j) {
        cout << l1[i] << " " << l2[j] << endl;
      }
    }
  }
  cerr << "END.\n";
}


