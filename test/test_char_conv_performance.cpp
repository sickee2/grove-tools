#include "gr/detail/toy_charconv.hh"
#include <cstdint>
#include <gr/console.hh>
#include <gr/performance_timer.hh>
#include <stdexcept>
using namespace gr;

void test_chars_conv_performance() {
  int iteration = 100000;
  console::writeln("\n=== test char conv performance ==> iteration {}", iteration);
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
    uint64_t t1, t2;
    {
      PerformanceTimer t("toy::sstof", t1);
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
      PerformanceTimer t("std::from_chars", t2);
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
    console::writeln("  ==> toy/std {:.1f}%", ((double)(t1)/t2)*100);
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
      int64_t value = 0;
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
      int64_t value = 0;
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

