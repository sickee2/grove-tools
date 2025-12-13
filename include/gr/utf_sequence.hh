/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file utf_sequence.hh
 * @brief Unicode encoding validation and code point operations
 * @ingroup unicode
 *
 * Provides core Unicode encoding validation, sequence processing, and code point
 * operations for UTF-8, UTF-16, and UTF-32 encodings. This module forms the
 * foundation of the Unicode processing infrastructure.
 *
 * ## Core Features
 * - **Multi-Encoding Validation**: UTF-8, UTF-16, UTF-32 sequence validation
 * - **Code Point Operations**: Case conversion, classification, properties
 * - **Encoding Conversion**: UTF-8/16/32 encoding and decoding
 * - **Endianness Support**: Byte order handling for UTF-16/32
 * - **Performance Optimized**: Efficient algorithms with minimal overhead
 *
 * ## Key Components
 *
 * ### codepoint Class
 * Unicode code point wrapper providing:
 * - Character classification (whitespace, printable, alphabetic, digit)
 * - Case conversion (upper/lower) with Unicode-compliant mapping
 * - Display width calculation (East Asian Width property)
 * - Encoding conversion (UTF-8, UTF-16 chunk generation)
 * - Validation and property checking
 *
 * ### sequence<char_type> Template Specializations
 * Encoding-specific sequence processors:
 * - `sequence<char>`: UTF-8 validation and decoding
 * - `sequence<char16_t>`: UTF-16 validation and decoding
 * - `sequence<char32_t>`: UTF-32 validation and decoding
 *
 * ### chunk_proxy Template Classes
 * Encoding chunk buffers for efficient string operations:
 * - `chunk_proxy<char>`: UTF-8 chunk (6-byte buffer)
 * - `chunk_proxy<char16_t>`: UTF-16 chunk (4-element buffer)
 *
 * ## Unicode Compliance
 *
 * ### Supported Standards
 * - Unicode 13.0 character database
 * - Unicode Case Mapping (SpecialCasing.txt)
 * - Unicode East Asian Width property
 * - Unicode General Category properties
 *
 * ### Character Classification
 * - **Whitespace**: Includes all Unicode whitespace characters
 * - **Printable**: Excludes control, format, and private-use characters
 * - **Alphabetic**: Latin, Greek, Cyrillic, and extended alphabetic ranges
 * - **Digit**: ASCII digits (0-9) with Unicode digit support planned
 *
 * ## Case Conversion Implementation
 *
 * ### Multi-Stage Processing
 * 1. **Special Cases**: Irregular mappings (e.g., ß → ẞ, ς → Σ)
 * 2. **Latin Extended**: Efficient bit manipulation for Latin ranges
 * 3. **Range Mapping**: Systematic case conversion using offset tables
 * 4. **Validation**: Surrogate and validity checking
 *
 * ### Supported Scripts
 * - Latin (Basic, Extended-A, Extended-B)
 * - Greek (monotonic and polytonic)
 * - Cyrillic (basic and extended)
 * - Armenian
 *
 * ## Performance Characteristics
 *
 * ### Optimized Algorithms
 * - **Fast Paths**: Common ASCII cases handled with minimal branching
 * - **Lookup Tables**: Efficient range-based case conversion
 * - **Memory Efficient**: Stack-based buffers for encoding operations
 * - **Branch Prediction**: Optimized for common character ranges
 *
 * ### Encoding-Specific Optimizations
 * - **UTF-8**: Byte-by-byte validation with early termination
 * - **UTF-16**: Surrogate pair validation with endianness support
 * - **UTF-32**: Direct code point access with validation
 *
 * ## Usage Examples
 *
 * ### Basic Code Point Operations
 * ```cpp
 * gr::uc::codepoint cp('A');
 * auto upper = cp.upper();      // 'A' → 'A'
 * auto lower = cp.lower();      // 'A' → 'a'
 * bool is_alpha = cp.is_alphabetic();  // true
 * int width = cp.display_width();      // 1
 * ```
 *
 * ### Encoding Validation
 * ```cpp
 * const char* utf8_data = "Hello 世界";
 * auto info = gr::uc::sequence<char>::check(utf8_data, utf8_data + 12);
 * if (info.status == gr::uc::sequence_status::valid) {
 *     auto cp = gr::uc::sequence<char>::decode(utf8_data, info.length, info.status);
 * }
 * ```
 *
 * ### Encoding Conversion
 * ```cpp
 * gr::uc::codepoint cp(0x4E16);  // 世
 * auto utf8_chunk = cp.chunk_u8();   // UTF-8 encoding
 * auto utf16_chunk = cp.chunk_u16(); // UTF-16 encoding
 * ```
 *
 * ## Error Handling
 *
 * ### Sequence Status
 * - `valid`: Properly formed Unicode sequence
 * - `invalid_continuation`: Malformed continuation bytes
 * - `truncated`: Incomplete sequence at buffer end
 * - `invalid_starbyte`: Invalid starting byte
 *
 * ### Replacement Strategy
 * Invalid sequences are replaced with U+FFFD (REPLACEMENT CHARACTER)
 * in decoding operations to ensure forward progress.
 *
 * ## Integration Points
 *
 * ### With gr::uc::iter
 * - Provides sequence validation for iterator advancement
 * - Supplies code point decoding for dereference operations
 * - Enables efficient multi-encoding traversal
 *
 * ### With gr::str::utf
 * - Foundation for Unicode string operations
 * - Encoding conversion backend
 * - Character property checking
 *
 * ## Dependencies
 * - Standard Library: `<cstdint>`, `<string_view>`, `<type_traits>`
 * - Platform: Endianness detection utilities
 * - Internal: `gr/config.hh` for feature detection
 *
 * ## Configuration
 * - `GR_HAS_CPP20`: C++20 `<bit>` header for endianness
 * - Platform-specific endianness detection
 *
 * @see gr::uc::iter for Unicode iteration
 * @see gr::str::utf for Unicode string classes
 * @see gr::utils for platform utilities
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <gr/config.hh>

#if GR_HAS_CPP20
#include <bit>
#endif

namespace gr{

enum class endian : uint8_t { native, little, big };

constexpr bool is_little_endian() {
#if GR_HAS_CPP20
  return std::endian::native == std::endian::little;
#else
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      return true;
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      return false;
#elif defined(_WIN32) || defined(__i386__) || defined(__x86_64__)
      return true;
#else
      // 无法在编译时确定
      return false; // 需要运行时检测
#endif
#endif
}

template <typename T> T swap_bytes(T value) {
  union {
    T value;
    std::byte bytes[sizeof(T)];
  } src, dst;

  src.value = value;
  for (size_t i = 0; i < sizeof(T); ++i) {
    dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];
  }
  return dst.value;
}

/**
 * @brief Convert value between different endianness
 * @param value The value to convert
 * @param endian Target endianness
 * @return Converted value
 * @note Returns original value if target endianness matches native endianness
 */

template <typename T> T convert_endian(T value, endian target) {
  if (target == endian::native) {
    return value;
  }

  return (target == endian::little) == is_little_endian() ? value
                                                       : swap_bytes(value);
}
}

namespace gr::uc {

// template <typename char_type> class iter;

/**
 * @brief Character chunk buffer template
 * @tparam CharT Character type
 */
template <typename char_type> struct chunk_proxy;

/**
 * @brief UTF-8 chunk specialization (6-byte buffer)
 */
template <> struct chunk_proxy<char> {
  /**
   * @brief default constructor
   */
  chunk_proxy() = default;

  /**
   * @brief construct by sequence view
   * @note - if size of view large than 4, the sequnence size set as 4
   * @note - this is unsafe constructor
   */
  chunk_proxy(std::basic_string_view<char> v) {
    size_t _size = v.size();
    std::copy_n(v.data(), (_size < 5 ? _size : 4), buf);
    buf[5] = _size;
  }

  /**
   * @brief Get valid sequence length
   * @return Length of valid sequence in buffer
   */
  constexpr int size() const { return buf[5]; }

  /**
   * @brief Convet current buffer to std::string_view<T>
   */
  auto view() const -> std::basic_string_view<char> {
    return {buf, size_t(this->size())}; 
  }

  /**
   * @brief make a proxy of replacement buffer
   */
  static constexpr chunk_proxy<char> make_replacement() {
    chunk_proxy<char> result;
    result.buf[0] = '\xEF';
    result.buf[1] = '\xBF';
    result.buf[2] = '\xBD';
    result.buf[5] = 3;
    return result;
  }

  /**
   * @brief C-Style string buffer
   */
  char buf[6] = {0}; ///< Data buffer with length stored in last byte
};

/**
 * @brief UTF-16 chunk specialization (4-element buffer)
 */
template <> struct chunk_proxy<char16_t> {
  /**
   * @brief default constructor
   */
  chunk_proxy() = default;

  /**
   * @brief construct by sequence view
   * @note - if size of view large than 2, the sequnence size set as 2
   * @note - this is unsafe constructor
   */
  chunk_proxy(std::basic_string_view<char16_t> v) {
    size_t _size = v.size() < 3;
    std::copy_n(v.data(), (_size < 3 ? _size : 2), buf);
    buf[3] = _size;
  }

  /**
   * @brief Get valid sequence length
   * @return Length of valid sequence in buffer
   */
  constexpr int size() const { return buf[3]; }

  /**
   * @brief Convet current buffer to std::string_view<T>
   */
  auto view() const {
    return std::basic_string_view<char16_t>(buf, this->size());
  }

  /**
   * @brief make a proxy of replacement buffer
   */
  static constexpr chunk_proxy<char16_t> make_replacement() {
    chunk_proxy<char16_t> result;
    result.buf[0] = 0xFFFD;
    result.buf[3] = 1;
    return result;
  }

  /**
   * @brief C-Style string buffer
   */
  char16_t buf[4] = {0}; ///< Data buffer with length stored in last element
};

using chunk_proxy8 = chunk_proxy<char>;
using chunk_proxy16 = chunk_proxy<char16_t>;

/**
 * @brief Sequence status enumeration
 */
enum class sequence_status : uint8_t {
  valid,               /// Valid sequence
  invalid_continuation, /// Invalid continuation byte
  truncated,           /// Truncated sequence
  invalid_starbyte      /// Invalid start byte
};

/**
 * @brief Convert uc::sequences_status to string
 */
const char *get_status_info(sequence_status status);
/**
 * @brief Error handling strategy enumeration
 */
enum class on_failed : uint8_t {
  skip,     /// Skip invalid sequences
  keep,     /// Continue processing but mark as invalid
  error     /// Throw exception
};

/**
 * @brief Unicode code point wrapper class
 */
class codepoint {
  char32_t m_value; ///< Internal code point value

public:
  /**
   * @brief Default constructor (null code point)
   */
  constexpr codepoint() noexcept : m_value(0) {}
  /**
   * @brief Explicit constructor
   * @param v Code point value
   */
  constexpr explicit codepoint(char32_t ch) noexcept : m_value(ch) {}

  /**
   * @brief Constructor from UTF-8 string view (first code point)
   * @param sv UTF-8 string view
   * @note Extracts first valid code point from UTF-8 sequence
   */
  explicit codepoint(std::string_view sv);

  /**
   * @brief Constructor from UTF-16 string view (first code point)
   * @param sv UTF-16 string view
   * @note Extracts first valid code point from UTF-16 sequence
   */
  explicit codepoint(std::u16string_view sv);

  /**
   * @brief Constructor from UTF-32 string view (first code point)
   * @param sv UTF-32 string view
   */
  explicit codepoint(std::u32string_view sv) : m_value(sv.empty() ? 0 : sv[0]) {}

  constexpr codepoint &operator=(char32_t ch) noexcept { m_value = ch; return *this; }
  /**
   * @brief Conversion to char32_t
   * @return The code point value
   */
  constexpr explicit operator char32_t() const noexcept { return m_value; }

  bool operator==(codepoint rhs) const noexcept {
    return m_value == rhs.m_value;
  }

  bool operator!=(codepoint rhs) const noexcept {
    return m_value != rhs.m_value;
  }

  bool operator==(char32_t rhs) const noexcept {
    return m_value == rhs;
  }

  bool operator!=(char32_t rhs) const noexcept {
    return m_value != rhs;
  }

  /**
   * @brief Check if code point is valid Unicode and not a surrogate
   * @return true if code point is valid Unicode and not a surrogate
   * @note NULL character (U+0000) is considered valid by Unicode standard
   */
  constexpr bool is_valid() const noexcept {
    return m_value <= 0x10FFFF &&
           !(m_value >= 0xD800 && m_value <= 0xDFFF); // Exclude surrogates
  }

  /**
   * @brief Check if code point represents a printable/visible character
   * @return true if code point represents a visible character
   * @note Excludes control characters, format characters, and other
   * non-printing characters
   * @see Unicode General_Category property for detailed classification
   */
  bool is_printable() const;

  /**
   * @brief Check if code point is whitespace character
   * @return true if code point is whitespace
   */
  bool is_whitespace() const;

  constexpr bool is_digit() { return m_value >= '0' && m_value <= '9'; }

  constexpr bool is_alphabetic() const {
    return (m_value >= 'A' && m_value <= 'Z') ||
           (m_value >= 'a' && m_value <= 'z') ||
           (m_value >= 0xC0 && m_value <= 0x2FF); // 扩展拉丁字符
  }

  constexpr bool is_ascii() const noexcept { return m_value <= 0x7F; }
  constexpr bool is_bmp() const noexcept { return m_value <= 0xFFFF; }

  explicit operator bool() const noexcept { return m_value != 0; }

  /**
   * @brief Get display width of code point in terminal columns
   * @return Display width (1 for narrow, 2 for wide, 0 for control characters)
   * @note Based on Unicode East Asian Width property
   */
  int display_width() const;

  /**
   * @brief Convert to UTF-8 encoding
   * @return chunk8 containing UTF-8 bytes
   * @note Invalid code points are replaced with U+FFFD
   */
  chunk_proxy8 chunk_u8() const;

  /**
   * @brief Convert to UTF-16 encoding
   * @return chunk16 containing UTF-16 bytes
   * @note Invalid code points are replaced with U+FFFD
   */
  chunk_proxy16 chunk_u16() const;

  /**
   * @brief Convert code point to uppercase form
   * @return Uppercase code point, or original code point if no conversion
   * applies
   */
  codepoint upper() const;

  /**
   * @brief Convert code point to lowercase form
   * @return Lowercase code point, or original code point if no conversion
   * applies
   */
  codepoint lower() const;

  /*
   * @brief get code point original value
   * @return the code point value as char32_t
   */
  constexpr char32_t value() const noexcept { return m_value; }

  /*
   * @brief get code point original value
   * @return the code point value as uint32_t
   */
  constexpr uint32_t code() const noexcept { return m_value; }

  static constexpr codepoint make_replacement() { return codepoint(0xFFFD); };
};

/**
 * @brief Sequence processing result
 */

struct sequence_info {
  size_t length;          ///< Sequence length in bytes
  sequence_status status; ///< Sequence status
};

/**
 * @brief UTF specialization implementation
 */
struct sequence {
  /**
   * @brief Calculate UTF-8 sequence length (SSE accelerated)
   */
  static sequence_info
  check(const char* current, const char* end,
        gr::endian endian = gr::endian::native);

  /**
   * @brief convert sequence to codepoint(char32_t) after checked
   */
  static codepoint
  decode(const char* current, uint8_t seq_len,
         sequence_status status,
         gr::endian endian = gr::endian::native);

  /**
   * @brief after check(...), get original sequence from string_view
   */
  static constexpr std::string_view view(const char* pos,
                               sequence_info res) {
    return std::string_view(pos, res.length);
  }
  /**
   * @brief Calculate UTF-16 sequence length
   */
  static sequence_info
  check(const char16_t* current, const char16_t* end,
        gr::endian endian = gr::endian::native);

  /**
   * @brief convert sequence to codepoint(char32_t) after checked
   */
  static codepoint
  decode(const char16_t* current, uint8_t seq_len,
         sequence_status status,
         gr::endian endian = gr::endian::native);

  /**
   * @brief after check(...), get original sequence from string_view
   */
  static constexpr std::u16string_view view(const char16_t* pos,
                                  sequence_info res) {
    return std::u16string_view(pos, res.length);
  }
  /**
   * @brief Calculate UTF-32 sequence length
   */
  static sequence_info
  check(const char32_t* current, const char32_t* end,
        gr::endian endian_ = gr::endian::native) {
    (void)endian_;
    (void)end;
    (void)current;
    // return {1, current < end ? sequence_status::valid
    //                            : sequence_status::truncated};
    return {1, sequence_status::valid};
  }

  /**
   * @brief convert sequence to codepoint(char32_t) after checked
   */
  static codepoint
  decode(const char32_t* current, uint8_t seq_len, sequence_status status,
         gr::endian endian_ = gr::endian::native) {

    (void)seq_len;
    char32_t value = gr::convert_endian(*current, endian_);

    return codepoint(
        (status == sequence_status::valid) ? value : 0xFFFD);
  }

  /**
   * @brief after check(...), get original sequence from string_view
   */
  static constexpr std::u32string_view view(std::u32string_view sv, size_t pos,
                                  sequence_info) {
    return sv.substr(pos, 1);
  }
};

} // namespace gr::uc
