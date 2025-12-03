/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file toy_charconv.hh
 * @brief High-performance character conversion utilities (for learning and
 * research purposes)
 *
 * This library implements functionality similar to C++17 std::charconv,
 * providing bidirectional conversion between strings and numeric types.
 * Primarily designed for studying standard library implementation principles,
 * not recommended for production use.
 *
 * Performance benchmarks (100,000 iterations):
 * - Float to string (fixed): toy::ftoss (15699μs) vs fmt::format (24419μs) - ~35.7% faster
 * - Float to string (scientific): toy::ftoss (18225μs) vs fmt::format (23696μs) - ~23.1% faster
 * - Integer to string: toy::itoss (1572μs) vs std::to_chars (2582μs) - ~39.1% faster
 * - String to float: toy::sstof (4112μs) vs std::from_chars (4442μs) - ~7.4% faster
 * - String to integer: toy::sstoi (2682μs) vs std::from_chars (3050μs) - ~12.1% faster
 *
 * Key Features:
 * - Support for integer types (including 128-bit integers) and floating-point types
 * - Base conversion from 2 to 36
 * - Multiple formatting options: fixed, scientific, and general formats
 * - Performance optimizations using static tables and specialized algorithms
 * - Stack-based buffers to avoid dynamic memory allocation
 *
 * Implementation Notes:
 * - Uses lookup tables for digit conversion and power-of-10 calculations
 * - Implements specialized algorithms for power-of-2 bases (binary, octal,
 * hexadecimal)
 * - Handles special floating-point values (NaN, infinity)
 * - Provides overflow detection and error reporting via std::errc
 * - Optimized division/modulo operations for common bases (2, 4, 8, 10, 16, 32)
 * - Two-digit lookup table for fast decimal conversion
 *
 * Known Limitations:
 * - Floating-point conversion precision may have minor differences from
 * std::charconv in edge cases (will be optimized in future versions)
 * - Limited to 17 decimal digits of precision for floating-point types
 * @warning This implementation is for educational purposes only and may have
 *          incomplete edge case handling. Do not use in production
 * environments.
 *
 * @author Standard library implementation researcher
 * @date 2025
 *
 * @author 学习标准库实现的研究者
 * @date 2025
 * @warning 请勿在生产环境中使用此代码
 */
#pragma once
#include <algorithm>
#include <array>
#include <bits/error_constants.h>
#include <cmath>
#include <cstdint>
// #include <iostream>
#include <gr/string.hh>
#include <gr/utils.hh>
#include <limits>
#include <tuple>
#include <type_traits>

namespace gr::toy {
enum class chars_format { fixed, scientific, general };
// some tools are studying standard library and re-write them.
struct sstov_result {
  const char *ptr = nullptr;
  std::errc ec{};
};

namespace detail {

static constexpr uint64_t POW10_TABLE[] = {1,
                                           10,
                                           100,
                                           1000,
                                           10000,
                                           100000,
                                           1000000,
                                           10000000,
                                           100000000,
                                           1000000000,
                                           10000000000,
                                           100000000000,
                                           1000000000000,
                                           10000000000000,
                                           100000000000000,
                                           1000000000000000,
                                           10000000000000000,
                                           100000000000000000,
                                           1000000000000000000};

template <typename T> struct make_unsigned;

template <typename T> struct make_unsigned {
  using type = std::make_unsigned_t<T>;
};

template <> struct make_unsigned<__int128_t> {
  using type = __uint128_t;
};

template <> struct make_unsigned<__uint128_t> {
  using type = __uint128_t;
};

template <typename T> using make_unsigned_t = make_unsigned<T>::type;

template <typename T> struct supports_integer {
  static const bool value = std::is_integral_v<T> ||
                            std::is_same_v<T, __int128_t> ||
                            std::is_same_v<T, __uint128_t>;
};

template <typename T>
inline constexpr bool supports_integer_v = supports_integer<T>::value;

template <typename unsigned_type>
inline size_t smart_mod_u(unsigned_type n, size_t m) {
  // check n^2
  if ((m & (m - 1)) == 0) {
    return n & (m - 1);
  }
  // special
  switch (m) {
  case 10:
    return n % 10;
    break;
  case 5:
    return n % 5;
    break;
  case 3:
    return n % 3;
    break;
  default:
    return n % m;
  }
}

template <typename unsigned_type>
inline unsigned_type smart_div_u(unsigned_type n, size_t d) {
  switch (d) {
  case 10:
    return n / 10; // compiler auto optimized
  case 2:
    return n >> 1;
  case 4:
    return n >> 2;
  case 8:
    return n >> 3;
  case 16:
    return n >> 4;
  case 32:
    return n >> 5;
  case 5:
    return n / 5;
  case 3:
    return n / 3;
  default:
    return n / d; // normal divider
  }
}

template <typename T> struct fp_traits;

template <> struct fp_traits<float> {
  using uint_type = uint32_t;
  using store_type = uint32_t;
  using up_store_type = uint64_t; // Upper type for intermediate calculations
  using promote_type = double;
  static constexpr int mantissa_bits = 23;
  static constexpr int exponent_bits = 8;
  static constexpr int exponent_bias = 127;
  static constexpr uint32_t mantissa_mask = 0x7FFFFF;
  static constexpr uint32_t implicit_bit = 0x800000;
  static constexpr int max_decimal_digits = 8;
  static constexpr int max_shift_bits = 63;
};

template <> struct fp_traits<double> {
  using uint_type = uint64_t;
  using store_type = uint64_t;
  using up_store_type = __uint128_t; // Upper type for intermediate calculations
  using promote_type = long double;
  static constexpr int mantissa_bits = 52;
  static constexpr int exponent_bits = 11;
  static constexpr int exponent_bias = 1023;
  static constexpr uint64_t mantissa_mask = 0xFFFFFFFFFFFFFULL;
  static constexpr uint64_t implicit_bit = 0x10000000000000ULL;
  static constexpr int max_decimal_digits = 17;
  static constexpr int max_shift_bits = 127;
};

#ifdef __SIZEOF_FLOAT128__
// If 128-bit floating-point support is available
template <> struct fp_traits<long double> {
  using uint_type = __uint128_t;
  using store_type = __uint128_t;
  using up_store_type = __uint128_t; // Upper type for intermediate calculations
  using promote_type = long double;
  static constexpr int mantissa_bits = 112;
  static constexpr int exponent_bits = 15;
  static constexpr int exponent_bias = 16383;
  static constexpr __uint128_t mantissa_mask =
      (static_cast<__uint128_t>(1) << 112) - 1;
  static constexpr __uint128_t implicit_bit = static_cast<__uint128_t>(1)
                                              << 112;
  static constexpr int max_decimal_digits = 34;
  static constexpr int max_shift_bits = 255;
};
#endif

template <typename T>
using promote_float_t = typename fp_traits<T>::promote_type;

template <typename T> using fp_store_t = typename fp_traits<T>::store_type;

template <typename T> struct promote_integer_u;
template <> struct promote_integer_u<int8_t> {
  using type = uint16_t;
};
template <> struct promote_integer_u<uint8_t> {
  using type = uint16_t;
};
template <> struct promote_integer_u<int16_t> {
  using type = uint32_t;
};
template <> struct promote_integer_u<uint16_t> {
  using type = uint32_t;
};
template <> struct promote_integer_u<int32_t> {
  using type = uint64_t;
};
template <> struct promote_integer_u<uint32_t> {
  using type = uint64_t;
};
template <> struct promote_integer_u<int64_t> {
  using type = __uint128_t;
};
template <> struct promote_integer_u<uint64_t> {
  using type = __uint128_t;
};

template <typename T> struct is_signed {
  constexpr static bool value = std::is_signed_v<T>;
};
template <> struct is_signed<__uint128_t> {
  constexpr static bool value = false;
};
template <> struct is_signed<__int128_t> {
  constexpr static bool value = true;
};

template <typename T> struct numeric_limits {
  constexpr static T max() { return std::numeric_limits<T>::max(); }
  constexpr static T min() { return std::numeric_limits<T>::min(); }
};

template <> struct numeric_limits<__uint128_t> {
  constexpr static __uint128_t max() { return __uint128_t(-1); }
  constexpr static __uint128_t min() { return 0; }
};

template <> struct numeric_limits<__int128_t> {
  constexpr static __int128_t max() {
    return ((__int128_t(0x7FFFFFFFFFFFFFFF) << 64) | 0xFFFFFFFFFFFFFFFF);
  }
  constexpr static __int128_t min() {
    return ((__int128_t(0x8000000000000000) << 64));
  }
};

#define CHAR_TO_DIGIT_METHOD 2
constexpr int char_to_digit(char c) {
#if CHAR_TO_DIGIT_METHOD == 0
  /// no memory used
  if (c >= 0 && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'Z') {
    return c - 55; // c - 'A' + 10
  } else if (c >= 'a' && c <= 'z') {
    return c - 87; // c - 'a' + 10
  }
  return -1;
#elif CHAR_TO_DIGIT_METHOD == 1
  constexpr unsigned TABLE_N = 75;
  using table_type = std::array<int8_t, TABLE_N>;
  static constexpr auto make_ch_table = [] -> table_type {
    table_type table{};
    int8_t i = 0;
    std::fill_n(table.data(), TABLE_N, -1);
    for (unsigned char c = 0; c <= 9; ++c, ++i) {
      table[c] = i;
    }
    i = 10;
    for (unsigned char c = 'A' - '0'; c <= 'Z' - '0'; ++c, ++i) {
      table[c] = i;
    }
    i = 10;
    for (unsigned char c = 'a' - '0'; c <= 'z' - '0'; ++c, ++i) {
      table[c] = i;
    }
    return table;
  };
  static constexpr auto table = make_ch_table();
  if (c < '0' || c > 'z')
    return -1;
  uint8_t ch = c - '0';
  return table[ch];
#else
  /// use table
  static constexpr auto make_c2d_table = [] -> std::array<int8_t, 256> {
    std::array<int8_t, 256> table{};
    int8_t i = 0;
    std::fill_n(table.data(), 256, -1);
    for (unsigned char c = '0'; c <= '9'; ++c, ++i) {
      table[c] = i;
    }
    i = 10;
    for (unsigned char c = 'A'; c <= 'Z'; ++c, ++i) {
      table[c] = i;
    }
    i = 10;
    for (unsigned char c = 'a'; c <= 'z'; ++c, ++i) {
      table[c] = i;
    }
    return table;
  };
  static constexpr auto c2d_table = make_c2d_table();
  return c2d_table[c];
#endif
}

template <bool IsOctal, typename UnsignedType>
inline sstov_result stoi_pow2_base_u(const char *first, const char *last,
                                     UnsignedType &result, unsigned base) {
  const unsigned shift = IsOctal ? 3 : (base == 2 ? 1 : 4);
  constexpr UnsignedType max_value = UnsignedType(-1);
  const UnsignedType max_safe = max_value >> shift;

  result = 0;
  std::errc ec{};
  while (first < last && ec == std::errc{}) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= base) {
      break;
    }
    if (result > max_safe) {
      ec = std::errc::result_out_of_range;
      break;
    }
    UnsignedType new_result = result << shift;
    if (new_result > max_value - digit) {
      ec = std::errc::result_out_of_range;
      break;
    }
    result = new_result + UnsignedType(digit);
    ++first;
  }
  while (first < last) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= 10) {
      break;
    }
    ++first;
  }
  return {first, ec};
}

template <typename UnsignedType>
inline sstov_result stoi_base10_u(const char *first, const char *last,
                                  UnsignedType &result) {

  result = 0;
  std::errc ec{};
  constexpr UnsignedType max_value = UnsignedType(-1);
  constexpr UnsignedType max_safe = UnsignedType(-1) / 10;

  while (first < last && ec == std::errc{}) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= 10) {
      break;
    }

    if (result > max_safe) {
      ec = std::errc::result_out_of_range;
      break;
    }

    UnsignedType new_result = result * 10;

    if (new_result > max_value - digit) {
      ec = std::errc::result_out_of_range;
      break;
    }

    result = new_result + UnsignedType(digit);
    ++first;
  }

  while (first < last) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= 10) {
      break;
    }
    ++first;
  }
  return {first, ec};
}

template <typename UnsignedType>
inline sstov_result stoi_alnum_u(const char *first, const char *last,
                                 UnsignedType &result, unsigned base) {
  result = 0;
  std::errc ec{};
  constexpr UnsignedType max_value = UnsignedType(-1);
  const UnsignedType max_safe = UnsignedType(-1) / base;

  while (first < last && ec == std::errc{}) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= base) {
      break;
    }

    if (result > max_safe) {
      ec = std::errc::result_out_of_range;
      break;
    }

    UnsignedType new_result = result * base;

    if (new_result > max_value - digit) {
      ec = std::errc::result_out_of_range;
      break;
    }

    result = new_result + UnsignedType(digit);
    ++first;
  }

  while (first < last) {
    unsigned digit = toy::detail::char_to_digit(*first);
    if (digit >= base) {
      break;
    }
    ++first;
  }
  return {first, ec};
}
template <typename part_store_u> struct fp_parts_store {
  part_store_u int_part;
  part_store_u frac_part;
};

template <typename fp_type>
inline auto get_fp_parts(fp_type value, unsigned precision)
    -> fp_parts_store<detail::fp_store_t<fp_type>> {
  using traits = fp_traits<fp_type>;
  using uint_type = typename traits::uint_type;
  using store_type = typename traits::store_type;
  using up_type = typename traits::up_store_type;

  // Type punning to access bit representation
  union {
    fp_type f;
    uint_type u;
  } conv = {value};

  const int exponent =
      ((conv.u >> traits::mantissa_bits) & ((1 << traits::exponent_bits) - 1)) -
      traits::exponent_bias;
  const uint_type mantissa =
      (conv.u & traits::mantissa_mask) | traits::implicit_bit;

  store_type integer_part = 0;
  store_type fractional_part = 0;
  store_type pow10_precision = static_cast<store_type>(POW10_TABLE[precision]);

  // Unified calculation parameters
  int effective_exponent = exponent;
  uint_type effective_mantissa = mantissa;
  bool is_pure_integer = false;

  if (exponent >= 0) {
    if (exponent > traits::mantissa_bits) {
      // Pure integer case
      integer_part = static_cast<store_type>(mantissa)
                     << (exponent - traits::mantissa_bits);
      is_pure_integer = true;
    }
  } else {
    integer_part = 0;
    effective_exponent = -exponent;
  }

  if (!is_pure_integer) {
    const int shift_bits = (exponent >= 0)
                               ? (traits::mantissa_bits - exponent)
                               : (traits::mantissa_bits + effective_exponent);

    if (exponent >= 0) {
      integer_part = static_cast<store_type>(mantissa >> shift_bits);
      effective_mantissa =
          mantissa & ((static_cast<uint_type>(1) << shift_bits) - 1);
    } else {
      effective_mantissa = mantissa;
    }

    // Calculate fractional part
    if (shift_bits <= traits::max_shift_bits) {
      up_type temp = static_cast<up_type>(effective_mantissa) * pow10_precision;

      // Rounding
      if (shift_bits > 0) {
        if constexpr (std::is_same_v<up_type, uint64_t>) {
          // 64-bit version
          temp += (static_cast<uint64_t>(1) << (shift_bits - 1));
        } else {
          // 128-bit version
          up_type round_offset = 1;
          round_offset <<= (shift_bits - 1);
          temp += round_offset;
        }
      }

      fractional_part = static_cast<store_type>(temp >> shift_bits);

      // Carry handling
      if (fractional_part >= pow10_precision) {
        fractional_part -= pow10_precision;
        integer_part++;
        // Rare case requiring second carry
        if (fractional_part >= pow10_precision) {
          fractional_part -= pow10_precision;
          integer_part++;
        }
      }
    }
  }

  return {integer_part, fractional_part};
}

template <typename fp_type>
inline auto _split_float_u(fp_type value, int precision)
    -> fp_parts_store<detail::fp_store_t<fp_type>> {
  return get_fp_parts<fp_type>(value, precision);
}

template <>
inline auto _split_float_u<long double>(long double value, int precision)
    -> fp_parts_store<detail::fp_store_t<long double>> {

  using store_type = detail::fp_store_t<long double>;
  using promote_type = detail::promote_float_t<long double>;

  store_type scale_factor = POW10_TABLE[precision];
  store_type scale_factor_extend = POW10_TABLE[precision + 1];
  store_type integer_part = static_cast<uint64_t>(value);
  promote_type fractional_part_f = value - integer_part;
  store_type fractional_part_u =
      (store_type(fractional_part_f * scale_factor_extend) + 5) / 10;
  // uint64_t fractional_part_u = uint64_t(fractional_part_f * scale_factor +
  // 0.5);
  if (fractional_part_u >= scale_factor) {
    integer_part++;
    fractional_part_u = 0;
  }
  return {integer_part, fractional_part_u};
}

template <typename T> struct exponent_calculator;

template <> struct exponent_calculator<float> {
  static int calculate(float abs_value) {
    // Quick check for common range
    if (abs_value >= 1e-4f && abs_value <= 1e6f) {
      // Fast estimation using bit manipulation
      uint32_t bits;
      std::memcpy(&bits, &abs_value, sizeof(float));
      int32_t e = ((bits >> 23) & 0xFF) - 127;

      // log10(2) ≈ 0.30102999566
      // Using fixed-point arithmetic
      int exponent = (e * 30103 + 50000) / 100000; // 四舍五入

      // Quick adjustment
      static constexpr float COMMON_POW10[] = {
          1e-4f, 1e-3f, 1e-2f, 1e-1f, 1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f};

      // Binary search adjustment
      int low = 0, high = 10;
      while (low <= high) {
        int mid = (low + high) / 2;
        if (abs_value >= COMMON_POW10[mid]) {
          exponent = mid - 4; // 偏移：-4到6
          low = mid + 1;
        } else {
          high = mid - 1;
        }
      }
      return exponent;
    } else {
      // Use standard method for uncommon ranges
      return static_cast<int>(std::floor(std::log10f(abs_value)));
    }
  }
};

template <> struct exponent_calculator<double> {
  static int calculate(double abs_value) {
    // Quick check for common range
    if (abs_value >= 1e-4 && abs_value <= 1e6) {
      // Fast estimation using bit manipulation
      uint64_t bits;
      std::memcpy(&bits, &abs_value, sizeof(double));
      int32_t e = ((bits >> 52) & 0x7FF) - 1023;

      // More precise fixed-point arithmetic
      int exponent = (e * 301029995 + 500000000) / 1000000000;

      // Common range adjustment
      static constexpr double COMMON_POW10[] = {
          1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6};

      int low = 0, high = 10;
      while (low <= high) {
        int mid = (low + high) / 2;
        if (abs_value >= COMMON_POW10[mid]) {
          exponent = mid - 4;
          low = mid + 1;
        } else {
          high = mid - 1;
        }
      }
      return exponent;
    } else {
      // Use standard method for uncommon ranges
      return static_cast<int>(std::floor(std::log10(abs_value)));
    }
  }
};

template <typename T> struct exponent_calculator {
  static int calculate(T abs_value) {
    // Use standard method
    return static_cast<int>(std::floor(std::log10l(abs_value)));
  }
};
} // namespace detail

namespace detail {
/**
 * @brief Integer to string converter
 */
class to_chars_helper {
public:
  struct buffer_pack {
    const char *ptr = nullptr;
    size_t size = 0;
  };

public:
  constexpr to_chars_helper(char *buffer, size_t size)
      : buffer_(buffer), ptr_(buffer + size), end_(buffer + size) {}

  /**
   * @brief Convert integer to string
   * @param value Integer value to convert
   * @param base Base (2-36)
   * @param uppercase Use uppercase letters (only for base 16)
   * @param alternate Show base prefix (0x, 0b, 0)
   * @return Conversion result (pointer, length)
   */
  template <typename T>
  [[nodiscard]] buffer_pack from_integer(T value, unsigned base = 10,
                                         bool uppercase = false,
                                         bool alternate = false) {
    // static_assert(std::is_integral_v<T>, "T must be integral type");

    if (base < 2 || base > 36) {
      return {nullptr, 0};
    }

    ptr_ = end_; // Start from the end of buffer

    // Pre-parse base settings
    auto [actual_base, prefix, prefix_len, actual_uppercase] =
        _prepare_integer_format_type(base, uppercase);

    // Handle zero value (consider base prefix)
    if (value == 0) {
      // Write digit '0' first
      if (ptr_ <= buffer_) {
        return {nullptr, 0};
      }
      *--ptr_ = '0';

      // Then add prefix (before the number)
      if (alternate && prefix != nullptr) {
        // Check if there's enough space
        if (ptr_ - buffer_ < static_cast<std::ptrdiff_t>(prefix_len)) {
          return {nullptr, 0};
        }

        // Write prefix (from back to front)
        for (size_t i = 0; i < prefix_len; ++i) {
          *--ptr_ = prefix[prefix_len - 1 - i];
        }
      }

      return {ptr_, size_t(end_ - ptr_)};
    }
    // Handle sign
    bool negative = false;
    if constexpr (detail::is_signed<T>::value) {
      if (value < 0) {
        negative = true;
        value = -value;
      }
    }

    using unsigned_type = detail::make_unsigned_t<T>;
    unsigned_type unsigned_value = static_cast<unsigned_type>(value);

    // Convert digits (write from back to front)
    _convert_integer_u(unsigned_value, actual_base, actual_uppercase);

    // Add base prefix
    if (alternate && prefix != nullptr) {
      for (size_t i = prefix_len; i > 0; --i) {
        if (ptr_ > buffer_) {
          *--ptr_ = prefix[i - 1];
        } else {
          return {nullptr, 0};
        }
      }
    }

    // Add sign
    if (negative) {
      if (ptr_ > buffer_) {
        *--ptr_ = '-';
      } else {
        return {nullptr, 0};
      }
    }

    return {ptr_, size_t(end_ - ptr_)};
  }

  template <typename T>
  [[nodiscard]]
  inline buffer_pack
  from_float(T value, toy::chars_format format = toy::chars_format::fixed,
             int precision = -1, bool uppercase = false) {
    static_assert(std::is_floating_point_v<T>,
                  "from_float => T must be a float type");
    _convert_float_dispatch(value, format, precision, uppercase);
    return {buffer_, size_t(ptr_ - buffer_)};
  }

  constexpr char *data() const { return buffer_; }
  constexpr char *ptr() const { return ptr_; }

  size_t size() const { return ptr_ - buffer_; }

  static constexpr uint64_t get_pow10(unsigned x) { return POW10_TABLE[x]; }

private:
  char *buffer_;
  char *ptr_;
  char *end_;

  static constexpr char digits_lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  static constexpr char digits_upper[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr char two_digit_table[200] = {
      '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
      '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
      '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
      '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
      '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
      '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
      '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
      '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
      '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
      '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
      '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
      '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
      '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
      '7', '9', '8', '9', '9'};

private:
  inline bool _write_string(const char *str, size_t len) {
    if (ptr_ + len <= end_) {
      std::copy_n(str, len, ptr_);
      ptr_ += len;
      return true;
    }
    return false;
  }

  inline void _write_char(char c) {
    *(ptr_++) = c;
    // if(ptr_ < end_){
    //   *(ptr_++) = c;
    //   return;
    // }
    // throw std::runtime_error("[ERROR]to_chars_helper => buffer size is
    // small");
  }

  static inline std::tuple<int, const char *, uint8_t, bool>
  _prepare_integer_format_type(unsigned base, bool uppercase = false) {
    const char *prefix = nullptr;
    uint8_t prefix_len = 0;

    switch (base) {
    case 2:
      prefix = uppercase ? "0B" : "0b";
      prefix_len = 2;
      break;
    case 8:
      prefix = "0";
      prefix_len = 1;
      break;
    case 16:
      prefix = uppercase ? "0X" : "0x";
      prefix_len = 2;
      break;
    default:
      // Decimal and other bases have no prefix
      break;
    }

    return std::make_tuple(base, prefix, prefix_len, uppercase);
  }

  template <typename unsigned_type>
  static inline void _convert_integer_u_base10(char *&current,
                                               unsigned_type value) {
    while (value >= 10000) {
      // 一次处理4位数字
      auto const q = value / 10000;
      auto const r = value % 10000;
      value = q;

      // 处理千位和百位
      auto const r1 = r / 100;
      auto const r2 = r % 100;

      const char *pair1 = two_digit_table + (r1 << 1);
      const char *pair2 = two_digit_table + (r2 << 1);

      *--current = pair2[1];
      *--current = pair2[0];
      *--current = pair1[1];
      *--current = pair1[0];
    }
    while (value >= 100) {
      auto const q = value / 100;
      auto const r = value % 100;
      value = q;
      // *--current = digits[r % 10];
      // *--current = digits[r / 10];
      const char *pair = two_digit_table + (r << 1); // r * 2
      *--current = pair[1];
      *--current = pair[0];
    }
    if (value >= 10) {
      // *--current = digits[value % 10];
      // *--current = digits[value / 10];
      const char *pair = two_digit_table + (value << 1);
      *--current = pair[1];
      *--current = pair[0];
    } else {
      *--current = digits_lower[value];
    }
  }

  /**
   * @brief Convert digits to specified base (write from back to front)
   */
  template <typename unsigned_type>
  void _convert_integer_u(unsigned_type value, int base, bool uppercase) {
    const char *digits = uppercase ? digits_upper : digits_lower;

    char *current = ptr_;

    if (base == 10) {
      _convert_integer_u_base10(current, value);
    } else {
      while (value > 0) {
        auto digit = detail::smart_mod_u(value, base); // value % base
        *--current = digits[digit];
        value = smart_div_u(value, base); // value /= base
      }
    }

    ptr_ = current;
  }

  template <typename T> void _convert_float_integer_part(T value) {
    if (value == 0) {
      _write_char('0');
      return;
    }

    char buffer[32];

    char *p = buffer + sizeof(buffer);
    _convert_integer_u_base10(p, value);

    size_t len = buffer + sizeof(buffer) - p;
    std::copy_n(p, len, ptr_);
    ptr_ += len;

    return;
  }

  void _convert_float_fractional_part(uint64_t fractional, int precision) {
    if (precision <= 0)
      return;
    uint64_t divisor = POW10_TABLE[precision - 1];
    for (int i = 0; i < precision; ++i) {
      int digit = (fractional / divisor) % 10;
      char digit_char = '0' + digit;
      _write_string(&digit_char, 1);
      divisor /= 10;
    }
  }

  template <typename T>
  inline void _convert_float_as_fixed(T value, int precision) {
    constexpr auto max_of_limit = (long double)(uint64_t(-1));
    if (std::is_same_v<T, float> || std::abs(value) < max_of_limit) {
      _convert_fixed_with_low_precision(value, precision);
    } else {
      _convert_fixed_with_high_precision(value, precision);
    }
  }

  template <typename T>
  void _convert_fixed_with_high_precision(T value, int precision) {
    auto [integer_part, fractional_part_u] =
        _split_float_u<detail::promote_float_t<T>>(value, precision);
    _convert_float_integer_part(integer_part);

    if (precision > 0) {
      _write_char('.');
      _convert_float_fractional_part(fractional_part_u, precision);
    }
  }

  template <typename T>
  void _convert_fixed_with_low_precision(T value, int precision) {
    auto [integer_part, fractional_part_u] =
        _split_float_u<T>(value, precision);
    _convert_float_integer_part(integer_part);

    if (precision > 0) {
      _write_char('.');
      _convert_float_fractional_part(fractional_part_u, precision);
    }
  }

  template <typename T>
  void _convert_float_as_scientific(T value, int precision, bool uppercase) {
    if (value == T{0}) {
      _convert_float_as_fixed(T{0}, precision);
      _write_char(uppercase ? 'E' : 'e');
      _write_string("+00", 3);
      return;
    }

    // bool negative = value < T{0};
    T abs_value = std::abs(value);

    // 优化：对于常见范围使用快速近似
    int exponent = exponent_calculator<T>::calculate(abs_value);

    // 计算尾数
    T mantissa = value;
    if (exponent != 0) {
      // 使用预计算的10的幂次表
      if (exponent > 0 && exponent <= 18) {
        mantissa /= static_cast<T>(POW10_TABLE[exponent]);
      } else if (exponent < 0 && exponent >= -18) {
        mantissa *= static_cast<T>(POW10_TABLE[-exponent]);
      } else {
        // 非常见指数，使用标准方法
        T power_of_ten = std::pow(T{10}, static_cast<T>(exponent));
        mantissa /= power_of_ten;
      }
    }

    // 确保尾数在正确范围内
    T abs_mantissa = std::abs(mantissa);
    const T epsilon = std::numeric_limits<T>::epsilon() * 10;

    if (abs_mantissa >= 10.0 - epsilon) {
      mantissa /= 10.0;
      exponent += 1;
    } else if (abs_mantissa < 1.0 - epsilon && abs_mantissa > epsilon) {
      mantissa *= 10.0;
      exponent -= 1;
    }

    // 格式化和输出
    if (precision < 0)
      precision = 6;

    _convert_float_as_fixed(mantissa, precision);

    // 写入指数部分
    _write_char(uppercase ? 'E' : 'e');

    if (exponent >= 0) {
      _write_char('+');
    } else {
      _write_char('-');
      exponent = -exponent;
    }

    // 格式化指数
    if (exponent < 10) {
      _write_char('0');
      _write_char('0' + exponent);
    } else if (exponent < 100) {
      _write_char('0' + (exponent / 10));
      _write_char('0' + (exponent % 10));
    } else {
      _convert_float_integer_part(static_cast<uint64_t>(exponent));
    }
  }

  void _remove_float_trailing_zeros() {
    char *end = ptr_ - 1;
    while (end >= buffer_ && *end == '0') {
      --end;
    }

    if (end >= buffer_ && *end == '.') {
      --end;
    }

    ptr_ = end + 1;
  }

  template <typename T> void _convert_float_as_general(T value, int precision) {
    if (precision < 0) {
      precision = 6; // Default precision
    }

    if (value == 0.0) {
      _convert_float_as_fixed(value, precision);
      _remove_float_trailing_zeros();
      return;
    }

    // Calculate exponent
    int exponent = 0;
    if (value != 0.0) {
      // exponent = static_cast<int>(std::floor(std::log10(std::abs(value))));
      exponent = exponent_calculator<T>::calculate(std::abs(value));
    }

    // Condition: -4 <= exponent < precision
    if (exponent >= -4 && exponent < precision) {
      // Dispatch to fixed format
      int integer_digits = (exponent >= 0) ? exponent + 1 : 0;
      int fixed_precision = precision - integer_digits;
      if (fixed_precision < 0) {
        fixed_precision = 0;
      }

      _convert_float_as_fixed(value, fixed_precision);
      _remove_float_trailing_zeros();
    } else {
      // Dispatch to scientific notation
      _convert_float_as_scientific(value, precision - 1, false);
      _remove_float_trailing_zeros();
    }
  }

  template <typename float_type>
  inline bool _before_convert_float_special_value(float_type &value,
                                                  bool uppercase) {
    ptr_ = buffer_;
    bool negative = std::signbit(value);
    if (std::isnan(value)) {
      if (uppercase) {
        _write_string("NAN", 3);
      } else {
        _write_string("nan", 3);
      }
      return true;
    }
    if (std::isinf(value)) {
      if (negative) {
        uppercase ? _write_string("-INF", 4) : _write_string("-inf", 4);
      } else {
        uppercase ? _write_string("INF", 3) : _write_string("inf", 3);
      }
      return true;
    }

    if (negative) {
      _write_char('-');
      value = -value;
    }
    return false;
  }

  template <typename T>
  void
  _convert_float_dispatch(T value,
                          toy::chars_format format = toy::chars_format::fixed,
                          int precision = -1, bool uppercase = false) {
    // Catch special value
    // this must run before all of other function
    if (_before_convert_float_special_value(value, uppercase)) {
      return;
    };

    // Forced to use scientifc when value is large than max value of uint64_t
    // type, to convert value maybe lost precision.
    constexpr double LDB_UINT64_MAX = (double)(uint64_t(-1));
    constexpr int MAX_FLOAT_PRECISION = 17;
    if (value > LDB_UINT64_MAX) {
      if (precision < 0) {
        precision = MAX_FLOAT_PRECISION;
      }
      _convert_float_as_scientific(value, precision, uppercase);
      return;
    }

    // Clamp precision
    if (precision > MAX_FLOAT_PRECISION) {
      precision = MAX_FLOAT_PRECISION;
    }
    // Dispatch method
    if (format == toy::chars_format::fixed) {
      if (precision < 0)
        precision = 6;
      _convert_float_as_fixed(value, precision);
    } else if (format == toy::chars_format::scientific) {
      if (precision < 0)
        precision = 6;
      _convert_float_as_scientific(value, precision, uppercase);
    } else {
      _convert_float_as_general(value, precision);
    }
    return;
  }
};

/**
 * @brief test first char and offset pointer
 */

inline int get_sign_for_sstov(const char *&first) {
  int sign = 1;
  if (*first == '-') {
    sign = -1;
    ++first;
  } else if (*first == '+') {
    sign = 1;
    ++first;
  }
  return sign;
}

template <typename integer_type, typename unsigned_type>
inline std::errc check_integer_overflow(int str_sign, unsigned_type pre_result,
                                        integer_type &value_out) {
  if constexpr (detail::is_signed<integer_type>::value) { // signed type
    if (str_sign == -1) {
      value_out = -integer_type(pre_result);
      if (pre_result >
          unsigned_type(detail::numeric_limits<integer_type>::max()) + 1) {
        return std::errc::result_out_of_range;
      }
    } else {
      value_out = integer_type(pre_result);
      if (pre_result >
          unsigned_type(detail::numeric_limits<integer_type>::max())) {
        return std::errc::result_out_of_range;
      }
    }
  } else {
    if (str_sign == -1) {
      value_out = integer_type(pre_result);
      return std::errc::invalid_argument;
    }
    value_out = integer_type(pre_result); // unsigned_type
  }
  return std::errc{};
}

} // namespace detail

/**
 * @brief this tool is a clone as std::from_chars(...)
 * @note when errcode genrated, value should set zero
 */
template <typename integer_type>
inline sstov_result sstoi(const char *first, const char *last,
                          integer_type &value, int base = 10) noexcept {
  static_assert(detail::supports_integer_v<integer_type>,
                "Only integral types supported");

  if (last <= first || base < 2 || base > 36) {
    value = 0;
    return {first, std::errc::invalid_argument};
  }

  int pre_sign = detail::get_sign_for_sstov(first);

  using unsigned_type = detail::make_unsigned_t<integer_type>;
  unsigned_type pre_result = 0;

  sstov_result status;

  if (base == 10) {
    status = detail::stoi_base10_u(first, last, pre_result);
  } else if ((base & (base - 1)) == 0) {
    if (base == 2) {
      status = detail::stoi_pow2_base_u<false>(first, last, pre_result, base);
    } else if (base <= 8) {
      status = detail::stoi_pow2_base_u<true>(first, last, pre_result, base);
    } else {
      status = detail::stoi_pow2_base_u<false>(first, last, pre_result, base);
    }
  } else {
    status = detail::stoi_alnum_u(first, last, pre_result, base);
  }
  if (status.ec == std::errc{}) {
    return {status.ptr,
            detail::check_integer_overflow(pre_sign, pre_result, value)};
  } else {
    value = integer_type(pre_result);
  }
  return status;
}

template <typename integer_type>
inline sstov_result sstoi(const char *first, size_t len, integer_type &value,
                          int base = 10) noexcept {
  return sstoi(first, first + len, value, base);
}

template <typename integer_type>
inline sstov_result sstoi_base10(const char *first, const char *last,
                                 integer_type &value) {
  if (last <= first) {
    value = 0;
    return {first, std::errc::invalid_argument};
  }
  int pre_sign = detail::get_sign_for_sstov(first);
  using unsigned_type = detail::make_unsigned_t<integer_type>;
  unsigned_type pre_result = 0;
  sstov_result status = detail::stoi_base10_u(first, last, pre_result);
  if (status.ec == std::errc{}) {
    return {status.ptr,
            detail::check_integer_overflow(pre_sign, pre_result, value)};
  } else {
    value = integer_type(pre_result);
  }
  return status;
}

template <typename integer_type>
inline sstov_result sstoi_base10(const char *first, size_t len,
                                 integer_type &value) {
  return sstoi_base10(first, first + len, value);
}

template <typename float_type>
inline toy::sstov_result sstof(const char *first, const char *end,
                               float_type &value) {
  int sign = toy::detail::get_sign_for_sstov(first);

  union starts_pack {
    char ch[4];
    uint32_t v;
  };

  if (end - first >= 3) {
    starts_pack starts;
    starts.v = *((uint32_t *)(first));
    starts.ch[3] = '\0';

    constexpr starts_pack inf_low = {{"inf"}};
    constexpr starts_pack nan_low = {{"nan"}};
    constexpr starts_pack inf_upper = {{"INF"}};
    constexpr starts_pack nan_upper = {{"NAN"}};

    // constexpr starts_pack inf_low{.ch{"inf"}};
    // constexpr starts_pack nan_low{.ch{"nan"}};
    // constexpr starts_pack inf_upper{.ch{"INF"}};
    // constexpr starts_pack nan_upper{.ch{"NAN"}};

    // constexpr starts_pack inf_low = {{'i', 'n', 'f', '\0'}};
    // constexpr starts_pack nan_low = {{'n', 'a', 'n', '\0'}};
    // constexpr starts_pack inf_upper = {{'I', 'N', 'F', '\0'}};
    // constexpr starts_pack nan_upper = {{'N', 'A', 'N', '\0'}};

    if (starts.v == inf_low.v || starts.v == inf_upper.v) {
      value = std::numeric_limits<float_type>::infinity() * sign;
      return {first + 3, std::errc{}};
    } else if (starts.v == nan_low.v || starts.v == nan_upper.v) {
      value = std::numeric_limits<float_type>::quiet_NaN();
      return {first + 3, std::errc{}};
    }
  }

  __uint128_t int_part_128 = 0;
  uint64_t int_part_64 = 0;
  auto res = toy::detail::stoi_base10_u(first, end, int_part_64);
  if (res.ec == std::errc::result_out_of_range) {
    res = toy::detail::stoi_base10_u(first, end, int_part_128);
  }

  const char *frac_p = nullptr;
  if (*res.ptr == '.') {
    frac_p = res.ptr + 1;
  }

  int fallback128 = int_part_128 != 0;
  value = float_type((fallback128)*int_part_128 + int_part_64 * (!fallback128));
  // value = float_type(int_part_128 ? int_part_128 : int_part_64);
  if (frac_p) {
    uint64_t frac_part = 0;
    auto frac_part_end = (end - frac_p) > 17 ? frac_p + 17 : end;
    res = toy::detail::stoi_base10_u(frac_p, frac_part_end, frac_part);
    unsigned frac_digits_count = res.ptr - frac_p;

    if (res.ptr - frac_p > 17) {
      frac_digits_count = 17;
    }

    value += float_type(frac_part) /
             toy::detail::to_chars_helper::get_pow10(frac_digits_count);
  }

  const char *exp_p = res.ptr;

  while (exp_p < end) {
    if (*exp_p == 'e' || *exp_p == 'E') {
      ++exp_p;
      unsigned exponent = 0;
      unsigned remaining = end - exp_p;
      if (remaining > 17)
        remaining = 17;
      int sign_e = toy::detail::get_sign_for_sstov(exp_p);
      res = toy::detail::stoi_base10_u(exp_p, exp_p + remaining, exponent);
      if (sign_e < 0) {
        value /= toy::detail::to_chars_helper::get_pow10(exponent);
      } else {
        value *= toy::detail::to_chars_helper::get_pow10(exponent);
      }
      break;
    }
    ++exp_p;
  }
  value *= sign;

  return {end, std::errc{}};
}

template <typename float_type>
inline sstov_result sstof(const char *first, size_t len, float_type &value) {
  return sstof(first, first + len, value);
}

/**
 * @brief convert floating-point to string conversion
 * Uses stack buffer to avoid dynamic memory allocation
 * @note TODO: this performance can be optimized after ...
 */
template <typename T>
inline detail::to_chars_helper::buffer_pack
ftoss(char *out_buffer, size_t buffer_size, T value,
      toy::chars_format format = toy::chars_format::general, int precision = -1,
      bool uppercase = false) {
  static_assert(std::is_floating_point_v<T>,
                "gr::toy::ftoss => T must be floating point type");

  if (precision < 0) {
    precision = 0;
  }
  if (precision > 17) {
    precision = 17;
  }
  detail::to_chars_helper converter(out_buffer, buffer_size);
  return converter.from_float(value, format, precision, uppercase);
}

template <typename T>
inline detail::to_chars_helper::buffer_pack
itoss(char *out_buffer, size_t buffer_size, T value, unsigned base = 10,
      bool uppercase = false, bool alternate = false) {
  static_assert(detail::supports_integer_v<T>,
                "gr::toy::itoss => T must be integral type");
  detail::to_chars_helper converter(out_buffer, buffer_size);
  return converter.from_integer(value, base, uppercase, alternate);
}

} // namespace gr::toy
