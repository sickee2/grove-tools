#include <cstdint>
#include <gr/console.hh>
#include <gr/performance_timer.hh>

// 预计算 10^0 ~ 10^6（避免重复乘法）
constexpr uint64_t POW10_TABLE[] = {1,
                                    10,
                                    100,
                                    1000,
                                    10000,
                                    100000,
                                    1000000,
                                    10000000,
                                    100000000,
                                    1000000000,
                                    10000000000,
                                    100000000000,
                                    1000000000000,
                                    10000000000000,
                                    100000000000000,
                                    1000000000000000,
                                    10000000000000000,
                                    100000000000000000,
                                    1000000000000000000};


struct fp_part {
  uint64_t int_part;
  uint64_t frac_part;
  bool sign;
  int precision; // 实际使用的小数位数
};

template<typename T> struct fp_traits;

template<> struct fp_traits<float> {
  using uint_type = uint32_t;
  using store_type = uint32_t;
  using up_store_type = uint64_t;
  static constexpr int mantissa_bits = 23;
  static constexpr int exponent_bits = 8;
  static constexpr int exponent_bias = 127;
  static constexpr uint32_t mantissa_mask = 0x7FFFFF;
  static constexpr uint32_t implicit_bit = 0x800000;
  static constexpr int max_decimal_digits = 8;
  static constexpr int max_shift_bits = 63; // float可以使用64位计算
};

template<> struct fp_traits<double> {
  using uint_type = uint64_t;
  using store_type = uint64_t;
  using up_store_type = __uint128_t;
  static constexpr int mantissa_bits = 52;
  static constexpr int exponent_bits = 11;
  static constexpr int exponent_bias = 1023;
  static constexpr uint64_t mantissa_mask = 0xFFFFFFFFFFFFFULL;
  static constexpr uint64_t implicit_bit = 0x10000000000000ULL;
  static constexpr int max_decimal_digits = 17;
  static constexpr int max_shift_bits = 127; // double可能需要128位
};

// 通用的浮点数部分提取函数
template<typename fp_type>
fp_part get_fp_parts(fp_type value, unsigned precision) {
  using traits = fp_traits<fp_type>;
  using uint_type = typename traits::uint_type;
  using store_type = typename traits::store_type;
  using up_type = typename traits::up_store_type;
  
  // 类型双关访问位表示
  union {
    fp_type f;
    uint_type u;
  } conv = {value};

  const bool is_negative = (conv.u >> (traits::mantissa_bits + traits::exponent_bits)) != 0;
  const int exponent = ((conv.u >> traits::mantissa_bits) & ((1 << traits::exponent_bits) - 1)) 
                       - traits::exponent_bias;
  const uint_type mantissa = (conv.u & traits::mantissa_mask) | traits::implicit_bit;

  store_type integer_part = 0;
  store_type fractional_part = 0;
  store_type pow10_precision = static_cast<store_type>(POW10_TABLE[precision]);

  // 统一计算参数
  int effective_exponent = exponent;
  uint_type effective_mantissa = mantissa;
  bool is_pure_integer = false;

  if (exponent >= 0) {
    if (exponent > traits::mantissa_bits) {
      // 纯整数情况
      integer_part = static_cast<store_type>(mantissa) << (exponent - traits::mantissa_bits);
      is_pure_integer = true;
    }
  } else {
    integer_part = 0;
    effective_exponent = -exponent;
  }

  if (!is_pure_integer) {
    const int shift_bits = (exponent >= 0) 
        ? (traits::mantissa_bits - exponent) 
        : (traits::mantissa_bits + effective_exponent);

    if (exponent >= 0) {
      integer_part = static_cast<store_type>(mantissa >> shift_bits);
      effective_mantissa = mantissa & ((static_cast<uint_type>(1) << shift_bits) - 1);
    } else {
      effective_mantissa = mantissa;
    }

    // 计算小数部分
    if (shift_bits <= traits::max_shift_bits) {
      up_type temp = static_cast<up_type>(effective_mantissa) * pow10_precision;

      // 四舍五入
      if (shift_bits > 0) {
        if constexpr (std::is_same_v<up_type, uint64_t>) {
          // 64位版本
          temp += (static_cast<uint64_t>(1) << (shift_bits - 1));
        } else {
          // 128位版本
          up_type round_offset = 1;
          round_offset <<= (shift_bits - 1);
          temp += round_offset;
        }
      }

      fractional_part = static_cast<store_type>(temp >> shift_bits);

      // 进位处理
      if (fractional_part >= pow10_precision) {
        fractional_part -= pow10_precision;
        integer_part++;
        // 极少数情况需要第二次进位
        if (fractional_part >= pow10_precision) {
          fractional_part -= pow10_precision;
          integer_part++;
        }
      }
    }
  }

  fp_part parts;
  
  // 修复符号处理：不再修改 integer_part，而是在 parts 中记录符号
  parts.sign = is_negative;
  parts.int_part = static_cast<uint64_t>(integer_part);
  parts.frac_part = static_cast<uint64_t>(fractional_part);
  parts.precision = precision;

  return parts;
}

std::string parts_to_string(const fp_part &parts) {
  std::string result;
  result.reserve(128);

  // 1. 拼接符号位
  if (parts.sign) {
    result += '-';
  }

  // 2. 整数部分转字符串
  uint64_t integer_part = parts.int_part;
  if (integer_part == 0) {
    result += '0';
  } else {
    char buf[40];
    int buf_idx = 0;
    while (integer_part > 0) {
      buf[buf_idx++] = '0' + (integer_part % 10);
      integer_part /= 10;
    }
    while (buf_idx > 0) {
      result += buf[--buf_idx];
    }
  }

  // 3. 拼接小数部分（使用指定的精度）
  result += '.';
  char frac_buf[20]; // 最多支持19位小数（POW10_TABLE的大小）
  uint64_t fractional_part = parts.frac_part;
  for (int i = parts.precision - 1; i >= 0; --i) {
    frac_buf[i] = '0' + (fractional_part % 10);
    fractional_part /= 10;
  }
  result.append(frac_buf, parts.precision);

  return result;
}

// 包装函数，保持原有接口
inline fp_part get_float_parts(float f, unsigned precision = fp_traits<float>::max_decimal_digits) {
  return get_fp_parts<float>(f, precision);
}

inline fp_part get_double_parts(double d, unsigned precision = fp_traits<double>::max_decimal_digits) {
  return get_fp_parts<double>(d, precision);
}

// 字符串转换函数
template<typename fp_type>
std::string fp_to_string(fp_type value, unsigned precision) {
  fp_part parts = get_fp_parts<fp_type>(value, precision);
  return parts_to_string(parts);
}

inline std::string float_to_string(float f, unsigned precision = fp_traits<float>::max_decimal_digits) {
  return fp_to_string<float>(f, precision);
}

inline std::string double_to_string(double d, unsigned precision = fp_traits<double>::max_decimal_digits) {
  return fp_to_string<double>(d, precision);
}

void test_negative_numbers() {
  using namespace gr;
  using namespace gr::console;
  
  console::writeln("\n=== 负数测试 ===");
  
  // 测试负数
  double negative_values[] = {
    -3.141592653589793,
    -0.999999999999999999,
    -123456.789012345,
    -0.1,
    -0.0,
  };
  
  for (auto v : negative_values) {
    auto parts = get_fp_parts<double>(v, 8);
    std::string result = parts_to_string(parts);
    console::writeln("值: {} -> {}", v, result);
  }
  
  // 与 toy 实现比较
  console::writeln("\n=== 与 toy 实现比较 ===");
  double test_value = -0.999999999999999999;
  
  auto our_parts = get_fp_parts<double>(test_value, 8);
  auto toy_parts = toy::detail::_split_float_u<double>(test_value, 8);
  
  console::writeln("我们的: sign={}, int={}, frac={}", 
                   our_parts.sign, our_parts.int_part, our_parts.frac_part);
  console::writeln("Toy的: int={}, frac={}", 
                   static_cast<uint64_t>(toy_parts.int_part), 
                   static_cast<uint64_t>(toy_parts.frac_part));
  
  // 注意：toy 实现可能将符号编码在 integer_part 中
  // 而我们的实现将符号单独存储
}

void test_ftoss() {
  using namespace gr;
  using namespace gr::console;

  double d[] = {
      3.141592653589793, 31.41592653589793, 314.1592653589793,
      3141.592653589793, 314159.2653589793, 31415926.53589793,
      314159265.3589793,
  };
  console::writeln("=== split double ===");
  volatile uint64_t opt = 0;
  unsigned iteration = 100000;
  {
    PerformanceTimer timer("toy::split => double");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : d) {
        auto parts = toy::detail::_split_float_u<double>(v, 8);
        opt += parts.int_part;
      }
    }
  }
  {
    opt = 0;
    PerformanceTimer timer("get_double_parts");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : d) {
        auto parts = get_double_parts(v, 8);
        opt += parts.int_part;
      }
    }
  }

  float f[] = {
      3.141592653589793f, 31.41592653589793f, 314.1592653589793f,
      3141.592653589793f, 314159.2653589793f, 31415926.53589793f,
      314159265.3589793f,
  };

  console::writeln("=== split float ===");
  opt = 0;
  {
    PerformanceTimer timer("toy::split => float");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : f) {
        auto parts = toy::detail::_split_float_u<float>(v, 8);
        opt += parts.int_part;
      }
    }
  }
  {
    opt = 0;
    PerformanceTimer timer("get_float_parts");
    for (unsigned i = 0; i < iteration; i++) {
      for (auto v : f) {
        auto parts = get_float_parts(v, 8);
        opt += parts.int_part;
      }
    }
  }

  console::writeln("\n=== 负数正确性测试 ===");
  
  double negative_test = -0.999999999999999999;
  auto neg_parts = get_double_parts(negative_test, 6);
  console::writeln("负值测试: {} -> {} {}{}.{}", negative_test,
                   double_to_string(negative_test, 6), neg_parts.sign? '-' : '+', neg_parts.int_part,
                   neg_parts.frac_part);

  // 测试 0 和 -0
  console::writeln("0: {}", double_to_string(0.0, 6));
  console::writeln("-0: {}", double_to_string(-0.0, 6));


  console::writeln("\n=== float_to_string_fast 不同精度测试 ===");
  console::writeln("精度2: {}", float_to_string(3.1415926f, 2));
  console::writeln("精度4: {}", float_to_string(3.1415926f, 4));
  console::writeln("精度6: {}", float_to_string(3.1415926f, 6));
  console::writeln("精度8: {}", float_to_string(3.1415926f, 8));

  console::writeln("\n=== double_to_string_fast 不同精度测试 ===");
  console::writeln("精度2: {}", double_to_string(3.141592653589793, 2));
  console::writeln("精度4: {}", double_to_string(3.141592653589793, 4));
  console::writeln("精度6: {}", double_to_string(3.141592653589793, 6));
  console::writeln("精度8: {}", double_to_string(3.141592653589793, 8));
  console::writeln("精度10: {}", double_to_string(3.141592653589793, 10));

  console::writeln("\n=== 原始测试（默认精度6）===");
  console::writeln("---- float ----");
  console::writeln("{}", float_to_string(0.1f));
  console::writeln("{}", float_to_string(3.1415926f));
  console::writeln("{}", float_to_string(-0.000123f));
  console::writeln("{}", float_to_string(12345.6789f));

  console::writeln("---- double ----");

  console::writeln("{}", double_to_string(0.1));
  console::writeln("{}", double_to_string(3.141592653589793));
  console::writeln("{}", double_to_string(-0.999999999999999999));
  console::writeln("{}", double_to_string(123456.789012345));
}
