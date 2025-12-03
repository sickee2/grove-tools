#include "test.hh"
#include "gr/performance_timer.hh"
#include <fmt/format.h>
#include <gr/console.hh>
using namespace gr;
// namespace term = gr::console;

void test_fmt_vs_toy_fmt() {
  float data[] = {
      3.141592653589793f, 31.41592653589793f, 314.1592653589793f,
      3141.592653589793f, 31415.92653589793f,
  };
  double data_d[] = {
      3.141592653589793, 31.41592653589793, 314.1592653589793,
      3141.592653589793, 31415.92653589793,
  };

  double test_cases[] = {
      // 常规值
      0.0,
      1.0,
      10.0,
      100.0,
      1000.0,
      0.1,
      0.01,
      0.001,
      123.456,
      123456.789,
      0.000123456,
      123456789.0,

      // 边界值
      1e-308,       // double 最小值
      1e308,        // double 最大值
      2.22507e-308, // 最小正规格化数

      // 特殊值
      std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::quiet_NaN(),

      // 负数
      -3.141592653589793,
      -31415.92653589793,
      -1e-308,
  };
  console::writeln("== float test fixed");
  unsigned iteration = 100000;
  {
    PerformanceTimer timer("fmt");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = fmt::format("{:.4f}", v);
      }
    }
  }
  {
    PerformanceTimer timer("toy");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = toy::format("{:.4f}", v);
      }
    }
  }
  console::writeln("== float test scientific");
  {
    PerformanceTimer timer("fmt");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = fmt::format("{:.4e}", v);
      }
    }
  }
  {
    PerformanceTimer timer("toy");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = toy::format("{:.4e}", v);
      }
    }
  }

  console::writeln("\n=======================================");
  console::writeln("==== float test");
  for (auto v : data) {
    // auto result_fmt_fixed = fmt::format("{:.4f}", v);
    // auto result_toy_fixed = toy::format("{:.4f}", v);
    auto result_fmt_sci = fmt::format("{:.4e}", v);
    auto result_toy_sci = toy::format("{:.4e}", v);
    // console::writeln("fixed :{} => {}", result_fmt_fixed, result_toy_fixed);
    console::writeln("scientific :{} => {}", result_fmt_sci, result_toy_sci);
  }
  console::writeln("\n==== double test");
  for (auto v : data_d) {
    // auto result_fmt_fixed = fmt::format("{:.4f}", v);
    // auto result_toy_fixed = toy::format("{:.4f}", v);
    auto result_fmt_sci = fmt::format("{:.4e}", v);
    auto result_toy_sci = toy::format("{:.4e}", v);
    // console::writeln("fixed :{} => {}", result_fmt_fixed, result_toy_fixed);
    console::writeln("scientific :{} => {}", result_fmt_sci, result_toy_sci);
  }

  console::writeln("\n==== test cases");
  for (auto v : test_cases) {
    try {
      auto result_fmt = fmt::format("{:.6e}", v);
      auto result_toy = toy::format("{:.6e}", v);

      // 对于 NaN，直接比较字符串可能有问题
      if (std::isnan(v)) {
        console::writeln("NaN test: fmt='{}', toy='{}'", result_fmt,
                         result_toy);
      } else {
        console::writeln("{:.6e} => fmt='{}', toy='{}'", v, result_fmt,
                         result_toy);
      }
    } catch (const std::exception &e) {
      console::writeln("Error for {}: {}", v, e.what());
    }
  }

  console::writeln("\n==== test cases general");
  for (auto v : test_cases) {
    auto result_fmt = fmt::format("{:g}", v);
    auto result_toy = toy::format("{:g}", v);
    console::writeln("{:g} => fmt='{}', toy='{}'", v, result_fmt, result_toy);
  }
}
int main() {
  test_all();

  // test_fmt_vs_toy_fmt();

  // double d = 0.0;
  //
  // console::writeln("{} => {}", fmt::format("{:.6e}", d), toy::format("{:.6e}", d));
  return 0;
}
