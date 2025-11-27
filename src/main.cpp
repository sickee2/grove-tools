#include "gr/detail/toy_charconv.hh"
#include "gr/format.hh"
#include <cstdint>
#include <gr/console.hh>
#include <gr/performance_timer.hh>
#include <stdexcept>
using namespace gr;

void test_permance() {
  int iteration = 10000;
  {
    double data[] = {
        134123.1234123,
        4123.1234123,
        123.1234123,
        23.1234123,
    };
    console::writeln("\n=== float to chars ===");
    {
      PerformanceTimer t("toy::ftoss");
      char buffer[128] = {};
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = toy::ftoss(buffer, 128, v, toy::chars_format::fixed, 8);
          if(res.ptr == nullptr){
            throw std::runtime_error("itoss error");
          }
        }
      }
    }
    {
      PerformanceTimer t("std::to_char");
      char buffer[128] = {};
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = std::to_chars(buffer, buffer + 128, v);
          if(res.ec != std::errc{}){
            throw std::runtime_error("std::to_char float");
          }
        }
      }
    }
  }
  {
    int64_t data[] = {
        1341231234123,
        41231234123,
        1231234123,
        231234123,
    };
    console::writeln("\n=== integer to chars ===");
    {
      PerformanceTimer t("toy::itoss");
      char buffer[128] = {};
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = toy::itoss(buffer, 128, v, 10);
          if(res.ptr == nullptr){
            throw std::runtime_error("itoss error");
          }
        }
      }
    }
    {
      PerformanceTimer t("std::to_char");
      char buffer[128] = {};
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = std::to_chars(buffer, buffer + 128, v, 10);
          if(res.ec != std::errc{}){
            throw std::runtime_error("to_chars error");
          }
        }
      }
    }
  }
  {
    gr::str::u8v data[] = {
        "134123.1234123",
        "4123.1234123",
        "123.1234123",
        "23.1234123",
    };
    console::writeln("\n=== chars to float ===");
    {
      PerformanceTimer t("toy::sstof");
      double value;
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = toy::sstof(v.data(), v.size(), value);
          if (res.ec != std::errc{}) {
            throw std::runtime_error("sstof error");
          }
        }
      }
    }
    {
      PerformanceTimer t("std::from_chars");
      double value;
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = std::from_chars(v.data(), v.data() + v.size(), value);
          if (res.ec != std::errc{}) {
            throw std::runtime_error("from_chars error");
          }
        }
      }
    }
  }
  {
    gr::str::u8v data[] = {
        "-1341231234123",
        "41231234123",
        "1231234123",
        "231234123",
    };
    console::writeln("\n=== chars to integer ===");
    {
      PerformanceTimer t("toy::sstoi");
      volatile int64_t ks = 0;
      int64_t value;
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = toy::sstoi(v.data(), v.size(), value);
          ks += value;
          if(res.ec != std::errc{} ){
            throw std::runtime_error("sstoi error");
          }
          // console::writeln("toy:sstoi => value: {}", value);
        }
      }
    }
    {
      PerformanceTimer t("std::from_chars");
      volatile int64_t ks = 0;
      int64_t value;
      for (int i = 0; i < iteration; i++) {
        for (auto v : data) {
          auto res = std::from_chars(v.data(), v.data() + v.size(), value);
          ks += value;
          if(res.ec != std::errc{}){
            throw std::runtime_error("from_chars error");
          }
        }
      }
    }
  }
}

int main() {
  test_permance();
  console::writeln("\n===============================");
  // test_sstoi_boundary();
  // console::writeln(" str check: {}\n cal int part: {}\n cal frac part: {}\n
  // cal exp part: {}",
  //                  check_timer, calculate_int_part_timer,
  //                  calculate_frac_part_timer, calculate_exponet_timer);

  console::writeln("\n===============================");
  gr::str::u8v data("-01");

  // data = "-123.4567e-02";
  data = "2345e-5";

  double value;
  console::writeln("original str: {}", data);
  auto res = toy::sstof(data.data(), data.size(), value);
  console::writeln("value: {:.18f}", value);
  console::writeln("remaining: {}", res.ptr);

  console::writeln("\n===============================");
  char ch[4] = "inf"; 
  console::writeln("=> {}", ch);
  return 0;
}
