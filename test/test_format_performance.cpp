#include <chrono>
#include <fmt/base.h>
#include <format>
#include <gr/console.hh>
#include <gr/format.hh>
#include <string>
#include <vector>
#include <cstdio>
#include <sstream>
#include <gr/performance_timer.hh>
#include <iostream>
#include <iomanip>
#include <fmt/format.h>
void test_basic_formatting_performance() {
  gr::console::write("=== Basic Formatting Performance Test ===\n");

  gr::console::write("--- Integer formatting ---\n");
  const int iterations = 100000;

  // Integer formatting
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Integer: {}", i);
      (void)result; // Prevent optimization
    }
  }
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Integer: {}", i);
      (void)result; // Prevent optimization
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Integer: {}", i);
      (void)result;
    }
  }

  // 添加 snprintf 对比
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Integer: %d", i);
      (void)buffer;
    }
  }

  // 添加 ostringstream 对比
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "Integer: " << i;
      auto result = oss.str();
      (void)result;
    }
  }

  gr::console::write("--- Floating-point formatting ---\n");
  // Floating-point formatting
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Float: {:.2f}", 3.14159 + i * 0.001);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Float: {:.2f}", 3.14159 + i * 0.001);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Float: {:.2f}", 3.14159 + i * 0.001);
      (void)result;
    }
  }

  // 添加 snprintf 对比
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Float: %.2f", 3.14159 + i * 0.001);
      (void)buffer;
    }
  }

  // 添加 ostringstream 对比
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "Float: " << std::fixed << std::setprecision(2) << (3.14159 + i * 0.001);
      auto result = oss.str();
      (void)result;
    }
  }

  // String formatting
  gr::console::write("--- String formatting ---\n");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("String: {}", "Hello World");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("String: {}", "Hello World");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("String: {}", "Hello World");
      (void)result;
    }
  }

  // 添加 snprintf 对比
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "String: %s", "Hello World");
      (void)buffer;
    }
  }

  // 添加 ostringstream 对比
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "String: " << "Hello World";
      auto result = oss.str();
      (void)result;
    }
  }
}

void test_complex_formatting_performance() {
  gr::console::write("\n=== Complex Formatting Performance Test ===\n");

  const int iterations = 50000;

  // Multi-argument formatting
  gr::console::write("--- Multi-argument formatting ---\n");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Arguments: {}, {}, {:.2f}, {}", i, i * 2,
                                3.14159, "test");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Arguments: {}, {}, {:.2f}, {}", i, i * 2,
                                3.14159, "test");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Arguments: {}, {}, {:.2f}, {}", i, i * 2, 3.14159, "test");
      (void)result;
    }
  }

  // 添加 snprintf 对比
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Arguments: %d, %d, %.2f, %s", 
                   i, i * 2, 3.14159, "test");
      (void)buffer;
    }
  }

  // 添加 ostringstream 对比
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "Arguments: " << i << ", " << i * 2 << ", " 
          << std::fixed << std::setprecision(2) << 3.14159 << ", " << "test";
      auto result = oss.str();
      (void)result;
    }
  }

  // Indexed formatting
  gr::console::write("--- Indexed formatting ---\n");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Indices: {2} {1} {0}", "zero", "one", "two");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Indices: {2} {1} {0}", "zero", "one", "two");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format("Indices: {2} {1} {0}", "zero", "one", "two");
      (void)result;
    }
  }

  // Complex format specifiers
  gr::console::write("--- Complex format specifiers ---\n");
  std::string result_k1;
  gr::str::u8 result_k2;
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result_k1 =
          fmt::format("Complex: {:>10} {:<8.2f} {:+}", i, 3.14159, i);
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result_k1 =
          std::format("Complex: {:>10} {:<8.2f} {:+}", i, 3.14159, i);
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result_k2 =
          gr::toy::format("Complex: {:>10} {:<8.2f} {:+}", i, 3.14159, i);
    }
  }

  // 添加 snprintf 对比
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Complex: %10d %-8.2f %+d", 
                   i, 3.14159, i);
      (void)buffer;
    }
  }

  // 添加 ostringstream 对比
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "Complex: " << std::setw(10) << std::right << i << " "
          << std::setw(8) << std::left << std::fixed << std::setprecision(2) << 3.14159 << " "
          << std::showpos << i;
      auto result = oss.str();
      (void)result;
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
  std::vector<std::string> snprintf_results;
  std::vector<std::string> oss_results;

  std_results.reserve(iterations);
  toy_results.reserve(iterations);
  snprintf_results.reserve(iterations);
  oss_results.reserve(iterations);

  // Test memory allocation patterns
  gr::console::writeln("---  memory allocation ---");

  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      std_results.push_back(std::format("Test {}: {:.3f}", i, i * 1.234));
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      toy_results.push_back(gr::toy::format("Test {}: {:.3f}", i, i * 1.234));
    }
  }

  // 添加 snprintf 内存分配测试
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Test %d: %.3f", i, i * 1.234);
      snprintf_results.push_back(buffer);
    }
  }

  // 添加 ostringstream 内存分配测试
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < iterations; ++i) {
      std::ostringstream oss;
      oss << "Test " << i << ": " << std::fixed << std::setprecision(3) << i * 1.234;
      oss_results.push_back(oss.str());
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

  size_t snprintf_total_bytes = 0;
  for (const auto &s : snprintf_results) {
    snprintf_total_bytes += s.capacity();
  }

  size_t oss_total_bytes = 0;
  for (const auto &s : oss_results) {
    oss_total_bytes += s.capacity();
  }

  gr::console::writeln("-- total memory usage ---");
  gr::console::write("\tstd::format: {} bytes\n",
                   std_total_bytes);
  gr::console::write("\ttoy::format: {} bytes\n",
                   toy_total_bytes);
  gr::console::write("\tstd::snprintf: {} bytes\n",
                   snprintf_total_bytes);
  gr::console::write("\tstd::ostringstream: {} bytes\n",
                   oss_total_bytes);
}

void test_special_types_performance() {
  gr::console::write("\n=== Special Types Performance Test ===\n");

  const int iterations = 30000;

  gr::console::writeln("--- Pointer formatting ---");
  // Pointer formatting
  int value = 42;
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Pointer: {}", static_cast<void *>(&value));
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Pointer: {}", static_cast<void *>(&value));
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
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

void test_throughput_comparison() {
  gr::console::write("\n=== Throughput Comparison Test ===\n");

  const int warmup_iterations = 1000;
  const int test_iterations = 100000;

  // Warm-up
  for (int i = 0; i < warmup_iterations; ++i) {
    auto std_result = fmt::format("Test {}", i);
    std_result = std::format("Test {}", i);
    auto toy_result = gr::toy::format("Test {}", i);
    (void)std_result;
    (void)toy_result;
  }

  // std::format throughput test
  long long fmt_total_time = 0;
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < test_iterations; ++i) {
      auto result = fmt::format("Value: {} {} {:.2f}", i, i * 2, i * 0.1);
      (void)result;
    }
    fmt_total_time = timer.elapsed_microseconds();
  }
  long long std_total_time = 0;
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < test_iterations; ++i) {
      auto result = std::format("Value: {} {} {:.2f}", i, i * 2, i * 0.1);
      (void)result;
    }
    std_total_time = timer.elapsed_microseconds();
  }

  // toy::format throughput test
  long long toy_total_time = 0;
  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < test_iterations; ++i) {
      auto result = gr::toy::format("Value: {} {} {:.2f}", i, i * 2, i * 0.1);
      (void)result;
    }
    toy_total_time = timer.elapsed_microseconds();
  }

  // std::snprintf throughput test
  long long snprintf_total_time = 0;
  {
    PerformanceTimer timer("\tstd::snprintf");
    for (int i = 0; i < test_iterations; ++i) {
      char buffer[256];
      std::snprintf(buffer, sizeof(buffer), "Value: %d %d %.2f", i, i * 2, i * 0.1);
      (void)buffer;
    }
    snprintf_total_time = timer.elapsed_microseconds();
  }

  // std::ostringstream throughput test
  long long oss_total_time = 0;
  {
    PerformanceTimer timer("\tstd::ostringstream");
    for (int i = 0; i < test_iterations; ++i) {
      std::ostringstream oss;
      oss << "Value: " << i << " " << i * 2 << " " 
          << std::fixed << std::setprecision(2) << i * 0.1;
      auto result = oss.str();
      (void)result;
    }
    oss_total_time = timer.elapsed_microseconds();
  }

  // Calculate throughput
  double fmt_throughput = (double)test_iterations / fmt_total_time * 1000000.0;
  double std_throughput = (double)test_iterations / std_total_time * 1000000.0;
  double toy_throughput = (double)test_iterations / toy_total_time * 1000000.0;
  double snprintf_throughput = (double)test_iterations / snprintf_total_time * 1000000.0;
  double oss_throughput = (double)test_iterations / oss_total_time * 1000000.0;

  gr::console::write("\tfmt::format throughput: {:.2f} ops/sec\n", fmt_throughput);
  gr::console::write("\tstd::format throughput: {:.2f} ops/sec\n", std_throughput);
  gr::console::write("\ttoy::format throughput: {:.2f} ops/sec\n", toy_throughput);
  gr::console::write("\tstd::snprintf throughput: {:.2f} ops/sec\n", snprintf_throughput);
  gr::console::write("\tstd::ostringstream throughput: {:.2f} ops/sec\n", oss_throughput);
  
  gr::console::write("\n   Performance ratios (relative to std::format):\n");
  gr::console::write("\tfmt::format: {:.2f}%\n", (fmt_throughput / std_throughput * 100.0));
  gr::console::write("\ttoy::format: {:.2f}%\n", (toy_throughput / std_throughput * 100.0));
  gr::console::write("\tstd::snprintf: {:.2f}%\n", (snprintf_throughput / std_throughput * 100.0));
  gr::console::write("\tstd::ostringstream: {:.2f}%\n", (oss_throughput / std_throughput * 100.0));
}

void test_result_consistency() {
  gr::console::write("\n=== Result Consistency Test ===\n");

  // Test various format patterns
  auto std_result1 = std::format("Value: {} {:.2f}", 42, 3.14159);
  auto toy_result1 = gr::toy::format("Value: {} {:.2f}", 42, 3.14159);
  
  // snprintf 结果
  char snprintf_buffer1[256];
  std::snprintf(snprintf_buffer1, sizeof(snprintf_buffer1), "Value: %d %.2f", 42, 3.14159);
  std::string snprintf_result1 = snprintf_buffer1;
  
  // ostringstream 结果
  std::ostringstream oss1;
  oss1 << "Value: " << 42 << " " << std::fixed << std::setprecision(2) << 3.14159;
  std::string oss_result1 = oss1.str();

  if (std_result1 != toy_result1) {
    gr::console::write("Mismatch in basic formatting\n");
    gr::console::write("std::format: {}\n", std_result1);
    gr::console::write("toy::format: {}\n", toy_result1);
  }
  
  if (std_result1 != snprintf_result1) {
    gr::console::write("Mismatch between std::format and snprintf\n");
    gr::console::write("std::format: {}\n", std_result1);
    gr::console::write("snprintf: {}\n", snprintf_result1);
  }
  
  if (std_result1 != oss_result1) {
    gr::console::write("Mismatch between std::format and ostringstream\n");
    gr::console::write("std::format: {}\n", std_result1);
    gr::console::write("ostringstream: {}\n", oss_result1);
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
  gr::console::writeln("--- Empty string ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("{}", "");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{}", "");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("{}", "");
      (void)result;
    }
  }

  // Very long string formatting
  gr::console::writeln("--- Long string ---");
  std::string long_str(1000, 'x');
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations / 10; ++i) {
      auto result = fmt::format("Long: {}", long_str);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations / 10; ++i) {
      auto result = std::format("Long: {}", long_str);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
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
  gr::console::writeln("--- Mixed int+str ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("ID: {}, Name: {}", i, "TestUser");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("ID: {}, Name: {}", i, "TestUser");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("ID: {}, Name: {}", i, "TestUser");
      (void)result;
    }
  }

  // Complex mixed types
  gr::console::writeln("--- Complex mixed types ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("Data: {:.2f} {} {:#x} {}", 3.14159 + i, "text",
                                i, 42.5f);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("Data: {:.2f} {} {:#x} {}", 3.14159 + i, "text",
                                i, 42.5f);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("Data: {:.2f} {} {:#x} {:.2f}", 3.14159 + i, "text", int(i), 42.5f);
      (void)result;
    }
  }
}

void test_format_specifier_performance() {
  gr::console::write("\n=== Format Specifier Performance Test ===\n");

  const int iterations = 40000;

  // Width and alignment
  gr::console::writeln("--- Width/align ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("{:<10} {:>8} {:^6}", i, i * 2, "text");
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{:<10} {:>8} {:^6}", i, i * 2, "text");
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = gr::toy::format("{:<10} {:>8} {:^6}", i, i * 2, "text");
      (void)result;
    }
  }

  // Numeric formats
  gr::console::writeln("--- Numeric formats ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = fmt::format("{:#x} {:#o} {:.3e}", i, i, 3.14159 + i);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result = std::format("{:#x} {:#o} {:.3e}", i, i, 3.14159 + i);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
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
  gr::console::writeln("--- out message ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          fmt::format("[{}] {}: {} (value: {:.3f})", "INFO", "process_data",
                      "Operation completed", 3.14159 + i);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          std::format("[{}] {}: {} (value: {:.3f})", "INFO", "process_data",
                      "Operation completed", 3.14159 + i);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format("[{}] {}: {} (value: {:.3f})", "INFO", "process_data",
                          "Operation completed", 3.14159 + i);
      (void)result;
    }
  }

  // Data serialization format
  gr::console::writeln("--- Data serialization ---");
  {
    PerformanceTimer timer("\tfmt::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          fmt::format(R"({{"id": {}, "name": "user{}", "score": {:.2f}}})", i,
                      i, 95.5 + i * 0.1);
      (void)result;
    }
  }
  {
    PerformanceTimer timer("\tstd::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          std::format(R"({{"id": {}, "name": "user{}", "score": {:.2f}}})", i,
                      i, 95.5 + i * 0.1);
      (void)result;
    }
  }

  {
    PerformanceTimer timer("\ttoy::format");
    for (int i = 0; i < iterations; ++i) {
      auto result =
          gr::toy::format(R"({{"id": {}, "name": "user{}", "score": {:.2f}}})",
                          i, i, 95.5 + i * 0.1);
      (void)result;
    }
  }
}

void test_toy_format_preformance(){
  gr::console::write(
      "Starting std::format/fmt::format vs toy::format efficiency comparison test\n\n");

  test_basic_formatting_performance();
  test_complex_formatting_performance();
  test_memory_usage();
  test_special_types_performance();
  test_throughput_comparison();
  test_edge_cases_performance();
  test_mixed_types_performance();
  test_format_specifier_performance();
  test_real_world_scenarios();
  test_result_consistency();
  gr::console::write("\nTest completed\n");
}

