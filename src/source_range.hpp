#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <ostream>
#include <sstream>
#include <string>

namespace soda {

  struct source_position {
    std::size_t offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;

    bool operator==(source_position const &other) const {
      return offset == other.offset && line == other.line &&
             column == other.column;
    }

    bool operator!=(source_position const &other) const {
      return !operator==(other);
    }
  };

  inline std::ostream &operator<<(std::ostream &out,
                                  source_position const &pos) {
    return out << pos.line << ':' << pos.column;
  }

  inline std::string to_string(source_position const &pos) {
    std::stringstream ss;
    ss << pos;
    return ss.str();
  }

  struct source_range {
    std::filesystem::path filename;
    source_position start;
    source_position end;

    source_range() = default;

    source_range(std::filesystem::path filename, source_position start,
                 source_position end)
        : filename{std::move(filename)}, start{std::move(start)},
          end{std::move(end)} {
    }

    source_range(source_range const &other)
        : filename{other.filename}, start{other.start}, end{other.end} {
    }

    source_range(source_range &&other)
        : filename{std::move(other.filename)}, start{std::move(other.start)},
          end{std::move(other.end)} {
    }

    source_range &operator=(source_range const &other) {
      if (&other != this) {
        filename = other.filename;
        start = other.start;
        end = other.end;
      }
      return *this;
    }

    source_range &operator=(source_range &&other) noexcept {
      std::swap(filename, other.filename);
      std::swap(start, other.start);
      std::swap(end, other.end);
      return *this;
    }

    bool operator==(source_range const &other) const {
      return filename == other.filename && start == other.start &&
             end == other.end;
    }

    bool operator!=(source_range const &other) const {
      return !operator==(other);
    }
  };

  inline std::ostream &operator<<(std::ostream &out,
                                  source_range const &range) {
    if (range.start == range.end) {
      if (range.filename.empty()) {
        out << range.start;
      } else {
        out << range.filename.c_str() << ':' << range.start;
      }
    } else {
      if (range.filename.empty()) {
        out << range.start.line << '.' << range.start.column << '-'
            << range.end.line << '.' << range.end.column;
      } else {
        out << range.filename.c_str() << ':' << range.start.line << '.'
            << range.start.column << '-' << range.end.line << '.'
            << range.end.column;
      }
    }
    return out;
  }

  inline std::string to_string(source_range const &range) {
    std::stringstream ss;
    ss << range;
    return ss.str();
  }

} // namespace soda
