#include "test.hh"
#include "gr/format.hh"
#include "gr/performance_timer.hh"
#include <charconv>
#include <cstdint>
#include <fmt/format.h>
#include <gr/console.hh>
using namespace gr;
// namespace term = gr::console;

void test_float_fmt_vs_toy_fmt() {
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
void test_integer_fmt_vs_toy() {
  int64_t data[] = {
      -31415926535,   314159265358,     3141592653589,
      31415926535897, 3141592653589793,
  };

  // console::writeln("{}", data[0]);

  unsigned iteration = 100000;

  console::writeln("\n== test iteration {}", iteration);

  console::writeln("\n== test integer base 10");
  uint64_t fmt_dur = 0, toy_dur = 0;
  {
    PerformanceTimer timer("fmt", fmt_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = fmt::format("{}", v);
      }
    }
  }
  {
    PerformanceTimer timer("toy", toy_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = toy::format("{}", v);
      }
    }
  }

  console::writeln(" ==> {:.1f}%", (double)(fmt_dur) / toy_dur * 100);

  console::writeln("\n== test integer base 16");
  {
    PerformanceTimer timer("fmt", fmt_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = fmt::format("{:x}", v);
      }
    }
  }
  {
    PerformanceTimer timer("toy", toy_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = toy::format("{:x}", v);
      }
    }
  }

  console::writeln(" ==> {:.1f}%", (double)(fmt_dur) / toy_dur * 100);

  console::writeln("\n== test integer base 8");
  {
    PerformanceTimer timer("fmt", fmt_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = fmt::format("{:o}", v);
      }
    }
  }
  {
    PerformanceTimer timer("toy", toy_dur);
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : data) {
        auto result = toy::format("{:o}", v);
      }
    }
  }
  console::writeln(" ==> {:.1f}%", (double)(fmt_dur) / toy_dur * 100);
}
void micro_benchmark() {
  console::writeln("\n== Integer formatting breakdown");

  unsigned loop = 5;
  int64_t test_value = -31415926535;
  unsigned iteration = 100000;

  for (unsigned lp = 0; lp < loop; lp++) {
    console::writeln("loop iteration {}", lp);
    {
      uint64_t itoss_time = 0;
      PerformanceTimer timer("\tstd::to_chars", itoss_time);
      for (unsigned i = 0; i < iteration; i++) {
        char buffer[32]{};
        auto [ptr, len] = std::to_chars(buffer, buffer + 32, test_value, 10);
        (void)ptr;
      }
    }
    {
      uint64_t itoss_time = 0;
      PerformanceTimer timer("\titoss only", itoss_time);
      for (unsigned i = 0; i < iteration; i++) {
        char buffer[32]{};
        auto [ptr, len] = toy::itoss(buffer, 32, test_value, 10);
        (void)ptr;
      }
    }
    {
      uint64_t itoss_time = 0;
      PerformanceTimer timer("\tcopy n itoss only", itoss_time);
      for (unsigned i = 0; i < iteration; i++) {
        char buffer[32]{};
        auto [ptr, len] = toy::itoss(buffer, 32, test_value, 10);
        str::u8 buffer_content;
        buffer_content.reserve(128);
        toy::format_output out(buffer_content);
        out.put(ptr, len);
        (void)out;
      }
    }
    // {
    //   uint64_t itoss_time = 0;
    //   PerformanceTimer timer("\tcopy 3 times itoss only", itoss_time);
    //   str::u8 buffer_content;
    //   toy::format_output out(buffer_content);
    //   for (unsigned i = 0; i < iteration; i++) {
    //     for(int j = 0; j < 3; j++){
    //       char buffer[32]{};
    //       auto [ptr, len] = toy::itoss(buffer, 32, test_value, 10);
    //       out.put(ptr, len);
    //     }
    //     (void)out;
    //
    //   }
    // }
    //
    // {
    //   uint64_t itoss_time = 0;
    //   PerformanceTimer timer("\tcopy 3 times toy::detail::format_integer_impl", itoss_time);
    //   str::u8 buffer_content;
    //   toy::format_output out(buffer_content);
    //   toy::format_spec spec;
    //   spec.type = 'd';
    //   for (unsigned i = 0; i < iteration; i++) {
    //     for(int j = 0; j < 3; j++){
    //       toy::detail::format_integer_impl(out, test_value, spec);
    //     }
    //     (void)out;
    //
    //   }
    // }

    {
      uint64_t toy_format_time = 0;
      PerformanceTimer timer("\ttoy::format", toy_format_time);
      for (unsigned i = 0; i < iteration; i++) {
        auto s = toy::format("{}", test_value);
        (void)s;
      }
    }
    {
      uint64_t fmt_format_time = 0;
      PerformanceTimer timer("\tfmt::format", fmt_format_time);
      for (unsigned i = 0; i < iteration; i++) {
        auto s = fmt::format("{}", test_value);
        (void)s;
      }
    }
    {
      uint64_t toy_format_time = 0;
      PerformanceTimer timer("\ttoy::format multity", toy_format_time);
      for (unsigned i = 0; i < iteration; i++) {
        auto s = toy::format("{} {} {}", test_value, test_value, test_value);
        (void)s;
      }
    }
    {
      uint64_t fmt_format_time = 0;
      PerformanceTimer timer("\tfmt::format multity", fmt_format_time);
      for (unsigned i = 0; i < iteration; i++) {
        auto s = fmt::format("{} {} {}", test_value, test_value, test_value);
        (void)s;
      }
    }
  }
}
int main() {
  test_all();

  // test_fmt_vs_toy_fmt();

  // test_integer_fmt_vs_toy();

  // micro_benchmark();

  // auto b = toy::detail::supports_integer_v<__int128_t>;
  //
  // console::writeln("is_integral_v<__int128> {}", b);
  //
  // // 测试转义
  // int i = 10;
  // console::writeln("{} |{:4d}| |{:2x}|", i, i, i);
  // console::writeln("{}", std::format("{} |{:4d}| |{:2x}|", i, i, i));
  // console::writeln("Data: {:.2f} {:s} {:2x} {:.2f} {:s} {:d}", 3.14159 + i, "text", i, 42.5f, 'a', true);
  // console::writeln("escape {{}} => {} {} {:.2e}", "test", 123, 123.2);
  // console::writeln("{{{{}}}} => {}", "test");
  // console::writeln("Literal {{ braces }} and value: {}", 42);
  // console::writeln("({{:.{{}}f}}, 3.1415926, {1}) => {0:.{1}f}", 3.1415926, 3);
  // console::writeln("{{Escaped}} and {:.{}f}", 3.1415926, 2);
  return 0;
}
