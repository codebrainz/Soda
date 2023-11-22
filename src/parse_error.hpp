#pragma once

#include "source_range.hpp"

#include <exception>
#include <sstream>
#include <string>

namespace soda {

  class parse_error : public std::exception {
  public:
    parse_error(source_range range, std::string message)
        : range_{std::move(range)}, message_{std::move(message)},
          what_{format_error(range_, message_)} {
    }

    source_range const &range() const noexcept {
      return range_;
    }

    std::string const &message() const noexcept {
      return message_;
    }

    const char *what() const noexcept override {
      return what_.c_str();
    }

  private:
    source_range range_;
    std::string message_;
    std::string what_;

    static std::string format_error(source_range const &range,
                                    std::string const &message) {
      std::stringstream ss;
      ss << range << ": error:" << message;
      return ss.str();
    }
  };

} // namespace soda
