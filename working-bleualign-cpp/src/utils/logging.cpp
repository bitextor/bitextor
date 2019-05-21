#include "logging.h"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/format.hpp>

#include <fstream>


namespace utils {

    void init() {
      boost::log::add_common_attributes();
      boost::log::add_console_log(std::cout, boost::log::keywords::format =
              (
                      boost::log::expressions::stream
                              << "(" << boost::log::trivial::severity << ") "
                              << "[" << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                                                            "%Y-%m-%d %H:%M:%S")
                              << "]: "
                              << boost::log::expressions::smessage
              ));

    }

    void log_done(std::string output_folder, std::string processed) {
      std::ofstream logfile;
      logfile.open(output_folder + "/" + "done.log", std::ios::out | std::ios::app);
      logfile << std::string(processed + "\n");
      logfile.close();
    }

    void log_error(std::string output_folder, std::string text) {
      std::ofstream logfile;
      logfile.open(output_folder + "/" + "error.log", std::ios::out | std::ios::app);
      logfile << std::string(text + "\n");
      logfile.close();
    }

} // namespace utils