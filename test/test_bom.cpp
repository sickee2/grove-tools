#include <gr/string.hh>

void bom_example() {
  using namespace gr::str;
  using namespace gr::literals;
  // Detect BOM
  u8 utf8_with_bom = bom_utils::make_u8_with_bom("Hello World");
  if (utf8_with_bom.has_bom()) {
    std::cout << "UTF-8 string has BOM" << std::endl;
  }

  // Auto-handle BOM and endianness iterator
  auto iter = utf8_with_bom.ubegin_auto();
  while (iter) {
    std::cout << "Code point: U+" << std::hex << (*iter).code() <<" "<< std::dec << iter.seq_view()
              << std::endl;
    ++iter;
  }

  // Remove BOM
  u8 without_bom = utf8_with_bom.without_bom();

  // Add BOM
  u16 utf16_str = u"Hello"_u16;
  utf16_str.add_bom(gr::endian::big);
  std::cout << utf16_str.to_u8() << std::endl;
}

int main(){
  bom_example();
  return 0;
}
