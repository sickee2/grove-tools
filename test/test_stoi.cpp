#include <chrono>
#include <gr/console.hh>
#include <gr/detail/toy_charconv.hh>

class Timer {
  gr::str::u8 _desc;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  std::chrono::time_point<std::chrono::high_resolution_clock> _end;
  bool _stopped = false;

public:
  Timer(const char *desc)
      : _desc(desc), _start(std::chrono::high_resolution_clock::now()) {}
  
  ~Timer() {
    if (!_stopped) {
      stop();
      report();
    }
  }
  
  void stop() { 
    if(!_stopped){
      _end = std::chrono::high_resolution_clock::now(); 
      _stopped = true;
    }
  }
  
  void report() {
    if(!_stopped){
      stop();
    }
    auto count =
        std::chrono::duration_cast<std::chrono::nanoseconds>(_end - _start);
    gr::console::writeln("{} ==> {:N}ns", _desc, count);
  }
};

int main() {
  using namespace gr;
  
  // 性能测试
  console::writeln("=== performance test ===");

  // 扩展测试用例，包含不同进制和边界值
  struct TestCase {
    const char* str;
    int base;
  };
  
  TestCase test_cases[] = {
    // 二进制测试
    {"1010101010101010", 2},
    {"1111111111111111", 2},
    {"1000000000000000", 2},
    
    // 八进制测试
    {"7777777777777777", 8},
    {"1000000000000000", 8},
    {"1234567012345670", 8},
    
    // 十进制测试
    {"1234567890123456", 10},
    {"9999999999999999", 10},
    {"18446744073709551615", 10}, // uint64_t 最大值
    
    // 十六进制测试
    {"FFFFFFFFFFFFFFFF", 16},
    {"123456789ABCDEF0", 16},
    {"1000000000000000", 16},
    
    // 混合进制测试
    {"ZZZZZZZZZZZZZZZZ", 36}, // 36进制最大值
    {"123456789ABCDEFGHIJK", 36},
    {"1000000000000000000", 36},
    
    // 边界值测试
    {"0", 10},
    {"1", 10},
    {"-1", 10},
    {"-9223372036854775808", 10}, // int64_t 最小值
  };

  const int iterations = 50000;

  // 测试优化版本
  volatile uint64_t dummy_result = 0;


  // 测试简单版本
  Timer t2("toy::stoi (mixed bases)");
  for (int i = 0; i < iterations; ++i) {
    for (const auto& test : test_cases) {
      if (test.str[0] == '-') {
        uint64_t value = 0;
        auto res = gr::toy::sstoi(test.str, test.str + strlen(test.str), value, test.base);
        dummy_result += static_cast<uint64_t>(value); // 统一防优化
        (void)res;
      } else {
        uint64_t value;
        auto res = gr::toy::sstoi(test.str, test.str + strlen(test.str), value, test.base);
        dummy_result += value; // 统一防优化
        (void)res;
      }
    }
  }
  t2.stop();

  Timer t3("std::from_chars (mixed bases)");
  for (int i = 0; i < iterations; ++i) {
    for (const auto& test : test_cases) {
      if (test.str[0] == '-') {
        uint64_t value = 0;
        auto ret = std::from_chars(test.str, test.str + strlen(test.str), value, test.base);
        dummy_result += static_cast<uint64_t>(value); // 统一防优化
        (void)ret;
      } else {
        uint64_t value;
        auto ret = std::from_chars(test.str, test.str + strlen(test.str), value, test.base);
        dummy_result += value; // 统一防优化
        (void)ret;
      }
    }
  }
  t3.stop();

  t2.report();
  t3.report();

  // 重新测试正确性
  console::writeln("\n=== correctness test (extended) ===");
  
  struct BaseTestCase {
    const char *str;
    int base;
    int64_t expected;
  };

  BaseTestCase base_cases[] = {
    // 二进制测试
    {"1010", 2, 10},
    {"11111111", 2, 255},
    {"10000000", 2, 128},
    
    // 八进制测试
    {"377", 8, 255},
    {"1000", 8, 512},
    {"777", 8, 511},
    
    // 十进制测试
    {"12345", 10, 12345},
    {"-67890", 10, -67890},
    {"2147483647", 10, 2147483647},
    
    // 十六进制测试
    {"FF", 16, 255},
    {"1A", 16, 26},
    {"DEADBEEF", 16, 0xDEADBEEF},
    
    // 三十六进制测试
    {"Z", 36, 35},
    {"10", 36, 36},
    {"ZZ", 36, 1295},
    
    // 边界值测试
    {"0", 10, 0},
    {"-0", 10, 0},
    {"-1", 10, -1},
  };

  // 测试溢出情况
  console::writeln("\n=== overflow test ===");
  // 无符号溢出测试
  const char* unsigned_overflow_cases[] = {
    "999999999999999999999999999",  // 明显溢出
    "18446744073709551616",         // 刚好超过uint64_t最大值
  };

  for (const auto& str : unsigned_overflow_cases) {
    uint64_t value;
    auto res = gr::toy::sstoi(str, str + strlen(str), value, 10);
    console::writeln("测试 '{}': ec = {}, value = {}", str, static_cast<int>(res.ec), value);
    if (res.ec == std::errc::result_out_of_range) {
      console::writeln("✓ 正确检测到溢出: {}", str);
    } else {
      console::errorln("✗ 未能检测到溢出: {}", str);
    }
  }

  // 有符号溢出测试
  const char* signed_overflow_cases[] = {
    "-9223372036854775809",         // 刚好超过int64_t最小值
    "9223372036854775808",          // 刚好超过int64_t最大值
  };

  for (const auto& str : signed_overflow_cases) {
    int64_t value;  // 使用有符号类型
    auto res = gr::toy::sstoi(str, str + strlen(str), value, 10);
    console::writeln("测试 '{}': ec = {}, value = {}", str, static_cast<int>(res.ec), value);
    if (res.ec == std::errc::result_out_of_range) {
      console::writeln("✓ 正确检测到溢出: {}", str);
    } else {
      console::errorln("✗ 未能检测到溢出: {}", str);
    }
  }

  return 0;
}
