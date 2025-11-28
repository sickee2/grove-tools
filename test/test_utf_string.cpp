#include <cassert>
#include <gr/string.hh>
#include <gr/console.hh>

using namespace gr::str;
using namespace gr::literals;

void test_constructors() {
  gr::console::writeln("=== Testing Constructors ===");

  // 默认构造
  u8 str1;
  assert(str1.empty());

  // C字符串构造 - 使用构造函数而不是赋值
  u8 str2("Hello");
  assert(str2 == "Hello");

  // 带长度构造
  u8 str3("Hello World", 5);
  assert(str3 == "Hello");

  // std::string构造
  std::string std_str = "Test";
  u8 str4(std_str);
  assert(str4 == "Test");

  // char8_t构造
  const char8_t *u8_str = u8"UTF-8字符串";
  u8 str5(u8_str);
  assert(str5.size() > 0);

  // 重复字符构造
  u8 str6(5, 'A');
  assert(str6 == "AAAAA");

  // 预分配构造
  u8 str7(100);
  assert(str7.capacity() >= 100);

  gr::console::writeln("✓ All constructor tests passed");
}

void test_assignment_operators() {
  gr::console::writeln("=== Testing Assignment Operators ===");

  // 修复赋值操作符歧义问题
  u8 str1;

  // 方法1: 使用显式构造
  str1 = u8("Hello");
  assert(str1 == "Hello");

  // 方法2: 使用 assign 方法
  u8 str2;
  str2.assign("Hello");
  assert(str2 == "Hello");

  // 方法3: 使用字符串视图
  u8 str3;
  str3 = u8v("Hello");
  assert(str3 == "Hello");

  // 正常的 utf 对象赋值
  u8 str4 = "Test";
  u8 str5;
  str5 = str4;
  assert(str5 == "Test");

  // 移动赋值
  u8 str6 = "Move";
  u8 str7 = std::move(str6);
  assert(str7 == "Move");
  assert(str6.empty());

  gr::console::writeln("✓ All assignment operator tests passed");
}

void test_unicode_iteration() {
  gr::console::writeln("=== Testing Unicode Iteration ===");

  u8 str("Hello 世界 🌍");

  // Unicode字符计数
  size_t code_point_count = str.usize();
  assert(code_point_count > 0);
  gr::console::writeln("Code points: {}", code_point_count);

  // Unicode迭代
  size_t count = 0;
  for (auto cp : str.urange()) {
    ++count;
    assert(cp.is_valid());
  }
  assert(count == code_point_count);

  gr::console::writeln("✓ All unicode iteration tests passed");
}

void test_case_conversion() {
  gr::console::writeln("=== Testing Case Conversion ===");

  u8 str1("Hello World");
  str1.to_upper();
  assert(str1 == "HELLO WORLD");

  u8 str2("HELLO WORLD");
  str2.to_lower();
  assert(str2 == "hello world");

  gr::console::writeln("✓ All case conversion tests passed");
}

void test_trimming() {
  gr::console::writeln("=== Testing Trimming ===");

  u8 str1("   Hello World   ");
  str1.trim();
  assert(str1 == "Hello World");

  u8 str2("   Hello World   ");
  str2.trim_left();
  assert(str2 == "Hello World   ");

  u8 str3("   Hello World   ");
  str3.trim_right();
  assert(str3 == "   Hello World");

  gr::console::writeln("✓ All trimming tests passed");
}

void test_substring_operations() {
  gr::console::writeln("=== Testing Substring Operations ===");

  u8 str("Hello World");

  // 子视图
  auto sub_view = str.sub_view(6, 5);
  assert(sub_view == "World");

  // 前缀检查
  assert(str.starts_with(u8v("Hello")));
  assert(!str.starts_with(u8v("World")));

  // 后缀检查
  assert(str.ends_with(u8v("World")));
  assert(!str.ends_with(u8v("Hello")));

  gr::console::writeln("✓ All substring operation tests passed");
}

void test_splitting() {
  gr::console::writeln("=== Testing Splitting ===");

  u8 str("apple,banana,cherry");
  auto parts = str.split(u8v(","));

  assert(parts.size() == 3);
  assert(parts[0] == "apple");
  assert(parts[1] == "banana");
  assert(parts[2] == "cherry");

  gr::console::writeln("✓ All splitting tests passed");
}

void test_bom_operations() {
  gr::console::writeln("=== Testing BOM Operations ===");

  // 创建带BOM的字符串
  u8 str_with_bom = bom_utils::make_u8_with_bom("Hello World");
  assert(str_with_bom.has_bom());

  // 移除BOM
  str_with_bom.remove_bom();
  assert(!str_with_bom.has_bom());
  assert(str_with_bom == "Hello World");

  // 添加BOM
  str_with_bom.add_bom();
  assert(str_with_bom.has_bom());

  gr::console::writeln("✓ All BOM operation tests passed");
}

void test_alignment() {
  gr::console::writeln("=== Testing Alignment ===");

  u8 str("Hello");

  // 居中对齐
  auto centered = str.center(10);
  assert(centered.size() == 10);

  // 左对齐
  auto left = str.ljust(10);
  assert(left.size() == 10);
  assert(left.starts_with(u8v("Hello")));

  // 右对齐
  auto right = str.rjust(10);
  assert(right.size() == 10);
  assert(right.ends_with(u8v("Hello")));

  gr::console::writeln("✓ All alignment tests passed");
}

void test_conversion_functions() {
  gr::console::writeln("=== Testing Conversion Functions ===");

  u8 utf8_str("Hello 世界");

  // 转换为视图
  auto view = utf8_str.as_view();
  assert(view == utf8_str);

  // 转换为标准字符串
  auto std_str = utf8_str.as_std_string();
  assert(std_str == "Hello 世界");

  gr::console::writeln("✓ All conversion function tests passed");
}

void test_user_defined_literals() {
  gr::console::writeln("=== Testing User-Defined Literals ===");

  // 使用用户定义字面量
  auto str1 = "Hello"_u8;
  assert(str1 == "Hello");

  auto str2 = u8"UTF-8字符串"_u8;
  assert(str2.size() > 0);

  auto view1 = "Hello"_u8v;
  assert(view1 == "Hello");

  gr::console::writeln("✓ All user-defined literal tests passed");
}

void test_replace_operations() {
  gr::console::writeln("=== Testing Replace Operations ===");

  u8 str("Hello World");

  // 原地替换
  str.replace_all_inplace(u8v("World"), u8v("Universe"));
  assert(str == "Hello Universe");

  // 创建副本替换
  auto new_str = str.replace_all(u8v("Hello"), u8v("Hi"));
  assert(new_str == "Hi Universe");
  assert(str == "Hello Universe"); // 原字符串不变

  gr::console::writeln("✓ All replace operation tests passed");
}

void test_validation() {
  gr::console::writeln("=== Testing Validation ===");

  u8 str1("   ");
  assert(str1.is_blank());

  u8 str2("Hello");
  assert(!str2.is_blank());

  // 布尔转换
  u8 empty_str;
  assert(!static_cast<bool>(empty_str));

  u8 non_empty("Text");
  assert(static_cast<bool>(non_empty));

  gr::console::writeln("✓ All validation tests passed");
}

int main() {
  try {
    test_constructors();
    test_assignment_operators();
    test_unicode_iteration();
    test_case_conversion();
    test_trimming();
    test_substring_operations();
    test_splitting();
    test_bom_operations();
    test_alignment();
    test_conversion_functions();
    test_user_defined_literals();
    test_replace_operations();
    test_validation();

    gr::console::writeln("\n🎉 All tests passed successfully!");
    return 0;
  } catch (const std::exception &e) {
    gr::console::errorln("❌ Test failed: {}", e.what());
    return 1;
  }
}
