#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <assert.h>
#include <math.h>

double KL_CONST = 0.0001;
double CONST_2 = 1.0;
using namespace std;

string ToUpper(string input) {
  string res = input;
  for (int i = 0; i < res.size(); i++) {
    if (res[i] >= 'a' && res[i] <= 'z') {
      res[i] += int('A') - int('a');
    }
  }
  return res;
}

vector<string> Split(string input) {
  input = ToUpper(input);
  vector<string> ans;
  string word;
  stringstream ss(input);
  while (ss >> word) {
    ans.push_back(word);
  }
  return ans;
}

double ToDouble(string input) {
  stringstream ss(input);
  double ans = 0;
  ss >> ans;
  return ans;
}

unordered_map<string, double> GetTable(istream &input) {
  unordered_map<string, double> ans;

  string line;
  while (getline(input, line)) {
    vector<string> words = Split(line);
    string key = words[0];
    string value = words[1];
    double prob = ToDouble(words[2]);
    double tot = ToDouble(words[3]);

    ans[key + " " + value] = prob;
    ans[key] = tot; // meaning we have this word in the vocab
  }

  return ans;
}

void DoTranslate(const unordered_map<string, double> &table,
                 istream& input, istream &input2) {
  string line;
  string line2;
  int lines_processed = 0;
  while (getline(input, line)) {
    getline(input2, line2);
    vector<string> words = Split(line);
    vector<string> words2 = Split(line2);

    map<string, double> p;  // reference vector
    map<string, double> q;  // translated vector

    int size = words.size();
    int size2 = words2.size();

    for (int i = 0; i < size2; i++) {
      p[words2[i]] += 1.0 / size2;
      q[words2[i]] = KL_CONST;
    }

    for (int i = 0; i < size; i++) {
      for (auto j = p.begin(); j != p.end(); j++) {
        if (table.find(words[i]) != table.end()) {
          auto iter = table.find(words[i] + " " + j->first);
          if (iter != table.end()) {
            q[j->first] += 1.0 / size * iter->second;
          }
        } else {
          if (words[i] == j->first) {
            q[j->first] += 1.0 / size;
          }
        }
      }
    }

    double ans = 0.0;
    map<string, double>::iterator q_iter = q.begin(), p_iter = p.begin();
    for (; q_iter != q.end(); ) {
      assert(p_iter->first == q_iter->first);
//      cout << "for word " << q_iter->first << " adding " << p_iter->second << " * " <<  log(p_iter->second / q_iter->second) << " = " << p_iter->second * log(p_iter->second / q_iter->second) << endl;
      ans += p_iter->second * log(1.0 / q_iter->second);
//      ans += p_iter->second * log(p_iter->second / q_iter->second);
      q_iter++;
      p_iter++;
    }
//    ans = ans - log(size2);
    cout << ans << endl;

  }
}

int main(int argc, char** argv) {
  if (argc < 4 && argc > 6) {
    cout << argv[0] << " table-file src-file tgt-file [kl-const] " << endl
         << endl
         << argv[0] << " requires 4 parameters; got instead " << argc << endl;
    return -1;
  }

  string table_file = argv[1];
  string src_file = argv[2];
  string tgt_file = argv[3];

  if (argc >= 5) {
    KL_CONST = ToDouble(string(argv[4]));
  }

  if (argc >= 6) {
    CONST_2 = ToDouble(string(argv[5]));
  }

  if (table_file == "-" && src_file == "-") {
    cerr << "Can not have stdin for both inputs" << endl;
    return -1;
  }

  unordered_map<string, double> table;

  cerr << "# Starting Reading Table" << endl;

  if (table_file == "-") {
    table = GetTable(cin);
  } else {
    ifstream ifile(table_file);
    table = GetTable(ifile);
  }

  cerr << "# Starting Translation" << endl;

  ifstream ifile2(tgt_file);
  if (src_file == "-") {
    DoTranslate(table, cin, ifile2);
  } else {
    ifstream ifile(src_file);
    DoTranslate(table, ifile, ifile2);
  }
  return 0;
}
