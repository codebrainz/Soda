#pragma once

#include "parse_error.hpp"

#include <cassert>
#include <string>
#include <utility>

namespace soda {

  [[noreturn]] inline void unreachable() {
#if defined(__cpp_lib_unreachable) and __cpp_lib_unreachable >= 202202L
    std::unreachable();
#elif defined(__GNUC__)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(false);
#else
    assert(false && "unreachable");
#endif
  }

  unsigned long long int parse_int(source_range const &range,
                                   std::string const &s);

  long double parse_float(source_range const &range, std::string const &s);

} // namespace soda
