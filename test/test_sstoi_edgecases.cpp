#include <gr/console.hh>
using namespace gr;
template <typename test_type>
void test_from_chars_tpl(){

  test_type value_max = std::numeric_limits<test_type>::max();
  test_type value_min = std::numeric_limits<test_type>::min();
  // console::writeln("type max: {}", std::numeric_limits<test_type>::max());
  // console::writeln("type min: {}", std::numeric_limits<test_type>::min());
  // console::writeln("type uint64_t max: {}", std::numeric_limits<uint64_t>::max());
  console::writeln("============================");
  str::u8v int_strings[] = {
    "-340282366920938463463374607431768211455",
    "65536",
    "70000",
    "99999",
    "4294967295",
    "4294967296",
    "5000000000",
    "9999999999",
    "18446744073709551615",
    "18446744073709551616",
    "20000000000000000000",
    "99999999999999999999",
    "-456",
    "1234567890",
    "987654321", 
    "255",
    "128",
    "65535",
    "2147483647",
    "2147483648",
    "32768",
    "32769",
    "32760",
    "327670",
    "327671",
    "327672",
    "327673",
    "327674",
    "327675",
    "327676",
    "327677",
    "327678",
    "327679",
  };

  str::u8v hex[] = {
    "-ffffffffffffffffffffffffffffffff",
    "10000",
    "11170",
    "1869f",
    "ffffffff",
    "100000000",
    "12a05f200",
    "2540be3ff",
    "ffffffffffffffff",
    "10000000000000000",
    "1158e460913d00000",
    "56bc75e2d630fffff",
    "-1c8",
    "499602d2",
    "3ade68b1",
    "ff",
    "80",
    "ffff",
    "7fffffff",
    "80000000",
    "8000",
    "8001",
    "7ff8",
    "4fff6",
    "4fff7",
    "4fff8",
    "4fff9",
    "4fffa",
    "4fffb",
    "4fffc",
    "4fffd",
    "4fffe",
    "4ffff"
  };

  for(auto i = 0; i < 32; i++){
    test_type v1, v2;
    auto status1 = toy::sstoi(int_strings[i].data(), int_strings[i].size(), v1);
    if(status1.ec != std::errc{}){
      console::writeln("❌ out of range [min:{} max:{}] -> '{}' => wrong value out: {}", value_min, value_max, int_strings[i], v1);
    }else{
      auto status2 = toy::sstoi(hex[i].data(), hex[i].size(), v2, 16);
      (void)status2;
      if(v1 != v2){
        console::writeln("❌ {} value: {} hex value: {:x} hex string: {} v2: {}", int_strings[i], v1, v1, hex[i], v2);
      }else{
        console::writeln("✓ value: {:>24} <=> {} orignal str: '{}'", v1, v2, int_strings[i]);
      }
    }
  }
}

void test_from_chars(){
  console::writeln("\n=== test_from_chars => __int128_t ===");
  test_from_chars_tpl<__int128_t>();
  console::writeln("\n=== test_from_chars => __unt128_t ===");
  test_from_chars_tpl<__uint128_t>();

  console::writeln("\n=== test_from_chars => int64_t ===");
  test_from_chars_tpl<int64_t>();
  console::writeln("\n=== test_from_chars => unt64_t ===");
  test_from_chars_tpl<uint64_t>();

  console::writeln("\n=== test_from_chars => int32_t ===");
  test_from_chars_tpl<int32_t>();
  console::writeln("\n=== test_from_chars => unt32_t ===");
  test_from_chars_tpl<uint32_t>();

  console::writeln("\n=== test_from_chars => int16_t ===");
  test_from_chars_tpl<int16_t>();
  console::writeln("\n=== test_from_chars => unt16_t ===");
  test_from_chars_tpl<uint16_t>();

  console::writeln("\n=== test_from_chars => int8_t ===");
  test_from_chars_tpl<int8_t>();
  console::writeln("\n=== test_from_chars => unt8_t ===");
  test_from_chars_tpl<uint8_t>();
}
