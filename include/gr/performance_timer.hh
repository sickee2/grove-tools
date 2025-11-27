#pragma once
#include <gr/console.hh>

class PerformanceTimer {
private:
  std::chrono::high_resolution_clock::time_point start_time;
  std::string test_name;

public:
  PerformanceTimer(const std::string &name) : test_name(name) {
    start_time = std::chrono::high_resolution_clock::now();
  }

  ~PerformanceTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    gr::console::writeln("{} : {} μs", test_name, duration.count());
  }

  long long elapsed_microseconds() {
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                 start_time)
        .count();
  }
};
