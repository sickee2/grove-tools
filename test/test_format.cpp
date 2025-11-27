#include <chrono>
#include <gr/format.hh>
#include <gr/console.hh>

void comprehensive_chrono_test() {
  using namespace std::chrono;

  using namespace gr;
  gr::console::write("=== 全面的 Chrono 格式化测试 ===\n\n");

  // 创建各种时长
  auto ns_duration = nanoseconds(123456789);
  auto us_duration = microseconds(123456);
  auto ms_duration = milliseconds(1234);
  auto sec_duration = seconds(90);
  auto min_duration = minutes(125);
  auto hour_duration = hours(48);

  auto now = system_clock::now();

  console::writeln("=== 时长格式化测试 ===");
  console::writeln("纳秒: {:N}", ns_duration);
  console::writeln("微秒: {:U}", us_duration);
  console::writeln("毫秒: {:M}", ms_duration);
  console::writeln("秒:  {:s}", sec_duration);
  console::writeln("分钟: {:m}", min_duration);
  console::writeln("小时: {:h}", hour_duration);
  console::writeln("天:  {:d}", hour_duration);

  console::writeln("=== 时长格式化测试2 ===");
  console::writeln("纳秒: {:#N}", ns_duration);
  console::writeln("微秒: {:#U}", us_duration);
  console::writeln("毫秒: {:#M}", ms_duration);
  console::writeln("秒:  {:#s}", sec_duration);
  console::writeln("分钟: {:#m}", min_duration);
  console::writeln("小时: {:#h}", hour_duration);
  console::writeln("天:  {:#d}", hour_duration);

  console::writeln("\n=== 精度控制测试 ===");
  auto precise_duration = milliseconds(1234) + microseconds(567);
  console::writeln("默认精度: {:s}", precise_duration);
  console::writeln("精度3: {:.3s}", precise_duration);
  console::writeln("精度6: {:.6s}", precise_duration);

  console::writeln("\n=== 时间点格式化测试 ===");
  console::writeln("默认: {}", now);
  console::writeln("日期: {:d}", now);
  console::writeln("时间: {:t}", now);
  console::writeln("完整: {:f}", now);

  console::writeln("\n=== 便捷函数测试 ===");
  console::writeln("当前时间: {}" , gr::toy::chrono::now());

  console::writeln("=== Testing explicit indexing with nested format ===");
  for (int i = 0; i < 5; i++) {
    console::writeln("|{1:>{0}d}{2:>{3}d}", i + 1, i, '|', 5 - i);
  }

  console::writeln("=== Testing automatic indexing with nested format ===");
  for (int i = 0; i < 5; i++) {
    console::writeln("|{:>{}d}|", i + 1, 4);
  }

  console::writeln("=== Testing precision nesting ===");
  console::writeln("|{0:.{1}f}|", 3.14159, 4);
  console::writeln("|{:.4f}|", 3.14159);

  // 性能测试
  console::writeln("\n=== 性能测试 ===");
  auto start = high_resolution_clock::now();

  // 执行一些计算
  volatile int sum = 0;
  for (int i = 0; i < 1000000; ++i) {
    sum += i * i;
  }

  auto end = high_resolution_clock::now();
  auto elapsed = end - start;

  console::writeln("计算耗时: {:.3a}", elapsed);

  // 测试复杂的时间运算
  auto future_time = now + hours(24) + minutes(30);
  console::writeln("24小时30分钟后: {:f}", future_time);
}

struct MyType {
  int id;
  std::string name;
};

template <> struct gr::toy::detail::formatter<MyType> {
  void operator()(gr::toy::format_output& out, const MyType &value,
                                  const format_spec &) {
    auto res = gr::toy::format("id={}, name={}", value.id, value.name);
    out.put(res);
  }
};

struct MyCustomType{
  int x, y;
};

template <>
struct gr::toy::detail::formatter<MyCustomType> {
  void operator()(format_output &out, const MyCustomType &value,
                  const format_spec &spec) const {
    // std::string_view pattern(spec.fmt_beg, spec.fmt_end - spec.fmt_beg);
    auto pattern = spec.get_pattern();

    if (pattern == "json") {
      // JSON 格式输出
      // out.put("{");
      // out.put(gr::toy::format("\"x\": {}", value.x));
      // out.put(", ");
      // out.put(gr::toy::format("\"y\": {}", value.y));
      // out.put("}");
      out.put(gr::toy::format("{{x:{}}},{{y:{}}}", value.x, value.y));
    } else if (pattern == "simple") {
      // 简单格式输出
      out.put(gr::toy::format("({}, {})", value.x, value.y));
    } else if (spec.type == 'v') {
      // 详细格式输出
      out.put(gr::toy::format("MyCustomType(x={}, y={})", value.x, value.y));
    } else {
      // 默认格式
      out.put(gr::toy::format("{}:{}", value.x, value.y));
    }
  }
};

int main() {

  comprehensive_chrono_test();
  MyType obj{42, "test"};
  gr::console::writeln("OBJ: {}", obj);

  MyCustomType obj2{10, 20};
  gr::console::writeln("{:json}", obj2);
  gr::console::writeln("{:simple}", obj2);
  gr::console::writeln("{:v}", obj2);
  gr::console::writeln("{}", obj2);

  return 0;
}
