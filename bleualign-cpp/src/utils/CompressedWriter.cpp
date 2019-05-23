
#include <iostream>
#include "CompressedWriter.h"

#include <boost/iostreams/filter/gzip.hpp>
#ifdef XZ_COMPRESS
#include <boost/iostreams/filter/lzma.hpp>
#endif


namespace utils {

    CompressedWriter::CompressedWriter(const std::string &filename_out) 
    : sink_out(filename_out),
      oqout(&qout),
      isEmpty(true)
    {
      if (filename_out.size() > 3) {
	std::string ext = filename_out.substr(filename_out.size() - 3, 3);
	//std::cerr << ext << std::endl;
        if (ext == ".gz") {
	  //std::cerr << "gzip" << std::endl;
          qout.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
        }
#ifdef XZ_COMPRESS
        else if (ext == ".xz") {
	  //std::cerr << "lzma" << std::endl;
          qout.push(boost::iostreams::lzma_compressor(boost::iostreams::lzma::best_compression));
        }
#endif
      }

      qout.push(sink_out);
    }

    CompressedWriter::~CompressedWriter() {
      if (isEmpty) {
        write("\n");
      }
    }

    void CompressedWriter::write(const std::string &s) {
      oqout << s;
      isEmpty = false;
    }

}
