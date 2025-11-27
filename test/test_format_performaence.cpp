#include <chrono>
#include <format>
#include <gr/console.hh>
#include <gr/format.hh>
#include <string>
#include <vector>
#include <gr/performance_timer.hh>

void test_basic_formatting_performance() {
  gr::console::write("=== Basic Formatting Performance Test ===\n");

  const int iterations = 100000;

  // Integer formatting
  {
    PerformanceTimer timer("Integer formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Integer: {}", i);
      (void)result; // Prevent optimization
    }
  }

  {
    PerformanceTimer timer("Integer formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Integer: {}", i);
      (void)result;
    }
  }

  // Floating-point formatting
  {
    PerformanceTimer timer("Floating-point formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Float: {:.2f}", 3.14159 + i * 0.001);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Floating-point formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Float: {:.2f}", 3.14159 + i * 0.001);
      (void)result;
    }
  }

  // String formatting
  {
    PerformanceTimer timer("String formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("String: {}", "Hello World");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("String formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("String: {}", "Hello World");
      (void)result;
    }
  }
}

void test_complex_formatting_performance() {
  gr::console::write("\n=== Complex Formatting Performance Test ===\n");

  const int iterations = 50000;

  // Multi-argument formatting
  {
    PerformanceTimer timer("Multi-argument formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Arguments: {}, {}, {:.2f}, {}", i, i * 2,
                                3.14159, "test");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Multi-argument formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Arguments: {}, {}, {:.2f}, {}", i, i * 2,
                                    3.14159, "test");
      (void)result;
    }
  }

  // Indexed formatting
  {
    PerformanceTimer timer("Indexed formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Indices: {2} {1} {0}", "zero", "one", "two");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Indexed formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format("Indices: {2} {1} {0}", "zero", "one", "two");
      (void)result;
    }
  }

  // Complex format specifiers
  std::string result_k1;
  gr::str::u8 result_k2;
  {
    PerformanceTimer timer("Complex format specifiers (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result_k1 =
          std::format("Complex: {:>10} {:<8.2f} {:+}", i, 3.14159, i);
    }
  }

  {
    PerformanceTimer timer("Complex format specifiers (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result_k2 =
          gr::toy::format("Complex: {:>10} {:<8.2f} {:+}", i, 3.14159, i);
    }
  }
  if (result_k2 != result_k2) {
    gr::console::writeln("Error: std::format and toy::format results don't match");
  }
}

void test_memory_usage() {
  gr::console::write("\n=== Memory Usage Test ===\n");

  const int iterations = 10000;
  std::vector<std::string> std_results;
  std::vector<gr::str::u8> toy_results;

  std_results.reserve(iterations);
  toy_results.reserve(iterations);

  // Test memory allocation patterns
  {
    PerformanceTimer timer("std::format memory allocation");
    for (int i = 0; i < iterations; ++i) {
      std_results.push_back(std::format("Test {}: {:.3f}", i, i * 1.234));
    }
  }

  {
    PerformanceTimer timer("toy::format memory allocation");
    for (int i = 0; i < iterations; ++i) {
      toy_results.push_back(gr::toy::format("Test {}: {:.3f}", i, i * 1.234));
    }
  }

  // Calculate total memory usage
  size_t std_total_bytes = 0;
  for (const auto &s : std_results) {
    std_total_bytes += s.capacity();
  }

  size_t toy_total_bytes = 0;
  for (const auto &s : toy_results) {
    toy_total_bytes += s.capacity();
  }

  gr::console::write("std::format total memory usage: {} bytes\n",
                   std_total_bytes);
  gr::console::write("toy::format total memory usage: {} bytes\n",
                   toy_total_bytes);
}

void test_special_types_performance() {
  gr::console::write("\n=== Special Types Performance Test ===\n");

  const int iterations = 30000;

  // Pointer formatting
  int value = 42;
  {
    PerformanceTimer timer("Pointer formatting (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Pointer: {}", static_cast<void *>(&value));
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Pointer formatting (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Pointer: {}", static_cast<void *>(&value));
      (void)result;
    }
  }

  // Time formatting (toy::format only)
  {
    PerformanceTimer timer("Time formatting (toy::format)");
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Time: {:f}", now);
      (void)result;
    }
  }

  // Duration formatting (toy::format only)
  {
    PerformanceTimer timer("Duration formatting (toy::format)");
    auto duration = std::chrono::milliseconds(1234);
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Duration: {:a}", duration);
      (void)result;
    }
  }
}

void test_unicode_performance() {
  gr::console::write("\n=== String Performance Test ===\n");

  const int iterations = 40000;

  // UTF-8 string
  {
    PerformanceTimer timer("UTF-8 string (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Unicode: {}", "Hello World");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("UTF-8 string (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Unicode: {}", "Hello World");
      (void)result;
    }
  }

  // gr::str::u8 type
  {
    PerformanceTimer timer("gr::str::u8 type (toy::format)");
    gr::str::u8 u8_str = "UTF-8 test string 文本测试";
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("u8: {}", u8_str);
      (void)result;
    }
  }
}

void test_throughput_comparison() {
  gr::console::write("\n=== Throughput Comparison Test ===\n");

  const int warmup_iterations = 1000;
  const int test_iterations = 100000;

  // Warm-up
  for (int i = 0; i < warmup_iterations; ++i) {
    auto std_result = std::format("Test {}", i);
    auto toy_result = gr::toy::format("Test {}", i);
    (void)std_result;
    (void)toy_result;
  }

  // std::format throughput test
  long long std_total_time = 0;
  {
    PerformanceTimer timer("std::format throughput");
    for (int i = 0; i < test_iterations; ++i) {
      auto result = std::format("Value: {} {} {:.2f}", i, i * 2, i * 0.1);
      (void)result;
    }
    std_total_time = timer.elapsed_microseconds();
  }

  // toy::format throughput test
  long long toy_total_time = 0;
  {
    PerformanceTimer timer("toy::format throughput");
    for (int i = 0; i < test_iterations; ++i) {
      auto result = gr::toy::format("Value: {} {} {:.2f}", i, i * 2, i * 0.1);
      (void)result;
    }
    toy_total_time = timer.elapsed_microseconds();
  }

  // Calculate throughput
  double std_throughput = (double)test_iterations / std_total_time * 1000000.0;
  double toy_throughput = (double)test_iterations / toy_total_time * 1000000.0;

  gr::console::write("std::format throughput: {:.2f} ops/sec\n", std_throughput);
  gr::console::write("toy::format throughput: {:.2f} ops/sec\n", toy_throughput);
  gr::console::write("Performance ratio: {:.2f}%\n",
                   (toy_throughput / std_throughput * 100.0));
}

void test_result_consistency() {
  gr::console::write("\n=== Result Consistency Test ===\n");

  // Test various format patterns
  auto std_result1 = std::format("Value: {} {:.2f}", 42, 3.14159);
  auto toy_result1 = gr::toy::format("Value: {} {:.2f}", 42, 3.14159);

  if (std_result1 != toy_result1) {
    gr::console::write("Mismatch in basic formatting\n");
    gr::console::write("std::format: {}\n", std_result1);
    gr::console::write("toy::format: {}\n", toy_result1);
  }
  auto std_result2 = std::format("Value: {} {:e}", 42, 31.4159);
  auto toy_result2 = gr::toy::format("Value: {} {:e}", 42, 31.4159);

  if (std_result2 != toy_result2) {
    gr::console::write("Mismatch in basic formatting\n");
    gr::console::write("std::format: {}\n", std_result2);
    gr::console::write("toy::format: {}\n", toy_result2);
  }
  auto std_result3 = std::format("Value: {} {:.15f}", 42, 31.4159);
  auto toy_result3 = gr::toy::format("Value: {} {:.15f}", 42, 31.4159);

  if (std_result3 != toy_result3) {
    gr::console::write("Mismatch in basic formatting\n");
    gr::console::write("std::format: {}\n", std_result3);
    gr::console::write("toy::format: {}\n", toy_result3);
  }

  gr::console::write("test result all passed\n");
  // Add more consistency checks...
}
void test_edge_cases_performance() {
  gr::console::write("\n=== Edge Cases Performance Test ===\n");

  const int iterations = 50000;

  // Empty string formatting
  {
    PerformanceTimer timer("Empty string (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{}", "");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Empty string (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("{}", "");
      (void)result;
    }
  }

  // Very long string formatting
  std::string long_str(1000, 'x');
  {
    PerformanceTimer timer("Long string (std::format)");
    for (int i = 0; i < iterations / 10; ++i) {
      auto result = std::format("Long: {}", long_str);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Long string (toy::format)");
    for (int i = 0; i < iterations / 10; ++i) {
      auto result = gr::toy::format("Long: {}", long_str);
      (void)result;
    }
  }
}
void test_mixed_types_performance() {
  gr::console::write("\n=== Mixed Types Performance Test ===\n");

  const int iterations = 30000;

  // Mixed integer and string
  {
    PerformanceTimer timer("Mixed int+str (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("ID: {}, Name: {}", i, "TestUser");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Mixed int+str (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("ID: {}, Name: {}", i, "TestUser");
      (void)result;
    }
  }

  // Complex mixed types
  {
    PerformanceTimer timer("Complex mixed (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Data: {:.2f} {} {:#x} {}", 3.14159 + i, "text",
                                i, 42.5f);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Complex mixed (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Data: {:.2f} {} {:#x} {}", 3.14159 + i,
                                    "text", i, 42.5f);
      (void)result;
    }
  }
}

void test_format_specifier_performance() {
  gr::console::write("\n=== Format Specifier Performance Test ===\n");

  const int iterations = 40000;

  // Width and alignment
  {
    PerformanceTimer timer("Width/align (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{:<10} {:>8} {:^6}", i, i * 2, "text");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Width/align (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("{:<10} {:>8} {:^6}", i, i * 2, "text");
      (void)result;
    }
  }

  // Numeric formats
  {
    PerformanceTimer timer("Numeric formats (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{:#x} {:#o} {:.3e}", i, i, 3.14159 + i);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Numeric formats (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("{:#x} {:#o} {:.3e}", i, i, 3.14159 + i);
      (void)result;
    }
  }
}

void test_real_world_scenarios() {
  gr::console::write("\n=== Real-world Scenarios Test ===\n");

  const int iterations = 20000;

  // out message formatting (common use case)
  {
    PerformanceTimer timer("out message (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          std::format("[{}] {}: {} (value: {:.3f})", "INFO", "process_data",
                      "Operation completed", 3.14159 + i);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("out message (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format("[{}] {}: {} (value: {:.3f})", "INFO", "process_data",
                          "Operation completed", 3.14159 + i);
      (void)result;
    }
  }

  // Data serialization format
  {
    PerformanceTimer timer("Data serialization (std::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          std::format(R"({{"id": {}, "name": "user{}", "score": {:.2f}}})", i,
                      i, 95.5 + i * 0.1);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("Data serialization (toy::format)");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format(R"({{"id": {}, "name": "user{}", "score": {:.2f}}})",
                          i, i, 95.5 + i * 0.1);
      (void)result;
    }
  }
}
int main() {

  gr::console::write(
      "Starting std::format vs toy::format efficiency comparison test\n\n");

  test_basic_formatting_performance();
  test_complex_formatting_performance();
  test_memory_usage();
  test_special_types_performance();
  test_unicode_performance();
  test_throughput_comparison();
  test_result_consistency();
  test_edge_cases_performance();
  test_mixed_types_performance();
  test_format_specifier_performance();
  test_real_world_scenarios();
  gr::console::write("\nTest completed\n");

  // using namespace gr::literals;
  // std::cout << "{:.5}"_fmt("0123456789") << std::endl;;
  return 0;
}
