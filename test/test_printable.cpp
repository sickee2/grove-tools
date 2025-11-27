// filepath: test/codepoint_printable_test.cpp
#include <gr/utf_sequence.hh>
#include <cassert>
#include <iostream>

using namespace gr::uc;
void test_is_printable_basic() {
  std::cout << "测试基础可打印字符..." << std::endl;

  // ASCII 可打印字符
  assert(codepoint('A').is_printable()); // 大写字母
  assert(codepoint('z').is_printable()); // 小写字母
  assert(codepoint('0').is_printable()); // 数字
  assert(codepoint('!').is_printable()); // 标点符号
  assert(codepoint(' ').is_printable()); // 空格

  std::cout << "✓ 基础可打印字符测试通过" << std::endl;
}

void test_is_printable_control_chars() {
  std::cout << "测试控制字符..." << std::endl;

  // C0 控制字符
  assert(!codepoint(0x0000).is_printable()); // NULL
  assert(!codepoint(0x0001).is_printable()); // SOH
  assert(!codepoint(0x000A).is_printable()); // LF (不可打印)
  assert(!codepoint(0x000D).is_printable()); // CR (不可打印)
  assert(!codepoint(0x001F).is_printable()); // US

  // C1 控制字符
  assert(!codepoint(0x007F).is_printable()); // DEL
  assert(!codepoint(0x0080).is_printable()); // PAD
  assert(!codepoint(0x009F).is_printable()); // APC

  std::cout << "✓ 控制字符测试通过" << std::endl;
}

void test_is_printable_whitespace() {
  std::cout << "测试空白字符..." << std::endl;

  // 可打印的空白字符
  assert(codepoint(0x0009).is_printable()); // Tab (可打印)
  assert(codepoint(0x0020).is_printable()); // Space (可打印)
  assert(codepoint(0x00A0).is_printable()); // No-Break Space
  assert(codepoint(0x1680).is_printable()); // Ogham Space Mark
  assert(codepoint(0x2000).is_printable()); // En Quad（空格）
  assert(codepoint(0x3000).is_printable()); // Ideographic Space

  // 不可打印的空白字符（控制字符）
  assert(!codepoint(0x000A).is_printable()); // LF (不可打印)
  assert(!codepoint(0x000D).is_printable()); // CR (不可打印)

  std::cout << "✓ 空白字符测试通过" << std::endl;
}

void test_is_printable_format_chars() {
  std::cout << "测试格式字符..." << std::endl;

  // 格式字符应该不可打印（排除空格）
  assert(!codepoint(0x2001).is_printable()); // Em Quad（格式字符）
  assert(!codepoint(0x200F).is_printable()); // 格式字符
  assert(!codepoint(0x2028).is_printable()); // Line Separator
  assert(!codepoint(0x202F).is_printable()); // Narrow No-Break Space
  assert(
      !codepoint(0x205F).is_printable()); // Medium Mathematical Space
  assert(!codepoint(0x206F).is_printable()); // 格式字符
  assert(!codepoint(0xFEFF).is_printable()); // BOM

  std::cout << "✓ 格式字符测试通过" << std::endl;
}

void test_is_printable_non_characters() {
  std::cout << "测试非字符..." << std::endl;

  // 非字符块1
  assert(!codepoint(0xFDD0).is_printable());
  assert(!codepoint(0xFDEF).is_printable());

  // 非字符模式 (0xFFFE-0xFFFF)
  assert(!codepoint(0xFFFE).is_printable());
  assert(!codepoint(0xFFFF).is_printable());
  assert(!codepoint(0x1FFFE).is_printable());
  assert(!codepoint(0x10FFFF).is_printable()); // 最大码点，但属于非字符

  std::cout << "✓ 非字符测试通过" << std::endl;
}

void test_is_printable_private_use() {
  std::cout << "测试私有使用区..." << std::endl;

  // 私有使用区
  assert(!codepoint(0xE000).is_printable());   // PUA开始
  assert(!codepoint(0xF8FF).is_printable());   // PUA结束
  assert(!codepoint(0xF0000).is_printable());  // SPUA-A开始
  assert(!codepoint(0xFFFFD).is_printable());  // SPUA-A结束
  assert(!codepoint(0x100000).is_printable()); // SPUA-B开始
  assert(!codepoint(0x10FFFD).is_printable()); // SPUA-B结束

  std::cout << "✓ 私有使用区测试通过" << std::endl;
}

void test_is_printable_unicode_chars() {
  std::cout << "测试Unicode字符..." << std::endl;

  // 各种Unicode字符应该可打印
  assert(codepoint(0x00A9).is_printable());  // © 版权符号
  assert(codepoint(0x263A).is_printable());  // ☺ 笑脸
  assert(codepoint(0x4E2D).is_printable());  // 中 汉字
  assert(codepoint(0x1F600).is_printable()); // 😀 表情符号

  std::cout << "✓ Unicode字符测试通过" << std::endl;
}

void test_is_printable_invalid() {
  std::cout << "测试无效码点..." << std::endl;

  // 无效码点
  assert(!codepoint(0x110000).is_printable()); // 超出Unicode范围
  assert(!codepoint(0xD800).is_printable());   // 高代理
  assert(!codepoint(0xDFFF).is_printable());   // 低代理

  std::cout << "✓ 无效码点测试通过" << std::endl;
}

void test_is_whitespace() {
  std::cout << "测试is_whitespace方法..." << std::endl;

  // 空白字符
  assert(codepoint(0x0020).is_whitespace()); // Space
  assert(codepoint(0x0009).is_whitespace()); // Tab
  assert(codepoint(0x3000).is_whitespace()); // Ideographic Space

  // 非空白字符
  assert(!codepoint('A').is_whitespace());    // 字母
  assert(!codepoint('0').is_whitespace());    // 数字
  assert(!codepoint(0x00A1).is_whitespace()); // ¡ 倒感叹号
  assert(!codepoint(0x0085).is_whitespace()); // Next Line (C1控制字符)

  std::cout << "✓ is_whitespace测试通过" << std::endl;
}

void debug_is_printable() {
  codepoint tab(0x0009);

  std::cout << "调试 Tab 字符 (0x0009):" << std::endl;
  std::cout << "is_valid(): " << tab.is_valid() << std::endl;
  std::cout << "is_whitespace(): " << tab.is_whitespace() << std::endl;
  std::cout << "is_printable(): " << tab.is_printable() << std::endl;

  // 逐步检查 is_printable() 的逻辑（使用更新后的逻辑）
  if (!tab.is_valid()) {
    std::cout << "失败: 无效码点" << std::endl;
    return;
  }

  // ASCII 快速路径（更新后的逻辑）
  if (tab.value() <= 0x7F) {
    bool ascii_result = (tab.value() >= 0x20 && tab.value() != 0x7F) ||
                        (tab.value() >= 0x09 && tab.value() <= 0x0D);
    std::cout << "ASCII 路径结果: " << ascii_result << std::endl;
    if (ascii_result) {
      std::cout << "成功: ASCII 路径返回 true" << std::endl;
      return;
    } else {
      std::cout << "ASCII 路径返回 false，继续后续检查" << std::endl;
    }
  }

  // 控制字符检查
  bool control_check = (tab.value() >= 0x0001 && tab.value() <= 0x0008) ||
                       (tab.value() >= 0x000E && tab.value() <= 0x001F) ||
                       (tab.value() >= 0x007F && tab.value() <= 0x009F);
  std::cout << "控制字符检查: " << control_check << std::endl;
  if (control_check) {
    std::cout << "失败: 被识别为控制字符" << std::endl;
    return;
  }

  // 格式字符检查
  bool format_check = (tab.value() >= 0x2001 && tab.value() <= 0x200F) ||
                      (tab.value() >= 0x2028 && tab.value() <= 0x202F) ||
                      (tab.value() >= 0x205F && tab.value() <= 0x206F) ||
                      (tab.value() == 0xFEFF);
  std::cout << "格式字符检查: " << format_check << std::endl;
  if (format_check) {
    std::cout << "失败: 被识别为格式字符" << std::endl;
    return;
  }

  // 非字符检查
  bool nonchar_check = (tab.value() >= 0xFDD0 && tab.value() <= 0xFDEF) ||
                       ((tab.value() & 0xFFFE) == 0xFFFE);
  std::cout << "非字符检查: " << nonchar_check << std::endl;
  if (nonchar_check) {
    std::cout << "失败: 被识别为非字符" << std::endl;
    return;
  }

  // 私有使用区检查
  bool pua_check = (tab.value() >= 0xE000 && tab.value() <= 0xF8FF) ||
                   (tab.value() >= 0xF0000 && tab.value() <= 0xFFFFD) ||
                   (tab.value() >= 0x100000 && tab.value() <= 0x10FFFD);
  std::cout << "私有使用区检查: " << pua_check << std::endl;
  if (pua_check) {
    std::cout << "失败: 被识别为私有使用区" << std::endl;
    return;
  }

  // 空白字符检查
  bool whitespace_check = tab.is_whitespace();
  std::cout << "空白字符检查: " << whitespace_check << std::endl;
  if (whitespace_check) {
    std::cout << "成功: 被识别为空白字符" << std::endl;
  } else {
    std::cout << "失败: 未被识别为空白字符" << std::endl;
  }
}
int main() {
  debug_is_printable();
  std::cout << "开始测试 codepoint::is_printable()..." << std::endl;

  try {
    test_is_printable_basic();
    test_is_printable_control_chars();
    test_is_printable_whitespace();
    test_is_printable_format_chars();
    test_is_printable_non_characters();
    test_is_printable_private_use();
    test_is_printable_unicode_chars();
    test_is_printable_invalid();
    test_is_whitespace();

    std::cout << "\n🎉 所有测试通过！" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ 测试失败: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ 未知测试失败" << std::endl;
    return 1;
  }
}
