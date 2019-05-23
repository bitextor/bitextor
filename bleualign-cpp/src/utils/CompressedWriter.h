
#pragma once

#include <string>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>

namespace utils {

    class CompressedWriter {

    public:

        CompressedWriter(const std::string &filename_out);;
        virtual ~CompressedWriter();

        void write(const std::string &s);


    private:

        boost::iostreams::stream<boost::iostreams::file_sink> sink_out;
        boost::iostreams::filtering_streambuf<boost::iostreams::output> qout;
        std::ostream oqout;
        bool isEmpty;
    };

}


