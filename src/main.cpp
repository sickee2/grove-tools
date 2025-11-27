#include "gr/format.hh"
#include <gr/console.hh>
#include <gr/performance_timer.hh>
using namespace gr;

int main() {
  gr::console::writeln("helle world {}", gr::toy::chrono::now());
  return 0;
}
