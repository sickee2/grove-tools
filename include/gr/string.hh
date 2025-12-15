/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file string.hh
 * @brief UTF-8/16/32 string handling utilities with full Unicode support
 * @ingroup unicode
 *
 * Provides comprehensive Unicode-aware string classes and utilities supporting
 * UTF-8, UTF-16, and UTF-32 encodings with advanced text processing capabilities.
 *
 * ## Core Features
 * - **Multi-Encoding Support**: UTF-8, UTF-16, UTF-32 string operations
 * - **Unicode-Aware Iteration**: Code point-level traversal and manipulation
 * - **Encoding Conversion**: Cross-encoding transformation utilities
 * - **Case Conversion**: Unicode-compliant case mapping
 * - **BOM Handling**: Automatic byte order mark detection and management
 * - **Text Processing**: Trimming, splitting, joining, alignment operations
 * - **Regular Expressions**: RE2 integration for pattern matching (when available)
 * - **Encoding Detection**: Character encoding conversion via iconv (when available)
 *
 * ## Key Components
 *
 * ### utf_view<char_type> Template Class
 * Lightweight string view wrapper providing:
 * - Zero-copy Unicode string operations
 * - BOM detection and automatic handling
 * - Unicode-aware iteration and range adapters
 * - String manipulation utilities (trim, split, join, etc.)
 *
 * ### utf<char_type, Alloc> Template Class
 * Full-featured Unicode string class inheriting from `std::basic_string`:
 * - Complete STL container compatibility
 * - Advanced Unicode operations
 * - Memory management with custom allocators
 * - In-place and copy-based string transformations
 *
 * ### code_converter Class
 * Character encoding conversion using iconv library:
 * - Support for legacy encodings (GB18030, GBK, Big5, etc.)
 * - Configurable error handling
 * - Efficient batch conversion
 *
 * ## Integration with Unicode Infrastructure
 *
 * Tightly integrated with Unicode core components:
 * - `gr::uc::iter`: Unicode-aware string iteration
 * - `gr::uc::sequence`: Encoding validation and sequence processing
 * - `gr::uc::codepoint`: Individual code point operations
 *
 * ## String Operations
 *
 * ### Basic Manipulation
 * - Trimming (Unicode-aware and ASCII variants)
 * - Case conversion (to_lower, to_upper)
 * - Prefix/suffix checking (starts_with, ends_with)
 * - Substring operations
 *
 * ### Advanced Processing
 * - String splitting with configurable delimiters
 * - Efficient string joining with pre-calculation
 * - Display width calculation considering Unicode character widths
 * - Text alignment (left, right, center) with Unicode column awareness
 *
 * ### Pattern Matching
 * - KMP algorithm for efficient substring search
 * - RE2 regex support (when GR_HAS_RE2 is defined)
 * - Multiple search strategies (find, rfind, find_all)
 *
 * ## BOM and Endianness Handling
 *
 * Automatic detection and management of:
 * - UTF-8 BOM (EF BB BF)
 * - UTF-16 BOM (FEFF/FFFE)
 * - UTF-32 BOM (0000FEFF/FFFE0000)
 * - Byte order conversion for UTF-16/32
 *
 * ## Usage Examples
 *
 * ### Basic String Operations
 * ```cpp
 * gr::str::u8 text = "Hello 世界"_u8;
 * text.utrim().to_upper(); // Unicode-aware operations
 * ```
 *
 * ### Encoding Conversion
 * ```cpp
 * auto utf16_text = text.to_u16(); // UTF-8 to UTF-16 conversion
 * auto utf32_text = text.to_u32(); // UTF-8 to UTF-32 conversion
 * ```
 *
 * ### BOM Handling
 * ```cpp
 * if (text.has_bom()) {
 *     auto clean_text = text.without_bom(); // Remove BOM
 * }
 * text.add_bom(gr::endian::little); // Add BOM with specified endianness
 * ```
 *
 * ### Advanced Text Processing
 * ```cpp
 * auto words = text.split(" "_u8v); // Split by space
 * auto joined = ", "_u8v.join_ls(words); // Join with comma
 * auto centered = text.ucenter(20); // Center align considering display width
 * ```
 *
 * ## Dependencies
 * - `gr/utf_iter.hh`: Unicode iteration infrastructure
 * - `gr/utf_sequence.hh`: Encoding validation and code point operations
 * - `gr/utils.hh`: Utility functions and platform abstractions
 * - `re2/re2.h`: Regular expression support (optional)
 * - iconv library: Encoding conversion (optional)
 *
 * ## Configuration Options
 * - `GR_HAS_RE2`: Enable RE2 regex support
 * - `GR_HAS_CHAR8_T`: C++20 char8_t support
 * - `DISABLE_SUPPORT_ICONV`: Disable iconv-based encoding conversion
 * - `GR_HAS_CPP20`: C++20 language features
 *
 * ## Performance Characteristics
 * - Zero-copy operations for view-based classes
 * - Efficient memory pre-allocation for string building
 * - Lazy evaluation for expensive operations
 * - Optimized algorithms for common string operations
 *
 * @see gr::uc::iter for Unicode iteration
 * @see gr::uc::codepoint for individual code point operations
 * @see gr::uc::sequence for encoding validation
 */

#pragma once
#include <algorithm>
#include <cstdlib>
#include <gr/config.hh>
#include <gr/utf_iter.hh>
#include <gr/utf_sequence.hh>
#include <gr/utils.hh>
#include <initializer_list>
#include <memory>
#if GR_HAS_RE2
#include <re2/re2.h>
#endif
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace gr {
namespace str {
namespace toy {
void hack_string_data(void *s, unsigned char_width, gr::utils::cbuf<char> cbuf);
void hack_string_size(void *s, size_t n);
} // namespace toy
namespace bom {
/**
 * @brief BOM 检测结果
 */
struct info {
  bool has_bom = false; ///< 是否包含 BOM
  gr::endian endian;    ///< 检测到的字节序
  uint8_t bom_size = 0; ///< BOM 大小（字节数）
};

// UTF-8 BOM
inline constexpr char8_t utf8[] = u8"\uFEFF";
inline constexpr char utf8_bytes[] = "\xEF\xBB\xBF";

// UTF-16 BOM
inline constexpr char16_t utf16_le = 0xFEFF; // Little Endian
inline constexpr char16_t utf16_be = 0xFFFE; // Big Endian

// UTF-32 BOM
inline constexpr char32_t utf32_le = 0x0000FEFF; // Little Endian
inline constexpr char32_t utf32_be = 0xFFFE0000; // Big Endian

template <typename char_type> info detect(const char_type *, size_t);
template <> info detect(const char *s, size_t n);
template <> info detect(const char16_t *s, size_t n);
template <> info detect(const char32_t *s, size_t n);

} // namespace bom

/**
 * @brief Calculate length of null-terminated string with optional limit
 * @tparam char_type Character type
 * @param s Null-terminated string
 * @param limit Maximum length to check (0 for unlimited)
 * @return Length in code units
 */
template <typename char_type>
size_t length(const char_type *s, size_t limit = 0) {
  if (limit == 0 || limit == utils::nopos) {
    size_t len = 0;
    for (auto p = s; *p != 0; ++p) {
      ++len;
    }
    return len;
  } else {
    size_t len = 0;
    for (auto p = s; *p != 0 && len < limit; ++p) {
      ++len;
    }
    return len;
  }
}

template <typename char_type, typename Alloc = std::allocator<char_type>>
class utf;

template <typename char_type> class utf_view;

/**
 * @brief Batch process Unicode code points in a UTF string view
 * @tparam char_type Character type (char, char16_t, or char32_t)
 * @tparam Func Function type that accepts (codepoint, sequence_status) and
 * returns bool
 * @param utfview UTF string view to process
 * @param func Callback function to process each code point
 * @param endian Endianness for byte order interpretation
 *
 * @details
 * This function iterates through the UTF string view and processes each Unicode
 * code point sequentially. For each valid or invalid sequence found, it calls
 * the provided function with the decoded code point and sequence status.
 *
 * The callback function should have the signature:
 *   bool func(gr::uc::codepoint cp, gr::uc::sequence_status status)
 *
 * If the callback returns false, processing stops immediately.
 *
 * @note
 * - Invalid sequences are handled according to the sequence checking logic
 * - The function automatically advances through multi-byte sequences
 * - Byte order is respected for UTF-16 and UTF-32 encodings
 *
 * @example
 * // Count valid code points
 * size_t count = 0;
 * batch_process_utf(my_string, [&count](gr::uc::codepoint cp,
 * gr::uc::sequence_status status) { if (status ==
 * gr::uc::sequence_status::valid) { count++;
 *     }
 *     return true; // Continue processing
 * });
 */
template <typename char_type, typename Func>
void batch_process_utf(utf_view<char_type> utfview, Func &&func,
                       gr::endian endian = gr::endian::native);

/**
 * @brief Batch check Unicode code sequence status in a UTF string view
 * @tparam char_type Character type (char, char16_t, or char32_t)
 * @tparam Func Function type that accepts (sequence_status) and returns bool
 * @param utfview UTF string view to process
 * @param func Callback function to process each code point
 * @param endian Endianness for byte order interpretation
 *
 * @details
 * This function iterates through the UTF string view and check each Unicode
 * code sequence sequentially. For each valid or invalid sequence found, it
 * calls the provided function with the sequence status.
 *
 * The callback function should have the signature:
 *   bool func(gr::uc::sequence_status status)
 *
 * If the callback returns false, processing stops immediately.
 *
 * @note
 * - Invalid sequences are handled according to the sequence checking logic
 * - The function automatically advances through multi-byte sequences
 * - Byte order is respected for UTF-16 and UTF-32 encodings
 *
 * @example
 * // Count valid code points
 * size_t count = 0;
 * batch_process_utf(my_string, [&count](gr::uc::sequence_status status) {
 *     if (status == gr::uc::sequence_status::valid) {
 *         count++;
 *     }
 *     return true; // Continue processing
 * });
 */
template <typename char_type, typename Func>
void batch_check_utf(utf_view<char_type> utfview, Func &&func,
                     gr::endian endian = gr::endian::native);
/**
 * @brief UTF string view template
 * @tparam char_type Character type (char, char16_t, char32_t)
 */
template <typename char_type>
class utf_view : public std::basic_string_view<char_type> {
  static_assert(gr::uc::detail::_valid_char_type_helper<char_type>::value,
                "gr::str::utf_view<...> arg char_type must be char, char16_t, or char32_t");

public:
  using value_type = char_type;
  using base_type = std::basic_string_view<char_type>;
  // using base_type::base_type;

  /**
   * @brief Construct empty view
   */
  GR_CONSTEXPR_OR_INLINE utf_view() = default;

  /**
   * @brief Construct from null-terminated string
   * @param s Null-terminated string
   */
  GR_CONSTEXPR_OR_INLINE utf_view(const char_type *s) : base_type(s) {
  }

  /**
   * @brief Construct from null-terminated string
   * @param s Null-terminated string
   * @param n size of string
   */
  GR_CONSTEXPR_OR_INLINE utf_view(const char_type *s, size_t n)
      : base_type(s, n) {
  }

  GR_CONSTEXPR_OR_INLINE utf_view(const char_type *s, const char_type *_end)
      : base_type(s, _end - s) {
  }
  /**
   * @brief Construct from string literal
   * @tparam N String literal length (automatically deduced)
   * @param s String literal
   */
  template <size_t N>
  GR_CONSTEXPR_OR_INLINE utf_view(const char_type (&s)[N])
      : base_type(s, N - 1)
  {
  }

  /**
   * @brief Construct from std::basic_string_view
   * @param s Null-terminated string
   */
  GR_CONSTEXPR_OR_INLINE utf_view(const base_type &s) : base_type(s) {}

  /**
   * @brief Construct from utf_view
   * @param s Null-terminated string
   */
  GR_CONSTEXPR_OR_INLINE utf_view(const utf_view &s)
      : base_type(s.data(), s.size()) {}

#if GR_HAS_CHAR8_T
  /**
   * @brief Construct from char8_t string (UTF-8)
   * @param s UTF-8 null-terminated string
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  utf_view(const char8_t *s) : base_type(reinterpret_cast<const char *>(s)) {
    static_assert(std::is_same_v<T, char>,
                  "utf_view<char8_t> constructor only available for char_type == char");
  }

  /**
   * @brief Construct from char8_t string with size (UTF-8)
   * @param s UTF-8 character array
   * @param n Number of characters to use
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  GR_CONSTEXPR_OR_INLINE utf_view(const char8_t *s, size_t n)
      : base_type(reinterpret_cast<const char *>(s), n) {
    static_assert(std::is_same_v<T, char>,
                  "utf_view<u8string_view> constructor only available for char_type == char");
  }

  /**
   * @brief Construct from std::u8string_view
   * @param sv UTF-8 string view
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  GR_CONSTEXPR_OR_INLINE utf_view(std::u8string_view sv)
      : base_type(reinterpret_cast<const char *>(sv.data()), sv.size()) {
    static_assert(std::is_same_v<T, char>,
                  "utf_view<u8string_view> constructor only available for char_type == char");
  }

#endif
  /**
   * @brief Copy assignment operator
   */
  GR_CONSTEXPR_OR_INLINE utf_view &operator=(const utf_view &s) {
    base_type::operator=(s);
    return *this;
  }

  /**
   * @brief Detect BOM and endianness from string data
   * @return BOM detection result
   */
  [[nodiscard]]
  bom::info detect_bom() const {
    return bom::detect(this->data(), this->size());
  }

  /**
   * @brief Get view without BOM
   * @return View excluding BOM if present
   */
  [[nodiscard]]
  utf_view without_bom() const {
    auto bom_info = detect_bom();
    if (bom_info.has_bom) {
      return this->sub_view(bom_info.bom_size);
    }
    return *this;
  }

  /**
   * @brief Check if string starts with BOM
   * @return true if BOM is present
   */
  [[nodiscard]]
  bool has_bom() const {
    return detect_bom().has_bom;
  }

  /**
   * @brief Get detected endianness from BOM
   * @return Detected endianness or native if no BOM
   */
  [[nodiscard]]
  gr::endian detected_endian() const {
    return detect_bom().endian;
  }

  /**
   * @brief Create iterator that automatically handles BOM and endianness
   * @param fb Invalid sequence handling strategy
   * @return Iterator starting after BOM with correct endianness
   */
  [[nodiscard]]
  auto ubegin_auto(uc::on_failed fb = uc::on_failed::skip) const {
    auto bom_info = detect_bom();
    size_t start_pos = bom_info.has_bom ? bom_info.bom_size : 0;
    gr::endian endian = bom_info.has_bom ? bom_info.endian : gr::endian::native;

    return gr::uc::make_iterator(*this, start_pos, fb, endian);
  }

  /**
   * @brief Create range that automatically handles BOM and endianness
   * @param fb Invalid sequence handling strategy
   * @return Range starting after BOM with correct endianness
   */
  [[nodiscard]]
  auto urange_auto(uc::on_failed fb = uc::on_failed::skip) const {
    auto bom_info = detect_bom();
    size_t start_pos = bom_info.has_bom ? bom_info.bom_size : 0;
    gr::endian endian = bom_info.has_bom ? bom_info.endian : gr::endian::native;

    return uc::range<char_type>(
        gr::uc::make_iterator(*this, start_pos, fb, endian),
        gr::uc::make_iterator(*this, this->size(), fb, endian));
  }

  /**
   * @brief Get Unicode-aware begin iterator
   * @param fb Invalid sequence handling strategy
   * @return Iterator to first code point
   */
  [[nodiscard]]
  auto ubegin(uc::on_failed fb = uc::on_failed::skip,
              gr::endian endian = gr::endian::native) const {
    return gr::uc::make_iterator(*this, 0, fb, endian);
  }

  /**
   * @brief Get Unicode-aware end iterator
   * @param fb Invalid sequence handling strategy
   * @return Iterator to end marker
   */
  [[nodiscard]]
  auto uend(uc::on_failed fb = uc::on_failed::skip,
            gr::endian endian = gr::endian::native) const {
    return gr::uc::make_iterator(*this, this->size(), fb, endian);
  }

  /**
   * @brief Get Unicode-aware last iterator
   * @param fb Invalid sequence handling strategy
   * @return Iterator to end marker
   */
  [[nodiscard]]
  auto ulast(uc::on_failed fb = uc::on_failed::skip,
             gr::endian endian = gr::endian::native) const {
    auto it = uend(fb, endian);
    --it;
    return it;
  }

  /**
   * @brief Count valid Unicode code points
   * @return Number of code points (skipping invalid sequences)
   * @note If char_type is char32_t should ignore uc::on_failed and endian
   */
  [[nodiscard]]
  size_t usize(uc::on_failed fb = uc::on_failed::skip,
               gr::endian endian_ = gr::endian::native) {
    if (std::is_same_v<char_type, char32_t>) {
      return this->size();
    }
    size_t n = 0;
    const char_type *current = this->data();
    const char_type *end = current + this->size();
    while (current < end) {
      auto seq_info = uc::sequence::check(current, end, endian_);
      if (seq_info.status == uc::sequence_status::valid) {
        n++;
      } else {
        if (fb == uc::on_failed::keep) {
          n++;
        }
      //   else if (fb == uc::on_failed::error) {
      //     throw std::runtime_error("Invalid UTF sequence encountered");
      //   }
      }
      current += seq_info.length > 0 ? seq_info.length : 1;
    }
    return n;
  }

  /**
   * @brief Unicode-aware trim leading/trailing whitespace
   * @return Reference to modified view
   * @post View will contain no leading/trailing Unicode whitespace
   */
  utf_view &utrim();

  /**
   * @brief Unicode-aware trim whitespace from left
   * @return Reference to modified view
   * @post View will contain no leading Unicode whitespace
   */
  utf_view &utrim_left();

  /**
   * @brief Unicode-aware trim whitespace from right
   * @return Reference to modified view
   * @post View will contain no trailing Unicode whitespace
   */
  utf_view &utrim_right();

  /**
   * @brief Trim leading/trailing whitespace for ASCII
   * @return Reference to modified view
   * @post View will contain no leading/trailing whitespace
   */

  utf_view &trim();

  /**
   * @brief Trim whitespace from left for ASCII
   * @return Reference to modified view
   * @post View will contain no leading/trailing whitespace
   */
  utf_view &trim_left();

  /**
   * @brief Trim whitespace from right for ASCII
   * @return Reference to modified view
   * @post View will contain no leading/trailing whitespace
   */
  utf_view &trim_right();

  /**
   * @brief Split string by delimiter
   * @param delimiter View of delimiter string
   * @return Vector of substrings
   * @note Empty substrings are ignore
   */
  [[nodiscard]]
  auto split(utf_view delimiter) const -> std::vector<utf_view<char_type>>;

 /**
  * @brief Join multiple strings from a container with the separator
  * @tparam container Container type that holds utf_view<char_type> elements
  * @param strings Container of strings to join
  * @return New string containing all strings joined with separator
  * @note If separator is empty, strings are concatenated without separation
  * @note This function pre-calculates total length for optimal performance
  * @example
  *   std::vector<u8v> words = {"apple", "banana", "orange"};
  *   ":"_u8v.join_ls(", ", words); // "apple, banana, orange"
  */
  template<typename container>
  [[nodiscard]] utf<char_type> join_ls(const container& strings);
  [[nodiscard]] utf<char_type> join(const std::initializer_list<utf_view>& strings){
    return this->join_ls(strings);
  }

  [[nodiscard]]
  auto word_boundaries() const -> std::vector<uint32_t>;

  /**
   * @brief align center
   * @param n width of block
   */
  [[nodiscard]]
  utf<char_type> center(size_t width, char_type ch = ' ') const;

  /**
   * @brief align letf
   * @param n width of block
   */
  [[nodiscard]]
  utf<char_type> ljust(size_t width, char_type ch = ' ') const;

  /**
   * @brief align right
   * @param n width of block
   */
  [[nodiscard]]
  utf<char_type> rjust(size_t width, char_type ch = ' ') const;

  /**
   * @brief Calculate display width considering Unicode character widths
   * @return Total display width in columns
   */
  [[nodiscard]]
  size_t udisplay_width() const;

  /**
   * @brief Unicode-aware center alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Centered string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> ucenter(size_t width, char_type ch = ' ') const;

  /**
   * @brief Unicode-aware left alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Left-aligned string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> uljust(size_t width, char_type ch = ' ') const;

  /**
   * @brief Unicode-aware right alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Right-aligned string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> urjust(size_t width, char_type ch = ' ') const;

  /**
   * @brief Convert to a std::basic_string_view<char_type>
   */
  [[nodiscard]]
  auto as_string_view() const {
    return base_type(*this);
  }

  /**
   * @brief Convert to a std::basic_string<char_type>
   */
  [[nodiscard]]
  utf<char_type> to_str() const;

  [[nodiscard]]
  utf<char> to_u8() const;

  [[nodiscard]]
  utf<char16_t> to_u16() const;

  [[nodiscard]]
  utf<char32_t> to_u32() const;

  [[nodiscard]]
  auto at(size_t pos) const {
    if (pos >= this->size()) {
      throw std::out_of_range("gr::utf_view<>::at => ouf of range");
    }
    return (*this)[pos];
  }

  /**
   * @brief Get sub-uft_view
   * @param pos start index of str
   * @param n number bytes
   * @retrun gr::str::utf_view
   */
  [[nodiscard]]
  GR_CONSTEXPR_OR_INLINE auto sub_view(size_t pos, size_t n = 0) const {
    auto _size = this->size();
    if (pos >= _size)
      return utf_view<char_type>();
    if (n == 0)
      n = _size - pos;
    if (pos + n > _size)
      n = _size - pos;
    return utf_view<char_type>(this->data() + pos, n);
  }

  /**
   * @brief Make a uc::range of this view. The range is valid only as long as
   * the utf_view<char_type> remains alive.  This operation doesn't allocate any
   * memory.
   */
  [[nodiscard]]
  auto urange(uc::on_failed fb = uc::on_failed::skip,
              gr::endian endian_ = gr::endian::native) const {
    return uc::range<char_type>(this->ubegin(fb, endian_),
                                this->uend(fb, endian_));
  }

  /**
   * @brief Get bytes of the string
   * @return size_t
   */
  [[nodiscard]]
  GR_CONSTEXPR_OR_INLINE size_t bytes() const noexcept {
    return this->size() * sizeof(char_type);
  }


  /**
   * @brief Find substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for search
   * @return Position of first occurrence or nopos if not found
   * @note Uses Knuth-Morris-Pratt algorithm for efficient substring search
   */
  [[nodiscard]]
  size_t find_kmp(utf_view<char_type> pattern, size_t pos = 0) const;
  /**
   * @brief Find last occurrence of substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for reverse search ( 0 = last_char_pos)
   * @return Position of last occurrence or nopos if not found
   * @note Uses reverse KMP algorithm for efficient substring search
   */
  [[nodiscard]]
  size_t rfind_kmp(utf_view<char_type> pattern, size_t pos = 0) const;
  /**
   * @brief Find all occurrences of substring using KMP algorithm
   * @param pattern Substring to find
   * @return Vector of positions where pattern occurs
   * @note Uses KMP algorithm for efficient substring search
   */
  [[nodiscard]]
  std::vector<size_t> find_all_kmp(utf_view<char_type> pattern) const;
};

/**
 * @class utf
 * @brief Unicode-aware string class supporting UTF-8/16/32 encodings
 * @tparam char_type Character type (char, char16_t, or char32_t)
 *
 * Provides enhanced string operations with full Unicode support including:
 * - Encoding-aware iteration and manipulation
 * - Case conversion
 * - Validation and normalization
 * - Cross-encoding conversion
 * - STL container compatibility
 */
template <typename char_type, typename Alloc>
class utf
    : public std::basic_string<char_type, std::char_traits<char_type>, Alloc> {
  static_assert(
      gr::uc::detail::_valid_char_type_helper<char_type>::value,
      "gr::str::utf<...> arg char_type must be char, char16_t, or char32_t");

public:
  using value_type = char_type;
  using base_type =
      std::basic_string<char_type, std::char_traits<char_type>, Alloc>;
  using base_vw_type = std::basic_string_view<char_type>;

  /**
   * @brief Default constructor
   */
  GR_CONSTEXPR_OR_INLINE utf() = default;

  /**
   * @brief Construct from null-terminated string
   * @param s Null-terminated character array
   */
  GR_CONSTEXPR_OR_INLINE utf(const char_type *s) : base_type(s) {}

  /**
   * @brief Construct from character array with size
   * @param s Character array
   * @param n Number of characters to use
   */
  GR_CONSTEXPR_OR_INLINE utf(const char_type *s, size_t n) : base_type(s, n) {}

  /**
   * @brief Construct from a std::basic_string
   * @param s std::basic_string
   */
  GR_CONSTEXPR_OR_INLINE utf(const base_type &s) : base_type(s) {}

  /**
   * @brief Construct from rvalue of std::basic_string
   * @param s std::basic_string
   */
  GR_CONSTEXPR_OR_INLINE utf(base_type &&s) : base_type(std::move(s)) {}

  /**
   * @brief Copy construct from a gr::str::utf object
   * @param s gr::str::utf object
   */
  GR_CONSTEXPR_OR_INLINE utf(const utf<char_type> &s)
      : base_type(s.data(), s.size()) {}

  /**
   * @brief Copy construct from a rvalue of gr::str::utf object
   * @param s gr::str::utf object
   */
  GR_CONSTEXPR_OR_INLINE utf(utf &&s) : base_type(std::move(s)) {}

  /**
   * @brief Copy construct with repeated character
   * @param n Number of characters to copy
   * @param c Character to copy
   */
  GR_CONSTEXPR_OR_INLINE utf(size_t n, char_type c) : base_type(n, c) {}

  /**
   * @brief construct empty gr::str::utf object and pre reserve memory
   * @param n number characters memory to reserved.
   */
  GR_CONSTEXPR_OR_INLINE utf(size_t n) : base_type() { base_type::reserve(n); }

  /**
   * @brief Copy construct from a gr::str::utf_view object
   * @param s gr::uc::utf_view object
   */
  GR_CONSTEXPR_OR_INLINE utf(const base_vw_type &s)
      : base_type(s.data(), s.size()) {}

#if GR_HAS_CHAR8_T
  /**
   * @brief Construct from char8_t null-terminated string (UTF-8)
   * @param s UTF-8 null-terminated character array
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  utf(const char8_t *s) : base_type(reinterpret_cast<const char *>(s)) {
    static_assert(std::is_same_v<T, char>,
                  "utf<char8_t> constructor only available for char_type == char");
  }

  /**
   * @brief Construct from char8_t character array with size (UTF-8)
   * @param s UTF-8 character array
   * @param n Number of characters to use
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  utf(const char8_t *s, size_t n)
      : base_type(reinterpret_cast<const char *>(s), n) {
    static_assert(std::is_same_v<T, char>,
                  "utf<char8_t> constructor only available for char_type == char");
  }

  /**
   * @brief Construct from std::u8string (UTF-8)
   * @param s UTF-8 string
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
  utf(const std::u8string &s)
      : base_type(reinterpret_cast<const char *>(s.c_str()), s.size()) {
    static_assert(std::is_same_v<T, char>,
                  "utf<char8_t> constructor only available for char_type == char");
  }

  /**
   * @brief Construct from std::u8string_view (UTF-8)
   * @param sv UTF-8 string view
   * @note Only available for char_type == char
   */
  template <typename T = char_type>
    requires std::is_same_v<T, char>
  utf(std::u8string_view sv)
      : base_type(reinterpret_cast<const char *>(sv.data()), sv.size()) {}

#endif

  /**
   * @brief Construct from iterator range
   * @param first Begin iterator
   * @param last End iterator
   */
  template <typename InputIt>
  utf(InputIt first, InputIt last)
      : base_type(first, last) {
    static_assert(std::is_same_v<char_type, typename InputIt::value_type>, " iterator value type error");
  }

  /**
   * @brief Construct from initializer list
   * @param ilist Initializer list of characters
   */
  utf(std::initializer_list<char_type> ilist) : base_type(ilist) {}

  utf &operator=(const char_type *s) {
    this->assign(s);
    return *this;
  }

  utf &operator=(const base_type &s) {
    this->assign(s.data(), s.size());
    return *this;
  }

  utf &operator=(const base_vw_type &s) {
    this->assign(s.data(), s.size());
    return *this;
  }

  utf &operator=(const utf &s) {
    this->assign(s.c_str(), s.size());
    return *this;
  }

  utf &operator=(utf &&s) {
    *(base_type *)(this) = std::move(s);
    return *this;
  }

  /**
   * @brief Create iterator that automatically handles BOM and endianness
   * @param fb Invalid sequence handling strategy
   * @return Iterator starting after BOM with correct endianness
   */
  [[nodiscard]]
  auto ubegin_auto(uc::on_failed fb = uc::on_failed::skip) const {
    return this->as_view().ubegin_auto(fb);
  }

  /**
   * @brief Create range that automatically handles BOM and endianness
   * @param fb Invalid sequence handling strategy
   * @return Range starting after BOM with correct endianness
   */
  [[nodiscard]]
  auto urange_auto(uc::on_failed fb = uc::on_failed::skip) const {
    return this->as_view().urange_auto(fb);
  }

  /**
   * @brief Create Unicode range view
   * @return Range object for code point iteration
   */
  [[nodiscard]]
  auto urange(uc::on_failed fb = uc::on_failed::skip,
              gr::endian endian_ = gr::endian::native) const {
    return uc::range<char_type>(this->ubegin(fb, endian_),
                                this->uend(fb, endian_));
  }

  /**
   * @brief Unicode-aware begin iterator
   * @param fb on_error strategy for invalid sequences
   * @return Iterator to first code point
   */
  [[nodiscard]]
  auto ubegin(uc::on_failed fb = uc::on_failed::skip,
              gr::endian endian_ = gr::endian::native) const {
    return gr::uc::make_iterator(this->as_std_view(), 0, fb, endian_);
  }

  /**
   * @brief Unicode-aware end iterator
   * @param fb on_error strategy for invalid sequences
   * @return Iterator to end position
   */
  [[nodiscard]]
  auto uend(uc::on_failed fb = uc::on_failed::skip,
            gr::endian endian_ = gr::endian::native) const {
    return gr::uc::make_iterator(this->as_std_view(), this->size(), fb,
                                 endian_);
  }

  /**
   * @brief Unicode-aware last iterator
   * @param fb on_error strategy for invalid sequences
   * @return Iterator to end position
   */
  [[nodiscard]]
  auto ulast(uc::on_failed fb = uc::on_failed::skip,
             gr::endian endian_ = gr::endian::native) const {
    auto it = uend(fb, endian_);
    --it;
    return it;
  }

  /**
   * @brief Count valid Unicode code points
   * @return Number of code points (skipping invalid sequences)
   * @note If char_type is char32_t should ignore uc::on_failed and endian
   */
  [[nodiscard]]
  size_t usize(uc::on_failed fb = uc::on_failed::skip,
               gr::endian endian_ = gr::endian::native) const {
    return this->as_view().usize(fb, endian_);
  }

  /**
   * @@brief Get a reference to the underlying `std::basic_string` object.
   * @return A reference to the underlying `std::basic_string` object.
   */
  [[nodiscard]]
  base_type &as_std_string() const {
    return *((base_type *)(this));
  }

  /**
   * @brief Convert to string view
   * @return View of underlying data
   */
  [[nodiscard]]
  auto as_std_view() const {
    return base_vw_type(this->c_str(), this->size());
  }

  /**
   * @brief Convert to utf_view
   * @return View of underlying data
   */
  [[nodiscard]]
  auto as_view() const {
    return utf_view(this->c_str(), this->size());
  }

  /**
   * @brief auto convert to utf_view<char_type>.
   */
  operator utf_view<char_type>() const{
    return utf_view(this->c_str(), this->size());
  }

  [[nodiscard]]
  utf<char, std::allocator<char>> to_u8() const;

  [[nodiscard]]
  utf<char16_t, std::allocator<char16_t>> to_u16() const;

  [[nodiscard]]
  utf<char32_t, std::allocator<char32_t>> to_u32() const;

  /**
   * @brief Get sub-uft_view
   * @param pos start index of str
   * @param n number bytes
   * @retrun gr::str::utf_view
   */
  [[nodiscard]]
  auto sub_view(size_t pos, size_t n = 0) const {
    auto len = this->size();
    if (pos >= len)
      return utf_view<char_type>();
    if (n == 0)
      n = len - pos;
    if (pos + n > len)
      n = len - pos;
    return utf_view<char_type>(this->data() + pos, n);
  }

  /**
   * @brief Return a boolean indicating whether the string is empty.
   * @return true if the string is empty, false otherwise.
   */
  explicit operator bool() const { return this->empty() == false; }

  /**
   * @brief Unicode-aware trim leading/trailing whitespace
   * @return Reference to modified string
   * @note Uses Unicode-aware whitespace detection
   */
  utf &utrim();

  /**
   * @brief Unicode-aware trim leading whitespace
   * @return Reference to modified string
   * @note Uses Unicode-aware whitespace detection
   */
  utf &utrim_left();

  /**
   * @brief Unicode-aware trim trailing whitespace
   * @return Reference to modified string
   * @note Uses Unicode-aware whitespace detection
   */
  utf &utrim_right();

  /**
   * @brief Trim leading/trailing whitespace for ASCII
   * @return Reference to modified string
   * @note Uses std::isspace for whitespace detection
   */
  utf &trim();

  /**
   * @brief Trim leading whitespace for ASCII
   * @return Reference to modified string
   * @note Uses std::isspace for whitespace detection
   */
  utf &trim_left();

  /**
   * @brief Trim trailing whitespace for ASCII
   * @return Reference to modified string
   * @note Uses std::isspace for whitespace detection
   */
  utf &trim_right();

  /**
   * @brief Case conversion to lowercase for ASCII
   * @return Reference to modified string
   * @note Locale-sensitive operation
   */
  utf &to_lower() {
    for (auto &c : *this) {
      c = std::tolower(static_cast<unsigned char>(c));
    }
    return *this;
  }

  /**
   * @brief Case conversion to uppercase for ASCII
   * @return Reference to modified string
   * @note Locale-sensitive operation
   */
  utf &to_upper() {
    for (auto &c : *this) {
      c = std::toupper(static_cast<unsigned char>(c));
    }
    return *this;
  }

  /**
   * @brief Check string prefix
   * @param prefix View of prefix to check
   * @return true if string starts with prefix
   * @note Case-sensitive comparison
   */
  [[nodiscard]]
  bool starts_with(const utf_view<char_type> &prefix) const {
    if (prefix.size() > this->size())
      return false;
    return std::equal(prefix.begin(), prefix.end(), this->begin());
  }

  /**
   * @brief Check string suffix
   * @param suffix View of suffix to check
   * @return true if string ends with suffix
   * @note Case-sensitive comparison
   */
  [[nodiscard]]
  bool ends_with(const utf_view<char_type> &suffix) const {
    if (suffix.size() > this->size())
      return false;
    return std::equal(suffix.rbegin(), suffix.rend(), this->rbegin());
  }

  /**
   * @brief Split string by delimiter
   * @param delimiter View of delimiter string
   * @return Vector of substrings
   * @note Empty substrings are ignore
   */
  [[nodiscard]]
  auto split(utf_view<char_type> delimiter) const
      -> std::vector<utf_view<char_type>> {
    return this->as_view().split(delimiter);
  }

 /**
  * @brief Join multiple strings from a container with the separator
  * @tparam container Container type that holds utf_view<char_type> elements
  * @param strings Container of strings to join
  * @return New string containing all strings joined with separator
  * @note If separator is empty, strings are concatenated without separation
  * @note This function pre-calculates total length for optimal performance
  * @example
  *   std::vector<u8v> words = {"apple", "banana", "orange"};
  *   ":"_u8.join_ls(", ", words); // "apple, banana, orange"
  */
  template<typename container>
  [[nodiscard]] utf join_ls(const container& strings){
    return this->as_view().join_ls(strings);
  }
  [[nodiscard]] utf join(const std::initializer_list<utf_view<char_type>>& strings){
    return this->as_view().join(strings);
  }

  /**
   * @brief Replace all occurrences of substring
   * @param from Substring to replace
   * @param to Replacement string
   * @return Reference to modified string
   * @note Handles size changes efficiently
   */

  utf &replace_all_inplace(utf_view<char_type> from, utf_view<char_type> to);

  /**
   * @brief Create a copy and replece occurrences of substring
   * @param from Substring to replace
   * @param to Replacement string
   * @return Reference to modified string
   * @note Handles size changes efficiently
   */
  [[nodiscard]]
  utf replace_all(utf_view<char_type> from, utf_view<char_type> to) const;

#if GR_HAS_RE2
  /**
   * @brief Regex replacement using RE2
   * @param pattern RE2 pattern to match
   * @param new_s Replacement string
   * @return Reference to modified string
   * @requires char_type == char
   */
  template<typename T = char_type>
  auto replace_by_re2_inplace(const char_type *pattern,
                              const utf_view<char> new_s)
    -> std::enable_if_t<std::is_same_v<T, char>, utf&>
  {
    re2::RE2::GlobalReplace(this, pattern, new_s);
    return *this;
  }

  /**
   * @brief Check if string matches RE2 pattern
   * @param pattern RE2 pattern to match
   * @return true if entire string matches pattern
   * @requires char_type == char
   */
  template<typename T = char_type>
  [[nodiscard]] auto match(const char_type *pattern) const
    -> std::enable_if_t<std::is_same_v<T, char>, bool>
  {
    return re2::RE2::FullMatch(this->as_std_view(), pattern);
  }

  /**
   * @brief Check if string contains RE2 pattern
   * @param pattern RE2 pattern to search for
   * @return true if pattern is found anywhere in string
   * @requires char_type == char
   */
  template<typename T = char_type>
  [[nodiscard]] auto contains(const char_type *pattern) const
    -> std::enable_if_t<std::is_same_v<T, char>, bool>
  {
    return re2::RE2::PartialMatch(this->as_std_view(), pattern);
  }

  /**
   * @brief Extract first match of RE2 pattern
   * @param pattern RE2 pattern to match
   * @return Extracted substring or empty string if no match
   * @requires char_type == char
   */
  template<typename T = char_type>
  [[nodiscard]] auto extract(const char_type *pattern) const
  -> std::enable_if_t<std::is_same_v<T, char>, utf<char_type>>
  {
    re2::StringPiece result;
    if (re2::RE2::PartialMatch(this->as_std_view(), pattern, &result)) {
      return utf<char_type>(result.data(), result.size());
    }
    return utf<char_type>();
  }


  /**
   * @brief Extract all matches of RE2 pattern
   * @param pattern RE2 pattern to match
   * @return Vector of extracted substrings
   * @requires char_type == char
   */
  template<typename T = char_type>
  [[nodiscard]]
  auto extract_all(const char_type *pattern) const
    -> std::enable_if_t< std::is_same_v<T, char> , std::vector<utf_view<char_type>>>
  {
    std::vector<utf_view<char_type>> results;
    re2::StringPiece input(this->data(), this->size());
    re2::RE2 re(pattern);
    re2::StringPiece match;

    while (re.Match(input, 0, input.size(), re2::RE2::UNANCHORED, &match, 1)) {
      if (match.empty())
        break;
      results.emplace_back(match.data(), match.size());
      input = re2::StringPiece(match.data() + match.size(),
                              input.size() - (match.data() - input.data()) -
                                  match.size());
    }
    return results;
  }

  /**
   * @brief Split string by RE2 pattern
   * @param pattern RE2 pattern to use as delimiter
   * @return Vector of substrings
   * @requires char_type == char
   */
  template<typename T = char_type>
  [[nodiscard]]
   auto split_by_re2(const char_type *pattern) const
    -> std::enable_if_t<std::is_same_v<T, char>, std::vector<utf_view<char_type>>>
  {
    std::vector<utf_view<char_type>> results;
    re2::StringPiece input(this->data(), this->size());
    re2::RE2 re(pattern);
    re2::StringPiece match;
    size_t last_pos = 0;

    while (re.Match(input, last_pos, input.size(), re2::RE2::UNANCHORED, &match,
                    1)) {
      if (match.data() == nullptr)
        break;

      size_t match_pos = match.data() - input.data();
      if (match_pos > last_pos) {
        results.emplace_back(input.data() + last_pos, match_pos - last_pos);
      }
      last_pos = match_pos + match.size();
    }

    if (last_pos < input.size()) {
      results.emplace_back(input.data() + last_pos, input.size() - last_pos);
    }

    return results;
  }
#endif

  /**
   * @brief Reverse string contents with valid sequence
   * @return New string with reversed code points
   * @note Properly handles multi-byte sequences
   * @note this only for valid character sequences
   */
  [[nodiscard]]
  utf reverse() const;

  [[nodiscard]]
  utf reverse_bytes() const;

  /**
   * @brief Swap the contents of two `utf` instances.
   * @param other The `utf` instance to swap with.
   * @return A reference to this instance after the swap.
   */
  utf &swap(utf<char_type> &other) {
    base_type::swap(*static_cast<base_type *>(&other));
    return *this;
  }

  /**
   * @brief Remark if reserved size. ([WARNING:] unsafe operation)
   * @param n new size
   * @param hack if true and n < capacity, do remark size
   * @note This is an unsafe experimental function:
   *       - For testing purposes only, do not use in production code
   *       - Tested with g++ on Arch Linux x64 and Windows MSYS2 UCRT64
   *       - Dangerous if std::string data layout structure changes
   *       - String structure differs in Termux environment, do not use there
   */
  void remark_size(size_t n, bool fix_end = false) {
    if (n < this->capacity()) {
      toy::hack_string_size(this, n);
      if (fix_end)
        (*this)[n] = '\0';
    }
  }

  /**
   * @brief Take ownership of string buffer using cbuf ([WARNING:] unsafe
   * operation)
   * @param cbuf Character buffer to take over
   * @note This is an unsafe experimental function:
   *       - For testing purposes only, do not use in production code
   *       - Tested with g++ on Arch Linux x64 and Windows MSYS2 UCRT64
   *       - Dangerous if std::string data layout structure changes
   *       - String structure differs in Termux environment, do not use there
   *       - This operation moves buffer ownership, original string becomes
   * empty
   *       - WARNING: The buffer should have space for null terminator if
   *         needed for c_str()
   */
  void hack_with_cbuf(utils::cbuf<char_type> cbuf) {
    toy::hack_string_data(this, sizeof(char_type),
                          cbuf.template convert_as<char>());
  }

  /**
   * @brief Convert the `utf` object to a `std::basic_string`.
   * @return A copy of the underlying `std::basic_string` object.
   */
  operator base_type() const { return *((base_type *)(this)); }

  /**
   * @brief Check if string is whitespace-only
   * @return true if all characters are whitespace
   */
  [[nodiscard]]
  bool is_blank() const {
    for (auto c : *this) {
      if (!std::isspace(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    return true;
  }

  /**
   * @brief align center
   * @param n width of block
   */
  [[nodiscard]]
  utf center(size_t width, char_type ch = ' ') const {
    return this->as_view().center(width, ch);
  }

  /**
   * @brief align letf
   * @param n width of block
   */
  [[nodiscard]]
  utf ljust(size_t width, char_type ch = ' ') const {
    return this->as_view().ljust(width, ch);
  }

  /**
   * @brief align right
   * @param n width of block
   */
  [[nodiscard]]
  utf rjust(size_t width, char_type ch = ' ') const {
    return this->as_view().rjust(width, ch);
  }

  [[nodiscard]]
  size_t udisplay_width() const {
    return this->as_view().udisplay_width();
  }
  /**
   * @brief Unicode-aware center alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Centered string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> ucenter(size_t width, char_type ch = ' ') const {
    return this->as_view().ucenter(width, ch);
  }

  /**
   * @brief Unicode-aware left alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Left-aligned string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> uljust(size_t width, char_type ch = ' ') const {
    return this->as_view().uljust(width, ch);
  }

  /**
   * @brief Unicode-aware right alignment
   * @param width Target display width in columns
   * @param ch Fill character (must be single column width)
   * @return Right-aligned string with proper Unicode column width calculation
   */
  [[nodiscard]]
  utf<char_type> urjust(size_t width, char_type ch = ' ') const {
    return this->as_view().urjust(width, ch);
  }

  /**
   * @brief Detect BOM and endianness
   * @return BOM detection result
   */
  [[nodiscard]]
  bom::info detect_bom() const {
    return this->as_view().detect_bom();
  }

  /**
   * @brief Get string without BOM
   * @return New string excluding BOM if present
   */
  [[nodiscard]]
  utf_view<char_type> without_bom() const {
    auto bom_info = detect_bom();
    if (bom_info.has_bom) {
      return this->sub_view(bom_info.bom_size);
    }
    return this->as_view();
  }

  /**
   * @brief Check if string starts with BOM
   * @return true if BOM is present
   */
  [[nodiscard]]
  bool has_bom() const {
    return detect_bom().has_bom;
  }

  /**
   * @brief Get detected endianness from BOM
   * @return Detected endianness or native if no BOM
   */
  [[nodiscard]]
  gr::endian detected_endian() const {
    return detect_bom().endian;
  }

  /**
   * @brief Add BOM to the beginning of the string
   * @param endian Endianness for the BOM (only for UTF-16/32)
   * @return Reference to modified string
   */
  utf &add_bom(gr::endian endian = gr::endian::native) {
    if (this->has_bom()) {
      return *this; // Already has BOM
    }

    if constexpr (std::is_same_v<char_type, char>) {
      // UTF-8 BOM
      this->insert(0, bom::utf8_bytes, 3);
    } else if constexpr (std::is_same_v<char_type, char16_t>) {
      // UTF-16 BOM
      char16_t bom_char =
          (endian == gr::endian::big) ? bom::utf16_be : bom::utf16_le;
      this->insert(0, 1, bom_char);
    } else if constexpr (std::is_same_v<char_type, char32_t>) {
      // UTF-32 BOM
      char32_t bom_char =
          (endian == gr::endian::big) ? bom::utf32_be : bom::utf32_le;
      this->insert(0, 1, bom_char);
    }

    return *this;
  }

  /**
   * @brief Remove BOM from the beginning of the string
   * @return Reference to modified string
   */
  utf &remove_bom() {
    auto bom_info = detect_bom();
    if (bom_info.has_bom) {
      this->erase(0, bom_info.bom_size);
    }
    return *this;
  }
  /*
   * @brief Get bytes of the string
   * @return size_t
   */
  [[nodiscard]]
  size_t bytes() const noexcept {
    return this->size() * sizeof(char_type);
  }
  /*
   * @brief Get bytes of the string
   * @return size_t
   */
  [[nodiscard]]
  size_t capcity_bytes() const noexcept {
    return this->capacity() * sizeof(char_type);
  }

  /**
   * @brief Find substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for search
   * @return Position of first occurrence or nopos if not found
   * @note Uses Knuth-Morris-Pratt algorithm for efficient substring search
   */
  [[nodiscard]]
  size_t find_kmp(utf_view<char_type> pattern, size_t pos = 0) const{
    return this->as_view().find_kmp(pattern, pos);
  }

  /**
   * @brief Find last occurrence of substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for reverse search ( 0 = last_char_pos)
   * @return Position of last occurrence or nopos if not found
   * @note Uses reverse KMP algorithm for efficient substring search
   */
  [[nodiscard]]
  size_t rfind_kmp(utf_view<char_type> pattern, size_t pos = 0) const{
    return this->as_view().rfind_kmp(pattern, pos);
  }

  /**
   * @brief Find all occurrences of substring using KMP algorithm
   * @param pattern Substring to find
   * @return Vector of positions where pattern occurs
   * @note Uses KMP algorithm for efficient substring search
   */
  [[nodiscard]]
  std::vector<size_t> find_all_kmp(utf_view<char_type> pattern) const{
    return this->as_view().find_all_kmp(pattern);
  }
};

extern template class utf_view<char>;
extern template class utf_view<char16_t>;
extern template class utf_view<char32_t>;

extern template class utf<char, std::allocator<char>>;
extern template class utf<char16_t, std::allocator<char16_t>>;
extern template class utf<char32_t, std::allocator<char32_t>>;

using u8 = utf<char, std::allocator<char>>;
using u16 = utf<char16_t, std::allocator<char16_t>>;
using u32 = utf<char32_t, std::allocator<char32_t>>;

using u8v = utf_view<char>;
using u16v = utf_view<char16_t>;
using u32v = utf_view<char32_t>;

#ifndef DISABLE_SUPPORT_ICONV

#if GR_HAS_CONCEPTS

template <typename T>
concept _iconv_input_type = std::is_same_v<T, u8> || std::is_same_v<T, u16> ||
                            std::is_same_v<T, u32> || std::is_same_v<T, u8v> ||
                            std::is_same_v<T, u16v> || std::is_same_v<T, u32v>;

template <typename T>
concept _iconv_output_type =
    std::is_same_v<T, u8> || std::is_same_v<T, u16> || std::is_same_v<T, u32>;
#else
template <typename T>
using _iconv_input_type =
    typename std::enable_if<std::is_same_v<T, u8> || std::is_same_v<T, u16> ||
                            std::is_same_v<T, u32> || std::is_same_v<T, u8v> ||
                            std::is_same_v<T, u16v> ||
                            std::is_same_v<T, u32v>>::type;

template <typename T>
using _iconv_output_type = typename std::enable_if<
    std::is_same_v<T, u8> || std::is_same_v<T, u16> ||
    std::is_same_v<T, u32>>::type;

#endif

/**
 * @brief Character encoding converter using iconv library
 *
 * Provides encoding conversion between different character encodings
 * using the iconv library. Supports various encodings including:
 * - UTF-8, UTF-16, UTF-32
 * - Legacy encodings (GB18030, GBK, Big5, etc.)
 * - Other iconv-supported encodings
 *
 * @note Requires iconv library and DISABLE_SUPPORT_ICONV not defined
 */

class code_converter {
  class impl;
  std::unique_ptr<impl> pimpl;
  enum class string_type { uchar8, uchar16, uchar32 };
  struct contain_proxy {
    void_ptr obj_p;
    string_type type;
    void append(const char *s, size_t bytes);
    std::pair<char *, size_t> get_info() const;
  };

  void transform_impl(const contain_proxy &in_obj, contain_proxy &out,
                      size_t buffer_bytes = 1024);

public:
  /**
   * @brief Construct a new code converter
   * @param to_code Target encoding name (e.g., "UTF-8", "UTF-16LE")
   * @param from_code Source encoding name (e.g., "GB18030", "UTF-8")
   * @param ignore_error Whether to ignore conversion errors and continue
   * @throws std::runtime_error if iconv_open fails and ignore_error is false
   */
  code_converter(u8v to_code, u8v from_code, bool ignore_error = true);

  /**
   * @brief Destructor - closes the iconv converter
   */
  ~code_converter();

  /**
   * @brief Convert input string to output string with specified encoding
   * @tparam instr_obj Input string type (utf or utf_view)
   * @tparam outstr_obj Output string type (utf)
   * @param in_obj Input string to convert
   * @param out Output string to store converted result
   * @param buffer_bytes Internal buffer size for conversion (default: 1024)
   * @throws std::runtime_error if conversion fails and ignore_error is false
   */
  // template <_iconv_input_type instr_obj, _iconv_output_type outstr_obj>
  template <typename instr_obj, typename outstr_obj>
  std::enable_if_t<
      (std::is_same_v<instr_obj, u8> || std::is_same_v<instr_obj, u16> ||
      std::is_same_v<instr_obj, u32> || std::is_same_v<instr_obj, u8v> ||
      std::is_same_v<instr_obj, u16v> || std::is_same_v<instr_obj, u32v>) &&
      (std::is_same_v<outstr_obj, u8> || std::is_same_v<outstr_obj, u16> ||
      std::is_same_v<outstr_obj, u32>),
  void>
   transform(const instr_obj &in_obj, outstr_obj &out,
                 size_t buffer_bytes = 1024) {

    constexpr code_converter::string_type t1 = []() GR_CONSTEVAL {
      if constexpr (std::is_same_v<instr_obj, u8> ||
                    std::is_same_v<instr_obj, u8v>) {
        return string_type::uchar8;
      } else if constexpr (std::is_same_v<instr_obj, u16> ||
                           std::is_same_v<instr_obj, u16v>) {
        return string_type::uchar16;
      } else {
        static_assert(std::is_same_v<instr_obj, u32> ||
                          std::is_same_v<instr_obj, u32v>,
                      "Unsupported input type");
        return string_type::uchar32;
      }
    }();

    constexpr code_converter::string_type t2 = []() GR_CONSTEVAL {
      if constexpr (std::is_same_v<outstr_obj, u8>) {
        return string_type::uchar8;
      } else if constexpr (std::is_same_v<outstr_obj, u16>) {
        return string_type::uchar16;
      } else {
        static_assert(std::is_same_v<outstr_obj, u32>,
                      "Unsupported output type");
        return string_type::uchar32;
      }
    }();

    contain_proxy in_proxy{(void *)&in_obj, t1};
    contain_proxy out_proxy{(void *)&out, t2};

    this->transform_impl(in_proxy, out_proxy, buffer_bytes);
  }

  /**
   * @brief Convert input string and return as new string
   * @tparam outstr_obj Output string type (utf)
   * @tparam instr_obj Input string type (utf or utf_view)
   * @param instr Input string to convert
   * @return New string with converted encoding
   * @throws std::runtime_error if conversion fails and ignore_error is false
   */
  template <typename outstr_obj, typename instr_obj>
  std::enable_if_t<
      (std::is_same_v<outstr_obj, u8> || std::is_same_v<outstr_obj, u16> ||
      std::is_same_v<outstr_obj, u32>) &&
      (std::is_same_v<instr_obj, u8> || std::is_same_v<instr_obj, u16> ||
      std::is_same_v<instr_obj, u32> || std::is_same_v<instr_obj, u8v> ||
      std::is_same_v<instr_obj, u16v> || std::is_same_v<instr_obj, u32v>),
  outstr_obj>
   transform_as(const instr_obj &instr) {
    outstr_obj out;
    this->transform(instr, out);
    return out;
  }
};

#endif

#if GR_HAS_CHAR8_T
/**
 * Create a gr::utf_view<char> from a UTF-8 std::u8string view
 * @param sv The UTF-8 string view to convert
 */
[[nodiscard]]
inline u8v make_u8v(std::u8string_view sv) {
  return u8v((char *)(sv.data()), sv.size());
}

/**
 * Create a gr::utf<char> from a UTF-8 std::u8string view
 * @param sv The UTF-8 string view to convert
 */
[[nodiscard]]
inline u8 make_u8(std::u8string_view sv) {
  return u8((char *)(sv.data()), sv.size());
}

#endif

/**
 * Create a gr::utf_view<char> from a UTF-8 std::string_view
 * @param sv The UTF-8 string view to convert
 */
[[nodiscard]]
inline u8v make_u8v(std::string_view sv) {
  return u8v(sv);
}

/**
 * Create a gr::utf<char> from a UTF-8 std::string_view
 * @param sv The UTF-8 string view to convert
 */
[[nodiscard]]
inline u8 make_u8(std::string_view sv) {
  return u8(sv);
}

/**
 * @brief Convert UTF-16 string view type to UTF-16 type
 * @param utf16 Input UTF-16 string view
 * @return Converted UTF-16 string
 * @note this function is make a copy without code transform
 */
[[nodiscard]]
inline u16 to_utf16(u16v utf16, uc::on_failed fallback = uc::on_failed::skip,
                    gr::endian endian = gr::endian::native) {
  (void)fallback;
  (void)endian;
  return u16(utf16.data(), utf16.size());
}

/**
 * @brief Convert UTF-32 string to UTF-16
 * @param utf32 Input UTF-32 string view
 * @return Converted UTF-16 string
 */
[[nodiscard]]
u16 to_utf16(u32v utf32, uc::on_failed fallback = uc::on_failed::skip,
             gr::endian endian = gr::endian::native);

/**
 * @brief Convert UTF-8 string to UTF-16
 * @param utf8 Input UTF-8 string view
 * @return Converted UTF-16 string
 */
[[nodiscard]]
u16 to_utf16(u8v utf8, uc::on_failed fallback = uc::on_failed::skip,
             gr::endian endian = gr::endian::native);

/**
 * @brief Convert UTF-32 string view type to UTF-32 type
 * @param utf32 Input UTF-32 string view
 * @return Converted UTF-32 string
 * @note this function is make a copy without code transform
 */
[[nodiscard]]
inline u32 to_utf32(u32v utf32, uc::on_failed fallback = uc::on_failed::skip,
                    gr::endian endian = gr::endian::native) {
  (void)fallback;
  (void)endian;
  return u32(utf32.data(), utf32.size());
}

/**
 * @brief Convert UTF-8 string to UTF-32
 * @param utf8 Input UTF-8 string view
 * @return Converted UTF-32 string
 */
[[nodiscard]]
u32 to_utf32(u8v utf8, uc::on_failed fallback = uc::on_failed::skip,
             gr::endian endian = gr::endian::native);

/**
 * @brief Convert UTF-16 string to UTF-16
 * @param utf16 Input UTF-16 string view
 * @return Converted UTF-32 string
 */
[[nodiscard]]
u32 to_utf32(u16v utf16, uc::on_failed fallback = uc::on_failed::skip,
             gr::endian endian = gr::endian::native);

/**
 * @brief Convert UTF-8 string view type to UTF-8 string type
 * @param utf Input UTF-8 string view
 * @return Converted UTF-8 string
 * @note this function is make a copy without code transform
 */
[[nodiscard]]
inline u8 to_utf8(u8v utf8, uc::on_failed fallback = uc::on_failed::skip,
                  gr::endian endian = gr::endian::native) {
  (void)fallback;
  (void)endian;
  return u8(utf8.data(), utf8.size());
}

/**
 * @brief Convert UTF-16 string to UTF-8
 * @param utf16 Input UTF-16 string view
 * @return Converted UTF-8 string
 */
[[nodiscard]]
u8 to_utf8(u16v utf16, uc::on_failed fallback = uc::on_failed::skip,
           gr::endian endian = gr::endian::native);

/**
 * @brief Convert UTF-32 string to UTF-8
 * @param utf32 Input UTF-32 string view
 * @return Converted UTF-32 string
 */
[[nodiscard]]
u8 to_utf8(u32v utf32, uc::on_failed fallback = uc::on_failed::skip,
           gr::endian endian = gr::endian::native);

template <typename char_type, typename Alloc>
utf<char, std::allocator<char>> utf<char_type, Alloc>::to_u8() const {
  return str::to_utf8(this->as_view());
}

template <typename char_type, typename Alloc>
utf<char16_t, std::allocator<char16_t>> utf<char_type, Alloc>::to_u16() const {
  return str::to_utf16(this->as_view());
}

template <typename char_type, typename Alloc>
utf<char32_t, std::allocator<char32_t>> utf<char_type, Alloc>::to_u32() const {
  return str::to_utf32(this->as_view());
}

template <typename char_type>
utf<char_type> utf_view<char_type>::to_str() const {
  return utf<char_type>(this->data(), this->size());
}

template <typename char_type> u8 utf_view<char_type>::to_u8() const {
  return to_utf8(*this);
}

template <typename char_type> u16 utf_view<char_type>::to_u16() const {
  return to_utf16(*this);
}

template <typename char_type> u32 utf_view<char_type>::to_u32() const {
  return to_utf32(*this);
}

template <typename char_type, typename Func>
void batch_process_utf(utf_view<char_type> utfview, Func &&func,
                       gr::endian endian) {
  const char_type *current = utfview.data();
  const char_type *end = current + utfview.size();
  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);
    auto cp = uc::sequence::decode(current, seq_info.length,
                                              seq_info.status, endian);

    if (!func(cp, seq_info.status))
      break;
    current += seq_info.length > 0 ? seq_info.length : 1;
  }
}

template <typename char_type, typename Func>
void batch_check_utf(utf_view<char_type> utfview, Func &&func,
                     gr::endian endian) {

  const char_type *current = utfview.data();
  const char_type *end = current + utfview.size();
  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);
    if (!func(seq_info.status))
      break;
    current += seq_info.length > 0 ? seq_info.length : 1;
  }
}

template<typename char_type>
template<typename container>
utf<char_type> utf_view<char_type>::join_ls(const container& strings){
  if (strings.size() == 0) {
    return utf<char_type>();
  }

  // Calculate total length including separators
  size_t total_length = 0;
  size_t count = 0;
  for (const auto &str : strings) {
    total_length += str.size();
    ++count;
  }
  bool is_empty = this->empty();
  if (!is_empty && count > 1) {
    total_length += this->size() * (count - 1);
  }

  // Create result string with pre-allocated capacity
  utf<char_type> result(total_length);

  // Join strings with separator
  auto it = strings.begin();
  result.append(*it);
  ++it;

  for (; it != strings.end(); ++it) {
    if (!is_empty) {
      result.append(*this);
    }
    result.append(*it);
  }

  return result;
}

namespace bom_utils {

/**
 * @brief Create UTF-8 string with BOM
 * @param content String content
 * @return UTF-8 string with BOM
 */
[[nodiscard]]
inline u8 make_u8_with_bom(std::string_view content) {
  u8 result;
  result.append(bom::utf8_bytes, 3);
  result.append(content);
  return result;
}

/**
 * @brief Create UTF-16 string with BOM
 * @param content String content
 * @param endian Endianness for the BOM
 * @return UTF-16 string with BOM
 */
[[nodiscard]]
inline u16 make_u16_with_bom(std::u16string_view content,
                             gr::endian endian = gr::endian::native) {
  u16 result;
  char16_t bom_char =
      (endian == gr::endian::big) ? bom::utf16_be : bom::utf16_le;
  result.push_back(bom_char);
  result.append(content);
  return result;
}

/**
 * @brief Create UTF-32 string with BOM
 * @param content String content
 * @param endian Endianness for the BOM
 * @return UTF-32 string with BOM
 */
[[nodiscard]]
inline u32 make_u32_with_bom(std::u32string_view content,
                             gr::endian endian = gr::endian::native) {
  u32 result;
  char32_t bom_char =
      (endian == gr::endian::big) ? bom::utf32_be : bom::utf32_le;
  result.push_back(bom_char);
  result.append(content);
  return result;
}

} // namespace bom_utils
} // namespace str
namespace literals {

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return u8 string object
 */
GR_CONSTEXPR_OR_INLINE str::u8 operator""_u8(const char8_t *s, size_t len) {
  return str::u8(s, len);
}

#if GR_HAS_CHAR8_T
/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return u8 string object
 */
GR_CONSTEXPR_OR_INLINE str::u8 operator""_u8(const char *s, size_t len) {
  return str::u8(s, len);
}

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return utf object
 */
GR_CONSTEXPR_OR_INLINE str::u8v operator""_u8v(const char8_t *s, size_t len) {
  return str::u8v(s, len);
}

#endif

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return utf object
 */
GR_CONSTEXPR_OR_INLINE str::u8v operator""_u8v(const char *s, size_t len) {
  return str::u8v(s, len);
}

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return u8 string object
 */
GR_CONSTEXPR_OR_INLINE str::u16 operator""_u16(const char16_t *s, size_t len) {
  return str::u16(s, len);
}

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return utf object
 */
GR_CONSTEXPR_OR_INLINE str::u16v operator""_u16v(const char16_t *s,
                                                 size_t len) {
  return str::u16v(s, len);
}

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return u8 string object
 */
GR_CONSTEXPR_OR_INLINE str::u32 operator""_u32(const char32_t *s, size_t len) {
  return str::u32(s, len);
}

/**
 * @brief User-defined literal for u8 string
 * @param s Character pointer
 * @param len size of string
 * @return utf object
 */
GR_CONSTEXPR_OR_INLINE str::u32v operator""_u32v(const char32_t *s,
                                                 size_t len) {
  return str::u32v(s, len);
}
} // namespace literals
} // namespace gr

#if GR_HAS_STD_FORMATTER && !defined(DISABLE_SUPPORT_STD_FMT)

/**
 * @brief Formatter specialization for u8v type
 */
template <>
struct std::formatter<gr::str::u8v> : std::formatter<std::string_view> {
  auto format(const gr::str::u8v &v, std::format_context &ctxt) const noexcept {
    return std::formatter<std::string_view>::format(v.as_string_view(), ctxt);
  }
};

/**
 * @brief Formatter specialization for u8 type
 */
template <>
struct std::formatter<gr::str::u8> : std::formatter<std::string_view> {
  auto format(const gr::str::u8 &v, std::format_context &ctxt) const noexcept {
    return std::formatter<std::string_view>::format(v.as_std_view(), ctxt);
  }
};

/**
 * @brief Formatter specialization for gr::uc::chunk8 type
 */
template <>
struct std::formatter<gr::uc::chunk_proxy8> : std::formatter<std::string_view> {
  auto format(const gr::uc::chunk_proxy8 &buf8,
              std::format_context &ctxt) const {
    return std::formatter<std::string_view>::format(buf8.view(), ctxt);
  }
};

/**
 * @brief Formatter specialization for gr::uc::codepoint to utf8 type
 */
template <>
struct std::formatter<gr::uc::codepoint> : std::formatter<std::string_view> {
  auto format(const gr::uc::codepoint &v,
              std::format_context &ctxt) const noexcept {
    return std::formatter<std::string_view>::format(v.chunk_u8().view(), ctxt);
  }
};

#endif

namespace gr{
/**
 * @brief operator of ostream for gr::uc::u8
 */
inline std::ostream &operator<<(std::ostream &os, gr::str::u8 u8) {
  os << u8.as_std_view();
  return os;
}

/**
 * @brief operator of ostream for gr::uc::u8v
 */
inline std::ostream &operator<<(std::ostream &os, gr::str::u8v u8) {
  os << u8.as_string_view();
  return os;
}
}
