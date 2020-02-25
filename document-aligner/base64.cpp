#include "base64.h"
#include <vector>

namespace bitextor {

namespace {
char const *TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
	// TODO(jelmervdl): precompute this table.
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++)
		T[TABLE[i]] = i; 

	int val=0, valb=-8;
	for (const unsigned char *c = reinterpret_cast<const unsigned char*>(in.data()); c != reinterpret_cast<const unsigned char*>(in.data()) + in.size(); ++c) {
		if (T[*c] == -1)
			// TODO(jelmervdl): error handling.
			break;

		val = (val << 6) + T[*c];
		valb += 6;
		
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
}

} // namespace bitextor
