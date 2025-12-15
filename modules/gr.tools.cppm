module;
#include <gr/utils.hh>
#include <gr/utf_sequence.hh>
#include <gr/utf_iter.hh>
#include <gr/string.hh>
#include <gr/detail/toy_charconv.hh>
#include <gr/format.hh>
#include <gr/logger.hh>
#include <gr/console.hh>

export module gr.tools;

export namespace gr{
using ::gr::void_ptr;
namespace utils{
using ::gr::utils::pointer_to_type;
using ::gr::utils::nopos;
using ::gr::utils::cptr;
using ::gr::utils::cbuf;
using ::gr::utils::make_cptr;
using ::gr::utils::build_lps;
using ::gr::utils::build_reverse_lps;
using ::gr::utils::find_mark_kmp;
using ::gr::utils::rfind_mark_kmp;
}
using ::gr::make_cptr;
using ::gr::make_cbuf;


using ::gr::endian;
using ::gr::is_little_endian;
using ::gr::swap_bytes;
using ::gr::convert_endian;

namespace uc{
using ::gr::uc::chunk_proxy;
using ::gr::uc::chunk_proxy8;
using ::gr::uc::chunk_proxy16;
using ::gr::uc::sequence_status;
using ::gr::uc::get_status_info;
using ::gr::uc::on_failed;
using ::gr::uc::codepoint;
using ::gr::uc::sequence_info;
using ::gr::uc::sequence;
using ::gr::uc::iter;
using ::gr::uc::u8iter;
using ::gr::uc::u16iter;
using ::gr::uc::u32iter;
using ::gr::uc::range;
using ::gr::uc::make_iterator;
}

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

namespace toy{
using ::gr::toy::chars_format;
using ::gr::toy::sstov_result;
using ::gr::toy::sstoi;
using ::gr::toy::sstof;
using ::gr::toy::itoss;
using ::gr::toy::ftoss;

using ::gr::toy::format_error;
using ::gr::toy::format_spec;
using ::gr::toy::format_output;
using ::gr::toy::detail::formatter;
using ::gr::toy::fmt_string;
using ::gr::toy::format_to;
using ::gr::toy::format;

namespace chrono{
using ::gr::toy::chrono::now;
}
}

namespace console{
using ::gr::console::write;
using ::gr::console::writeln;
using ::gr::console::error;
using ::gr::console::errorln;
}

namespace log{
using ::gr::log::level;
using ::gr::log::level_to_string;
using ::gr::log::level_to_color;
using ::gr::log::sink;
using ::gr::log::console_sink;
using ::gr::log::file_sink;
using ::gr::log::rotating_file_sink;
using ::gr::log::Logger;
using ::gr::log::logger_manager;
using ::gr::log::get_default_logger;
using ::gr::log::init_logger;
using ::gr::log::trace;
using ::gr::log::debug;
using ::gr::log::info;
using ::gr::log::warn;
using ::gr::log::error;
using ::gr::log::fatal;
}
}
