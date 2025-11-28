// filepath: src/main.cpp
#include <charconv>
#include <gr/detail/toy_charconv.hh>
#include <gr/console.hh>

using namespace gr;
namespace term = gr::console;

void test() {
  // 整数测试用例
  int64_t int_test_values[] = {0,    1,          -1,         123,
                               -456, 1234567890, -987654321, 255,
                               -128, 65535,      2147483647, -2147483648};

  // 浮点数测试用例
  double float_test_values[] = {0.0,
                                -0.0,
                                1.0,
                                -1.0,
                                3.141592653589793,
                                -2.718281828459045,
                                123.456789,
                                -987.654321,
                                1.23456789e-10,
                                -9.87654321e+10,
                                1.0e-100,
                                -1.0e+100,
                                std::numeric_limits<double>::infinity(),
                                -std::numeric_limits<double>::infinity(),
                                std::numeric_limits<double>::quiet_NaN(),
                                std::numeric_limits<double>::denorm_min()};

  char buffer[64];

  // === 整数测试 ===
  term::writeln("=== 整数测试 ===");

  term::writeln("十进制:");
  for (auto i : int_test_values) {
    auto result = gr::toy::itoss(buffer, 64, i, 10);
    if (result.ptr) {
      term::writeln("  {} -> {}", i, str::u8v(result.ptr, result.size));
    }
  }

  term::writeln("\n十六进制（带前缀）:");
  for (auto i : int_test_values) {
    auto result = gr::toy::itoss(buffer, 64, i, 16, false, true);
    if (result.ptr) {
      term::writeln("  {} -> {}", i, str::u8v(result.ptr, result.size));
    }
  }

  term::writeln("\n十六进制（大写，带前缀）:");
  for (auto i : int_test_values) {
    auto result = gr::toy::itoss(buffer, 64, i, 16, true, true);
    if (result.ptr) {
      term::writeln("  {} -> {}", i, str::u8v(result.ptr, result.size));
    }
  }

  term::writeln("\n二进制（带前缀）:");
  for (auto i : int_test_values) {
    if (i >= 0 && i < 256) { // 限制范围避免输出太长
      auto result = gr::toy::itoss(buffer, 64, i, 2, false, true);
      if (result.ptr) {
        term::writeln("  {} -> {}", i, str::u8v(result.ptr, result.size));
      }
    }
  }

  term::writeln("\n八进制（带前缀）:");
  for (auto i : int_test_values) {
    auto result = gr::toy::itoss(buffer, 64, i, 8, false, true);
    if (result.ptr) {
      term::writeln("  {} -> {}", i, str::u8v(result.ptr, result.size));
    }
  }

  // 测试其他进制
  term::writeln("\n=== 其他进制测试 ===");
  for (auto i : {10, 15, 20, 25, 30, 35}) {
    auto result = gr::toy::itoss(buffer, 64, 123456789, i);
    if (result.ptr) {
      term::writeln("  123456789 (base {}) -> {}", i,
                 str::u8v(result.ptr, result.size));
    }
  }

  // === 浮点数测试 ===
  term::writeln("\n=== 浮点数测试 ===");

  term::writeln("固定小数点格式:");
  for (auto d : float_test_values) {
    auto result = gr::toy::ftoss(buffer, 64, d, toy::chars_format::fixed, 6);
    if (result.ptr) {
      term::writeln("  {:20} -> {}", d, str::u8v(result.ptr, result.size));
    } else {
      term::writeln("  {:20} -> FAILED", d);
    }
  }

  term::writeln("\n科学计数法格式:");
  for (auto d : float_test_values) {
    auto result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::scientific, 6);
    if (result.ptr) {
      term::writeln("  {:20} -> {}", d, str::u8v(result.ptr, result.size));
    } else {
      term::writeln("  {:20} -> FAILED", d);
    }
  }

  term::writeln("\n通用格式:");
  for (auto d : float_test_values) {
    auto result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::general, 6);
    if (result.ptr) {
      term::writeln("  {:20} -> {}", d, str::u8v(result.ptr, result.size));
    } else {
      term::writeln("  {:20} -> FAILED", d);
    }
  }

  // 测试不同精度
  term::writeln("\n=== 浮点数精度测试 ===");
  double test_value = 3.141592653589793;
  for (auto precision : {0, 2, 4, 6, 10, 15}) {
    auto result = gr::toy::ftoss(buffer, 64, test_value,
                                    toy::chars_format::fixed, precision);
    if (result.ptr) {
      term::writeln("  π (precision {}) -> {}", precision,
                 str::u8v(result.ptr, result.size));
    }
  }

  // 边界值测试
  term::writeln("\n=== 边界值测试 ===");
  double boundary_values[] = {std::numeric_limits<double>::min(),
                              std::numeric_limits<double>::max(),
                              std::numeric_limits<double>::epsilon(),
                              std::numeric_limits<double>::lowest()};

  for (auto d : boundary_values) {
    auto result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::scientific, 6);
    if (result.ptr) {
      term::writeln("  {:20} -> {}", d, str::u8v(result.ptr, result.size));
    }
  }

  // === 性能对比测试 ===
  term::writeln("\n=== 性能对比测试 ===");

  // 简单性能测试：转换1000次并计时
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; ++i) {
    auto result =
        gr::toy::ftoss(buffer, 64, i + 56.789, toy::chars_format::fixed, 6);
    (void)result;
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto toy_duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; ++i) {
    char std_buffer[64];
    std::to_chars(std_buffer, std_buffer + 64, i + 456.789,
                  std::chars_format::fixed, 6);
  }
  end = std::chrono::high_resolution_clock::now();
  auto std_duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  term::writeln("  1000次浮点数转换耗时:");
  term::writeln("  gr::toy::to_chars: {} μs", toy_duration.count());
  term::writeln("  std::to_chars: {} μs", std_duration.count());
  term::writeln("  性能比: {:.2f}x",
             static_cast<double>(std_duration.count()) / toy_duration.count());

  // === 浮点数验证测试 ===
  term::writeln("\n=== 浮点数验证测试 ===");

  struct FloatTest {
    double value;
    const char *expected_fixed;
    const char *expected_scientific;
  };

  FloatTest verification_tests[] = {
      {0.0, "0.000000", "0.000000e+00"},
      {-0.0, "-0.000000", "-0.000000e+00"},
      {1.0, "1.000000", "1.000000e+00"},
      {-1.0, "-1.000000", "-1.000000e+00"},
      {3.14159, "3.141590", "3.141590e+00"},
      {-2.71828, "-2.718280", "-2.718280e+00"},
      {123.456, "123.456000", "1.234560e+02"},
      {-987.654, "-987.654000", "-9.876540e+02"},
      {1.23e-5, "0.000012", "1.230000e-05"},
      {-4.56e+8, "-456000000.000000", "-4.560000e+08"}};

  for (const auto &test : verification_tests) {
    // 测试固定小数点
    auto fixed_result =
        gr::toy::ftoss(buffer, 64, test.value, toy::chars_format::fixed, 6);
    std::string fixed_actual =
        fixed_result.ptr
            ? std::string(fixed_result.ptr, fixed_result.size)
            : "FAILED";

    // 测试科学计数法
    auto sci_result = gr::toy::ftoss(buffer, 64, test.value,
                                        toy::chars_format::scientific, 6);
    std::string sci_actual =
        sci_result.ptr ? std::string(sci_result.ptr, sci_result.size)
                         : "FAILED";

    term::writeln("  {:.6f} -> fixed: '{}' {}, scientific: '{}' {}", test.value,
               fixed_actual, fixed_actual == test.expected_fixed ? "✅" : "❌",
               sci_actual,
               sci_actual == test.expected_scientific ? "✅" : "❌");
  }
  // === 特殊值测试 ===
  term::writeln("\n=== 特殊值测试 ===");

  double special_values[] = {std::numeric_limits<double>::infinity(),
                             -std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::quiet_NaN()};

  for (auto d : special_values) {
    auto fixed_result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::fixed, 6);
    auto sci_result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::scientific, 6);
    auto gen_result =
        gr::toy::ftoss(buffer, 64, d, toy::chars_format::general, 6);

    term::writeln("  Special value:");
    if (fixed_result.ptr) {
      term::writeln("    fixed: '{}'",
                 str::u8v(fixed_result.ptr, fixed_result.size));
    }
    if (sci_result.ptr) {
      term::writeln("    scientific: '{}'",
                 str::u8v(sci_result.ptr, sci_result.size));
    }
    if (gen_result.ptr) {
      term::writeln("    general: '{}'",
                 str::u8v(gen_result.ptr, gen_result.size));
    }
  }
}

int main() {
  test();
  return 0;
}
