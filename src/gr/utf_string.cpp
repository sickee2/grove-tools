/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 */

#include <gr/utf_sequence.hh>
#include "gr/utils.hh"
#include <gr/string.hh>
#ifndef DISABLE_SUPPORT_ICONV
#include <iconv.h>
#endif
#include <stdexcept>
namespace gr::utils {
template <typename T>
void build_lps(const T* pattern, size_t pattern_len, utils::cbuf<size_t>& lps) {
  lps.fillzero();
  size_t len = 0;
  size_t i = 1;
  
  while (i < pattern_len) {
    if (pattern[i] == pattern[len]) {
      len++;
      lps[i] = len;
      i++;
    } else {
      if (len != 0) {
        len = lps[len - 1];
      } else {
        lps[i] = 0;
        i++;
      }
    }
  }
}

template <typename T>
void build_reverse_lps(const T* pattern, size_t pattern_len, utils::cbuf<size_t>& lps) {
  lps.fillzero();
  size_t len = 0;
  size_t i = 1;

  while (i < pattern_len) {
    if (pattern[pattern_len - 1 - i] == pattern[pattern_len - 1 - len]) {
      len++;
      lps[i] = len;
      i++;
    } else {
      if (len != 0) {
        len = lps[len - 1];
      } else {
        lps[i] = 0;
        i++;
      }
    }
  }
}

template <typename T>
size_t find_mark_kmp(const T *src_beg, size_t src_count, const T *mark_beg,
                     size_t mark_count, size_t pos, utils::cbuf<size_t>& lps) {

  if (pos >= src_count || mark_count == 0)
    return gr::utils::nopos;

  // KMP
  size_t src_idx = pos;
  size_t mark_idx = 0;

  while (src_idx < src_count) {
    if (*(src_beg + src_idx) == *(mark_beg + mark_idx)) {
      src_idx++;
      mark_idx++;

      if (mark_idx == mark_count) {
        return src_idx - mark_count;
      }
    } else {
      // 字符不匹配时的处理
      if (mark_idx != 0) {
        mark_idx = lps[mark_idx - 1];
      } else {
        src_idx++;
      }
    }
  }

  return gr::utils::nopos;
}

template <typename T>
size_t find_mark_kmp(const T *src_beg, size_t src_count, const T *mark_beg,
                     size_t mark_count, size_t pos) {

  if (pos >= src_count || mark_count == 0)
    return gr::utils::nopos;

  utils::cbuf<size_t> lps = utils::cbuf<size_t>::create(mark_count);
  build_lps(mark_beg, mark_count, lps);

  return find_mark_kmp(src_beg, src_count, mark_beg, mark_count, pos, lps);
}

template <typename T>
size_t rfind_mark_kmp(const T *src_beg, size_t src_count, const T *mark_beg,
                      size_t mark_count, size_t pos, utils::cbuf<size_t>& rev_lps) {

  if (pos >= src_count || mark_count == 0 || mark_count > src_count)
    return gr::utils::nopos;

  size_t start_idx = (src_count - 1) - pos;
  if (start_idx >= src_count)
    start_idx = src_count - 1;


  size_t src_idx = start_idx;
  size_t mark_idx = 0;
  // KMP
  while (src_idx < src_count) {
    T src_char = src_beg[src_idx - mark_idx];
    T mark_char = mark_beg[mark_count - 1 - mark_idx];

    if (src_char == mark_char) {
      mark_idx++;
      
      if (mark_idx == mark_count) {
        return src_idx - mark_count + 1;
      }
    } else {
      if (mark_idx != 0) {
        mark_idx = rev_lps[mark_idx - 1];
      } else {
        if (src_idx == 0) break;
        src_idx--;
      }
    }
  }

  return gr::utils::nopos;
}

template <typename T>
size_t rfind_mark_kmp(const T *src_beg, size_t src_count, const T *mark_beg,
                      size_t mark_count, size_t pos) {

  if (pos >= src_count || mark_count == 0 || mark_count > src_count)
    return gr::utils::nopos;
  utils::cbuf<size_t> rev_lps = utils::cbuf<size_t>::create(mark_count);
  build_reverse_lps(mark_beg, mark_count, rev_lps);
  return rfind_mark_kmp(src_beg, src_count, mark_beg, mark_count, pos, rev_lps);
}

} // namespace gr::utils

#define VOID_ITER_TYPE(T) template void iter<T>

#define INSTANTIATE_ITER_FOR_TYPE(T)                                           \
  template class iter<T>;

#define REF_VIEW_TYPE(T) template utf_view<T> &utf_view<T>
#define UTF_VIEW_2_UTF_TYPE(T) template utf<T> utf_view<T>
#define UTF_VIEW_SIZE_TYPE(T) template size_t utf_view<T>

#define INSTANTIATE_UTF_VIEW_FOR_TYPE(T)                                       \
  template class utf_view<T>;                                                  \
  REF_VIEW_TYPE(T)::utrim();                                                   \
  REF_VIEW_TYPE(T)::utrim_left();                                              \
  REF_VIEW_TYPE(T)::utrim_right();                                             \
  REF_VIEW_TYPE(T)::trim();                                                    \
  REF_VIEW_TYPE(T)::trim_left();                                               \
  REF_VIEW_TYPE(T)::trim_right();                                              \
  UTF_VIEW_SIZE_TYPE(T)::udisplay_width() const;                               \
  UTF_VIEW_2_UTF_TYPE(T)::ucenter(size_t, T) const;                            \
  UTF_VIEW_2_UTF_TYPE(T)::uljust(size_t, T) const;                             \
  UTF_VIEW_2_UTF_TYPE(T)::urjust(size_t, T) const;                             \
  UTF_VIEW_2_UTF_TYPE(T)::center(size_t, T) const;                             \
  UTF_VIEW_2_UTF_TYPE(T)::ljust(size_t, T) const;                              \
  UTF_VIEW_2_UTF_TYPE(T)::rjust(size_t, T) const;                              \
  template auto utf_view<T>::split(utf_view<T>)                                \
      const->std::vector<utf_view<T>>;                                         \
  template auto utf_view<T>::word_boundaries() const -> std::vector<uint32_t>; \
  template size_t utf_view<T>::find_kmp(utf_view<T>, size_t) const;            \
  template size_t utf_view<T>::rfind_kmp(utf_view<T>, size_t) const;           \
  template std::vector<size_t> utf_view<T>::find_all_kmp(utf_view<T>) const;

#define REF_UTF_TYPE(T) template utf<T> &utf<T>

#define UTF_TYPE(T) template utf<T> utf<T>
#define VOID_UTF_TYPE(T) template void utf<T>

#define INSTANTIATE_UTF_FOR_TYPE(T)                                            \
  template class utf<T, std::allocator<T>>;                                    \
  REF_UTF_TYPE(T)::utrim();                                                    \
  REF_UTF_TYPE(T)::utrim_left();                                               \
  REF_UTF_TYPE(T)::utrim_right();                                              \
  REF_UTF_TYPE(T)::trim();                                                     \
  REF_UTF_TYPE(T)::trim_left();                                                \
  REF_UTF_TYPE(T)::trim_right();                                               \
  REF_UTF_TYPE(T)::replace_all_inplace(utf_view<T>, utf_view<T>);              \
  UTF_TYPE(T)::reverse() const;                                                \
  UTF_TYPE(T)::reverse_bytes() const;                                          \
  UTF_TYPE(T)::replace_all(utf_view<T>, utf_view<T>) const;

#if GR_HAS_RE2
#define INSTANTIATE_UTF_CHAR_SPECIALS()                                        \
  template utf<char> utf<char>::extract(const char *) const;                   \
  template std::vector<utf_view<char>> utf<char>::extract_all(const char *)    \
      const;                                                                   \
  template std::vector<utf_view<char>> utf<char>::split_by_re2(const char *)   \
      const;
#endif

namespace gr::str {

u8 to_utf8(u16v utf16, uc::on_failed fb, gr::endian endian) {
  u8 result(utf16.size() * 3);

  const char16_t *current = utf16.data();
  const char16_t *end = current + utf16.size();

  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);

    if (seq_info.status != uc::sequence_status::valid) {
      switch (fb) {
      case gr::uc::on_failed::skip:
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      case gr::uc::on_failed::keep:
        result.append(uc::codepoint::make_replacement().chunk_u8().buf, 3);
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      default:
#if GR_HAS_STD_FORMATTER
        throw std::runtime_error(std::format(
            "convert utf-16 to utf-8 {} at index {}",
            uc::get_status_info(seq_info.status), current - utf16.data()));
#else
        throw std::runtime_error(uc::get_status_info(seq_info.status));
#endif
      }
    }
    auto cp = uc::sequence::decode(current, seq_info.length,
                                             seq_info.status, endian);

    auto chunk = cp.chunk_u8();
    result.append(chunk.buf, chunk.size());

    current += seq_info.length;
  }

  return result;
}

u8 to_utf8(u32v utf32, uc::on_failed fb, gr::endian endian) {
  (void)fb;
  gr::str::u8 result(utf32.size() * 3);
  const char32_t *current = utf32.data();
  const char32_t *end = current + utf32.size();
  while (current < end) {
    auto cp = uc::sequence::decode(current, 1, uc::sequence_status::valid, endian);

    auto chunk = cp.chunk_u8();
    result.append(chunk.buf, chunk.size());

    ++current;
  }
  return result;
}

u16 to_utf16(u8v utf8, uc::on_failed fb, gr::endian endian) {

  u16 result(utf8.size()); // 预分配空间，UTF-16 通常比 UTF-8 短

  const char *current = utf8.data();
  const char *end = current + utf8.size();

  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);

    if (seq_info.status != uc::sequence_status::valid) {
      switch (fb) {
      case gr::uc::on_failed::skip:
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      case gr::uc::on_failed::keep:
        result.append(uc::codepoint::make_replacement().chunk_u16().buf, 1);
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      default:
#if GR_HAS_STD_FORMATTER
        throw std::runtime_error(std::format(
            "convert utf-8 to utf-16 {} at index {}",
            uc::get_status_info(seq_info.status), current - utf8.data()));
#else
        throw std::runtime_error(uc::get_status_info(seq_info.status));
#endif
      }
    }

    auto cp = uc::sequence::decode(current, seq_info.length,
                                         seq_info.status, endian);

    auto chunk = cp.chunk_u16();
    result.append(chunk.buf, chunk.size());

    current += seq_info.length;
  }

  return result;
}

u16 to_utf16(u32v utf32, uc::on_failed fb, gr::endian endian) {

  u16 result(utf32.size()); // 预分配空间，大多数 BMP 字符是 1:1 映射

  const char32_t *current = utf32.data();
  const char32_t *end = current + utf32.size();

  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);

    if (seq_info.status != uc::sequence_status::valid) {
      switch (fb) {
      case gr::uc::on_failed::skip:
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      case gr::uc::on_failed::keep:
        result.append(uc::codepoint::make_replacement().chunk_u16().buf, 1);
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      default:
#if GR_HAS_STD_FORMATTER
        throw std::runtime_error(std::format(
            "convert utf-32 to utf-16 {} at index {}",
            uc::get_status_info(seq_info.status), current - utf32.data()));
#else
        throw std::runtime_error(uc::get_status_info(seq_info.status));
#endif
      }
    }

    auto cp = uc::sequence::decode(current, seq_info.length,
                                             seq_info.status, endian);

    auto chunk = cp.chunk_u16();
    result.append(chunk.buf, chunk.size());

    current += seq_info.length;
  }

  return result;
}

u32 to_utf32(u8v utf8, uc::on_failed fb, gr::endian endian) {

  gr::str::u32 result(utf8.size()); // 预分配空间，UTF-32 通常比 UTF-8 长

  const char *current = utf8.data();
  const char *end = current + utf8.size();

  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);

    if (seq_info.status != uc::sequence_status::valid) {
      switch (fb) {
      case gr::uc::on_failed::skip:
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      case gr::uc::on_failed::keep:
        result.push_back(uc::codepoint::make_replacement().value());
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      default:
#if GR_HAS_STD_FORMATTER
        throw std::runtime_error(std::format(
            "convert utf-8 to utf-32 {} at index {}",
            uc::get_status_info(seq_info.status), current - utf8.data()));
#else
        throw std::runtime_error(uc::get_status_info(seq_info.status));
#endif
      }
    }

    auto cp = uc::sequence::decode(current, seq_info.length,
                                         seq_info.status, endian);

    result.push_back(cp.value());

    current += seq_info.length;
  }

  return result;
}
u32 to_utf32(u16v utf16, uc::on_failed fb, gr::endian endian) {

  gr::str::u32 result(utf16.size()); // 预分配空间，大多数字符是 1:1 映射

  const char16_t *current = utf16.data();
  const char16_t *end = current + utf16.size();

  while (current < end) {
    auto seq_info = uc::sequence::check(current, end, endian);

    if (seq_info.status != uc::sequence_status::valid) {
      switch (fb) {
      case gr::uc::on_failed::skip:
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      case gr::uc::on_failed::keep:
        result.push_back(uc::codepoint::make_replacement().value());
        current += (seq_info.length > 0 ? seq_info.length : 1);
        continue;
      default:
#if GR_HAS_STD_FORMATTER
        throw std::runtime_error(std::format(
            "convert utf-16 to utf-32 {} at index {}",
            uc::get_status_info(seq_info.status), current - utf16.data()));
#else
        throw std::runtime_error(uc::get_status_info(seq_info.status));
#endif
      }
    }

    auto cp = uc::sequence::decode(current, seq_info.length,
                                             seq_info.status, endian);

    result.push_back(cp.value());

    current += seq_info.length;
  }

  return result;
}

#ifndef DISABLE_SUPPORT_ICONV
class code_converter::impl {
  iconv_t m_converter = iconv_t(-1);
  bool m_ignore_errors = false;

public:
  impl(u8v to_code, u8v from_code, bool ignore_error)
      : m_converter(iconv_open(to_code.data(), from_code.data())),
        m_ignore_errors(ignore_error) {}

  ~impl() {
    if (m_converter != iconv_t(-1)) {
      iconv_close(m_converter);
    }
  }

  void transform(const contain_proxy &in_obj, contain_proxy &out,
                 size_t buffer_bytes) {
    if (m_converter == iconv_t(-1)) {
      const char *logmessage =
          "iconv open code page failed, please check code name.";
      if (m_ignore_errors) {
        std::cout << logmessage << std::endl;
      } else {
        throw std::runtime_error(logmessage);
      }
      return;
    }

    // get input string info
    auto [inbuf, inbytes_remaining] = in_obj.get_info();

    auto outbuf_content = gr::make_cbuf<char>(buffer_bytes);

    while (true) {
      char *outbuf = outbuf_content.begin();
      size_t outbytes_remaining = buffer_bytes;

      size_t size = iconv(m_converter, &inbuf, &inbytes_remaining, &outbuf,
                          &outbytes_remaining);
      int err = errno;
      size_t convert_size = buffer_bytes - outbytes_remaining;

      if (convert_size > 0) {
        out.append(outbuf_content.begin(), convert_size);
      }

      if (inbytes_remaining == 0) {
        break;
      }

      if (size == size_t(-1)) {
        int skipped_bytes = 0;
        if (handle_error(err, skipped_bytes)) {
          break;
        }
        if (inbytes_remaining > 0) {
          inbytes_remaining -= skipped_bytes;
          inbuf += skipped_bytes;
          continue;
        } else {
          break;
        }
      }
    }
  }

  bool handle_error(int error, int &skipped_bytes) {
    switch (error) {
    case E2BIG: // buffer is overload
      return false;
    case EINVAL: // incomplete multibyte sequence has been encountered
    case EILSEQ: // invalid multibyte sequence has been encountered
      if (m_ignore_errors) {
        // skipp byte
        skipped_bytes = 1;
        return false;
      }
      return true;
    default:
      if (m_ignore_errors) {
        skipped_bytes = 1;
        return false;
      }
      return true;
    }
  }
};

// code_convertor 成员函数实现
code_converter::code_converter(u8v to_code, u8v from_code, bool ignore_error)
    : pimpl(std::make_unique<impl>(to_code, from_code, ignore_error)) {}

code_converter::~code_converter() = default;

void code_converter::contain_proxy::append(const char *s, size_t bytes) {
  using u8str_p = u8 *;
  using u16str_p = u16 *;
  using u32str_p = u32 *;

  if (bytes == 0)
    return;

  switch (type) {
  case gr::str::code_converter::string_type::uchar8: {
    u8str_p(obj_p)->append((const char *)(s), bytes);
  } break;
  case gr::str::code_converter::string_type::uchar16: {
    size_t char_count = bytes / sizeof(char16_t);
    u16str_p(obj_p)->append((const char16_t *)(s), char_count);
  } break;
  default: {
    size_t char_count = bytes / sizeof(char32_t);
    u32str_p(obj_p)->append((const char32_t *)(s), char_count);
  }
  }
}

std::pair<char *, size_t> code_converter::contain_proxy::get_info() const {
  using u8srt_p = const u8 *;
  using u16str_p = const u16 *;
  using u32str_p = const u32 *;

  switch (type) {
  case code_converter::string_type::uchar8: {
    auto *_p = u8srt_p(obj_p);
    return {(char *)(_p->data()), _p->bytes()};
  } break;
  case code_converter::string_type::uchar16: {
    auto *_p = u16str_p(obj_p);
    return {(char *)(_p->data()), _p->bytes()};
  } break;
  default: {
    auto *_p = u32str_p(obj_p);
    return {(char *)(_p->data()), _p->bytes()};
  }
  }
}

void code_converter::transform_impl(const contain_proxy &in_obj,
                                    contain_proxy &out, size_t buffer_bytes) {

  pimpl->transform(in_obj, out, buffer_bytes);
}

#endif

} // namespace gr::str

namespace gr::uc {

INSTANTIATE_ITER_FOR_TYPE(char)
INSTANTIATE_ITER_FOR_TYPE(char16_t)
INSTANTIATE_ITER_FOR_TYPE(char32_t)


} // namespace gr::uc

namespace gr::str {

INSTANTIATE_UTF_VIEW_FOR_TYPE(char)
INSTANTIATE_UTF_VIEW_FOR_TYPE(char16_t)
INSTANTIATE_UTF_VIEW_FOR_TYPE(char32_t)

INSTANTIATE_UTF_FOR_TYPE(char)
INSTANTIATE_UTF_FOR_TYPE(char16_t)
INSTANTIATE_UTF_FOR_TYPE(char32_t)

INSTANTIATE_UTF_CHAR_SPECIALS()

/**
 * @brief Unicode-aware trim leading/trailing whitespace
 * @return Reference to modified view
 * @post View will contain no leading/trailing Unicode whitespace
 */
template <typename char_type>
utf_view<char_type> &utf_view<char_type>::utrim() {
  if (this->empty())
    return *this;

  auto range = this->urange();
  auto begin = range.begin();
  auto end = range.end();

  // 跳过开头的 Unicode 空白字符
  while (begin != end && (*begin).is_whitespace()) {
    ++begin;
  }

  // 跳过结尾的 Unicode 空白字符
  auto last = end;
  if (begin != end) {
    --last;
    while (last != begin && (*last).is_whitespace()) {
      --last;
    }
    ++last; // 指向最后一个有效字符之后
  }

  // 重新构造视图
  if (begin == end) {
    *this = utf_view<char_type>();
  } else {
    size_t start_pos = begin.pos();
    size_t end_pos = last.pos();
    *this = this->sub_view(start_pos, end_pos - start_pos);
  }
  return *this;
}

/**
 * @brief Unicode-aware trim whitespace from left
 * @return Reference to modified view
 * @post View will contain no leading Unicode whitespace
 */
template <typename char_type>
utf_view<char_type> &utf_view<char_type>::utrim_left() {
  if (this->empty()) {
    return *this;
  }

  auto range = this->urange();
  auto begin = range.begin();
  auto end = range.end();

  // 跳过开头的 Unicode 空白字符
  while (begin != end && (*begin).is_whitespace()) {
    ++begin;
  }

  if (begin == end) {
    *this = utf_view<char_type>();
  } else {
    *this = this->sub_view(begin.pos());
  }
  return *this;
}

/**
 * @brief Unicode-aware trim whitespace from right
 * @return Reference to modified view
 * @post View will contain no trailing Unicode whitespace
 */
template <typename char_type>
utf_view<char_type> &utf_view<char_type>::utrim_right() {
  if (this->empty()) {
    return *this;
  }

  auto range = this->urange();
  auto begin = range.begin();
  auto end = range.end();

  if (begin == end) {
    *this = utf_view<char_type>();
    return *this;
  }

  // 从后向前找到第一个非空白字符
  auto last = end;
  --last;
  while (last != begin && (*last).is_whitespace()) {
    --last;
  }

  // 如果最后一个字符是空白，需要调整
  if ((*last).is_whitespace()) {
    *this = utf_view<char_type>();
  } else {
    *this = this->sub_view(0, last.pos() + last.seq_len());
  }
  return *this;
}

/**
 * @brief Trim leading/trailing whitespace for ASCII
 * @return Reference to modified view
 * @post View will contain no leading/trailing whitespace
 */
template <typename char_type> utf_view<char_type> &utf_view<char_type>::trim() {
  if (this->empty())
    return *this;

  const char_type *left = this->data();
  const char_type *right = left + this->size() - 1;

  while (left <= right && std::isspace(static_cast<unsigned char>(*left)))
    ++left;
  while (right >= left && std::isspace(static_cast<unsigned char>(*right)))
    --right;

  if (left > right) {
    *this = utf_view<char_type>();
  } else {
    *this = utf_view<char_type>(left, right - left + 1);
  }
  return *this;
}

/**
 * @brief Trim whitespace from left for ASCII
 * @return Reference to modified view
 * @post View will contain no leading/trailing whitespace
 */
template <typename char_type>
utf_view<char_type> &utf_view<char_type>::trim_left() {
  if (this->empty()) {
    return *this;
  }

  const char_type *left = this->data();
  const char_type *right = left + this->size();

  while (left < right && std::isspace(static_cast<unsigned char>(*left))) {
    ++left;
  }
  *this = utf_view<char_type>(left, right - left);
  return *this;
}

/**
 * @brief Trim whitespace from right for ASCII
 * @return Reference to modified view
 * @post View will contain no leading/trailing whitespace
 */
template <typename char_type>
utf_view<char_type> &utf_view<char_type>::trim_right() {
  if (this->empty()) {
    return *this;
  }

  const char_type *left = this->data();
  const char_type *right = left + this->size();

  while (right > left &&
         std::isspace(static_cast<unsigned char>(*(right - 1)))) {
    --right;
  }
  *this = utf_view<char_type>(left, right - left);
  return *this;
}

/**
 * @brief Split string by delimiter
 * @param delimiter View of delimiter string
 * @return Vector of substrings
 * @note Empty substrings are ignore
 */
template <typename char_type>
auto utf_view<char_type>::split(utf_view<char_type> delimiter) const
    -> std::vector<utf_view<char_type>> {
  std::vector<utf_view<char_type>> result;
  size_t _start = 0;
  size_t _end = this->find(delimiter);

  while (_end != utils::nopos) {
    auto n = _end - _start;
    if (n > 0) {
      auto vw = this->sub_view(_start, n);
      result.push_back(vw);
    }
    _start = _end + delimiter.size();
    _end = this->find(delimiter, _start);
  }

  auto vw = this->sub_view(_start);
  if (vw.size() > 0) {
    result.push_back(vw);
  }
  return result;
}

/**
 * @brief Calculate display width considering Unicode character widths
 * @return Total display width in columns
 */
template <typename char_type>
size_t utf_view<char_type>::udisplay_width() const {
  size_t width = 0;
  const char_type *current = this->data();
  const char_type *end = current + this->size();

  while (current < end) {
    auto info =
        uc::sequence::check(current, end, gr::endian::native);
    if (info.status == uc::sequence_status::valid) {
      auto cp = uc::sequence::decode(
          current, info.length, info.status, gr::endian::native);
      width += cp.display_width();
      current += info.length;
    } else {
      // 无效序列，使用替换字符宽度
      width += 1;
      current += (info.length > 0 ? info.length : 1);
    }
  }
  return width;
}

/**
 * @brief align center
 * @param n width of block
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::center(size_t width, char_type ch) const {
  size_t _size = this->size();
  if (width <= _size) {
    return *this;
  }

  size_t offset = (width - _size) / 2;

  utf<char_type> res(width, ch);

  for (size_t i = 0; i < _size; i++, offset++) {
    res[offset] = (*this)[i];
  }
  return res;
}

/**
 * @brief align letf
 * @param n width of block
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::ljust(size_t width, char_type ch) const {
  size_t _size = this->size();
  if (width <= _size) {
    return *this;
  }

  utf<char_type> res(width, ch);

  for (size_t i = 0; i < _size; i++) {
    res[i] = (*this)[i];
  }
  return res;
}

/**
 * @brief align right
 * @param n width of block
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::rjust(size_t width, char_type ch) const {
  size_t _size = this->size();
  if (width <= _size) {
    return *this;
  }

  utf<char_type> res(width, ch);

  size_t offset = width - _size;
  for (size_t i = 0; i < _size; i++, offset++) {
    res[offset] = (*this)[i];
  }
  return res;
}
/**
 * @brief Unicode-aware center alignment
 * @param width Target display width in columns
 * @param ch Fill character (must be single column width)
 * @return Centered string with proper Unicode column width calculation
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::ucenter(size_t width, char_type ch) const {
  size_t display_width = this->udisplay_width();
  if (width <= display_width) {
    return this->to_str();
  }

  size_t left_padding = (width - display_width) / 2;
  size_t right_padding = width - display_width - left_padding;

  utf<char_type> result(this->size() + left_padding + right_padding);
  result.append(left_padding, ch);
  result.append(*this);
  result.append(right_padding, ch);
  return result;
}

/**
 * @brief Unicode-aware left alignment
 * @param width Target display width in columns
 * @param ch Fill character (must be single column width)
 * @return Left-aligned string with proper Unicode column width calculation
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::uljust(size_t width, char_type ch) const {
  size_t display_width = this->udisplay_width();
  if (width <= display_width) {
    return this->to_str();
  }

  utf<char_type> result(width - display_width);
  result.append(*this);
  result.append(width - display_width, ch);
  return result;
}

/**
 * @brief Unicode-aware right alignment
 * @param width Target display width in columns
 * @param ch Fill character (must be single column width)
 * @return Right-aligned string with proper Unicode column width calculation
 */
template <typename char_type>
utf<char_type> utf_view<char_type>::urjust(size_t width, char_type ch) const {
  size_t display_width = this->udisplay_width();
  if (width <= display_width) {
    return this->to_str();
  }

  utf<char_type> result(width - display_width, ch);
  result.append(*this);
  return result;
}

  /**
   * @brief Find substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for search
   * @return Position of first occurrence or nopos if not found
   * @note Uses Knuth-Morris-Pratt algorithm for efficient substring search
   */
template <typename char_type>
  [[nodiscard]]
  size_t utf_view<char_type>::find_kmp(utf_view<char_type> pattern, size_t pos) const {
    if (pattern.empty() || pos >= this->size()) {
      return gr::utils::nopos;
    }
    return gr::utils::find_mark_kmp(this->data(), this->size(), 
                                   pattern.data(), pattern.size(), pos);
  }

  /**
   * @brief Find last occurrence of substring using KMP algorithm
   * @param pattern Substring to find
   * @param pos Starting position for reverse search
   * @return Position of last occurrence or nopos if not found
   * @note Uses reverse KMP algorithm for efficient substring search
   */
template <typename char_type>
  [[nodiscard]]
  size_t utf_view<char_type>::rfind_kmp(utf_view<char_type> pattern, size_t pos) const {
    if (pattern.empty() || this->empty()) {
      return gr::utils::nopos;
    }
    
    size_t search_pos = (pos == gr::utils::nopos) ? this->size() - 1 : pos;
    if (search_pos >= this->size()) {
      search_pos = this->size() - 1;
    }
    
    return gr::utils::rfind_mark_kmp(this->data(), this->size(),
                                    pattern.data(), pattern.size(), search_pos);
  }

  /**
   * @brief Find all occurrences of substring using KMP algorithm
   * @param pattern Substring to find
   * @return Vector of positions where pattern occurs
   * @note Uses KMP algorithm for efficient substring search
   */
template <typename char_type>
  [[nodiscard]] std::vector<size_t> utf_view<char_type>::find_all_kmp(utf_view<char_type> pattern) const {
    std::vector<size_t> positions;
    if (pattern.empty()) {
      return positions;
    }

    // Precompute LPS array once
    utils::cbuf<size_t> lps = utils::cbuf<size_t>::create(pattern.size());
    gr::utils::build_lps(pattern.data(), pattern.size(), lps);

    size_t pos = 0;
    while (pos <= this->size() - pattern.size()) {
      size_t found_pos = gr::utils::find_mark_kmp(this->data(), this->size(), 
                                                 pattern.data(), pattern.size(), 
                                                 pos, lps);
      if (found_pos == gr::utils::nopos) {
        break;
      }
      positions.push_back(found_pos);
      pos = found_pos + 1; // Move to next position to continue search
    }
    
    return positions;
  }


/**
 * @brief Unicode-aware trim leading/trailing whitespace
 * @return Reference to modified string
 * @note Uses Unicode-aware whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::utrim() {
  auto v = this->as_view().utrim();
  if (v.empty()) {
    this->clear();
    return *this;
  }
  auto n = v.size();
  if (v.data() != this->data()) {
    std::copy_n(v.data(), n, this->data());
  }
  this->resize(n);
  return *this;
}

/**
 * @brief Unicode-aware trim leading whitespace
 * @return Reference to modified string
 * @note Uses Unicode-aware whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::utrim_left() {
  auto v = this->as_view().utrim_left();
  if (v.empty()) {
    this->clear();
    return *this;
  }
  auto n = v.size();
  if (v.data() > this->data()) {
    std::copy_n(v.data(), n, this->data());
  }
  this->resize(n);
  return *this;
}

/**
 * @brief Unicode-aware trim trailing whitespace
 * @return Reference to modified string
 * @note Uses Unicode-aware whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::utrim_right() {
  auto v = this->as_view().utrim_right();
  if (v.empty()) {
    this->clear();
    return *this;
  }
  auto n = v.size();
  this->resize(n);
  return *this;
}

/**
 * @brief Trim leading/trailing whitespace for ASCII
 * @return Reference to modified string
 * @note Uses std::isspace for whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::trim() {
  if (this->empty())
    return *this;

  size_t start = 0;
  size_t end = this->size() - 1;

  while (start <= end && std::isspace(this->data()[start]))
    ++start;
  while (end >= start && std::isspace(this->data()[end]))
    --end;

  if (start > end) {
    this->clear();
  } else {
    if (start > 0) {
      std::copy(this->data() + start, this->data() + end + 1, this->data());
    }
    this->resize(end - start + 1);
  }
  return *this;
}

/**
 * @brief Trim leading whitespace for ASCII
 * @return Reference to modified string
 * @note Uses std::isspace for whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::trim_left() {
  auto v = this->as_view().trim_left();
  if (v.empty()) {
    this->clear();
    return *this;
  }
  auto n = v.size();
  if (v.data() > this->data()) {
    std::copy_n(v.data(), n, this->data());
  }
  this->resize(n);
  return *this;
}

/**
 * @brief Trim trailing whitespace for ASCII
 * @return Reference to modified string
 * @note Uses std::isspace for whitespace detection
 */
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &utf<char_type, Alloc>::trim_right() {
  auto v = this->as_view().trim_right();
  if (v.empty()) {
    this->clear();
    return *this;
  }
  auto n = v.size();
  this->resize(n);
  return *this;
}

/// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///
template <typename char_type, typename Alloc>
utf<char_type, Alloc> &
utf<char_type, Alloc>::replace_all_inplace(utf_view<char_type> from,
                                           utf_view<char_type> to) {

  if (from.empty() || from.size() > this->size()) {
    return *this;
  }

  // store all of found indexs
  std::vector<unsigned> matches;
  size_t pos = 0;
  while ((pos = this->find(from, pos)) != std::string::npos) {
    matches.push_back(pos);
    pos += from.size();
  }

  if (matches.empty()) {
    return *this;
  }

  const size_t old_size = this->size();
  const int64_t size_diff = to.size() - from.size();
  const size_t new_size = old_size + matches.size() * size_diff;

  // check new size
  if (new_size > old_size) {
    // resize
    this->resize(new_size);
    char_type *data = this->data();
    size_t read_pos = old_size;
    size_t write_pos = new_size;

    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
      const size_t match_pos = *it;
      const size_t after_match = match_pos + from.size();

      // reverse copy
      const int64_t chunk_size = read_pos - after_match;
      write_pos -= chunk_size;
      read_pos -= chunk_size;
      if (chunk_size > 0) {
        std::copy_n(data + read_pos, chunk_size, data + write_pos);
      }

      // insert replacement
      write_pos -= to.size();
      std::copy_n(to.data(), to.size(), data + write_pos);

      read_pos = match_pos;
    }
  } else if (new_size < old_size) {
    char_type *data = this->data();
    size_t write_pos = 0;
    size_t last_match_end = 0;

    for (size_t match_pos : matches) {
      // copy matched ...
      const size_t chunk_size = match_pos - last_match_end;
      if (chunk_size > 0) {
        std::copy_n(data + last_match_end, chunk_size, data + write_pos);
        write_pos += chunk_size;
      }

      // insert replacements
      std::copy_n(to.data(), to.size(), data + write_pos);
      write_pos += to.size();

      last_match_end = match_pos + from.size();
    }

    // copy last matches
    if (last_match_end < old_size) {
      const size_t chunk_size = old_size - last_match_end;
      std::copy_n(data + last_match_end, chunk_size, data + write_pos);
    }

    this->resize(new_size);
  }
  // direct replace when new size same as old size
  else {
    for (size_t match_pos : matches) {
      std::copy_n(to.data(), to.size(), this->data() + match_pos);
    }
  }

  return *this;
}

template <typename char_type, typename Alloc>
utf<char_type, Alloc>
utf<char_type, Alloc>::replace_all(utf_view<char_type> from,
                                   utf_view<char_type> to) const {

  if (from.empty() || from.size() > this->size()) {
    return utf(this->data(), this->size());
  }

  // store all matched positions
  std::vector<unsigned> matches;
  size_t pos = 0;
  while ((pos = this->find(from, pos)) != std::string::npos) {
    matches.push_back(pos);
    pos += from.size();
  }

  if (matches.empty()) {
    return utf(this->data(), this->size());
  }

  if (from.size() == to.size()) {
    for (size_t match_pos : matches) {
      std::copy_n((char_type *)to.data(), to.size(),
                  (char_type *)(this->data() + match_pos));
    }
    return utf(this->data(), this->size());
  }

  size_t new_size =
      this->size() - from.size() * matches.size() + to.size() * matches.size();
  utf result(new_size);
  size_t last_pos = 0;
  for (auto match : matches) {
    result.append(this->begin() + last_pos, this->begin() + match);
    result.append(to);
    last_pos = match + from.size();
  }
  result.append(this->begin() + last_pos, this->end());
  return result;
}

template <typename char_type, typename Alloc>
utf<char_type, Alloc> utf<char_type, Alloc>::reverse() const {
  utf result(this->size());
  if (std::is_same_v<char_type, char32_t>) {
    auto k = this->rbegin();
    auto kn = this->rend();
    for (; k != kn; ++k) {
      result.push_back(*k);
    }
  } else {
    auto it = this->ulast(uc::on_failed::skip);
    while (it) {
      auto chunk_view = it.seq_view();
      result.append(chunk_view.data(), chunk_view.size());
      --it;
    }
  }
  return result;
}

template <typename char_type, typename Alloc>
utf<char_type, Alloc> utf<char_type, Alloc>::reverse_bytes() const {
  utf result(this->size());
  auto r = result.begin();
  for (auto i = this->rbegin(); i != this->rend(); ++i, ++r) {
    *r = *i;
  }
  return result;
}

#if GR_HAS_RE2

template <typename char_type, typename Alloc>
template<typename T>
auto utf<char_type, Alloc>::split_by_re2(const char_type *pattern) const
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

template <typename char_type, typename Alloc>
template <typename T>
auto utf<char_type, Alloc>::extract_all(const char_type *pattern) const
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

template <typename char_type, typename Alloc>
template<typename T>
auto utf<char_type, Alloc>::extract(const char_type *pattern) const
  -> std::enable_if_t<std::is_same_v<T, char>, utf<char_type>>
{
  re2::StringPiece result;
  if (re2::RE2::PartialMatch(this->as_std_view(), pattern, &result)) {
    return utf<char_type>(result.data(), result.size());
  }
  return utf<char_type>();
}

#endif

template <typename char_type>
auto utf_view<char_type>::word_boundaries() const -> std::vector<uint32_t> {
  std::vector<uint32_t> boundaries;

  auto it = this->ubegin();
  if (it) {
    boundaries.push_back(it.pos());
    bool in_word = (*it).is_alphabetic();

    const char_type *current = this->data();
    const char_type *end = current + this->size();

    while (current < end) {
      auto seq_info =
          uc::sequence::check(current, end, gr::endian::native);
      if (seq_info.status == uc::sequence_status::valid) {
        auto cp = uc::sequence::decode(
            current, seq_info.length, seq_info.status, gr::endian::native);
        bool current_alpha = cp.is_alphabetic();
        if (current_alpha != in_word) {
          boundaries.push_back(current - this->data());
          in_word = current_alpha;
        }
      }
      current += seq_info.length > 0 ? seq_info.length : 1;
    }
  }

  return boundaries;
}
namespace toy {
struct string_stack {
  const char *ptr;
  size_t size;
  char ch[16];
};

struct string_heep {
  const char *ptr;
  size_t size;
  size_t cap;
};

union string_data {
  string_stack stack;
  string_heep heap;
};

struct termux_string_stack {
  bool in_heap : 1;
  uint8_t size : 7;
  char s[23];
};

struct termux_string_heap {
  bool in_heap : 1;
  size_t cap : 63;
  size_t size;
  char *ptr;
};

union termux_string_data {
  termux_string_stack stack;
  termux_string_heap heap;
};

void hack_string_data(void *s, unsigned char_width,
                      gr::utils::cbuf<char> cbuf) {
#if TERMUX_CLANG_CPP
  using hack_p = termux_string_data *;
  auto sp = hack_p(s);
  bool in_heap = sp->stack.in_heap;
  if (in_heap) {
    delete[] sp->heap.ptr;
  }

  auto bytes = cbuf.bytes();
  auto n_elem = bytes / char_width;

  constexptr unsigned platform_bytes = 23;
  size_t n_elem_limit = (platform_bytes >> (char_width / 2));

  if (n_elem < n_elem_limit) {
    for (int i = 0; i < bytes; i++) {
      sp->stack.s[i] = cbuf[i];
    }
    sp->stack.s[bytes] = 0;
    sp->stack.in_heap = false;
    sp->stack.size = n_elem;
  } else {
    auto [data, bytes_] = cbuf.detach();
    sp->heap.in_heap = true;
    sp->heap.cap = ((n_elem + 1) >> 1) + 1;
    sp->heap.size = n_elem;
    sp->heap.ptr = data;
  }
#else
  using hack_p = string_data *;
  using namespace std;
  auto sp = hack_p(s);
  // bool in_heap = (size_t(sp->stack.ptr) != size_t(sp->stack.ch));
  bool in_heap = (sp->stack.ptr != sp->stack.ch);

  if (in_heap) {
    delete[] sp->heap.ptr;
  }

  auto buf_bytes = cbuf.bytes();
  size_t n_elem = buf_bytes / char_width;

  if (n_elem > (16 / char_width - 1)) {
    // cbuf release ownership to std::string...
    auto [p, nbytes] = cbuf.detach();
    // do move ptr in heap mode
    sp->heap.ptr = p;
    sp->heap.cap = n_elem;
    sp->heap.size = n_elem;
  } else {
    // "do copy data in stack mode
    size_t *order_p = (size_t *)(sp->stack.ch);
    size_t *buf_p = (size_t *)(cbuf.begin());
    *order_p = *buf_p;
    *(++order_p) = *(++buf_p);

    sp->stack.ch[n_elem] = 0;
    sp->stack.size = n_elem;
    sp->stack.ptr = sp->stack.ch;
  }
#endif
}

void hack_string_size(void *s, size_t n) {
#if TERMUX_CLANG_CPP
  using hack_p = termux_string_data *;
  auto sp = hack_p(s);
  if (sp->stack.in_heap) {
    sp->heap.size = n;
  } else {
    sp->stack.size = n;
  }
#else
  using hack_p = string_data *;
  using namespace std;
  auto sp = hack_p(s);
  sp->stack.size = n;
#endif
}
} // namespace toy

namespace bom {

template <> info detect(const char *s, size_t n) {

  info result;
  // UTF-8 BOM detection
  if (n >= 3 && uint8_t(s[0]) == 0xEF && uint8_t(s[1]) == 0xBB &&
      uint8_t(s[2]) == 0xBF) {
    result.has_bom = true;
    result.endian = gr::endian::native; // UTF-8 has no endianness
    result.bom_size = 3;
  }
  return result;
}

template <> info detect(const char16_t *s, size_t n) {
  info result;
  if (n >= 1) {
    char16_t first_char = s[0];
    if (first_char == bom::utf16_le) {
      result.has_bom = true;
      result.endian = gr::endian::little;
      result.bom_size = 1;
    } else if (first_char == bom::utf16_be) {
      result.has_bom = true;
      result.endian = gr::endian::big;
      result.bom_size = 1;
    }
  }
  return result;
}

template <> info detect(const char32_t *s, size_t n) {
  info result;
  if (n >= 1) {
    char32_t first_char = s[0];
    if (first_char == bom::utf32_le) {
      result.has_bom = true;
      result.endian = gr::endian::little;
      result.bom_size = 1;
    } else if (first_char == bom::utf32_be) {
      result.has_bom = true;
      result.endian = gr::endian::big;
      result.bom_size = 1;
    }
  }
  return result;
} // namespace gr::str::bom
} // namespace bom
} // namespace gr::str
