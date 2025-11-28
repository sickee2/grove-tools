#include "gr/format.hh"
#include <gr/console.hh>
#include <gr/performance_timer.hh>

int main() {
  using namespace gr;
  console::writeln("helle world {}", gr::toy::chrono::now());
  console::writeln("test .gitignore");
  console::writeln("new modify for git testing");
  return 0;
}
