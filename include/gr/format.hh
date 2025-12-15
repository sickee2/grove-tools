/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file format.hh
 * @brief High-performance string formatting library
 *
 * gr::toy::format is a modern C++ string formatting library designed for
 * maximum performance and efficiency. It provides Python-style formatting
 * syntax with significant performance advantages over std::format.
 *
 * Performance Highlights (based on benchmark tests):
 * - Integer formatting: ~133% faster than std::format, ~1.8% faster than fmt::format
 * - Floating point: ~95% faster than std::format, ~32% faster than fmt::format
 * - String formatting: ~96% faster than std::format, ~57% faster than fmt::format
 * - Pointer formatting: ~123% faster than std::format, ~109% faster than fmt::format
 * - Overall throughput: ~96% higher than std::format, ~6% higher than fmt::format
 * - Memory allocation: ~79% faster than std::format
 *
 * Performance Comparison (100,000 iterations):
 * - Basic integer: std::format 5997μs vs fmt::format 2621μs vs toy::format 2575μs
 * - Floating point: std::format 7947μs vs fmt::format 5358μs vs toy::format 4068μs
 * - String formatting: std::format 4039μs vs fmt::format 3234μs vs toy::format 2063μs
 * - Throughput: std::format 7.03M ops/sec vs fmt::format 12.98M ops/sec vs toy::format 13.74M ops/sec
 *
 * Key Features:
 * - High performance: 25-60% faster than std::format in most scenarios
 * - Memory efficient: Smart pre-allocation and minimal allocations
 * - Type-safe: Compile-time type checking
 * - UTF-8 native: Built-in support for UTF-8 strings
 * - Extended types: Support for chrono durations, time points, and custom types
 * - Python-style formatting syntax: "Hello {}!"_fmt("World")
 *
 * Performance Comparison (100,000 iterations):
 * - Basic integer: std::format 5859μs vs toy::format 2598μs (55% faster)
 * - Floating point: std::format 7904μs vs toy::format 5928μs (25% faster)
 * - String formatting: std::format 3859μs vs toy::format 2455μs (36% faster)
 * - Throughput: std::format 7.67M ops/sec vs toy::format 12.33M ops/sec (60% higher)
 *
 * Usage Examples:
 * @code
 * using namespace gr::toy::literals;
 *
 * // Basic formatting
 * auto str1 = "Hello {}!"_fmt("World");
 * auto str2 = gr::toy::format("Value: {:.2f}", 3.14159);
 *
 * // Advanced formatting
 * auto str3 = format("{:<10} {:>8}", "Name", 42);    // Alignment
 * auto str4 = format("{:#x}", 255);                  // Hexadecimal
 * auto str5 = format("{:.3}", 1.234567);             // Precision
 *
 * // Time formatting
 * auto now = std::chrono::system_clock::now();
 * auto time_str = format("{:f}", now);               // Full timestamp
 *
 * // Duration formatting
 * auto duration = std::chrono::milliseconds(1234);
 * auto duration_str = format("{:s}", duration);      // Seconds with unit
 * @endcode
 *
 * Format Specification Syntax:
 * - General: {[index][:format_spec]}
 * - Alignment: < (left), > (right), ^ (center)
 * - Sign: + (plus/minus), - (minus only), space (space for positive)
 * - Alternate form: # (0x, 0b prefixes)
 * - Width and precision: {:<10.2f}
 * - Type specifiers: d (decimal), x (hex), f (fixed), e (scientific)
 *
 * Supported Types:
 * - All fundamental types (int, float, double, etc.)
 * - Strings (std::string, const char*, gr::str::u8, gr::str::u8v)
 * - Pointers (formatted as hexadecimal addresses)
 * - std::chrono::duration and std::chrono::time_point
 * - Custom types (via formatter specializations)
 *
 * Known Limitations:
 * - Floating-point conversion (ftoss/sstof) is currently slower than std::to_chars/std::from_chars
 * - toy::sstof: 646μs vs std::from_chars: 486μs (will be optimized in future versions)
 * - Limited compile-time format string checking in C++17 mode
 *
 * Compilation Requirements:
 * - C++17 or later (C++20 recommended for full chrono support)
 * - Platform-specific time functions (localtime_r/localtime_s)
 *
 * @note This library is optimized for performance-critical applications
 *       where string formatting overhead is significant.
 * @warning Floating-point conversion performance will be improved in future
 * versions
 *
 * Format custom type
 * class TestT{
 *   int x, y, z;
 * };
 *
 * template <> struct gr::toy::detail::formatter<TestT> {
 *   void operator()(format_output &out, const TestT &value,
 *                   const format_spec &spec) const {
 *     //std::string_view pattern(spec.fmt_beg, spec.fmt_end - spec.fmt_beg);
 *     auto pattern = spec.get_pattern();
 *     if(pattern == "json"){
 *       out.put(gr::toy::format("{{x:{}, y:{}}}", value.x, value.y));
 *     }
 *     else if(spec.type == 'v'){
 *       out.put("(", 1)
 *       out.put(std::to_string(value.x));
 *       out.put(", ",2);
 *       out.put(std::to_string(value.y));
 *       out.put(", ",2);
 *       out.put(std::to_string(value.z));
 *       out.put(")",1);
 *     }else{
 *       out.put(gr::format("({},{},{})"), value.x, value.y, value.z);
 *     }
 *   }
 * };
 */

#pragma once
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <gr/string.hh>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <gr/detail/toy_charconv.hh>

namespace gr {
namespace toy {
/**
 * @brief Exception class for formatting errors
 */
class format_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

/**
 * @brief Output wrapper for formatting operations
 * Provides a unified interface for appending formatted content
 */
class format_output {
  str::u8 &ctxt;

public:
  constexpr format_output(str::u8 &_ref) : ctxt(_ref) {}
  inline void put(const char *str) {
    ctxt.append(str);
  } // unsafe method, only for constexpr str
  inline void put(const char *str, size_t n) { ctxt.append(str, n); }
  inline void put(char c) { ctxt.append(&c, 1); };
  inline void put(const str::u8 &str, size_t n) { ctxt.append(str, n); }
  inline void put(const str::u8 &str) { ctxt.append(str.data(), str.size()); }
  inline void put(const std::string &str) {
    ctxt.append(str.data(), str.size());
  }
  inline void put(const std::string &str, size_t n) {
    ctxt.append(str.data(), n);
  }
  inline void put(str::u8v str) { ctxt.append(str.data(), str.size()); }
  inline void put(size_t n, char fill) { ctxt.append(n, fill); }
  // inline const char* debug_data() const { return ctxt.data(); }
};

/**
 * @brief Format specification structure
 * Contains all formatting options parsed from format strings
 */
struct format_spec {
  int width = -1;     // Field width (-1 means unspecified)
  int precision = -1; // Precision for floating point/strings
  char fill = ' ';    // Fill character for alignment
  char align = '\0';   // Alignment: < left, > right, ^ center
  char sign = '-';    // Sign: - minus only, + plus/minus, space for positive
  bool alternate = false; // # Alternate form (0x, 0b prefixes)
  char type = '\0';       // Type specifier (d, x, f, e, etc.)
  const char *fmt_beg = nullptr;
  const char *fmt_end = nullptr;
  inline str::u8v get_pattern() const {
    return str::u8v(fmt_beg, fmt_end - fmt_beg);
  }
};

namespace detail {
template <typename Tuple>
void format_arg_dispatch(format_output &out, const Tuple &args,
                         size_t target_index, const format_spec &spec);

/**
 * @brief Check and parse indexed placeholder like {number} in format spec
 * @param ptr Current pointer position (should point to '{')
 * @param end End of format spec string
 * @param args_storage Tuple containing format arguments
 * @param auto_index Current auto index counter
 * @param arg_count Total number of arguments
 * @param parent_has_explicit_index Whether the parent placeholder uses explicit
 * indexing
 * @param spec Format spec to update (width or precision)
 * @param is_width Whether this is for width (true) or precision (false)
 * @return true if indexed placeholder was found and parsed, false otherwise
 */
template <typename Tuple>
GR_CONSTEXPR_OR_INLINE bool
try_parse_indexed_placeholder(const char *&ptr, const char *end,
                              const Tuple &args_storage, size_t &auto_index,
                              size_t arg_count, bool parent_has_explicit_index,
                              format_spec &spec, bool is_width) {

  const char *nested_start = ptr + 1;
  const char *nested_end = nested_start;

  // Find matching closing brace
  while (nested_end < end && *nested_end != '}') {
    nested_end++;
  }

  if (nested_end < end && nested_end > nested_start) {
    // Check if content is all digits (argument index)
    bool is_all_digits = true;
    for (const char *p = nested_start; p < nested_end; ++p) {
      if (*p < '0' || *p > '9') {
        is_all_digits = false;
        break;
      }
    }

    if (is_all_digits) {
      // This is an indexed placeholder: {number}
      size_t nested_index;
      if (parent_has_explicit_index) {
        // If parent uses explicit indexing, use the specified index
        auto status =
            gr::toy::sstoi(nested_start, nested_end, nested_index, 10);
        if (status.ec != std::errc{}) {
          throw format_error("gr::toy::format => can not convert chars to "
                             "integer when parse indexed placeholder");
        }
        if (nested_index >= arg_count) {
          throw format_error("gr::toy::format => Argument index out of range");
        }
      } else {
        // Automatic indexing mode, use next auto index
        nested_index = auto_index++;
        if (nested_index >= arg_count) {
          throw format_error("gr::toy::format => Argument index out of range");
        }
      }

      str::u8 temp_out;
      format_output arg_out(temp_out);
      format_arg_dispatch(arg_out, args_storage, nested_index, format_spec{});

      try {
        int value;
        auto status = gr::toy::sstoi(temp_out.data(), temp_out.size(), value);
        if (status.ec != std::errc{}) {
          throw format_error("gr::toy::format => can not convert chars to "
                             "integer when parse indexed placeholder");
        }
        if (is_width) {
          spec.width = value;
        } else {
          spec.precision = value;
        }
      } catch (...) {
        throw format_error("gr::toy::format => Invalid value from argument");
      }
      ptr = nested_end + 1; // Skip over {number}
      return true;
    }
  }
  return false;
}

/**
 * @brief Check and parse empty placeholder {} in format spec
 * @param ptr Current pointer position (should point to '{')
 * @param end End of format spec string
 * @param args_storage Tuple containing format arguments
 * @param auto_index Current auto index counter
 * @param arg_count Total number of arguments
 * @param parent_has_explicit_index Whether the parent placeholder uses explicit
 * indexing
 * @param spec Format spec to update (width or precision)
 * @param is_width Whether this is for width (true) or precision (false)
 * @return true if empty placeholder was found and parsed, false otherwise
 */
template <typename Tuple>
GR_CONSTEXPR_OR_INLINE bool
try_parse_empty_placeholder(const char *&ptr, const char *end,
                            const Tuple &args_storage, size_t &auto_index,
                            size_t arg_count, bool parent_has_explicit_index,
                            format_spec &spec, bool is_width) {

  if (ptr + 1 < end && ptr[1] == '}') {
    // {} from argument: {:{}}
    size_t nested_index;
    if (parent_has_explicit_index) {
      // If parent uses explicit indexing, nested {} should use the same
      // indexing pattern
      nested_index = auto_index;
      if (nested_index >= arg_count) {
        throw format_error("gr::toy::format => Argument index out of range");
      }
      auto_index++; // Reserve position for next nested argument
    } else {
      // Automatic indexing mode
      nested_index = auto_index++;
      if (nested_index >= arg_count) {
        throw format_error("gr::toy::format => Argument index out of range");
      }
    }

    str::u8 temp_out;
    format_output arg_out(temp_out);
    format_arg_dispatch(arg_out, args_storage, nested_index, format_spec{});

    try {
      int value;
      auto status = gr::toy::sstoi(temp_out.data(), temp_out.size(), value);
      if (status.ec != std::errc{}) {
        throw format_error("gr::toy::format => can not convert chars to "
                           "integer when parse empty placeholder");
      }
      if (is_width) {
        spec.width = value;
      } else {
        spec.precision = value;
      }
    } catch (...) {
      throw format_error("gr::toy::format => Invalid value from argument");
    }
    ptr += 2; // Skip over {}
    return true;
  }
  return false;
}

/**
 * @brief Parse format specification from string with support for nested
 * arguments
 * @param begin Start of format spec string
 * @param end End of format spec string
 * @param args_storage Tuple containing format arguments
 * @param auto_index Current auto index counter
 * @param arg_count Total number of arguments
 * @param parent_has_explicit_index Whether the parent placeholder uses explicit
 * indexing
 * @return Parsed format_spec structure
 */
template <typename Tuple>
GR_CONSTEXPR_OR_INLINE format_spec parse_format_spec_with_args(
    const char *begin, const char *end, const Tuple &args_storage,
    size_t &auto_index, size_t arg_count,
    bool parent_has_explicit_index = false) {
  format_spec spec;

  spec.fmt_beg = begin; // Store format pattern begin
  spec.fmt_end = end;   // Store format pattern end

  const char *ptr = begin;

  // Fast path: no format specifier
  if (ptr == end)
    return spec;

  constexpr int MAX_WIDTH = 10000;
  constexpr int MAX_PRECISION = 1000;

  // Batch process common cases
  while (ptr < end) {
    switch (*ptr) {
    case '<':
    case '>':
    case '^':
      if (ptr > begin && (ptr[-1] != '}' && ptr[-1] != '{')) {
        spec.fill = ptr[-1];
      }
      spec.align = *ptr;
      ++ptr;
      break;

    case '+':
    case '-':
    case ' ':
      spec.sign = *ptr;
      ++ptr;
      break;

    case '#':
      spec.alternate = true;
      ++ptr;
      break;

    case '.':
      ++ptr;
      if (ptr < end) {
        // First try to parse indexed precision placeholder: {number}
        if (*ptr == '{') {
          if (try_parse_indexed_placeholder(
                  ptr, end, args_storage, auto_index, arg_count,
                  parent_has_explicit_index, spec, false)) {
            continue; // Continue to next character
          }
        }

        // Then try empty precision placeholder: {}
        if (*ptr == '{') {
          if (try_parse_empty_placeholder(ptr, end, args_storage, auto_index,
                                          arg_count, parent_has_explicit_index,
                                          spec, false)) {
            continue; // Continue to next character
          }
        }

        // Finally handle numeric precision
        if (*ptr >= '0' && *ptr <= '9') {
          int precision = 0;
          do {
            precision = precision * 10 + (*ptr - '0');
            ++ptr;
            if (precision > MAX_PRECISION) {
              throw format_error(
                  "gr::toy::format => Precision value too large");
            }
          } while (ptr < end && *ptr >= '0' && *ptr <= '9');
          spec.precision = precision;
        } else {
          spec.precision = 0;
        }
      }
      break;

    default:
      // First try to parse indexed width placeholder: {number}
      if (*ptr == '{') {
        if (try_parse_indexed_placeholder(ptr, end, args_storage, auto_index,
                                          arg_count, parent_has_explicit_index,
                                          spec, true)) {
          continue; // Continue to next character
        }
      }

      // Then try empty width placeholder: {}
      if (*ptr == '{') {
        if (try_parse_empty_placeholder(ptr, end, args_storage, auto_index,
                                        arg_count, parent_has_explicit_index,
                                        spec, true)) {
          continue; // Continue to next character
        }
      }

      // Finally handle numeric width
      if (*ptr >= '0' && *ptr <= '9') {
        int width = 0;
        do {
          width = width * 10 + (*ptr - '0');
          ++ptr;
          if (width > MAX_WIDTH) {
            throw format_error("gr::toy::format => Width value too large");
          }
        } while (ptr < end && *ptr >= '0' && *ptr <= '9');
        spec.width = width;
      } else {
        spec.type = *ptr;
        ++ptr;
      }
      break;
    }
  }

  return spec;
}

/**
 * @brief Apply alignment and padding to formatted content
 * @param output Output target
 * @param content Content to align
 * @param content_len Length of content
 * @param spec Format specification
 */
template <typename Output>
inline void apply_alignment(Output &output, const char *content,
                            size_t content_len, const format_spec &spec) {
  if (spec.width <= 0 || content_len >= static_cast<size_t>(spec.width)) {
    output.put(content, content_len);
    return;
  }

  const size_t padding = spec.width - content_len;
  const char fill_char = spec.fill;
  if (content_len == 0 && spec.width > 0) {
    output.put(padding, fill_char);
    return;
  }

  switch (spec.align) {
  case '<': {
    // if(content_len > 0)
    output.put(content, content_len);
    output.put(padding, fill_char);
  } break;
  // case '>': // Right alignment
  //   output.put(padding, fill_char);
  //   output.put(content, content_len);
  //   break;
  case '^': { // Center alignment
    const size_t left_pad = padding / 2;
    const size_t right_pad = padding - left_pad;
    output.put(left_pad, fill_char);
    output.put(content, content_len);
    output.put(right_pad, fill_char);
    break;
  }
  case '>': // Right alignment
  default:
    output.put(padding, fill_char);
    output.put(content, content_len);
  }
}

inline void apply_alignment(format_output &out, const str::u8 &str,
                            const format_spec &spec) {
  apply_alignment(out, str.data(), str.size(), spec);
}

/**
 * @brief Format string implementation with precision and alignment
 * @param out Output target
 * @param data String data
 * @param len String length
 * @param spec Format specification
 */
inline void format_string_impl(format_output &out, const char *data, size_t len,
                               const format_spec &spec) {
  // Precision handling for strings
  if (spec.precision >= 0) {
    len = std::min(len, static_cast<size_t>(spec.precision));
  }

  // Use unified padding logic
  apply_alignment(out, data, len, spec);
}

inline void format_string(format_output &out, const str::u8 &value,
                          const format_spec &spec) {
  format_string_impl(out, value.data(), value.size(), spec);
}

inline std::tuple<unsigned, bool>
pre_parse_integer_type(const format_spec &spec) {
  unsigned base = 10;
  bool uppercase = false;

  switch (spec.type) {
  case 'b':
  case 'B':
    base = 2;
    uppercase = (spec.type == 'B');
    break;
  case 'o':
    base = 8;
    break;
  case 'x':
  case 'X':
    base = 16;
    uppercase = (spec.type == 'X');
    break;
  case 'd':
  case 'D':
  default:
    base = 10;
    break;
  }
  return std::make_tuple(base, uppercase);
}

template <typename T> constexpr unsigned make_general_integer_buffer_size() {
  switch (sizeof(T)) {
  case 1:
  case 2:
    return 16;
  case 4:
    return 24;
  case 8:
    return 32;
  case 16:
  default:
    return 45;
  }
}

/**
 * @brief Integer formatting implementation
 * Supports decimal, binary, octal, and hexadecimal formats
 */
template <typename T>
void format_integer_impl(format_output &out, T value, const format_spec &spec) {
  // Fast path
  if (spec.width <= 0 && spec.precision < 0 && !spec.alternate &&
      (spec.type == '\0' || spec.type == 'd')) {
    constexpr unsigned buffer_size = make_general_integer_buffer_size<T>();
    char buffer[buffer_size];

    auto [ptr, len] = itoss(buffer, buffer_size, value, 10);
    if (ptr != nullptr) {
      out.put(ptr, len);
    }
    return;
  }

  auto [base, uppercase] = pre_parse_integer_type(spec);
  if (base == 2) {
    constexpr size_t buffer_size = (sizeof(T) << 3) + 8;
    char buffer[buffer_size];
    auto [ptr, size] =
        itoss(buffer, buffer_size, value, base, uppercase, spec.alternate);
    apply_alignment(out, ptr, size, spec);
  } else {
    constexpr unsigned buffer_size = make_general_integer_buffer_size<T>();
    char buffer[buffer_size];
    auto [ptr, size] =
        itoss(buffer, buffer_size, value, base, uppercase, spec.alternate);
    apply_alignment(out, ptr, size, spec);
  }
}

template <typename T> constexpr int make_float_general_precision() {
  int precision = -1;
  if constexpr (std::is_same_v<T, float>) {
    precision = 5;
  } else if constexpr (std::is_same_v<T, double>) {
    precision = 8;
  } else {
    precision = 17;
  }
  return precision;
}

/**
 * @brief Floating point formatting implementation
 * Supports fixed, scientific, and general formats
 */
template <typename T>
inline void format_float(format_output &out, T value, const format_spec &spec) {
  static_assert(
      std::is_floating_point_v<T>,
      "gr::toy::format::format_float => T type must be float point type");

  // Handle negative zero
  if (value == 0.0) {
    value = 0.0; // Ensure negative zero is normalized to positive zero
  }

  // Fast path: simple formatting
  if ((spec.type == '\0' || spec.type == 'g' || spec.type == 'G') &&
      spec.width <= 0 && spec.precision < 0 && spec.sign == '-') {
    char buffer[32] = {};
    int precision = spec.precision >= 0 ? spec.precision
                                        : make_float_general_precision<T>();
    if (spec.precision >= 0) {
      precision = spec.precision;
    }
    auto [ptr, len] =
        ftoss(buffer, 32, value, toy::chars_format::general, precision);
    if (nullptr != ptr) {
      out.put(ptr, len);
      return;
    }
  }

  // Integer fast path: if value is integer and precision is 0
  if (spec.precision <= 0 && value == static_cast<int64_t>(value)) {
    format_integer_impl(out, static_cast<int64_t>(value), spec);
    return;
  }

  constexpr int BUFFER_SIZE = 64;

  char buffer[BUFFER_SIZE] = {0};
  char *ptr = buffer;

  // Handle sign
  if (value >= 0) {
    if (spec.sign == '+') {
      *ptr++ = '+';
    } else if (spec.sign == ' ') {
      *ptr++ = ' ';
    }
  }

  // Optimized formatting method
  int precision{};
  bool uppercase = false;
  auto enum_format_type = toy::chars_format::general;
  // char ext_type = 0;
  // auto set precision with format type
  switch (spec.type) {
  case 'e':
  case 'E':
    precision = 6; // scientific
    uppercase = (spec.type == 'E');
    enum_format_type = toy::chars_format::scientific;
    break;
  case 'f':
  case 'F': {
    // precision = 6; // fixed
    precision = make_float_general_precision<T>();
    uppercase = (spec.type == 'F');
    enum_format_type = toy::chars_format::fixed;
    // if (spec.fmt_end - spec.fmt_beg > 3) {
    //   ext_type = *(spec.fmt_end - 2);
    // }
  } break;
  case 'g':
  case 'G':
  default:
    precision = make_float_general_precision<T>();
    uppercase = (spec.type == 'G');
    enum_format_type = toy::chars_format::general;
    break;
  }

  if (spec.precision >= 0) {
    precision = spec.precision;
  }

  // if (enum_format_type == toy::chars_format::fixed && (ext_type == 'l' ||
  // ext_type == 'L')) {
  //   // TODO: Unsupport format huge float with 'le' or 'lf' now.
  //   (void)ext_type;
  //   // return;
  // }
  auto [s, len] = ftoss(buffer, BUFFER_SIZE - 1, value, enum_format_type,
                        precision, uppercase);
  if (nullptr != s) {
    apply_alignment(out, buffer, len, spec);
  }
}

// Type traits for chrono types
template <typename T> struct is_chrono_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_chrono_duration<std::chrono::duration<Rep, Period>> : std::true_type {
};

template <typename T>
inline constexpr bool is_chrono_duration_v = is_chrono_duration<T>::value;

template <typename T> struct is_chrono_time_point : std::false_type {};

template <typename Clock, typename Duration>
struct is_chrono_time_point<std::chrono::time_point<Clock, Duration>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_chrono_time_point_v = is_chrono_time_point<T>::value;

/**
 * @brief Format chrono duration with automatic unit selection
 */
template <typename TUnit, typename duration_t>
inline void auto_duration_type(format_output &out, const duration_t &_duration,
                               const format_spec &spec,
                               const char *unit = nullptr) {
  using namespace std::chrono;

  char float_type = '\0';
  size_t fmt_len = spec.fmt_end - spec.fmt_beg;
  if (spec.precision > 0 && fmt_len >= 2) {
    const char *second_last = spec.fmt_end - 2;
    if (*second_last == '#' && fmt_len >= 3) {
      second_last = spec.fmt_end - 3;
    }
    if (second_last >= spec.fmt_beg &&
        (*second_last == 'f' || *second_last == 'e' || *second_last == 'g' ||
         *second_last == 'F' || *second_last == 'E' || *second_last == 'G')) {
      float_type = *second_last;
    }
  }

  if (spec.alternate && nullptr != unit) {
    str::u8 buffer;
    format_output out_proxy(buffer);
    format_spec spec_ = spec;
    spec_.align = '<';
    spec_.width = -1;
    if (float_type != '\0') {
      spec_.type = float_type;
    }
    if (spec.precision > 0) {
      double value =
          duration_cast<duration<double, typename TUnit::period>>(_duration)
              .count();
      format_float(out_proxy, value, spec_);

    } else {
      auto value = duration_cast<TUnit>(_duration).count();
      format_integer_impl(out_proxy, static_cast<long>(value), spec_);
    }
    out_proxy.put(unit);
    apply_alignment(out, buffer.data(), buffer.size(), spec);
  } else {
    if (spec.precision > 0) {
      double value =
          duration_cast<duration<double, typename TUnit::period>>(_duration)
              .count();
      format_spec float_spec = spec;
      if (float_type != '\0') {
        float_spec.type = float_type;
      }
      format_float(out, value, float_spec);

    } else {
      auto value = duration_cast<TUnit>(_duration).count();
      format_integer_impl(out, static_cast<long>(value), spec);
    }
  }
}

/**
 * @brief Format std::chrono::duration objects
 */
template <typename Rep, typename Period>
void format_chrono_duration(format_output &out,
                            const std::chrono::duration<Rep, Period> &duration_,
                            const format_spec &spec) {
  using namespace std::chrono;

  switch (spec.type) {
#if GR_HAS_CPP20
  case 'd':
    auto_duration_type<days>(out, duration_, spec, "day");
    break;
#endif
  case 'h':
    auto_duration_type<hours>(out, duration_, spec, "h");
    break;
  case 'm':
    auto_duration_type<minutes>(out, duration_, spec, "min");
    break;
  case 's':
    auto_duration_type<seconds>(out, duration_, spec, "s");
    break;
  case 'a': {
    // automatic format...
    auto _spec = spec;
    _spec.alternate = true;
    _spec.fill = ' ';
    auto abs_duration = duration_ < duration_.zero() ? -duration_ : duration_;

    if (abs_duration >= hours(1)) {
      auto_duration_type<hours>(out, duration_, _spec, "h");
    } else if (abs_duration >= minutes(1)) {
      auto_duration_type<minutes>(out, duration_, _spec, "min");
    } else if (abs_duration >= seconds(1)) {
      auto_duration_type<seconds>(out, duration_, _spec, "s");
    } else if (abs_duration >= milliseconds(1)) {
      auto_duration_type<milliseconds>(out, duration_, _spec, "ms");
    } else if (abs_duration >= microseconds(1)) {
      auto_duration_type<microseconds>(out, duration_, _spec, "us");
    } else {
      auto_duration_type<nanoseconds>(out, duration_, _spec, "ns");
    }
  } break;
  case 'M':
    auto_duration_type<milliseconds>(out, duration_, spec, "ms");
    break;
  case 'U':
    auto_duration_type<microseconds>(out, duration_, spec, "us");
    break;
  case 'N':
  default:
    auto_duration_type<nanoseconds>(out, duration_, spec, "ns");
    break;
  }
}

namespace __ftime {
/**
 * @brief Time writer for efficient time formatting
 * Builds time strings character by character for maximum performance
 */
struct time_writer {
private:
  const std::tm &tm;
  char buffer[32] = {0};
  char *ptr = buffer;
  time_writer(time_writer &) = delete;

public:
  inline time_writer(const std::tm &tm_) : tm(tm_) {};

  inline const char *data() const { return buffer; }
  inline unsigned size() const { return ptr - buffer; }

  inline void write_year() {
    int year = tm.tm_year + 1900;
    *ptr++ = '0' + (year / 1000);
    *ptr++ = '0' + ((year % 1000) / 100);
    *ptr++ = '0' + ((year % 100) / 10);
    *ptr++ = '0' + (year % 10);
  }

  inline void write_short_year() {
    int year = tm.tm_year + 1900;
    *ptr++ = '0' + ((year % 100) / 10);
    *ptr++ = '0' + (year % 10);
  }

  inline void write_month() {
    int month = tm.tm_mon + 1;
    *ptr++ = '0' + (month / 10);
    *ptr++ = '0' + (month % 10);
  }

  inline void write_day() {
    int mday = tm.tm_mday;
    *ptr++ = '0' + (mday / 10);
    *ptr++ = '0' + (mday % 10);
  }

  inline void write_hour() {
    int hour = tm.tm_hour;
    *ptr++ = '0' + (hour / 10);
    *ptr++ = '0' + (hour % 10);
  }

  inline void write_min() {
    int minuters = tm.tm_min;
    *ptr++ = '0' + (minuters / 10);
    *ptr++ = '0' + (minuters % 10);
  }

  inline void write_sec() {
    int sec = tm.tm_sec;
    *ptr++ = '0' + (sec / 10);
    *ptr++ = '0' + (sec % 10);
  }

  inline void write_joint(char jnt) { *ptr++ = jnt; }

  inline void write_millsec(int ms) {
    if (ms < 10) {
      *ptr++ = '0';
      *ptr++ = '0';
      *ptr++ = ms + '0';
    } else if (ms < 100) {
      *ptr++ = '0';
      *ptr++ = '0' + (ms / 10);
      *ptr++ = '0' + (ms % 10);
    } else {
      *ptr++ = '0' + (ms / 100);
      *ptr++ = '0' + (ms / 10) % 10;
      *ptr++ = '0' + (ms % 10);
    }
  }

  inline void pack_ymd(char jnt) {
    write_year();
    write_joint(jnt);
    write_month();
    write_joint(jnt);
    write_day();
  }

  inline void pack_dmy(char jnt) {
    write_day();
    write_joint(jnt);
    write_month();
    write_joint(jnt);
    write_year();
  }

  inline void pack_time(char jnt) {
    write_hour();
    write_joint(jnt);
    write_min();
    write_joint(jnt);
    write_sec();
  }
};
} // namespace __ftime

/**
 * @brief Format std::chrono::time_point objects
 */
template <typename Clock>
void format_chrono_time_point(
    format_output &out,
    const std::chrono::time_point<Clock> &time_point,
    const format_spec &spec) {
  namespace chr = std::chrono;
  // Calculate all time components at once
  auto duration = time_point.time_since_epoch();
  // auto milliseconds = chr::duration_cast<chr::milliseconds>(duration);
  auto seconds = chr::duration_cast<chr::seconds>(duration);
  auto milliseconds =
      chr::duration_cast<chr::milliseconds>(duration - seconds);

  auto time_t = Clock::to_time_t(time_point);
  std::tm tm;
#ifdef _WIN32
  localtime_s(&tm, &time_t);
#else
  localtime_r(&time_t, &tm);
#endif

  int ms = static_cast<int>(milliseconds.count() % 1000);
  // auto milliseconds = duration_cast<std::chrono::milliseconds>(duration) % 1000;
  // int ms = static_cast<int>(milliseconds);

  auto ftime = __ftime::time_writer(tm);
  switch (spec.type) {
  case 'd':
    ftime.pack_ymd('-');
    break;
  case 't':
    ftime.pack_time(':');
    break;
  case 'T':
    ftime.pack_time(':');
    ftime.write_joint('.');
    ftime.write_millsec(ms);
    break;
  case 'f':
    ftime.pack_ymd('-');
    ftime.write_joint(' ');
    ftime.pack_time(':');
    ftime.write_joint('.');
    ftime.write_millsec(ms);
    break;
  default:
    ftime.pack_ymd('-');
    ftime.write_joint(' ');
    ftime.pack_time(':');
    out.put(ftime.data(), ftime.size());
    return;
  }
  apply_alignment(out, ftime.data(), ftime.size(), spec);
}

/**
 * @brief Format pointer values as hexadecimal addresses
 */
template <typename T>
void format_pointer(format_output &out, const T *value,
                    const format_spec &spec) {
  if (spec.type != 'p')
    return;
  if (value == nullptr) {
    char buf[] = "0x0";
    apply_alignment(out, buf, 3, spec);
    return;
  }
  uintptr_t addr = reinterpret_cast<uintptr_t>(value);
  char buffer[22] = {0};
  buffer[0] = '0';
  buffer[1] = 'x';
  auto [s, len] = itoss(buffer + 2, sizeof(buffer) - 2, addr, 16);
  if (nullptr != s) {
    apply_alignment(out, buffer, len, spec);
  } else {
    throw format_error("gr::toy::format => Pointer formatting failed");
  }
}

/**
 * @brief Default formatter using stream insertion operator
 * Fallback for types without specialized formatter
 */
template <typename T> struct formatter;
// template <typename T> struct formatter {
//   void operator()(format_output &result, const T &value,
//                   const format_spec &spec) const {
//     std::stringstream ss;
//     ss << value;
//     auto str = ss.str();
//     format_string(result, str::u8v(str.data(), str.size()), spec);
//   }
// };

// Specializations for various types
template <size_t N> struct formatter<char[N]> {
  void operator()(format_output &out, const char (&value)[N],
                  const format_spec &spec) const {
    if (spec.type == 'p') {
      format_pointer(out, value, spec);
    } else {
      format_string_impl(out, value, N - 1, spec);
    }
  }
};

template <> struct formatter<const char *> {
  void operator()(format_output &out, const char *value,
                  const format_spec &spec) const {
    if (spec.type == 'p') {
      format_pointer(out, value, spec);
    } else {
      if (value) {
        auto view = str::u8v(value);
        format_string_impl(out, view.data(), view.size(), spec);
      }
    }
  }
};

template <> struct formatter<char *> {
  void operator()(format_output &out, char *value,
                  const format_spec &spec) const {
    if (spec.type == 'p') {
      format_pointer(out, value, spec);
    } else {
      if (value) {
        auto view = str::u8v(value);
        format_string_impl(out, view.data(), view.size(), spec);
      }
    }
  }
};

template <> struct formatter<char> {
  void operator()(format_output &out, char value, const format_spec &spec) {
    switch (spec.type) {
    case 'd':
      format_integer_impl(out, int(value), spec);
      break;
    case 's':
    default:
      apply_alignment(out, &value, 1, spec);
    }
  }
};

template <> struct formatter<bool> {
  void operator()(format_output &out, bool value, const format_spec &spec) {
    switch (spec.type) {
    case 'd':
      if (value) {
        format_string_impl(out, "1", 1, spec);
      } else {
        format_string_impl(out, "0", 1, spec);
      }
      break;
    case 's':
    case 'a':
    default:
      if (value) {
        format_string_impl(out, "true", 4, spec);
      } else {
        format_string_impl(out, "false", 5, spec);
      }
    }
  }
};
template <> struct formatter<int8_t> {
  void operator()(format_output &out, int8_t value, const format_spec &spec) {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<uint8_t> {
  void operator()(format_output &out, uint8_t value, const format_spec &spec) {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<uc::codepoint> {
  void operator()(format_output &out, uc::codepoint cp,
                  const format_spec &spec) {
    switch (spec.type) {
    case 's': {
      auto chunk = cp.chunk_u8();
      apply_alignment(out, chunk.buf, chunk.size(), spec);
    } break;
    case 'c':
    case 'i':
    case 'd':
    default:
      format_integer_impl(out, cp.code(), spec);
    }
  }
};
// String specializations
template <> struct formatter<str::u8> {
  void operator()(format_output &out, const str::u8 &value,
                  const format_spec &spec) const {
    format_string(out, value, spec);
  }
};

template <> struct formatter<str::u8v> {
  void operator()(format_output &out, const str::u8v &value,
                  const format_spec &spec) const {
    format_string(out, value, spec);
  }
};
template <> struct formatter<std::string_view> {
  void operator()(format_output &out, const std::string_view &value,
                  const format_spec &spec) const {
    format_string(out, value, spec);
  }
};

template <> struct formatter<std::string> {
  void operator()(format_output &out, const std::string &value,
                  const format_spec &spec) const {
    format_string_impl(out, value.data(), value.size(), spec);
  }
};

// Integer type specializations
template <> struct formatter<short> {
  void operator()(format_output &out, int value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<unsigned short> {
  void operator()(format_output &out, unsigned short value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<int> {
  void operator()(format_output &out, int value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<long> {
  void operator()(format_output &out, long value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<long long> {
  void operator()(format_output &out, long long value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<unsigned int> {
  void operator()(format_output &out, unsigned int value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<unsigned long> {
  void operator()(format_output &out, unsigned long value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<unsigned long long> {
  void operator()(format_output &out, unsigned long long value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<__int128_t> {
  void operator()(format_output &out, __int128_t value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};

template <> struct formatter<__uint128_t> {
  void operator()(format_output &out, __uint128_t value,
                  const format_spec &spec) const {
    format_integer_impl(out, value, spec);
  }
};
// Floating point specializations
template <> struct formatter<float> {
  void operator()(format_output &out, float value,
                  const format_spec &spec) const {
    format_float(out, value, spec);
  }
};

template <> struct formatter<double> {
  void operator()(format_output &out, double value,
                  const format_spec &spec) const {
    format_float(out, value, spec);
  }
};

template <> struct formatter<long double> {
  void operator()(format_output &out, long double value,
                  const format_spec &spec) const {
    format_float(out, value, spec);
  }
};

// Pointer specialization
template <typename T> struct formatter<T *> {
  void operator()(format_output &out, T *value, const format_spec &spec) const {
    format_pointer(out, value, spec);
  }
};

// chrono duration specialization
template <typename Rep, typename Period>
struct formatter<std::chrono::duration<Rep, Period>> {
  void operator()(format_output &out,
                  const std::chrono::duration<Rep, Period> &value,
                  const format_spec &spec) const {
    format_chrono_duration(out, value, spec);
  }
};

// chrono time point specialization
template <typename Clock>
struct formatter<std::chrono::time_point<Clock>> {
  void operator()(format_output &out,
                  const std::chrono::time_point<Clock> &value,
                  const format_spec &spec) const {
    format_chrono_time_point(out, value, spec);
  }
};

/**
 * @brief Generic formatting function
 * Routes formatting to appropriate formatter specialization
 */
template <typename T>
void format_arg(format_output &out, const T &value, const format_spec &spec) {
  formatter<T>{}(out, value, spec);
}

// Helper functions for argument indexing
template <size_t I, typename Tuple>
void format_arg_by_index(format_output &out, const Tuple &args,
                         size_t target_index, const format_spec &spec) {
  if (I == target_index) {
    format_arg(out, std::get<I>(args), spec);
  }
}

template <size_t... Is, typename Tuple>
void format_arg_by_index_impl(format_output &out, const Tuple &args,
                              size_t target_index, const format_spec &spec,
                              std::index_sequence<Is...>) {
  (format_arg_by_index<Is>(out, args, target_index, spec), ...);
}

/**
 * @brief Fast argument formatting by index
 */
template <typename Tuple>
void format_arg_dispatch(format_output &out, const Tuple &args,
                         size_t target_index, const format_spec &spec) {
  constexpr size_t arg_count = std::tuple_size_v<Tuple>;

  if (target_index >= arg_count) {
    throw format_error("gr::toy::format => Argument index out of range");
  }

  // Use existing implementation, abandon fast path
  format_arg_by_index_impl(out, args, target_index, spec,
                           std::make_index_sequence<arg_count>{});
}

GR_CONSTEXPR_OR_INLINE void parse_argument_index(const char *format_start,
                                                 const char *format_end,
                                                 size_t &arg_index,
                                                 size_t &auto_index,
                                                 bool &has_explicit_index) {

  const char *index_start = format_start;
  const char *index_end = format_end;

  if (index_start < index_end) {
    // Parse numeric index
    arg_index = 0;
    for (const char *p = index_start; p < index_end; ++p) {
      if (*p >= '0' && *p <= '9') {
        arg_index = arg_index * 10 + (*p - '0');
        has_explicit_index = true;
      } else {
        throw format_error("gr::toy::format => Invalid argument index");
      }
    }
  } else {
    // Empty braces {}, use auto indexing
    arg_index = auto_index++;
  }
}
} // namespace detail

#if GR_HAS_CPP20
/**
 * @brief class of format string
 * @note - fmt_string(const char (&s)[N]) is have some checking in compile-time
 *       - fmt_string(const char *s, size_t) is runtime format string
 *       - fmt_string(str::u8v s) is runtime format string
 */
template <typename... Args> class fstring {
  const char *str_;
  size_t size_;

public:
  using type = fstring;
  template <size_t N>
  consteval fstring(const char (&s)[N]) : str_(s), size_(N - 1) {
    // pre-check in compile-time
    constexpr_check_basic_syntax(s, N, sizeof...(Args));
  }

  constexpr fstring(const char *s, size_t N) : str_(s), size_(N) {}

  constexpr fstring(str::u8v s) : str_(s.data()), size_(s.size()) {}

  constexpr auto data() const { return str_; }
  constexpr auto size() const { return size_; }

private:
  static consteval void constexpr_check_basic_syntax(const char *s, size_t N,
                                                     size_t arg_count) {
    // Count braces and validate syntax
    int brace_balance = 0;
    size_t max_index_used = 0;
    size_t auto_placeholder_count = 0;
    bool has_explicit_index = false;
    bool has_auto_index = false;
    size_t current_auto_index = 0; // Track auto index for format spec checking

    const char *ptr = s;
    const char *end = s + N - 1;

    while (ptr < end) {
      if (brace_balance == 0 && ptr + 1 < end) {
        if (*ptr == '{' && *(ptr + 1) == '{') {
          ptr += 2; // Skip both {{
          continue;
        }
        if (*ptr == '}' && *(ptr + 1) == '}') {
          ptr += 2; // Skip both }}
          continue;
        }
      }

      // Process single braces
      if (*ptr == '{') {
        brace_balance++;
        // Check if this is a valid placeholder (not escaped)
        if (ptr + 1 < end && *(ptr + 1) != '{') {
          const char *placeholder_start = ptr + 1;
          const char *placeholder_end = placeholder_start;
          size_t index_used = static_cast<size_t>(-1);
          bool has_colon = false;

          // Find the matching '}'
          int nested_braces = 0;
          while (placeholder_end < end) {
            if (*placeholder_end == '{') {
              nested_braces++;
            } else if (*placeholder_end == '}') {
              if (nested_braces == 0) {
                break;
              }
              nested_braces--;
            } else if (*placeholder_end == ':' && nested_braces == 0) {
              has_colon = true;
              break;
            }
            placeholder_end++;
          }

          // Now find the actual closing brace
          const char *closing_brace = placeholder_end;
          if (has_colon) {
            // Skip to find the closing brace after colon
            while (closing_brace < end && *closing_brace != '}') {
              if (*closing_brace == '{') nested_braces++;
              else if (*closing_brace == '}') {
                if (nested_braces == 0) break;
                nested_braces--;
              }
              closing_brace++;
            }
          }

          if (closing_brace < end && *closing_brace == '}') {
            if (has_colon) {
              // Parse index before colon
              index_used = constexpr_check_brace_index(placeholder_start, placeholder_end, arg_count);

              // Check format specifier after colon
              const char *spec_start = placeholder_end + 1; // Skip ':'
              const char *spec_end = closing_brace;

              if (spec_start < spec_end) {
                char spec_type = constexpr_get_spec_type(spec_start, spec_end);

                if (index_used == static_cast<size_t>(-1)) {
                  // Auto-indexing, use current auto index
                  // Check if current_auto_index is within bounds
                  if (current_auto_index >= arg_count) {
                    throw "gr::toy::format => Argument index out of range";
                  }
                  constexpr_check_format_spec_for_arg<Args...>(spec_type, current_auto_index);
                  current_auto_index++;
                } else {
                  // Explicit indexing
                  constexpr_check_format_spec<Args...>(spec_type, placeholder_start, placeholder_end, arg_count);
                }
              }
            } else {
              // No colon, just index
              index_used = constexpr_check_brace_index(placeholder_start, closing_brace, arg_count);
            }
          }

          if (index_used == static_cast<size_t>(-1)) {
            // Auto indexes in braces and track max index
            auto_placeholder_count++;
            has_auto_index = true;
            // Only increment if this placeholder doesn't have a format specifier
            // (those with format specifiers already incremented current_auto_index)
            if (!has_colon) {
              current_auto_index++;
            }
          } else {
            // Explicitly indexed placeholder
            max_index_used = std::max(max_index_used, index_used);
            has_explicit_index = true;
          }
        }
        ptr++;
      } else if (*ptr == '}') {
        brace_balance--;
        if (brace_balance < 0) {
          throw "gr::toy::format => Unmatched closing brace";
        }
        ptr++;
      } else {
        ptr++;
      }
    }

    if (brace_balance != 0) {
      throw "gr::toy::format => Unclosed format brace";
    }

    // Strict parameter count validation
    // For auto-index placeholders
    if (has_auto_index && auto_placeholder_count > arg_count) {
      throw "gr::toy::format => Too many auto-index placeholders for provides "
            "arguments";
    }
    // For explicit-indexed placeholders
    if (has_explicit_index && max_index_used >= arg_count) {
      throw "gr::toy::format => Argument index out of range";
    }
    // Mixed indexing validation
    if (has_auto_index && has_explicit_index) {
      throw "gr::toy::format => Cannot mix automatic and manual indexing in "
            "format string";
    }
  }

  static consteval size_t constexpr_check_brace_index(const char *start,
                                                      const char *end,
                                                      size_t arg_count) {
    const char *ptr = start;
    bool has_index_content = false;
    size_t index_value = 0;

    // Check empty {} - auto indexing
    if (ptr < end && *ptr == '}') {
      return static_cast<size_t>(-1);
    }

    while (ptr < end && *ptr != '}' && *ptr != ':') {
      // Check indexing validation
      char c = *ptr;
      if (c < '0' || c > '9') {
        throw "gr::toy::format => Invalid argument index - must be numeric";
      }

      has_index_content = true;
      index_value = index_value * 10 + (c - '0');
      ptr++;
    }

    if (has_index_content) {
      if (arg_count > 0 && index_value >= arg_count) {
        throw "gr::toy::format => Argument index out of range";
      }

      if (arg_count == 0 && index_value > 0) {
        throw "gr::toy::format => No arguments provided but index used";
      }
      return index_value;
    }

    return static_cast<size_t>(-1);
  }

  static consteval char constexpr_get_spec_type(const char *start, const char *end) {
    if (start >= end) return '\0';
    
    const char *ptr = end - 1;
    // Find the last non-whitespace, non-digit character
    while (ptr >= start) {
      char c = *ptr;
      // Skip digits, width/precision specifiers
      if ((c >= '0' && c <= '9') || c == '.' || c == '<' || c == '>' || c == '^' || 
          c == '+' || c == '-' || c == ' ' || c == '#' || c == '{' || c == '}') {
        ptr--;
        continue;
      }
      // Found type specifier
      return c;
    }
    return '\0';
  }

  template<typename... Ts>
  static consteval void constexpr_check_format_spec(char spec_type, 
                                                   const char *index_start,
                                                   const char *index_end,
                                                   size_t arg_count) {
    // Parse the index to determine which argument this specifier applies to
    size_t arg_index = constexpr_check_brace_index(index_start, index_end, arg_count);
    
    if (arg_index == static_cast<size_t>(-1)) {
      // Auto-indexing, we need to check all arguments
      constexpr_check_format_spec_for_all<Ts...>(spec_type);
    } else {
      // Explicit indexing, check specific argument
      constexpr_check_format_spec_for_arg<Ts...>(spec_type, arg_index);
    }
  }

  template<typename T, typename... Rest>
  static consteval void constexpr_check_format_spec_for_arg(char spec_type, size_t arg_index) {
    if (arg_index == 0) {
      // Check if this type supports the specifier
      constexpr_check_type_specifier<T>(spec_type);
    } else if constexpr (sizeof...(Rest) > 0) {
      constexpr_check_format_spec_for_arg<Rest...>(spec_type, arg_index - 1);
    }
  }

  template<typename... Ts>
  static consteval void constexpr_check_format_spec_for_all(char spec_type) {
    // Check all argument types
    (constexpr_check_type_specifier<Ts>(spec_type), ...);
  }

  // Type trait to check if a type is supported for integer formatting
  template<typename T>
  static constexpr bool is_supported_integer_v = 
    std::is_integral_v<std::remove_reference_t<T>> && 
    !std::is_same_v<std::remove_cvref_t<T>, bool> && 
    !std::is_same_v<std::remove_cvref_t<T>, char>;

  // Type trait to check if a type is supported for floating point formatting
  template<typename T>
  static constexpr bool is_supported_float_v = std::is_floating_point_v<std::remove_reference_t<T>>;

  // Type trait to check if a type is supported for string formatting
  template<typename T>
  static constexpr bool is_supported_string_v = 
    std::is_same_v<std::remove_cvref_t<T>, const char*> || 
    std::is_same_v<std::remove_cvref_t<T>, char*> ||
    std::is_same_v<std::remove_cvref_t<T>, std::string> ||
    std::is_same_v<std::remove_cvref_t<T>, std::string_view> ||
    std::is_same_v<std::remove_cvref_t<T>, gr::str::u8> ||
    std::is_same_v<std::remove_cvref_t<T>, gr::str::u8v>;

  // Type trait to check if a type is a pointer
  template<typename T>
  static constexpr bool is_pointer_v = std::is_pointer_v<std::remove_reference_t<T>>;

  // Type trait to check if a type is bool
  template<typename T>
  static constexpr bool is_bool_v = std::is_same_v<std::remove_cvref_t<T>, bool>;

  // Type trait to check if a type is char
  template<typename T>
  static constexpr bool is_char_v = std::is_same_v<T, char>;


  template<typename T>
  static consteval void constexpr_check_type_specifier(char spec_type) {
    if (spec_type == '\0') {
      // Default specifier, always valid
      return;
    }

    // using BaseType = std::remove_cvref_t<T>;

    if constexpr (is_supported_integer_v<T>) {
      // Integer types support: d, x, X, o, b, B
      switch (spec_type) {
        case 'd': case 'D':
        case 'x': case 'X':
        case 'o':
        case 'b': case 'B':
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for integer argument";
      }
    } else if constexpr (is_supported_float_v<T>) {
      // Floating point types support: f, F, e, E, g, G
      switch (spec_type) {
        case 'f': case 'F':
        case 'e': case 'E':
        case 'g': case 'G':
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for floating point argument";
      }
    } else if constexpr (is_supported_string_v<T>) {
      // String types support: s (default)
      switch (spec_type) {
        case 's':
        case '\0': // Default
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for string argument";
      }
    } else if constexpr (is_pointer_v<T>) {
      // Pointer types support: p
      switch (spec_type) {
        case 'p':
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for pointer argument";
      }
    } else if constexpr (is_bool_v<T>) {
      // Bool types support: d, s, a
      switch (spec_type) {
        case 'd':
        case 's':
        case 'a':
        case '\0': // Default
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for bool argument";
      }
    } else if constexpr (is_char_v<T>) {
      // Char types support: d, s
      switch (spec_type) {
        case 'd':
        case 's':
        case '\0': // Default
          return;
        default:
          throw "gr::toy::format => Invalid type specifier for char argument";
      }
    } else {
      // Custom types - we can't check at compile time
      // They must provide their own formatter specialization
      return;
    }
  }

};

#else
template <size_t ArgCount = 0> class fmt_string {
  const char *str_;
  size_t size_;

public:
  template <size_t N>
  inline fmt_string(const char (&s)[N]) : str_(s), size_(N - 1) {
    // pre-check in compile-time
  }
  constexpr auto data() const { return str_; }
  constexpr auto size() const { return size_; }
};
#endif

template <typename... T> using fmt_string = fstring<T...>::type;

template <typename... Args>
void format_to(format_output &out, fmt_string<Args...> fmt, Args &&...args) {
  auto args_storage = std::forward_as_tuple(std::forward<Args>(args)...);
  const char *data = fmt.data();
  size_t size = fmt.size();

  // 预计算常量
  constexpr size_t arg_count = sizeof...(Args);
  size_t auto_index = 0;

  // 使用指针遍历而不是索引
  const char *ptr = data;
  const char *end = data + size;

  while (ptr < end) {
    // 快速查找下一个 '{' 或 '}'
    const char *next_special = ptr;

    while (next_special < end && *next_special != '{' && *next_special != '}') {
      ++next_special;
    }

    // 输出普通文本
    if (next_special > ptr) {
      out.put(ptr, next_special - ptr);
      ptr = next_special;
    }

    if (ptr >= end)
      break;

    if (*ptr == '{') {
      if (ptr + 1 < end && ptr[1] == '{') {
        // 转义 '{'
        out.put('{');
        ptr += 2;
      } else {
        // 使用智能方法匹配右括号，支持嵌套
        const char *placeholder_end = ptr + 1;
        int brace_level = 1; // 跟踪括号层级

        while (placeholder_end < end && brace_level > 0) {
          if (*placeholder_end == '{') {
            brace_level++;
          } else if (*placeholder_end == '}') {
            brace_level--;
          }
          placeholder_end++;
        }

        if (brace_level > 0) {
          throw format_error("gr::toy::format => Unclosed format brace");
        }

        // placeholder_end 现在指向 '}' 之后的位置
        placeholder_end--; // 回到 '}' 字符

        // 解析格式说明符
        const char *format_start = ptr + 1;
        const char *format_end = placeholder_end;

        // 查找冒号（但跳过嵌套的 {}）
        const char *colon = format_start;
        int nested_level = 0;
        while (colon < format_end) {
          if (*colon == '{') {
            nested_level++;
          } else if (*colon == '}') {
            nested_level--;
          } else if (*colon == ':' && nested_level == 0) {
            break;
          }
          ++colon;
        }

        size_t arg_index = 0;
        format_spec spec{};
        bool has_explicit_index = false;

        if (colon < format_end) {
          // 有格式说明符
          detail::parse_argument_index(format_start, colon, arg_index,
                                       auto_index, has_explicit_index);
          spec = detail::parse_format_spec_with_args(
              colon + 1, format_end, args_storage, auto_index, arg_count,
              has_explicit_index);
        } else {
          // 无格式说明符
          detail::parse_argument_index(format_start, format_end, arg_index,
                                       auto_index, has_explicit_index);
        }

        if (arg_index >= arg_count) {
          throw format_error("gr::toy::format => Argument index out of range");
        }

        // 格式化参数
        detail::format_arg_dispatch(out, args_storage, arg_index, spec);

        ptr = placeholder_end + 1;
      }
    } else { // *ptr == '}'
      if (ptr + 1 < end && ptr[1] == '}') {
        // 转义 '}'
        out.put('}');
        ptr += 2;
      } else {
        throw format_error("gr::toy::format => Unmatched closing brace");
      }
    }
  }
}

/**
 * @brief non format version
 *        fast to convert value to string auto
 * @param v any supported type value to convert
 */
template <typename T>
str::u8 format(const T& v){
  static format_spec spec;
  str::u8 res;
  res.reserve(32);
  format_output out(res);
  gr::toy::detail::format_arg(out, v, spec);
  return res;
}

template <typename... Args>
str::u8 format(fmt_string<Args...> fmt, Args &&...args) {
  // Precise memory pre-allocation
  size_t estimated_size = fmt.size();
  if constexpr (sizeof...(Args) > 0) {
    estimated_size += (sizeof...(Args) * 4); // Reduce base estimate
    estimated_size +=
        (fmt.size() * 0.5); // Estimate based on format string length
  }
  estimated_size = std::min(estimated_size, fmt.size() * 3);

  str::u8 result(estimated_size);
  // result.reserve(estimated_size);
  auto out = format_output(result);
  format_to(out, fmt, std::forward<Args>(args)...);
  return result;
}

namespace chrono {
inline auto now() { return std::chrono::system_clock::now(); }
} // namespace chrono

namespace literals {
/**
 * @brief Format function for none compile-time string literals
 */
class fmt_literal_proxy {
  const char *str_;
  size_t size_;

public:
  GR_CONSTEVAL fmt_literal_proxy(const char *s, size_t n) : str_(s), size_(n) {}

  template <typename... Args> gr::str::u8 operator()(Args &&...args) {
    // NOTE: fmt_string<size_t>(const char*, size_t) is not eval at
    // compile-time
    auto fmt = fmt_string<Args...>(str_, size_);
    return gr::toy::format(fmt, std::forward<Args>(args)...);
  };
};

/**
 * @brief This is toy likes tool for tests
 * @note - Becareful to use this testing method in product-works
 *       - This method can not check format syntax in compile-time
 */
GR_CONSTEVAL fmt_literal_proxy operator""_fmt(const char *s, size_t n) {
  return fmt_literal_proxy(s, n);
}

} // namespace literals
} // namespace toy
} // namespace gr
