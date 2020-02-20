#include "transparent_fstream.h"

bool ends_with(std::string const &str, std::string const &ext) {
	return str.length() >= ext.length() && str.compare(str.length() - ext.length(), ext.length(), ext) == 0;
}

namespace bitextor {

void transparent_filebuf::open(std::string const &path) {
	// TODO: can't we peek _file and look for gzip magic? I.e. {0x1f, 0x8b}?
	if (ends_with(path, ".gz"))
		this->open_gzipped(path);
	else
		this->open_plain(path);
}

void transparent_filebuf::open_gzipped(std::string const &path) {
	_file.open(path, std::ios_base::in | std::ios_base::binary);
	this->push(boost::iostreams::gzip_decompressor());
	this->push(_file);
}

void transparent_filebuf::open_plain(std::string const &path) {
	_file.open(path, std::ios_base::in);
	this->push(_file);
}

} // namespace bitextor
