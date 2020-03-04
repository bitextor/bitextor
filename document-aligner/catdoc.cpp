#include "util/file_piece.hh"
#include "base64.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

using namespace bitextor;
using namespace std;

struct Join {
	float score;
	size_t left_index;
	size_t right_index;
};

struct Row {
	vector<string> cells;
};

ostream &operator<<(ostream &out, Join const &join) {
	return out << join.score << '\t' << join.left_index << '\t' << join.right_index;
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

bool skip_row(FileSet &files) {
	StringPiece line;

	for (auto &file : files)
		if (!file->ReadLineOrEOF(line))
			return false;

	return true;
}

bool read_row(FileSet &files, Row &row) {
	row.cells.reserve(files.size());

	StringPiece line;
	for (auto &file : files) {
		if (!file->ReadLineOrEOF(line))
			return false;

		row.cells.emplace_back(line.data(), line.size());
	}

	return true;
}

int main(int argc, char *argv[]) {
	// Open the files on the left and right side of the join
	FileSet left_files, right_files;
	FileSet *files = &left_files;
	for (int pos = 1; pos < argc; ++pos) {
		if (argv[pos] == string("--"))
			files = &right_files;
		else
			files->emplace_back(new util::FilePiece(argv[pos]));
	}

	// Read our joins into memory
	vector<Join> joins;
	string line;
	while (getline(cin, line)) {
		istringstream iline(line);
		Join join;
		if (iline >> join.score >> join.left_index >> join.right_index)
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
		while (join_it->right_index - 1 > right_index) {
			if (!skip_row(right_files)) {
				cerr << "Right index " << join_it->right_index << " outside of range " << right_index << endl;
				return 1;
			}
			++right_index;
		}

		// Next row is the row we want, read it.
		if (join_it->right_index - 1 == right_index) {
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

		cout << left_rows[join_it->left_index - 1] << '\t' << right_row << endl;
	}

	return 0;
}
