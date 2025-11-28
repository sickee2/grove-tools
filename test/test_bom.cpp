#include <gr/string.hh>
#include <gr/console.hh>

void bom_example() {
  using namespace gr::str;
  using namespace gr::literals;
  // Detect BOM
  u8 utf8_with_bom = bom_utils::make_u8_with_bom("Hello World");
  if (utf8_with_bom.has_bom()) {
    gr::console::writeln("UTF-8 string has BOM");
  }

  // Auto-handle BOM and endianness iterator
  auto iter = utf8_with_bom.ubegin_auto();
  while (iter) {
    gr::console::writeln("Code point: U+{:x} <=> {}", (*iter).code(),  iter.seq_view());
    ++iter;
  }

  // Remove BOM
  u8 without_bom = utf8_with_bom.without_bom();

  // Add BOM
  u16 utf16_str = u"Hello"_u16;
  utf16_str.add_bom(gr::endian::big);
  gr::console::writeln(utf16_str.to_u8());
}

int main(){
  bom_example();
  return 0;
}
