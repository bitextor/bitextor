#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include "util/file_piece.hh"
#include "src/base64.h"


using namespace bitextor;
using namespace std;

struct Join {
	size_t left_index;
	size_t right_index;
};

struct Row {
	vector<string> cells;
};

ostream &operator<<(ostream &out, Join const &join) {
	return out << join.left_index << '\t' << join.right_index;
}

ostream &operator<<(ostream &out, Row const &row) {
	for (size_t i = 0; i < row.cells.size(); ++i) {
		if (i > 0)
			out << '\t';
		out << row.cells[i];
	}

	return out;
}

typedef vector<unique_ptr<util::FilePiece>> FileSet;

bool skip_rows(FileSet &files, size_t n) {
	util::StringPiece line;

	for (auto &file : files)
		for (size_t i = 0; i < n; ++i)
			if (!file->ReadLineOrEOF(line))
				return false;

	return true;
}

bool read_row(FileSet &files, Row &row) {
	row.cells.clear();
	row.cells.reserve(files.size());

	util::StringPiece line;
	for (auto &file : files) {
		if (!file->ReadLineOrEOF(line))
			return false;

		row.cells.emplace_back(line.data(), line.size());
	}

	return true;
}

enum Source {
	LEFT,
	RIGHT,
	LEFT_INDEX,
	RIGHT_INDEX
};

int usage(char *progname) {
	cout << "Usage: " << progname << " [ -l filename | -r filename | -li | -ri ] ...\n"
	        "Input via stdin: <left index> \"\\t\" <right index> \"\\n\"\n"
	        "\n"
	        "This program joins rows from two sets of files into tab-separated output.\n"
	      	"\n"
	        "Column options:\n"
	        "  -l    Use left index for the following files\n"
	        "  -r    Use right index for the following files\n"
	        "  -li   Print the left index\n"
	        "  -ri   Print the right index\n"
	        "\n"
	        "The order of the columns in the output is the same as the order of the\n"
	        "arguments given to the program.\n";
	return 127;
}

int main(int argc, char *argv[]) {
	if (argc <= 1)
		return usage(argv[0]);

	// Open the files on the left and right side of the join. Keep track of
	// which file is on the left and on the right side, but also of the order
	// of the arguments as this will dictate the order of the output.
	FileSet left_files, right_files;
	vector<Source> order;
	Source side = LEFT;
	FileSet *files[]{&left_files, &right_files};
	for (int pos = 1; pos < argc; ++pos) {
		if (string(argv[pos]) == "-l")
			side = LEFT;
		else if (string(argv[pos]) == "-r")
			side = RIGHT;
		else if (string(argv[pos]) == "-li")
			order.push_back(LEFT_INDEX);
		else if (string(argv[pos]) == "-ri")
			order.push_back(RIGHT_INDEX);
		else {
			files[side]->emplace_back(new util::FilePiece(argv[pos]));
			order.push_back(side);
		}
	}

	// Read our joins into memory
	vector<Join> joins;
	string line;
	while (getline(cin, line)) {
		istringstream iline(line);
		Join join;
		if (iline >> join.left_index >> join.right_index)
			joins.push_back(join);
	}

	// Sort our joins by the right index so we can go through all right rows
	// in a sequential order.
	sort(joins.begin(), joins.end(), [](Join const &left, Join const &right) {
		return left.right_index < right.right_index;
	});

	// Read all of the left in memory
	vector<Row> left_rows;
	while (true) {
		Row row;
		if (!read_row(left_files, row))
			break;
		left_rows.push_back(move(row));
	}

	// For all joins (sorted by their right index) start reading through right
	// and every time we encounter one that we need we print left + right.
	size_t right_index = 0; // the indices used by docalign start at 1
	Row right_row;
	for (auto join_it = joins.begin(); join_it != joins.end(); ++join_it) {
		// Assume we sorted correctly and we read from 1 to n without jumps...
		assert(join_it->right_index >= right_index);

		// While our index is still far off, skip rows
		if (right_index < join_it->right_index - 1) {
			if (!skip_rows(right_files, (join_it->right_index - 1) - right_index)) {
				cerr << "Right index " << join_it->right_index << " outside of range " << right_index << endl;
				return 1;
			}
			right_index = join_it->right_index - 1;
		}

		// Next row is the row we want, read it.
		if (right_index == join_it->right_index - 1) {
			if (!read_row(right_files, right_row)) {
				cerr << "Right index " << join_it->right_index << " outside of range " << right_index << endl;
				return 1;
			}
			++right_index;
		}

		// Our left index is outside of what's in memory? Sad!
		if (join_it->left_index - 1 > left_rows.size()) {
			cerr << "Left index " << join_it->left_index << " outside of range " << left_rows.size() << endl;
			return 2;
		}

		// Print the columns in the order in which the input files were given to the program
		for (size_t i = 0, l = 0, r = 0; i < order.size(); ++i) {
			if (i > 0)
				cout << '\t';
			
			switch (order[i]) {
				case LEFT:
					cout << left_rows[join_it->left_index - 1].cells[l++];
					break;
				case RIGHT:
					cout << right_row.cells[r++];
					break;
				case LEFT_INDEX:
					cout << join_it->left_index;
					break;
				case RIGHT_INDEX:
					cout << join_it->right_index;
					break;
			}
		}
		cout << endl;
	}

	return 0;
}
