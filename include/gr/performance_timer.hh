#pragma once
#include <cstdint>
#include <gr/console.hh>

class PerformanceTimer {
private:
  std::chrono::high_resolution_clock::time_point start_time;
  std::string test_name;
  uint64_t *dur = nullptr;

public:
  PerformanceTimer(const std::string &name) : test_name(name) {
    start_time = std::chrono::high_resolution_clock::now();
  }
  PerformanceTimer(const std::string &name, uint64_t& out) : test_name(name) {
    dur = &out;
    start_time = std::chrono::high_resolution_clock::now();
  }

  ~PerformanceTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    auto count = duration.count();
    if(dur){
      *dur = count;
    }
    gr::console::writeln("{} : {} μs", test_name, duration.count());
  }

  uint64_t elapsed_microseconds() {
    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    auto count = duration.count();
    if(dur){
      *dur = count;
    }

    return count;
  }
};
