#include <gr/utf_sequence.hh>
#include <cstdint>
#include <gr/utf_iter.hh>
#include <iostream>

std::ostream &operator<<(std::ostream &os, gr::uc::chunk_proxy8 u8) {
  os << u8.buf;
  return os;
}

std::ostream &operator<<(std::ostream &os, gr::uc::codepoint code) {
  os << code.chunk_u8();
  return os;
}

namespace gr::uc {

static const char *status_infos[] = {"Valid", "InvalidContinuation",
                                     "Truncated", "InvalidStarByte"};

const char *get_status_info(sequence_status status) {
  return status_infos[int(status)];
}

/**
 * @brief List of special characters that should not undergo case conversion
 *
 * Includes mathematical symbols, titlecase letters, combining characters,
 * etc. These characters should remain unchanged during case conversion.
 */
static constexpr char32_t NO_CASE_CHANGE[] = {
    0x00D7, // × Multiplication sign
    0x00F7, // ÷ Division sign
    0x0149, // ŉ Latin small letter n preceded by apostrophe
    0x01C5, // ǅ Latin capital letter D with small letter z with caron
    0x01C8, // ǈ Latin capital letter L with small letter j
    0x01CB, // ǋ Latin capital letter N with small letter j
    0x01F2, // ǲ Latin capital letter D with small letter z
    0x1F88, // ᾈ Greek capital letter alpha with psili and prosgegrammeni
    0x1F89, // ᾉ Greek capital letter alpha with dasia and prosgegrammeni
    0x1F8A, // ᾊ Greek capital letter alpha with psili and varia and
            // prosgegrammeni
    0x1F8B, // ᾋ Greek capital letter alpha with dasia and varia and
            // prosgegrammeni
    0x1F8C, // ᾌ Greek capital letter alpha with psili and oxia and
            // prosgegrammeni
    0x1F8D, // ᾍ Greek capital letter alpha with dasia and oxia and
            // prosgegrammeni
    0x1F8E, // ᾎ Greek capital letter alpha with psili and perispomeni and
            // prosgegrammeni
    0x1F8F, // ᾏ Greek capital letter alpha with dasia and perispomeni and
            // prosgegrammeni
};

/**
 * @brief Special case mapping table
 *
 * Contains irregular case conversion rules that cannot be computed by simple
 * offsets. Each mapping pair represents {lowercase/special form,
 * uppercase/standard form}
 */
static constexpr std::pair<char32_t, char32_t> CASE_PAIRS[] = {
    // Special lowercase to uppercase
    {0x00DF, 0x1E9E}, // ß -> ẞ
    {0x017F, 0x0053}, // ſ -> S
    {0x03C2, 0x03A3}, // ς -> Σ (final sigma)
    {0x03C3, 0x03A3}, // σ -> Σ (regular sigma)

    // Special lowercase to uppercase
    {0x1E9E, 0x00DF}, // ẞ -> ß
    {0x03A3, 0x03C3}, // Σ -> σ
};

/**
 * @brief Character range mapping structure
 */
struct range_shift {
  char32_t beg;
  char32_t end;
  int32_t shift;
};

/**
 * @brief Range mappings for lowercase to uppercase conversion
 */
static constexpr range_shift UPPER_RANGES[] = {
    // lowercase to uppercase
    {0x0061, 0x007A, -32}, // a-z -> A-Z
    {0x00E0, 0x00F6, -32}, // Latin-1 Supplement lowercase
    {0x00F8, 0x00FE, -32}, // Latin-1 Supplement lowercase (excluding ÷)
    {0x0180, 0x0233, -1},  // Latin Extended-B and beyond
    {0x03B1, 0x03C1, -32}, // Greek lowercase alpha-rho
    {0x03C3, 0x03CB, -32}, // Greek lowercase sigma-psi
    {0x03CD, 0x03CE, -32}, // Greek lowercase with accents
    {0x0430, 0x044F, -32}, // Cyrillic lowercase
    {0x0450, 0x045F, -80}, // Cyrillic lowercase with accents
    {0x0461, 0x0481, -1},  // Cyrillic Extended lowercase
    {0x0561, 0x0586, -48}, // Armenian lowercase
};

/**
 * @brief Range mappings for uppercase to lowercase conversion
 */
static constexpr range_shift LOWER_RANGES[] = {
    // uppercase to lowercase
    {0x0041, 0x005A, 32}, // A-Z -> a-z
    {0x00C0, 0x00D6, 32}, // Latin-1 Supplement uppercase
    {0x00D8, 0x00DE, 32}, // Latin-1 Supplement uppercase (excluding ×)
    {0x0181, 0x0232, 1},  // Latin Extended-B and beyond
    {0x0391, 0x03A1, 32}, // Greek uppercase alpha-rho
    {0x03A3, 0x03A9, 32}, // Greek uppercase sigma-omega
    {0x0410, 0x042F, 32}, // Cyrillic uppercase
    {0x0400, 0x040F, 80}, // Cyrillic uppercase with accents
    {0x0460, 0x0480, 1},  // Cyrillic Extended uppercase
    {0x0531, 0x0556, 48}, // Armenian uppercase
};

/**
 * @brief latin extended
 */
char32_t handle_latin_extended(char32_t ch, bool to_upper) {
  // latin extended A (U+0100-U+017F)
  if (ch >= 0x0100 && ch <= 0x017F) {
    if (to_upper) {
      // lowercase to uppercase：odd -> even
      if (ch & 1) {
        char32_t result = ch - 1;
        if (result >= 0x0100 && result <= 0x017E) {
          return result;
        }
      }
    } else {
      // uppercase to lowercase：even -> odd
      if (!(ch & 1)) {
        char32_t result = ch + 1;
        if (result >= 0x0101 && result <= 0x017F) {
          return result;
        }
      }
    }
  }

  // latin extended B (U+0180-U+024F)
  if (ch >= 0x0180 && ch <= 0x024F) {
    if (to_upper) {
      // lowercase to uppercase：odd -> even
      if (ch & 1) {
        char32_t result = ch - 1;
        if (result >= 0x0180 && result <= 0x024E) {
          return result;
        }
      }
    } else {
      // uppercase to lowercase: even -> odd
      if (!(ch & 1)) {
        char32_t result = ch + 1;
        if (result >= 0x0181 && result <= 0x024F) {
          return result;
        }
      }
    }
  }

  return ch; // Not convert
}

bool should_skip_case_change(char32_t ch) {
  for (char32_t skip_ch : NO_CASE_CHANGE) {
    if (ch == skip_ch)
      return true;
  }
  return false;
}

codepoint::codepoint(std::string_view sv) : m_value(0xFFFD) { // 默认替换字符
  if (sv.empty()) {
    m_value = 0;
    return;
  }

  auto info = sequence::check(sv.data(), sv.data() + sv.size());
  if (info.status == sequence_status::valid) {
    m_value = static_cast<char32_t>(
        sequence::decode(sv.data(), info.length, info.status));
  }
}

codepoint::codepoint(std::u16string_view sv) : m_value(0xFFFD) { // 默认替换字符
  if (sv.empty()) {
    m_value = 0;
    return;
  }

  // 使用现有的 UTF-16 序列检查机制
  auto info = sequence::check(sv.data(), sv.data() + sv.size());
  if (info.status == sequence_status::valid) {
    m_value = static_cast<char32_t>(
        sequence::decode(sv.data(), info.length, info.status));
  }
}

codepoint codepoint::upper() const {
  if (!is_valid())
    return *this;

  // Skip no case
  if (should_skip_case_change(m_value)) {
    return *this;
  }

  // 1. ASCII
  if (m_value <= 0x7F) {
    if (m_value >= 'a' && m_value <= 'z') {
      return codepoint(m_value - 32);
    }
    return *this;
  }

  // 2. Special mappings
  for (const auto &[from, to] : CASE_PAIRS) {
    if (m_value == from)
      return codepoint(to);
  }

  // 3. Latin extended
  char32_t latin_result = handle_latin_extended(m_value, true);
  if (latin_result != m_value) {
    return codepoint(latin_result);
  }

  // 4. Case map（upper to lower）
  for (const auto &[start, end, offset] : UPPER_RANGES) {
    if (m_value >= start && m_value <= end) {
      char32_t result = m_value + offset;
      // Check result
      if (result <= 0x10FFFF && !(result >= 0xD800 && result <= 0xDFFF)) {
        return codepoint(result);
      }
    }
  }

  // Not mapped
  return *this;
}

codepoint codepoint::lower() const {
  if (!is_valid())
    return *this;

  // Skip no case
  if (should_skip_case_change(m_value)) {
    return *this;
  }

  // 1. ASCII
  if (m_value <= 0x7F) {
    if (m_value >= 'A' && m_value <= 'Z') {
      return codepoint(m_value + 32);
    }
    return *this;
  }

  // 2. Special mappings(not normal)
  for (const auto &[from, to] : CASE_PAIRS) {
    if (m_value == from)
      return codepoint(to);
  }

  // 3. Latin extended
  char32_t latin_result = handle_latin_extended(m_value, false);
  if (latin_result != m_value) {
    return codepoint(latin_result);
  }

  // 4. Case map（upper to lower）
  for (const auto &[start, end, offset] : LOWER_RANGES) {
    if (m_value >= start && m_value <= end) {
      char32_t result = m_value + offset;
      // Check convert result
      if (result <= 0x10FFFF && !(result >= 0xD800 && result <= 0xDFFF)) {
        return codepoint(result);
      }
    }
  }

  // Not mapped
  return *this;
}

bool codepoint::is_printable() const {
  if (!is_valid())
    return false;

  // ASCII
  if (m_value <= 0x7F) {
    // ASCII printable range：0x20-0x7E
    if (m_value >= 0x20 && m_value != 0x7F) {
      return true;
    }
    // White space：only Tab (0x09) and Space (0x20) is printable
    if (m_value == 0x09 || m_value == 0x20) {
      return true;
    }
    return false;
  }

  // Control ranges
  static constexpr std::pair<char32_t, char32_t> CONTROL_RANGES[] = {
      {0x0001, 0x0008}, // C0 controls (!Tab)
      {0x000A, 0x001F}, // C0 controls (!CR, LF...)
      {0x007F, 0x009F}, // C1 controls
  };

  // Format ranges
  static constexpr std::pair<char32_t, char32_t> FORMAT_RANGES[] = {
      {0x2001, 0x200F}, // fmt（!space）
      {0x2028, 0x202F}, // separate and format
      {0x205F, 0x206F}, // operator of invisible
  };

  // None char
  static constexpr std::pair<char32_t, char32_t> NONCHAR_RANGES[] = {
      {0xFDD0, 0xFDEF},     // Nono char block 1
      {0xFFFE, 0xFFFF},     // Nono char block 2
      {0x1FFFE, 0x1FFFF},   // Nono char block 3
      {0x2FFFE, 0x2FFFF},   // Nono char block 4
      {0x3FFFE, 0x3FFFF},   // Nono char block 5
      {0x4FFFE, 0x4FFFF},   // Nono char block 6
      {0x5FFFE, 0x5FFFF},   // Nono char block 7
      {0x6FFFE, 0x6FFFF},   // Nono char block 8
      {0x7FFFE, 0x7FFFF},   // Nono char block 9
      {0x8FFFE, 0x8FFFF},   // Nono char block 10
      {0x9FFFE, 0x9FFFF},   // Nono char block 11
      {0xAFFFE, 0xAFFFF},   // Nono char block 12
      {0xBFFFE, 0xBFFFF},   // Nono char block 13
      {0xCFFFE, 0xCFFFF},   // Nono char block 14
      {0xDFFFE, 0xDFFFF},   // Nono char block 15
      {0xEFFFE, 0xEFFFF},   // Nono char block 16
      {0xFFFFE, 0xFFFFF},   // Nono char block 17
      {0x10FFFE, 0x10FFFF}, // Nono char block 18
  };

  // Private ranges
  static constexpr std::pair<char32_t, char32_t> PUA_RANGES[] = {
      {0xE000, 0xF8FF},     // PUA
      {0xF0000, 0xFFFFD},   // SPUA-A
      {0x100000, 0x10FFFD}, // SPUA-B
  };

  // Check control ranges
  for (const auto &[start, end] : CONTROL_RANGES) {
    if (m_value >= start && m_value <= end) {
      return false;
    }
  }

  // Check format ranges
  for (const auto &[start, end] : FORMAT_RANGES) {
    if (m_value >= start && m_value <= end) {
      return false;
    }
  }

  // Check nono char
  for (const auto &[start, end] : NONCHAR_RANGES) {
    if (m_value >= start && m_value <= end) {
      return false;
    }
  }

  // Private ranges
  for (const auto &[start, end] : PUA_RANGES) {
    if (m_value >= start && m_value <= end) {
      return false;
    }
  }

  // Special fmt code
  if (m_value == 0xFEFF) { // BOM
    return false;
  }

  // White space（default is printable）
  if (is_whitespace()) {
    return true;
  }

  return true;
}

bool codepoint::is_whitespace() const {
  // White space range list
  static constexpr std::pair<char32_t, char32_t> WHITESPACE_RANGES[] = {
      {0x0009, 0x000D}, // Tab, LF, VT, FF, CR
      {0x2002, 0x200A}, // 各种空格
      {0x2028, 0x2029}, // Line Separator, Paragraph Separator
  };

  // Single white space list
  static constexpr char32_t WHITESPACE_CHARS[] = {
      0x0020, // Space
      0x00A0, // No-Break Space
      0x1680, // Ogham Space Mark
      0x180E, // Mongolian Vowel Separator
      0x200B, // Zero Width Space (零宽空格)
      0x200C, // Zero Width Non-Joiner
      0x200D, // Zero Width Joiner
      0x202F, // Narrow No-Break Space
      0x205F, // Medium Mathematical Space
      0x2060, // Word Joiner
      0x3000, // Ideographic Space
      0xFEFF, // Zero Width No-Break Space (BOM)
  };

  // Check white space ranges
  for (const auto &[start, end] : WHITESPACE_RANGES) {
    if (m_value >= start && m_value <= end) {
      return true;
    }
  }

  // Check single white space char
  for (char32_t ch : WHITESPACE_CHARS) {
    if (m_value == ch) {
      return true;
    }
  }

  return false;
}

chunk_proxy8 codepoint::chunk_u8() const {
  if (!is_valid()) {
    return chunk_proxy8::make_replacement();
  }

  chunk_proxy8 result;
  if (m_value <= 0x7F) {
    result.buf[0] = char(m_value);
    result.buf[5] = 1;
  } else if (m_value <= 0x7FF) {
    result.buf[0] = char(0xC0 | (m_value >> 6));
    result.buf[1] = char(0x80 | (m_value & 0x3F));
    result.buf[5] = 2;
  } else if (m_value <= 0xFFFF) {
    result.buf[0] = char(0xE0 | (m_value >> 12));
    result.buf[1] = char(0x80 | ((m_value >> 6) & 0x3F));
    result.buf[2] = char(0x80 | (m_value & 0x3F));
    result.buf[5] = 3;
  } else if (m_value <= 0x10FFFF) {
    result.buf[0] = char(0xF0 | (m_value >> 18));
    result.buf[1] = char(0x80 | ((m_value >> 12) & 0x3F));
    result.buf[2] = char(0x80 | ((m_value >> 6) & 0x3F));
    result.buf[3] = char(0x80 | (m_value & 0x3F));
    result.buf[5] = 4;
  }
  return result;
}

/**
 * @brief Convert to UTF-16 encoding
 * @return chunk16 containing UTF-16 bytes
 * @note Invalid code points are replaced with U+FFFD
 */
chunk_proxy16 codepoint::chunk_u16() const {
  if (!is_valid()) {
    return chunk_proxy16::make_replacement();
  }

  chunk_proxy16 result;
  if (m_value <= 0xFFFF) {
    result.buf[0] = static_cast<char16_t>(m_value);
    result.buf[3] = 1;
  } else {
    char32_t v = m_value - 0x10000;
    result.buf[0] = char16_t(0xD800 | (v >> 10));
    result.buf[1] = char16_t(0xDC00 | (v & 0x3FF));
    result.buf[3] = 2;
  }
  return result;
}

int codepoint::display_width() const {
  if (!is_valid()) {
    return 1; // Replacement character usually takes 1 column
  }

  // Control characters and non-printable characters
  if (!is_printable()) {
    return 0;
  }

  // ASCII characters (most are narrow)
  if (m_value <= 0x7F) {
    return 1;
  }

  // East Asian Wide and Fullwidth characters (take 2 columns)
  static constexpr std::pair<char32_t, char32_t> WIDE_RANGES[] = {
    {0x1100, 0x115F}, // Hangul Jamo
    {0x231A, 0x231B}, // Watch, Hourglass
    {0x2329, 0x232A}, // Angle brackets
    {0x23E9, 0x23EC}, // Media control symbols
    {0x23F0, 0x23F0}, // Alarm clock
    {0x23F3, 0x23F3}, // Hourglass with flowing sand
    {0x25FD, 0x25FE}, // White/black small square
    {0x2614, 0x2615}, // Umbrella with rain drops, hot beverage
    {0x2648, 0x2653}, // Zodiac symbols
    {0x267F, 0x267F}, // Wheelchair symbol
    {0x2693, 0x2693}, // Anchor
    {0x26A1, 0x26A1}, // High voltage sign
    {0x26AA, 0x26AB}, // Medium white/black circle
    {0x26BD, 0x26BE}, // Soccer ball, baseball
    {0x26C4, 0x26C5}, // Snowman without/with snow
    {0x26CE, 0x26CE}, // Ophiuchus
    {0x26D4, 0x26D4}, // No entry
    {0x26EA, 0x26EA}, // Church
    {0x26F2, 0x26F3}, // Fountain, flag in hole
    {0x26F5, 0x26F5}, // Sailboat
    {0x26FA, 0x26FA}, // Tent
    {0x26FD, 0x26FD}, // Fuel pump
    {0x2705, 0x2705}, // White heavy check mark
    {0x270A, 0x270B}, // Raised fist, raised hand
    {0x2728, 0x2728}, // Sparkles
    {0x274C, 0x274C}, // Cross mark
    {0x274E, 0x274E}, // Negative squared cross mark
    {0x2753, 0x2755}, // Question/exclamation marks
    {0x2757, 0x2757}, // Heavy exclamation mark symbol
    {0x2795, 0x2797}, // Plus/minus/division signs
    {0x27B0, 0x27B0}, // Curly loop
    {0x27BF, 0x27BF}, // Double curly loop
    {0x2B1B, 0x2B1C}, // Black/white large square
    {0x2B50, 0x2B50}, // White medium star
    {0x2B55, 0x2B55}, // Heavy large circle
    {0x1F004, 0x1F004}, // Mahjong tile red dragon
    {0x1F0CF, 0x1F0CF}, // Playing card black joker
    {0x1F18E, 0x1F18E}, // Negative squared AB
    {0x1F191, 0x1F19A}, // Squared symbols
    {0x1F200, 0x1F2FF}, // Enclosed ideographic supplement
    {0x1F300, 0x1F64F}, // Miscellaneous Symbols and Pictographs
    {0x1F680, 0x1F6FF}, // Transport and Map Symbols
    {0x1F900, 0x1F9FF}, // Supplemental Symbols and Pictographs
  };

  // CJK Unified Ideographs and extensions (typically wide)
  if ((m_value >= 0x4E00 && m_value <= 0x9FFF) ||   // CJK Unified Ideographs
      (m_value >= 0x3400 && m_value <= 0x4DBF) ||   // CJK Extension A
      (m_value >= 0x20000 && m_value <= 0x2A6DF) || // CJK Extension B
      (m_value >= 0x2A700 && m_value <= 0x2B73F) || // CJK Extension C
      (m_value >= 0x2B740 && m_value <= 0x2B81F) || // CJK Extension D
      (m_value >= 0x2B820 && m_value <= 0x2CEAF) || // CJK Extension E
      (m_value >= 0x2CEB0 && m_value <= 0x2EBEF) || // CJK Extension F
      (m_value >= 0x30000 && m_value <= 0x3134F) || // CJK Extension G
      (m_value >= 0xF900 && m_value <= 0xFAFF) ||   // CJK Compatibility Ideographs
      (m_value >= 0x2F800 && m_value <= 0x2FA1F)) { // CJK Compatibility Supplement
    return 2;
  }

  // Check wide ranges
  for (const auto& [start, end] : WIDE_RANGES) {
    if (m_value >= start && m_value <= end) {
      return 2;
    }
  }

  // Default to narrow width
  return 1;
}


template<unsigned bytes>
bool check_u8_flow_bytest(const char*);

template<>
bool check_u8_flow_bytest<2>(const char* p){
  return ((p[1]&0xC0) == 0x80) ? true : false;
}

template<>
bool check_u8_flow_bytest<3>(const char* p){
  uint16_t value = *reinterpret_cast<const uint16_t*>(p + 1);
  return (value & 0xC0C0) == 0x8080 ? true : false;
}

template<>
bool check_u8_flow_bytest<4>(const char* c){
  constexpr bool endian = gr::is_little_endian();
  constexpr uint32_t mask = endian? 0xC0C0C0F0 : 0xF0C0C0C0;
  constexpr uint32_t v =    endian? 0x808080F0 : 0xF0808080;
  uint32_t value = *(uint32_t*)(c);
  return (value & mask) == v ? true : false;
}

unsigned check_u8_header(const char* p){
  unsigned char c = static_cast<unsigned char>(*p);
  if(c == 0) return 0;
  if((c & 0xF0) == 0xF0){
    if(check_u8_flow_bytest<4>(p))
      return 4;
  }else if((c & 0xE0) == 0xE0){
    if(check_u8_flow_bytest<3>(p))
      return 3;
  }else if((c & 0xC0) == 0xC0){
    if(check_u8_flow_bytest<2>(p))
      return 2;
  }else{
    return 1;
  }
  return 0;
}
/// sequence<char> .....
/**
 * @brief Validate UTF-8 sequence
 */
sequence_info sequence::check(const char *current, const char *end,
                                    gr::endian endian_) {
  (void)(endian_);
  if (current >= end)
    return {0, sequence_status::truncated};

  auto *p = (const unsigned char *)(current);
  unsigned char c = *p;

  /// Check the validity of the starting byte
  if (c < 0x80)
    return {1, sequence_status::valid}; // ASCII

  /// Check for invalid starting bytes (0xC0, 0xC1, 0xF5-0xFF)
  if (c == 0xC0 || c == 0xC1 || c >= 0xF5) {
    return {1, sequence_status::invalid_starbyte};
  }

  /// Determine the sequence length
  size_t len = check_u8_header(current);
  if(len < 2) return {len, sequence_status::invalid_continuation};
  /// Check the integrity of the sequence
  if (current + len > end)
    return {0, sequence_status::truncated};

  return {len, sequence_status::valid};
}

codepoint sequence::decode(const char *current, uint8_t seq_len,
                                 sequence_status status, gr::endian endian_) {
  (void)endian_;

  if (status != sequence_status::valid)
    return codepoint(0xFFFD);
  auto *p = (const unsigned char *)(current);
  switch (seq_len) {
  case 1:
    return codepoint(p[0]);
  case 2:
    return codepoint(((p[0] & 0x1F) << 6) | (p[1] & 0x3F));
  case 3:
    return codepoint(((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) |
                     (p[2] & 0x3F));
  case 4:
    return codepoint(((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
                     ((p[2] & 0x3F) << 6) | (p[3] & 0x3F));
  default:
    return codepoint(0xFFFD);
  }
}

/// sequence<char16_t> .....

sequence_info sequence::check(const char16_t *current,
                                        const char16_t *end,
                                        gr::endian endian_) {
  if (current >= end)
    return {0, sequence_status::truncated};

  char16_t w1 = gr::convert_endian(*current, endian_);
  if (w1 >= 0xD800 && w1 <= 0xDBFF) {
    if (current + 1 >= end)
      return {0, sequence_status::truncated};
    char16_t w2 = gr::convert_endian(*(current + 1), endian_);
    return (w2 >= 0xDC00 && w2 <= 0xDFFF)
               ? sequence_info{2, sequence_status::valid}
               : sequence_info{1, sequence_status::invalid_continuation};
  }
  return {1, sequence_status::valid};
}

codepoint sequence::decode(const char16_t *current, uint8_t seq_len,
                                     sequence_status status,
                                     gr::endian endian_) {
  if (status != sequence_status::valid)
    return codepoint(0xFFFD);

  char16_t w1 = gr::convert_endian(*current, endian_);

  if (seq_len == 1)
    return codepoint(char32_t(w1));

  char16_t w2 = gr::convert_endian(*(current + 1), endian_);

  return codepoint(((w1 - 0xD800) << 10) + (w2 - 0xDC00) + 0x10000);
}

} // namespace gr::uc
