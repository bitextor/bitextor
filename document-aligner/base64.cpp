#include "base64.h"
#include <vector>
#include <cmath>
#include <util/exception.hh>

namespace bitextor {

namespace {

char const *TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int const INV_TABLE[128] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

} // namespace

void base64_encode(const StringPiece &in, std::string &out)
{
	out.clear();

	int val = 0, valb = -6;

	for (const unsigned char *c = reinterpret_cast<const unsigned char*>(in.data()); c != reinterpret_cast<const unsigned char*>(in.data()) + in.size(); ++c) {
		val = (val << 8) + *c;
		valb += 8;
		while (valb >= 0) {
			out.push_back(TABLE[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}

	if (valb >- 6)
		out.push_back(TABLE[((val << 8) >> (valb + 8)) & 0x3F]);

	while (out.size() % 4)
		out.push_back('=');
}

void base64_decode(const StringPiece &in, std::string &out)
{
	out.clear();

	// Reserve worst case scenario memory (can be a few bytes smaller/accurate,
	// but need to count padding so meh)
	out.reserve(ceil(in.size() / 3) * 4);

	int val = 0, valb = -8;
	for (const unsigned char *c = reinterpret_cast<const unsigned char*>(in.data()); c != reinterpret_cast<const unsigned char*>(in.data()) + in.size(); ++c) {
		// Padding reached
		if (*c == '=')
			break;
		
		UTIL_THROW_IF(INV_TABLE[*c] == -1, util::Exception, "Cannot interpret character '" << *c << "' as part of base64");
		
		val = (val << 6) + INV_TABLE[*c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
}

} // namespace bitextor
