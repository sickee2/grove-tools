#include <chrono>
#include <gr/console.hh>
#include <map>
#include <print>

using namespace gr;
using namespace gr::str;

class PerformanceCollector {
private:
  std::map<std::string, long long> timings_;

public:
  void add_timing(const std::string &name, long long microseconds) {
    timings_[name] = microseconds;
  }

  void print_report() const {
    for (auto &i : timings_) {
      std::cout << i.first << "=>" << i.second << std::endl;
    }
  }
};

PerformanceCollector g_collector;

class Timer {
private:
  std::chrono::high_resolution_clock::time_point start_;
  std::string name_;

public:
  Timer(const std::string &name) : name_(name) {
    start_ = std::chrono::high_resolution_clock::now();
  }

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    long long microseconds = duration.count();

    // std::println("{}: {} μs", name_, microseconds);
    g_collector.add_timing(name_, microseconds);
  }
};

void test_gr_println_optimizations() {
  const int ITERATIONS = 100;
  std::println("\n=== gr::println 长文本优化测试 ===");

  // 测试原始性能
  {
    Timer timer("console::writeln test");
    for (int i = 0; i < ITERATIONS; ++i) {
      console::writeln(
          "This is a very long test string that should demonstrate the "
          "performance characteristics of the gr::println function "
          "with multiple parameters: {}, {}, {}, {}, {}",
          i, i * 2, i * 3, i * 4, i * 5);
    }
  }

  {
    Timer timer("std::println test");
    for (int i = 0; i < ITERATIONS; ++i) {
      std::println(
          "This is a very long test string that should demonstrate the "
          "performance characteristics of the gr::println function "
          "with multiple parameters: {}, {}, {}, {}, {}",
          i, i * 2, i * 3, i * 4, i * 5);
    }
  }
  // 测试不同长度的文本
  {
    Timer timer("console::writeln 短文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      console::writeln("Short: {}", i);
    }
  }

  {
    Timer timer("std::println 短文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      std::println("Short: {}", i);
    }
  }

  {
    Timer timer("console::writeln 中文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      console::writeln("Medium length text with some data: {} and more: {}", i,
                    i * 2);
    }
  }

  {
    Timer timer("std::println 中文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      std::println("Medium length text with some data: {} and more: {}", i,
                   i * 2);
    }
  }

  {
    Timer timer("console::writeln 长文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      console::writeln("Very long text with many parameters: {}, {}, {}, {}, {}, "
                    "{}, {}, {}, {}, {}",
                    i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7, i + 8,
                    i + 9);
    }
  }

  {
    Timer timer("std::println 长文本");
    for (int i = 0; i < ITERATIONS; ++i) {
      std::println("Very long text with many parameters: {}, {}, {}, {}, {}, "
                   "{}, {}, {}, {}, {}",
                   i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7, i + 8,
                   i + 9);
    }
  }
}

int main() {
  console::writeln("开始性能测试...");
  test_gr_println_optimizations();
  g_collector.print_report();
  console::writeln("性能测试完成!");
}
