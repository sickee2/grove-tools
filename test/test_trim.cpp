#include <cassert>
#include <chrono>
#include <gr/string.hh>
#include <iostream>

using namespace gr::str;
using namespace gr::literals;

void test_ascii_trim() {
  std::cout << "Testing ASCII trim functions...\n";

  // Test trim()
  {
    u8 str = "  hello world  "_u8;
    str.trim();
    assert(str == "hello world"_u8);

    u8 str2 = "  \t\n  "_u8;
    str2.trim();
    assert(str2.empty());

    u8 str3 = "no_trim"_u8;
    str3.trim();
    assert(str3 == "no_trim"_u8);
  }

  // Test trim_left()
  {
    u8 str = "  hello"_u8;
    str.trim_left();
    assert(str == "hello"_u8);

    u8 str2 = "  \t\n  "_u8;
    str2.trim_left();
    assert(str2.empty());
  }

  // Test trim_right()
  {
    u8 str = "hello  "_u8;
    str.trim_right();
    assert(str == "hello"_u8);

    u8 str2 = "  \t\n  "_u8;
    str2.trim_right();
    assert(str2.empty());
  }

  std::cout << "ASCII trim tests passed!\n";
}

void test_unicode_trim() {
  std::cout << "Testing Unicode trim functions...\n";

  // Test utrim() with various Unicode whitespace
  {
    // 全角空格 U+3000
    u8 str1 = "　hello world　"_u8;
    str1.utrim();
    assert(str1 == "hello world"_u8);

    // 零宽空格 U+200B - 添加调试信息
    u8 str2 = "​hello world​"_u8;
    std::cout << "Before utrim (zero-width space): '" << str2 << "'\n";
    str2.utrim();
    std::cout << "After utrim: '" << str2 << "'\n";
    std::cout << "Expected: 'hello world'\n";

    // 检查每个字符的 Unicode 值
    auto range = str2.urange();
    for (auto it = range.begin(); it != range.end(); ++it) {
      std::cout << "Char: U+" << std::hex << (*it).code() << std::dec << "\n";
    }

    assert(str2 == "hello world"_u8);

    // 窄空格 U+202F
    u8 str3 = " hello world "_u8;
    str3.utrim();
    assert(str3 == "hello world"_u8);

    // 细空格 U+2009
    u8 str4 = " hello world "_u8;
    str4.utrim();
    assert(str4 == "hello world"_u8);

    // 混合空白字符
    u8 str5 = " 　\t​\nhello world\t​　  "_u8;
    str5.utrim();
    assert(str5 == "hello world"_u8);
  }

  // Test utrim_left()
  {
    u8 str = "　  hello"_u8;
    str.utrim_left();
    assert(str == "hello"_u8);

    u8 str2 = "　\t​\n  "_u8;
    str2.utrim_left();
    assert(str2.empty());
  }

  // Test utrim_right()
  {
    u8 str = "hello　  "_u8;
    str.utrim_right();
    assert(str == "hello"_u8);

    u8 str2 = "  \t​\n　"_u8;
    str2.utrim_right();
    assert(str2.empty());
  }

  std::cout << "Unicode trim tests passed!\n";
}

void test_utf_view_trim() {
  std::cout << "Testing utf_view trim functions...\n";

  // Test utf_view trim()
  {
    u8v view = "  test view  "_u8v;
    view.trim();
    assert(view == "test view"_u8v);

    u8v view2 = "  \t\n  "_u8v;
    view2.trim();
    assert(view2.empty());
  }

  // Test utf_view utrim()
  {
    u8v view = "　unicode view　"_u8v;
    view.utrim();
    assert(view == "unicode view"_u8v);

    u8v view2 = "　\t​\n　"_u8v;
    view2.utrim();
    assert(view2.empty());
  }

  std::cout << "utf_view trim tests passed!\n";
}

void test_edge_cases() {
  std::cout << "Testing edge cases...\n";

  // Empty strings
  {
    u8 empty_str;
    empty_str.trim();
    assert(empty_str.empty());
    empty_str.utrim();
    assert(empty_str.empty());

    u8v empty_view;
    empty_view.trim();
    assert(empty_view.empty());
    empty_view.utrim();
    assert(empty_view.empty());
  }

  // Single character
  {
    u8 single = "a"_u8;
    single.trim();
    assert(single == "a"_u8);

    u8 single_space = " "_u8;
    single_space.trim();
    assert(single_space.empty());

    u8 single_unicode_space = "　"_u8;
    single_unicode_space.utrim();
    assert(single_unicode_space.empty());
  }

  // No trimming needed
  {
    u8 no_trim = "hello"_u8;
    no_trim.trim();
    assert(no_trim == "hello"_u8);
    no_trim.utrim();
    assert(no_trim == "hello"_u8);
  }

  // Mixed content
  {
    u8 mixed = "  hello　world  "_u8;
    mixed.trim(); // ASCII trim won't remove unicode spaces
    assert(mixed == "hello　world"_u8);

    mixed.utrim();                      // Unicode trim will remove all spaces
    assert(mixed == "hello　world"_u8); // 注意：中间的unicode空格不会被移除
  }

  std::cout << "Edge case tests passed!\n";
}

void test_utf16_utf32_trim() {
  std::cout << "Testing UTF-16/UTF-32 trim functions...\n";

  // UTF-16 tests
  {
    u16 str16 = u"  hello world  "_u16;
    str16.trim();
    assert(str16 == u"hello world"_u16);

    u16 str16_unicode = u"　hello　"_u16;
    str16_unicode.utrim();
    assert(str16_unicode == u"hello"_u16);
  }

  // UTF-32 tests
  {
    u32 str32 = U"  hello world  "_u32;
    str32.trim();
    assert(str32 == U"hello world"_u32);

    u32 str32_unicode = U"　hello　"_u32;
    str32_unicode.utrim();
    assert(str32_unicode == U"hello"_u32);
  }

  std::cout << "UTF-16/UTF-32 trim tests passed!\n";
}

void test_performance_comparison() {
  std::cout << "Running performance comparison...\n";

  // Create a test string with mixed whitespace
  u8 test_str = "  \t　​\nhello world\t​　\n  "_u8;
  u8 copy1 = test_str;
  u8 copy2 = test_str;

  // Time ASCII trim
  auto start = std::chrono::high_resolution_clock::now();
  copy1.trim();
  auto ascii_duration = std::chrono::high_resolution_clock::now() - start;

  // Time Unicode trim
  start = std::chrono::high_resolution_clock::now();
  copy2.utrim();
  auto unicode_duration = std::chrono::high_resolution_clock::now() - start;

  std::cout << "ASCII trim duration: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   ascii_duration)
                   .count()
            << " μs\n";
  std::cout << "Unicode trim duration: "
            << std::chrono::duration_cast<std::chrono::microseconds>(
                   unicode_duration)
                   .count()
            << " μs\n";

  assert(copy1 == "hello world\t​　"_u8); // ASCII trim leaves unicode spaces
  assert(copy2 == "hello world"_u8);         // Unicode trim removes all spaces

  std::cout << "Performance comparison completed!\n";
}

int main() {
  try {
    std::cout << "Starting trim function tests...\n\n";

    test_ascii_trim();
    test_unicode_trim();
    test_utf_view_trim();
    test_edge_cases();
    test_utf16_utf32_trim();

    // Optional: Uncomment to run performance tests
    // test_performance_comparison();

    std::cout << "\nAll trim tests passed successfully!\n";
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Test failed with unknown exception" << std::endl;
    return 1;
  }
}
