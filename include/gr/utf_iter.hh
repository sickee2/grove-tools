/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file utf_iter.hh
 * @brief Unicode-aware iterator for UTF-8/16/32 encoded strings
 * @ingroup unicode
 *
 * Provides bidirectional Unicode code point iteration over UTF-8, UTF-16, and
 * UTF-32 encoded strings with comprehensive encoding validation and conversion
 * capabilities.
 *
 * ## Core Features
 * - **Multi-Encoding Support**: UTF-8, UTF-16, UTF-32 encoding traversal
 * - **Bidirectional Iteration**: Forward and backward code point navigation
 * - **Encoding Validation**: Configurable invalid sequence handling
 * - **Endianness Support**: Byte order handling for UTF-16/32
 * - **Encoding Conversion**: Built-in UTF encoding conversion utilities
 * - **Range Adapters**: C++20 ranges compatibility
 *
 * ## Key Components
 *
 * ### iter<char_type> Template Class
 * Main Unicode iterator implementation providing:
 * - Standard iterator operations (++, --, *, ->)
 * - Automatic multi-byte sequence handling
 * - Configurable error handling strategies
 * - Byte order awareness
 *
 * ### range<char_type> Adapter
 * Iterator range wrapper for:
 * - C++20 ranges view interface support
 * - Range-based for loop compatibility
 * - Efficient code point iteration
 *
 * ### Helper Functions
 * - `make_iterator()`: Factory functions for various string types
 * - Type aliases: `u8iter`, `u16iter`, `u32iter`
 *
 * ## Integration with String Classes
 *
 * Tightly integrated with `gr::str::utf` and `gr::str::utf_view`:
 * - Unicode-aware iteration methods (`ubegin()`, `uend()`, `urange()`)
 * - Automatic BOM detection and handling
 * - Batch processing utilities (`batch_process_utf()`, `batch_check_utf()`)
 *
 * ## Error Handling Strategies
 *
 * Configurable via `uc::on_failed` enumeration:
 * - `skip`: Skip invalid sequences (default)
 * - `keep`: Process but mark as invalid
 * - `error`: Throw exceptions on invalid sequences
 *
 * ## Usage Examples
 *
 * ### Basic Iteration
 * ```cpp
 * auto iter = gr::uc::make_iterator(u8"Hello 世界");
 * while (iter) {
 *     std::cout << *iter << " ";
 *     ++iter;
 * }
 * ```
 *
 * ### Range-based Iteration
 * ```cpp
 * for (auto cp : text.urange()) {
 *     if (cp.is_alphabetic()) {
 *         // Process alphabetic characters
 *     }
 * }
 * ```
 *
 * ### Batch Processing
 * ```cpp
 * size_t valid_count = 0;
 * batch_process_utf(text, [&](gr::uc::codepoint cp, gr::uc::sequence_status
 * status) { if (status == gr::uc::sequence_status::valid) { valid_count++;
 *     }
 *     return true; // Continue processing
 * });
 * ```
 *
 * ## Dependencies
 * - `gr/utf_sequence.hh`: Encoding validation and code point operations
 * - `gr/utils.hh`: Utility functions and constants
 * - C++ Standard Library: `<string_view>`, `<type_traits>`, `<ranges>` (C++20)
 *
 * ## Thread Safety
 * @warning Not thread-safe. Iterators should not be shared between threads.
 *
 * ## Performance Notes
 * - Lazy code point decoding for optimal performance
 * - Minimal memory overhead during iteration
 * - Efficient sequence validation using lookup tables
 * - Automatic BOM skipping with zero-copy operations
 *
 * @see gr::uc::codepoint for code point operations
 * @see gr::str::utf for Unicode string classes
 * @see gr::uc::sequence for encoding validation
 */

#pragma once

#include <algorithm>
#include <assert.h>
#include <compare>
#include <cstdint>
#include <gr/utf_sequence.hh>
#include <gr/utils.hh>
#include <iostream>
#if GR_HAS_CPP20
#include <ranges>
#endif
#include <string_view>
#include <type_traits>

namespace gr::uc {

namespace detail {
template <typename T> struct _valid_char_type_helper {
  static constexpr bool value = std::is_same_v<T, char> ||
                                std::is_same_v<T, char16_t> ||
                                std::is_same_v<T, char32_t>;
};
template <typename T>
using _valid_char_type = std::enable_if_t<_valid_char_type_helper<T>::value, T>;
} // namespace detail
/**
 * @class iter
 * @brief Unicode-aware iterator for UTF-8/16/32 encoded strings
 * @tparam CharT Character type (char, char16_t, or char32_t)
 *
 * Provides bidirectional iteration over Unicode code points in strings of
 * various encodings, with configurable handling of invalid sequences. Supports
 * conversion between UTF encodings.
 * @warning Don't use this iterator in multi-thread
 */
template <typename char_type> class iter {
  static_assert(
      detail::_valid_char_type_helper<char_type>::value,
      "gr::uc::iter<...> arg char_type must be char, char16_t, or char32_t");

public:
  using value_type = codepoint;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;
  using pointer = const codepoint *;
  using reference = codepoint;

  using string_view_type = std::basic_string_view<char_type>;
  using string_type = std::basic_string<char_type>;

  iter(const iter &) noexcept = default;
  iter(iter &&) noexcept = default;
  ~iter() = default;

  /**
   * @brief Construct a new Unicode iterator
   * @param s const string pointer
   * @param len string length
   * @param pos Starting position (default 0)
   * @param fb Fallback strategy for invalid sequences (default SKIP)
   */
  explicit iter(const char_type *data, size_t size, size_t pos = 0,
                on_failed fb = on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : m_beg(data), m_end(data + size), m_current(data + std::min(pos, size)),
        /*m_seq_len(0), m_status(sequence_status::valid),*/ m_failed(fb),
        m_endian(endian) {
    auto info = sequence::check(m_current, m_end, m_endian);
    m_seq_len = info.length;
    m_status = info.status;
    // _seek_valid_forward();
  }

  /**
   * @brief Construct a new Unicode iterator
   * @param sv String view to iterate over
   * @param pos Starting position (default 0)
   * @param fb Fallback strategy for invalid sequences (default SKIP)
   */
  explicit iter(std::basic_string_view<char_type> sv, size_t pos = 0,
                on_failed fb = on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : iter(sv.data(), sv.size(), pos, fb, endian) {}

  /**
   * @brief set enqianness
   */
  iter &endian(gr::endian endian) {
    m_endian = endian;
    return *this;
  }

  /**
   * @brief get endianness
   */
  gr::endian endian() const { return m_endian; }

  iter &operator=(const iter &) noexcept = default;
  iter &operator=(iter &&) noexcept = default;

  /**
   * @brief Dereference operator
   * @return Reference to current code point
   * @note Updates internal cache if needed
   */
  reference operator*() const {
    if (m_code == 0) {
      m_code = sequence::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return m_code;
  }

  const codepoint value() const {
    if (m_code == 0) {
      m_code = sequence::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return m_code;
  }

  /**
   * @brief Member access operator
   * @return Pointer to current code point
   */
  pointer operator->() const {
    if (m_code == 0) {
      m_code = sequence::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return &m_code;
  }

  /**
   * @brief Prefix increment
   * @return Reference to advanced iterator
   * @note Skips invalid sequences according to fallback strategy
   */
  iter &operator++() {
    _seek_valid_forward();
    return *this;
  }

  /**
   * @brief Postfix increment
   * @return Copy of iterator before advancement
   */
  iter operator++(int) {
    auto tmp(*this);
    ++(*this);
    return tmp;
  }

  /**
   * @brief Prefix decrement
   * @return Reference to decremented iterator
   * @note Seeks previous valid sequence
   * @noge This operator ignore uc::on_failed::Throw, because this action is use
   * 'throw' and 'contine' are very stupid with low performance
   */
  iter &operator--() {
    _seek_valid_backward();
    return *this;
  }

  /**
   * @brief Postfix decrement
   * @return Copy of iterator before decrement
   * @noge This operator can not use uc::on_failed::Throw
   */
  iter operator--(int) {
    auto tmp(*this);
    --(*this);
    return tmp;
  }

  /**
   * @brief Check iterator validity
   * @return true if iterator points to valid position
   */
  explicit operator bool() const {
    // if (m_fallback == on_failed::skip)
    //   return m_current >= m_begin && m_current < m_end && m_status ==
    //   sequence_status::valid;
    return m_current >= m_beg && m_current < m_end && m_seq_len > 0;
  }

  /**
   * @brief Check iterator status validity
   */
  GR_CONSTEXPR_OR_INLINE bool valid() const noexcept {
    return m_status == sequence_status::valid;
  }

  /**
   * @brief Get current sequence length
   * @return Length of current sequence in code units
   */
  size_t seq_len() const { return m_current < m_end ? m_seq_len : 0; }

  /**
   * @brief Set fallback strategy
   * @param fb New fallback strategy
   * @return Reference to modified iterator
   */
  iter &fallback(on_failed fb) {
    m_failed = fb;
    return *this;
  }

  /**
   * @brief Equality comparison
   * @param rhs Iterator to compare with
   * @return true if iterators point to same position in same string
   */
  bool operator==(const iter &rhs) const {
    return m_current == rhs.m_current && m_beg == rhs.m_beg;
  }

  bool operator!=(const iter &rhs) const { return !(*this == rhs); }

  /**
   * @brief Three-way comparison
   * @param rhs Iterator to compare with
   * @return Ordering relationship
   * @note don't compare the iters in different range
   */
#if GR_HAS_CPP20
  auto operator<=>(const iter &rhs) const -> std::strong_ordering {
    return this->base() <=> rhs.base();
  }
#else
  bool operator<(const iter &rhs) const { return this->base() < rhs.base(); }
  bool operator>(const iter &rhs) const { return this->base() > rhs.base(); }
  bool operator<=(const iter &rhs) const { return this->base() <= rhs.base(); }
  bool operator>=(const iter &rhs) const { return this->base() >= rhs.base(); }
#endif

  // constexpr chunk_proxy8 to_u8() const noexcept{
  //   // return (*this)->chunk_u8();
  //   if(m_code == 0) {
  //     m_code = uc::sequence::decode(m_current, m_seq_len, m_status, m_endian);
  //   }
  //   return  std::move(m_code.chunk_u8());
  // }
  /**
   * @brief Get view of current sequence
   * @return String view of current code point's bytes
   */
  auto seq_view() const -> std::basic_string_view<char_type> {
    return std::basic_string_view<char_type>(m_current, m_seq_len);
  }

  /**
   * @brief Get copy of current sequence
   * @return String containing current code point's bytes
   */
  auto seq_string() const -> std::basic_string<char_type> {
    return std::basic_string<char_type>(seq_view());
  }

  auto to_string() const -> std::string {
    auto chunk_ = (*this)->chunk_u8();
    return std::string(chunk_.buf, chunk_.size());
  }

  bool is_whitespace() const { return (**this).is_whitespace(); }

  bool is_digit() const {
    auto cp = **this;
    return cp.value() >= '0' && cp.value() <= '9';
  }

  iter &skip_whitespace() {
    while (*this && (**this).is_whitespace()) {
      ++(*this);
    }
    return *this;
  }

  iter &skip_digits() {
    while (*this && is_digit()) {
      ++(*this);
    }
    return *this;
  }

  /**
   * @brief Get current byte position
   * @return Position in original string
   */
  size_t pos() const { return m_current - m_beg; }

  /**
   * @brief Get buffer point at current position
   * @return const char_type point
   */
  const char_type *base() const { return m_current; }

  /**
   * @brief Get buffer point at current position
   * @return void point
   */
  gr::void_ptr debug_ptr() const { return gr::void_ptr(m_current); }

  /**
   * @brief Get underlying string view
   * @return Reference to original string view
   */
  string_view_type underlying_view() const {
    return string_view_type(m_beg, m_end - m_beg);
  }

  /**
   * @brief Get current sequence status
   * @return Status of current sequence
   */
  sequence_status status() const { return m_status; }

  /**
   * @brief Convert iter<>::status to string
   */
  const char *status_info() const { return get_status_info(m_status); }

private:
  const char_type *m_beg = nullptr;
  const char_type *m_end = nullptr;
  const char_type *m_current = nullptr;
  mutable codepoint m_code{};
  uint8_t m_seq_len;
  sequence_status m_status;
  on_failed m_failed;
  gr::endian m_endian;

  /**
   * @brief Advance to next valid sequence
   * @details Skips invalid sequences according to fallback strategy,
   *          either continuing, skipping, or throwing exceptions
   */
  void _seek_valid_forward() {
    // reset codepoint
    m_code = 0;
    // move current position
    m_current += (m_status == sequence_status::valid) ? m_seq_len : 1;
    // search next sequence
    while (m_current < m_end) {
      auto res = sequence::check(m_current, m_end, m_endian);

      m_seq_len = res.length;
      m_status = res.status;

      if (m_status == sequence_status::valid || m_failed == on_failed::keep) {
        return;
      }

      if (m_failed == on_failed::skip) {
        m_current += (res.length > 0 ? res.length : 1);
        if (m_current >= m_end) {
          m_status = sequence_status::truncated;
          m_seq_len = 0;
          return;
        }
        continue;
      }

      if (m_failed == on_failed::error) {
        throw std::runtime_error(
            "gr::utf_iter<> => Invalid UTF sequence encountered");
      }
    }
    if (m_current >= m_end) {
      m_status = sequence_status::truncated;
      m_seq_len = 0;
    }
  }
  /**
   * @brief Retreat to previous valid sequence
   */
  void _seek_valid_backward() noexcept {
    m_code = 0;
    if (m_current == m_beg) {
      m_status = sequence_status::truncated;
      m_seq_len = 0;
      return;
    }

    m_current--;

    while (m_current >= m_beg) {
      auto res = sequence::check(m_current, m_end, m_endian);

      m_status = res.status;
      m_seq_len = res.length;

      if (res.status == sequence_status::valid || m_failed == on_failed::keep) {
        return;
      }

      if (m_current == m_beg)
        break;
      m_current--;
    }

    m_status = sequence_status::truncated;
    m_seq_len = 0;
    m_current = m_beg;
  }
};

extern template class iter<char>;
extern template class iter<char16_t>;
extern template class iter<char32_t>;

// alias name
using u8iter = iter<char>;
using u16iter = iter<char16_t>;
using u32iter = iter<char32_t>;

/**
 * @brief Unicode string iterator range adapter
 * @tparam CharT Character type
 */
template <typename CharT>
#if GR_HAS_CPP20
struct range : std::ranges::view_interface<range<CharT>> {
#else
struct range {
#endif
  using iterator = uc::iter<CharT>;
  using sentinel = uc::iter<CharT>;
  using value_type = uc::codepoint;
  using reference = uc::codepoint;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::input_iterator_tag;

  iterator current;  ///< Current iterator position
  iterator end_iter; ///< End iterator position

  range() = default;
  /**
   * @brief Construct a new range
   * @param start Beginning iterator
   * @param _end Ending iterator
   */
  template <typename It>
  range(It start, It _end) : current(start), end_iter(_end) {}
  /**
   * @brief Get begin iterator
   * @return Iterator to start of range
   */
  iterator begin() const { return current; }
  /**
   * @brief Get end iterator
   * @return Iterator to end of range
   */
  iterator end() const { return end_iter; }
  /**
   * @brief Check if range is empty
   * @return true if range is empty
   */
  bool empty() const { return current == end_iter; }
};

/**
 * @brief Create iterator from string view
 * @tparam CharT Character type
 * @param sv String view to iterate over
 * @param pos Starting position (default 0)
 * @param fb Fallback strategy (default SKIP)
 * @return UTF iterator
 */
template <typename CharT>
[[nodiscard]] auto make_iterator(std::basic_string_view<CharT> sv,
                                 size_t pos = 0, on_failed fb = on_failed::skip,
                                 gr::endian endian = gr::endian::native) {
  return iter<CharT>(sv, pos, fb, endian);
}

/**
 * @brief Create iterator from string
 * @tparam CharT Character type
 * @param s String to iterate over
 * @param pos Starting position (default 0)
 * @param fb Fallback strategy (default SKIP)
 * @return UTF iterator
 */
template <typename CharT>
[[nodiscard]] auto make_iterator(const std::basic_string<CharT> &s,
                                 size_t pos = 0, on_failed fb = on_failed::skip,
                                 gr::endian endian = gr::endian::native) {
  return iter<CharT>(std::basic_string_view<CharT>(s.data(), s.size()), pos, fb,
                     endian);
}

/**
 * @brief Create iterator from C-string
 * @tparam CharT Character type
 * @param s Null-terminated C-string
 * @param pos Starting position (default 0)
 * @param fb Fallback strategy (default SKIP)
 * @return UTF iterator
 * @note The string must be null-terminated
 */
template <typename CharT>
[[nodiscard]] inline auto
make_iterator(const CharT *s, size_t len, size_t pos = 0,
              on_failed fb = on_failed::skip,
              gr::endian endian = gr::endian::native) {
  return iter<CharT>(std::basic_string_view<CharT>(s, len), pos, fb, endian);
}

} // namespace gr::uc

/**
 * @brief Output stream operator for UTF-8 chunks
 * @param os Output stream
 * @param u8 UTF-8 chunk to output
 * @return Reference to output stream
 */
std::ostream &operator<<(std::ostream &os, gr::uc::chunk_proxy8 u8);
std::ostream &operator<<(std::ostream &os, gr::uc::codepoint code);
