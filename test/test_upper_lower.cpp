#include <gr/utf_sequence.hh>
#include <cassert>
#include <iostream>
#include <vector>

using namespace gr::uc;

void test_ascii_case_conversion() {
  std::cout << "Testing ASCII case conversion..." << std::endl;

  // 测试小写转大写
  assert(codepoint('a').upper().value() == 'A');
  assert(codepoint('z').upper().value() == 'Z');
  assert(codepoint('m').upper().value() == 'M');

  // 测试大写转小写
  assert(codepoint('A').lower().value() == 'a');
  assert(codepoint('Z').lower().value() == 'z');
  assert(codepoint('M').lower().value() == 'm');

  // 测试非字母字符保持不变
  assert(codepoint('1').upper().value() == '1');
  assert(codepoint('1').lower().value() == '1');
  assert(codepoint('@').upper().value() == '@');
  assert(codepoint('@').lower().value() == '@');

  std::cout << "ASCII case conversion tests passed!" << std::endl;
}

void test_special_mappings() {
  std::cout << "Testing special case mappings..." << std::endl;

  // 测试特殊映射：ß -> ẞ
  assert(codepoint(0x00DF).upper().value() == 0x1E9E); // ß -> ẞ
  assert(codepoint(0x1E9E).lower().value() == 0x00DF); // ẞ -> ß

  // 测试特殊映射：ſ -> S
  assert(codepoint(0x017F).upper().value() == 0x0053); // ſ -> S

  // 测试希腊字母特殊映射
  assert(codepoint(0x03C2).upper().value() == 0x03A3); // ς -> Σ (final sigma)
  assert(codepoint(0x03C3).upper().value() == 0x03A3); // σ -> Σ (regular sigma)
  assert(codepoint(0x03A3).lower().value() == 0x03C3); // Σ -> σ

  std::cout << "Special case mapping tests passed!" << std::endl;
}

void test_latin_extended() {
  std::cout << "Testing Latin Extended case conversion..." << std::endl;

  // 测试拉丁扩展A块 (U+0100-U+017F) 的奇偶映射
  assert(codepoint(0x0101).upper().value() == 0x0100); // ā -> Ā
  assert(codepoint(0x0103).upper().value() == 0x0102); // ă -> Ă
  assert(codepoint(0x0105).upper().value() == 0x0104); // ą -> Ą

  assert(codepoint(0x0100).lower().value() == 0x0101); // Ā -> ā
  assert(codepoint(0x0102).lower().value() == 0x0103); // Ă -> ă
  assert(codepoint(0x0104).lower().value() == 0x0105); // Ą -> ą

  std::cout << "Latin Extended case conversion tests passed!" << std::endl;
}

void test_greek_case_conversion() {
  std::cout << "Testing Greek case conversion..." << std::endl;

  // 测试希腊字母小写转大写
  assert(codepoint(0x03B1).upper().value() == 0x0391); // α -> Α
  assert(codepoint(0x03B2).upper().value() == 0x0392); // β -> Β
  assert(codepoint(0x03C3).upper().value() == 0x03A3); // σ -> Σ

  // 测试希腊字母大写转小写
  assert(codepoint(0x0391).lower().value() == 0x03B1); // Α -> α
  assert(codepoint(0x0392).lower().value() == 0x03B2); // Β -> β
  assert(codepoint(0x03A3).lower().value() == 0x03C3); // Σ -> σ

  std::cout << "Greek case conversion tests passed!" << std::endl;
}

void test_cyrillic_case_conversion() {
  std::cout << "Testing Cyrillic case conversion..." << std::endl;

  // 测试西里尔字母小写转大写
  assert(codepoint(0x0430).upper().value() == 0x0410); // а -> А
  assert(codepoint(0x0431).upper().value() == 0x0411); // б -> Б
  assert(codepoint(0x044F).upper().value() == 0x042F); // я -> Я

  // 测试西里尔字母大写转小写
  assert(codepoint(0x0410).lower().value() == 0x0430); // А -> а
  assert(codepoint(0x0411).lower().value() == 0x0431); // Б -> б
  assert(codepoint(0x042F).lower().value() == 0x044F); // Я -> я

  std::cout << "Cyrillic case conversion tests passed!" << std::endl;
}

void test_no_case_change() {
  std::cout << "Testing characters that should not change case..." << std::endl;

  // 测试数学符号保持不变
  assert(codepoint(0x00D7).upper().value() == 0x00D7); // ×
  assert(codepoint(0x00D7).lower().value() == 0x00D7); // ×
  assert(codepoint(0x00F7).upper().value() == 0x00F7); // ÷
  assert(codepoint(0x00F7).lower().value() == 0x00F7); // ÷

  // 测试标题字母保持不变
  assert(codepoint(0x01C5).upper().value() == 0x01C5); // ǅ
  assert(codepoint(0x01C5).lower().value() == 0x01C5); // ǅ

  std::cout << "No case change tests passed!" << std::endl;
}

void test_invalid_codepoints() {
  std::cout << "Testing invalid code points..." << std::endl;

  // 测试代理对码点
  assert(codepoint(0xD800).upper().value() == 0xD800); // 高位代理
  assert(codepoint(0xDFFF).lower().value() == 0xDFFF); // 低位代理

  // 测试超出Unicode范围的码点
  assert(codepoint(0x110000).upper().value() == 0x110000);
  assert(codepoint(0x110000).lower().value() == 0x110000);

  std::cout << "Invalid code point tests passed!" << std::endl;
}

void test_case_conversion_round_trip() {
  std::cout << "Testing case conversion round trips..." << std::endl;

  // 测试大小写转换的往返一致性
  std::vector<char32_t> test_chars = {
      'a',    'A',    'z', 'Z', 0x0101, 0x0100, // 拉丁扩展
      0x03B1, 0x0391,                           // 希腊字母
      0x0430, 0x0410,                           // 西里尔字母
  };

  for (char32_t ch : test_chars) {
    codepoint cp(ch);
    if (cp.is_valid()) {
      // 对于小写字母：lower() 应该保持不变，upper().lower() 应该恢复原状
      if (std::islower(static_cast<int>(ch))) {
        assert(cp.lower().value() == ch);
        assert(cp.upper().lower().value() == ch);
      }
      // 对于大写字母：upper() 应该保持不变，lower().upper() 应该恢复原状
      else if (std::isupper(static_cast<int>(ch))) {
        assert(cp.upper().value() == ch);
        assert(cp.lower().upper().value() == ch);
      }
    }
  }

  std::cout << "Case conversion round trip tests passed!" << std::endl;
}

void test_comprehensive_unicode_samples() {
  std::cout << "Testing comprehensive Unicode samples..." << std::endl;

  // 测试各种Unicode字符的大小写转换
  struct TestCase {
    char32_t input;
    char32_t expected_upper;
    char32_t expected_lower;
    const char *description;
  };

  std::vector<TestCase> test_cases = {
      // 拉丁字母
      {0x00E0, 0x00C0, 0x00E0, "Latin à -> À"}, // à -> À, à -> à (小写保持不变)
      {0x00C0, 0x00C0, 0x00E0, "Latin À -> à"}, // À -> À, À -> à

      // 拉丁扩展 - 修复这里的期望值
      {0x0111, 0x0110, 0x0111, "Latin đ -> Đ"}, // đ -> Đ, đ -> đ (小写保持不变)
      {0x0110, 0x0110, 0x0111, "Latin Đ -> đ"}, // Đ -> Đ, Đ -> đ

      // 希腊字母
      {0x03B8, 0x0398, 0x03B8, "Greek θ -> Θ"}, // θ -> Θ, θ -> θ (小写保持不变)
      {0x0398, 0x0398, 0x03B8, "Greek Θ -> θ"}, // Θ -> Θ, Θ -> θ

      // 西里尔字母
      {0x0436, 0x0416, 0x0436,
       "Cyrillic ж -> Ж"}, // ж -> Ж, ж -> ж (小写保持不变)
      {0x0416, 0x0416, 0x0436, "Cyrillic Ж -> ж"}, // Ж -> Ж, Ж -> ж
  };

  for (const auto &test_case : test_cases) {
    codepoint cp(test_case.input);
    char32_t actual_upper = cp.upper().value();
    char32_t actual_lower = cp.lower().value();

    std::cout << "Testing " << test_case.description << " (0x" << std::hex
              << uint32_t(test_case.input) << "): "
              << "upper=0x" << std::hex << uint32_t(actual_upper)
              << " (expected 0x" << std::hex
              << uint32_t(test_case.expected_upper) << "), "
              << "lower=0x" << std::hex << uint32_t(actual_lower)
              << " (expected 0x" << std::hex
              << uint32_t(test_case.expected_lower) << ")" << std::endl;

    assert(actual_upper == test_case.expected_upper);
    assert(actual_lower == test_case.expected_lower);
  }

  std::cout << "Comprehensive Unicode sample tests passed!" << std::endl;
}

void debug_latin_extended() {
  std::cout << "\nDebugging Latin Extended characters:" << std::endl;

  // 测试一些关键的拉丁扩展字符
  std::vector<char32_t> test_chars = {
      0x0100, 0x0101, // Ā, ā
      0x0102, 0x0103, // Ă, ă
      0x0110, 0x0111, // Đ, đ
      0x0112, 0x0113, // Ē, ē
  };

  for (char32_t ch : test_chars) {
    codepoint cp(ch);
    std::cout << "0x" << std::hex << uint32_t(ch) << " -> upper=0x"
              << cp.upper().code() << ", lower=0x" << cp.lower().code()
              << std::endl;
  }
}
int main() {
  std::cout << "Starting UTF iterator case conversion tests..." << std::endl;

  try {
    debug_latin_extended();

    test_ascii_case_conversion();
    test_special_mappings();
    test_latin_extended();
    test_greek_case_conversion();
    test_cyrillic_case_conversion();
    test_no_case_change();
    test_invalid_codepoints();
    test_case_conversion_round_trip();
    test_comprehensive_unicode_samples();
    std::cout << "\n✅ All tests passed successfully!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "\n❌ Unknown test failure!" << std::endl;
    return 1;
  }
}
