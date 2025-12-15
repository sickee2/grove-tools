module;
#include <gr/string.hh>
export module gr.str;
export namespace gr{
namespace str{
namespace bom{
using ::gr::str::bom::info;
using ::gr::str::bom::detect;
}
using ::gr::str::length;
using ::gr::str::utf_view;
using ::gr::str::utf;
using ::gr::str::batch_check_utf;
using ::gr::str::batch_process_utf;
#ifndef DISABLE_SUPPORT_ICONV
using ::gr::str::code_converter;
#endif
using ::gr::str::make_u8v;
using ::gr::str::make_u8;
using ::gr::str::to_utf8;
using ::gr::str::to_utf16;
using ::gr::str::to_utf32;
namespace bom_utils{
using ::gr::str::bom_utils::make_u8_with_bom;
using ::gr::str::bom_utils::make_u16_with_bom;
using ::gr::str::bom_utils::make_u32_with_bom;
}
// using u8 = gr::str::utf<char>;
// using u16 = gr::str::utf<char16_t>;
// using u32 = gr::str::utf<char32_t>;
// using u8v = gr::str::utf_view<char>;
// using u16v = gr::str::utf_view<char16_t>;
// using u32v = gr::str::utf_view<char32_t>;
using ::gr::str::u8;
using ::gr::str::u16;
using ::gr::str::u32;
using ::gr::str::u8v;
using ::gr::str::u16v;
using ::gr::str::u32v;
}
namespace literals{
using ::gr::literals::operator""_u8;
using ::gr::literals::operator""_u8v;
using ::gr::literals::operator""_u16;
using ::gr::literals::operator""_u16v;
using ::gr::literals::operator""_u32;
using ::gr::literals::operator""_u32v;
}
}
