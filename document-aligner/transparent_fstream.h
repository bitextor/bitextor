#pragma once
#include <fstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace bitextor {

/**
 * Streambuf backed by a file. If the opened file ends with ".gz" it will decompress it on the fly.
 */
class transparent_filebuf : public boost::iostreams::filtering_streambuf<boost::iostreams::input> {
public:
	void open(std::string const &path);
	
	void open_gzipped(std::string const &path);
	
	void open_plain(std::string const &path);
private:
	std::ifstream _file;
};

/**
 * It is an istream, but you should give it a path (or call open) and it will be an istream sourced from a
 * file. The file may be compressed. Hence the "transparent" part of the name. It might not seek properly
 * but it is sufficient for reading from start till end.
 * TODO: I want to call this transparent_ifstream but it really is an istream…
 */
class transparent_istream : public std::istream {
public:
	inline transparent_istream()
	: std::istream(&_filebuf) {
		//
	}
	
	inline transparent_istream(std::string const &path)
	: std::istream(&_filebuf) {
		open(path);
	}
	
	inline void open(std::string const &path) {
		_filebuf.open(path);
	}
private:
	transparent_filebuf _filebuf;
};

} // namespace bitextor
