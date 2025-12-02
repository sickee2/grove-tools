// filepath: src/main.cpp
#include <gr/detail/toy_charconv.hh>
#include <gr/console.hh>
// #include <charconv>
#include <limits>
using namespace gr;
// namespace term = gr::console;

void test_re2();
void test_cbuf();
void test_chars_conv_performance();
void test_ftoss();
void test_utf_convert();
void test_utf_iter();
void test_utf_iter2();
void test_cptr();
void test_endian();
void test_toy_format();
void test_toy_format_preformance();
void test_to_chars();
void test_iconv();
void test_logger();
void test_print();
void test_printable();
void test_stoi();
void test_utrim();
void test_upper_lower();
void test_utf_string();
void test_utf_iter_performance();

void test_from_chars();
void test_from_chars_edgecases();

int main(){
  // test_ftoss();
  // test_re2();
  // test_cbuf();
  // test_chars_conv_performance();
  // test_utf_convert();
  // test_utf_iter();
  // test_utf_iter2();
  // test_cptr();
  // test_endian();
  // test_toy_format();
  test_toy_format_preformance();
  // test_to_chars();
  // test_iconv();
  // test_logger();
  // test_print();
  // test_printable();
  // test_stoi();
  // test_utrim();
  // test_upper_lower();
  // test_utf_string();
  // test_utf_iter_performance();
  // test_from_chars();
  // test_from_chars_edgecases();
  // console::writeln("{} {}", 1, 2);
  return 0;
}
