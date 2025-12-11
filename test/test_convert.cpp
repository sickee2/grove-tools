#include "gr/string.hh"
#include "gr/utf_sequence.hh"
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>

using namespace gr;
using namespace gr::str;

// Original conversion function using uc::iter
inline u8
to_utf8_original(u16v utf16,
                 gr::endian endian_ = gr::endian::native) {
  u8 result;
  result.reserve(utf16.size() * 3);
  // auto func = [&result](uc::codepoint& cp, uc::sequence_status status){
  //   if(status == uc::sequence_status::valid){
  //     auto chunk = cp.chunk_u8();
  //     result.append(chunk.buf, chunk.size());
  //   }
  //   return true;
  // };
  //
  // gr::str::batch_process_utf<char16_t>(utf16, func, endian_ );
  for (auto it = uc::make_iterator(utf16, 0, uc::on_failed::skip, endian_); it;
       ++it) {
    result.append(it->chunk_u8().buf);
  }
  return result;
}

// Optimized conversion function using uc::sequence
inline u8
to_utf8_optimized(u16v utf16,
                  gr::endian endian = gr::endian::native) {
  return gr::str::to_utf8(utf16, uc::on_failed::skip, endian);
}


// Generate test data
std::vector<char16_t> generate_test_data(size_t count) {
  std::vector<char16_t> data;
  data.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());

  // Generate various characters including ASCII, BMP characters, and surrogate pairs
  std::uniform_int_distribution<uint16_t> dist_ascii(0x0020, 0x007F);
  std::uniform_int_distribution<uint16_t> dist_bmp(0x0080, 0xD7FF);
  std::uniform_int_distribution<uint16_t> dist_high_surrogate(0xD800, 0xDBFF);
  std::uniform_int_distribution<uint16_t> dist_low_surrogate(0xDC00, 0xDFFF);

  for (size_t i = 0; i < count; ++i) {
    int type = i % 10;
    if (type < 5) {
      // 50% ASCII
      data.push_back(dist_ascii(gen));
    } else if (type < 8) {
      // 30% BMP characters
      data.push_back(dist_bmp(gen));
    } else {
      // 20% surrogate pairs (requires two characters)
      if (i + 1 < count) {
        data.push_back(dist_high_surrogate(gen));
        data.push_back(dist_low_surrogate(gen));
        ++i;
      } else {
        data.push_back(dist_ascii(gen));
      }
    }
  }

  return data;
}

// Performance test function
template <typename Func>
void utf_benchmark(const std::string &name, Func &&func,
               const std::vector<char16_t> &test_data, int iterations = 1000) {
  auto start = std::chrono::high_resolution_clock::now();

  size_t total_size = 0;
  for (int i = 0; i < iterations; ++i) {
    const char16_t *data_ptr = test_data.data();
    u16v view(data_ptr, test_data.size());
    auto result = func(view, gr::endian::native); // Add second parameter
    total_size += result.size();                          // Prevent compiler optimization
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  double avg_time = static_cast<double>(duration.count()) / iterations;
  double throughput =
      (static_cast<double>(test_data.size()) / avg_time) * 1000.0; // characters/millisecond

  std::cout << name << ":\n";
  std::cout << "  Total time: " << duration.count() << " μs (" << iterations
            << " iterations)\n";
  std::cout << "  Average time: " << avg_time << " μs/iteration\n";
  std::cout << "  Throughput: " << throughput << " characters/ms\n";
  std::cout << "  Total output size: " << total_size << " bytes\n";
  std::cout << std::endl;
}

// Correctness validation
bool validate_correctness() {
  std::cout << "Validating correctness..." << std::endl;

  // Test cases
  struct TestCase {
    std::u16string input;
    std::string expected;
  };

  std::vector<TestCase> test_cases = {
      {u"Hello World", "Hello World"},
      {u"你好世界", "你好世界"},
      {u"🚀🌟✨", "🚀🌟✨"},
      {u"A\u00E9l\u00E8ve", "Aélève"},          // Characters with accents
      {u"\u4F60\u597D\u4E16\u754C", "你好世界"} // Chinese characters
  };

  bool all_passed = true;

  for (const auto &test_case : test_cases) {
    u16v input_view(test_case.input.data(), test_case.input.size());

    auto result_original = to_utf8_original(
        input_view, gr::endian::native); // Add second parameter
    auto result_optimized = to_utf8_optimized(
        input_view, gr::endian::native); // Add second parameter

    bool original_correct =
        (result_original.as_std_view() == test_case.expected);
    bool optimized_correct =
        (result_optimized.as_std_view() == test_case.expected);
    bool results_match =
        (result_original.as_std_view() == result_optimized.as_std_view());

    if (!original_correct || !optimized_correct || !results_match) {
      std::cout << "Test failed: " << std::endl;
      std::cout << "  Input: " << test_case.input.size() << " UTF-16 characters"
                << std::endl;
      std::cout << "  Expected: " << test_case.expected << std::endl;
      std::cout << "  Original result: " << result_original
                << " (correct: " << original_correct << ")" << std::endl;
      std::cout << "  Optimized result: " << result_optimized
                << " (correct: " << optimized_correct << ")" << std::endl;
      std::cout << "  Results match: " << results_match << std::endl;
      all_passed = false;
    }
  }

  if (all_passed) {
    std::cout << "All correctness tests passed!" << std::endl;
  }

  return all_passed;
}

void test_utf_convert(){
  std::cout << "UTF-16 to UTF-8 Conversion Performance Comparison Test\n";
  std::cout << "================================\n" << std::endl;

  // Validate correctness
  if (!validate_correctness()) {
    std::cerr << "Correctness validation failed, stopping performance test" << std::endl;
    return;
  }

  // Generate test data of different sizes
  std::vector<size_t> test_sizes = {100, 1000, 10000, 50000};

  for (size_t size : test_sizes) {
    std::cout << "\nTest data size: " << size << " UTF-16 characters\n";
    std::cout << "----------------------------------------\n";

    auto test_data = generate_test_data(size);

    // Warm-up
    const char16_t *data_ptr = test_data.data();
    u16v warmup_view(data_ptr, test_data.size());
    to_utf8_original(warmup_view, gr::endian::native); // Add second parameter
    to_utf8_optimized(warmup_view,
                      gr::endian::native); // Add second parameter

    // Performance test
    int iterations = std::max(
        1000, static_cast<int>(100000 / size)); // Adjust iterations based on data size

    utf_benchmark("Original method (uc::iter)", to_utf8_original, test_data, iterations);
    utf_benchmark("Optimized method (uc::sequence)", to_utf8_optimized, test_data,
              iterations);

    // Calculate performance improvement
    const char16_t *view_data_ptr = test_data.data();
    u16v view(view_data_ptr, test_data.size());

    auto start_orig = std::chrono::high_resolution_clock::now();
    auto result_orig =
        to_utf8_original(view, gr::endian::native); // Add second parameter
    auto end_orig = std::chrono::high_resolution_clock::now();

    auto start_opt = std::chrono::high_resolution_clock::now();
    auto result_opt =
        to_utf8_optimized(view, gr::endian::native); // Add second parameter
    auto end_opt = std::chrono::high_resolution_clock::now();

    auto time_orig = std::chrono::duration_cast<std::chrono::microseconds>(
        end_orig - start_orig);
    auto time_opt = std::chrono::duration_cast<std::chrono::microseconds>(
        end_opt - start_opt);

    double speedup = static_cast<double>(time_orig.count()) / time_opt.count();

    std::cout << "Performance improvement: " << std::fixed << std::setprecision(2) << speedup
              << "x (" << (speedup - 1.0) * 100.0 << "% faster)\n";
    std::cout << "Output size - original: " << result_orig.size()
              << ", optimized: " << result_opt.size() << std::endl;
  }

  // Memory usage analysis
  std::cout << "\nMemory usage analysis:\n";
  std::cout << "----------------------------------------\n";

  auto large_test_data = generate_test_data(10000);
  // Fix: Get underlying character data
  const char16_t *large_data_ptr = large_test_data.data();
  u16v large_view(large_data_ptr, large_test_data.size());

  // Measure memory allocation
  {
    auto result_orig = to_utf8_original(
        large_view, gr::endian::native); // Add second parameter
    std::cout << "Original method - capacity: " << result_orig.capacity()
              << " bytes, size: " << result_orig.size() << " bytes\n";
  }

  {
    auto result_opt = to_utf8_optimized(
        large_view, gr::endian::native); // Add second parameter
    std::cout << "Optimized method - capacity: " << result_opt.capacity()
              << " bytes, size: " << result_opt.size() << " bytes\n";
  }
}
