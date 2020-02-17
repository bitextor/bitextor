#pragma once
#include <string>

namespace bitextor {

std::string base64_encode(const std::string &in);
std::string base64_decode(const std::string &in);

}