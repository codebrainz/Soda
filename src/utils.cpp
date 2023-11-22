#include "utils.hpp"

#include <sstream>
#include <stdexcept>

namespace soda {

  unsigned long long int parse_int(source_range const &range,
                                   std::string const &s) {
    std::string ns;
    std::size_t pos = 0;
    int base = 0;

    if (s.starts_with("0b") || s.starts_with("0B"))
      ns = s.substr(2), base = 2;
    else if (s.starts_with("0d") || s.starts_with("0D"))
      ns = s.substr(2), base = 10;
    else if (s.starts_with("0o") || s.starts_with("0O"))
      ns = s.substr(2), base = 8;
    else if (s.starts_with("0x") || s.starts_with("0X"))
      ns = s.substr(2), base = 16;
    else if (s.starts_with("0"))
      ns = s, base = 8;
    else
      ns = s, base = 10;

    try {
      auto value = std::stoull(ns, &pos, base);
      if (pos != ns.size()) {
        std::stringstream ss;
        ss << "failed to completely parse integer literal '" << s << "'";
        throw parse_error{range, ss.str()};
      }
      return value;
    } catch (std::invalid_argument &e) {
      std::stringstream ss;
      ss << "failed to parse integer literal '" << s << "': " << e.what();
      throw parse_error{range, ss.str()};
    } catch (std::out_of_range &e) {
      std::stringstream ss;
      ss << "integer literal '" << s << "' is out-of-range: " << e.what();
      throw parse_error{range, ss.str()};
    }
  }

  long double parse_float(source_range const &range, std::string const &s) {
    std::size_t pos = 0;
    try {
      auto value = std::stold(s, &pos);
      if (pos != s.size()) {
        std::stringstream ss;
        ss << "failed to completely parse floating-point literal '" << s << "'";
        throw parse_error(range, ss.str());
      }
      return value;
    } catch (std::invalid_argument &e) {
      std::stringstream ss;
      ss << "failed to parse floating-point literal '" << s
         << "': " << e.what();
      throw parse_error{range, ss.str()};
    } catch (std::out_of_range &e) {
      std::stringstream ss;
      ss << "floating-point literal '" << s
         << "' is out-of-range: " << e.what();
      throw parse_error{range, ss.str()};
    }
  }

} // namespace soda
