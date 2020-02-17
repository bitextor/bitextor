#include "document.h"
#include <iostream>
#include <fstream>

using namespace bitextor;
using namespace std;


int main(int argc, char *argv[]) {
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " tokens.txt urls.txt" << endl;
		return 1;
	}

	std::vector<Document> documents;
	documents.reserve(100);

	ifstream tokens_in(argv[1]);
	ifstream urls_in(argv[2]);

	Document buffer;

	while (tokens_in >> buffer) {
		if (!(urls_in >> buffer.url)) { // TODO: Dangerous assumption that there is no space in url
			cerr << "Error while reading the url for the " << documents.size() << "th document" << endl;
			return 2;
		}

		documents.push_back(buffer);
	}

	cout << "Read " << documents.size() << " documents" << endl;
}