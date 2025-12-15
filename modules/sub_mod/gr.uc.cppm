module;
#include <gr/utf_sequence.hh>
#include <gr/utf_iter.hh>
export module gr.uc;
export namespace gr{
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
}
