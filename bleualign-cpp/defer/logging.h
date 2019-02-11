
#ifndef FAST_BLEUALIGN_LOGGING_H
#define FAST_BLEUALIGN_LOGGING_H


#define BOOST_LOG_DYN_LINK 1
#define LOG_INFO BOOST_LOG_TRIVIAL(info)
#define LOG_ERROR BOOST_LOG_TRIVIAL(error)

#include <boost/log/trivial.hpp>

#include <string>


namespace utils {

  void init();

  void log_info(std::string output_folder, std::string processed);

  void log_error(std::string output_folder, std::string text);

} // namespace utils

#endif //FAST_BLEUALIGN_LOGGING_H
