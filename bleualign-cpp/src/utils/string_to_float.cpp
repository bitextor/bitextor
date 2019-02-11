
#include "string_to_float.h"

#include "util/string_piece.hh"
#include "util/double-conversion/double-conversion.h"
#include "util/double-conversion/utils.h"

namespace utils {

    namespace {
        static const double_conversion::StringToDoubleConverter kConverter(
                double_conversion::StringToDoubleConverter::ALLOW_LEADING_SPACES |
                double_conversion::StringToDoubleConverter::ALLOW_HEX |
                double_conversion::StringToDoubleConverter::ALLOW_TRAILING_SPACES,
                0., 0., "inf", "nan");
    } // namespace

    float ToDouble(StringPiece sp) {
      int processed_characters_count = -1;
      size_t len = sp.size();
      return kConverter.StringToDouble(sp.data(), static_cast<int>(len), &processed_characters_count);

    }

    float ToFloat(StringPiece sp) {
      int processed_characters_count = -1;
      size_t len = sp.size();
      return kConverter.StringToFloat(sp.data(), static_cast<int>(len), &processed_characters_count);

    }

} // namespace utils
