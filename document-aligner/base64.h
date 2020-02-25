#pragma once
#include <string>
#include "util/string_piece.hh"

namespace bitextor {

void base64_encode(const StringPiece &in, std::string &out);

void base64_decode(const StringPiece &in, std::string &out);

// TODO: delete these functions.
inline std::string base64_encode(const StringPiece &in) {
	std::string ret;
	base64_encode(in, ret);
	return ret;
}

}
