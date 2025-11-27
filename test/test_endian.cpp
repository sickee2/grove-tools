#include <cassert>
#include <gr/string.hh>
#include <iomanip>
#include <iostream>

using namespace gr::str;

void print_bytes(const void *data, size_t size, const char *label) {
  std::cout << label << ": ";
  const unsigned char *bytes = static_cast<const unsigned char *>(data);
  for (size_t i = 0; i < size; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(bytes[i]) << " ";
  }
  std::cout << std::dec << std::endl;
}

void debug_basic_conversion() {
  std::cout << "=== Debug Basic Endian Conversion ===" << std::endl;

  uint16_t test_value = 0x1234;

  std::cout << "Original value: 0x" << std::hex << test_value << std::dec << std::endl;
  std::cout << "Native gr::endian: " << (gr::is_little_endian() ? "Little Endian" : "Big Endian") << std::endl;

  print_bytes(&test_value, sizeof(test_value), "Original bytes");

  uint16_t big_endian = gr::convert_endian(test_value, gr::endian::big);
  print_bytes(&big_endian, sizeof(big_endian), "Big Endian bytes");

  uint16_t back_to_native =
      gr::convert_endian(big_endian, gr::endian::big);
  print_bytes(&back_to_native, sizeof(back_to_native), "Read back to native");

  std::cout << "Converted back value: 0x" << std::hex << back_to_native << std::dec
            << std::endl;
  std::cout << "Equal: " << (back_to_native == test_value ? "Yes" : "No")
            << std::endl;

  if (back_to_native != test_value) {
    std::cout << "❌ Endian conversion failed!" << std::endl;
  } else {
    std::cout << "✓ Endian conversion successful" << std::endl;
  }
}

void debug_utf16_conversion() {
  std::cout << "\n=== Debug UTF-16 Conversion ===" << std::endl;

  char16_t simple_chars[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F}; // "Hello"

  char16_t big_endian_chars[5];
  for (int i = 0; i < 5; ++i) {
    big_endian_chars[i] = convert_endian(simple_chars[i], gr::endian::big);
  }

  for (int i = 0; i < 5; ++i) {
    char16_t original = simple_chars[i];
    char16_t read_from_big = convert_endian(big_endian_chars[i], gr::endian::big);

    std::cout << "Character " << i << ": 0x" << std::hex << uint16_t(original)
              << " -> Stored as big endian: 0x" << (uint16_t)big_endian_chars[i]
              << " -> Read as: 0x" << (uint16_t)read_from_big << " ("
              << (read_from_big == original ? "✓" : "❌") << ")" << std::dec
              << std::endl;
  }
}

void debug_utf16_iterator() {
  std::cout << "\n=== Debug UTF-16 Iterator ===" << std::endl;

  char16_t utf16_native[] = {0x0048, 0x0065, 0x006C, 0x006C, 0x006F, // "Hello"
                             0xD83D, 0xDE00}; // 😀 emoji

  char16_t utf16_big[7];
  for (int i = 0; i < 7; ++i) {
    utf16_big[i] = convert_endian(utf16_native[i], gr::endian::big);
  }

  auto iter_big =
      gr::uc::make_iterator(std::u16string_view(utf16_big, 7), 0,
                            gr::uc::on_failed::skip, gr::endian::big);

  auto iter_native =
      gr::uc::make_iterator(std::u16string_view(utf16_native, 7), 0,
                            gr::uc::on_failed::skip, gr::endian::native);

  std::cout << "Comparing iterator outputs:" << std::endl;
  size_t count = 0;
  while (iter_native && iter_big) {
    auto cp_native = *iter_native;
    auto cp_big = *iter_big;

    std::cout << "Code point " << count << ": Native=U+" << std::hex << cp_native.code()
              << " Big Endian=U+" << cp_big.code() << " ("
              << (cp_native == cp_big ? "✓" : "❌") << ")" << std::dec
              << std::endl;

    ++iter_native;
    ++iter_big;
    ++count;
  }

  std::cout << "Total processed code points: " << count << std::endl;
}

void debug_swap_bytes() {
  std::cout << "\n=== Debug swap_bytes Function ===" << std::endl;

  uint16_t test16 = 0x1234;
  uint32_t test32 = 0x12345678;

  auto swapped16 = gr::swap_bytes(test16);
  auto swapped32 = gr::swap_bytes(test32);

  std::cout << "0x" << std::hex << test16 << " -> 0x" << swapped16 << std::dec
            << std::endl;
  std::cout << "0x" << std::hex << test32 << " -> 0x" << swapped32 << std::dec
            << std::endl;

  uint16_t expected16 = 0x3412;
  uint32_t expected32 = 0x78563412;

  std::cout << "16-bit swap " << (swapped16 == expected16 ? "✓" : "❌")
            << std::endl;
  std::cout << "32-bit swap " << (swapped32 == expected32 ? "✓" : "❌")
            << std::endl;
}

int main() {
  debug_basic_conversion();
  debug_utf16_conversion();
  debug_utf16_iterator();
  debug_swap_bytes();
  return 0;
}
