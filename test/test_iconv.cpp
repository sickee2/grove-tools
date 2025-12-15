#include <chrono>
#include <fstream>
#include <gr/console.hh>
#include <gr/string.hh>
#include <gr/utf_sequence.hh>
#include <iostream>
using namespace std;

namespace gr::str {
template <typename char_type> class uiter {
  const char_type *m_beg = nullptr;
  const char_type *m_end = nullptr;
  const char_type *m_current = nullptr;
  mutable uc::codepoint m_codepoint;
  uint8_t m_seq_len;
  uc::sequence_status m_status;
  uc::on_failed m_failed;
  gr::endian m_endian;

public:
  explicit uiter(const char_type *data, size_t size, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : m_beg(data), m_end(data + size), m_current(data + std::min(pos, size)),
        m_codepoint(0), /*m_seq_len(0), m_status(uc::sequence_status::valid),*/ m_failed(fb),
        m_endian(endian) {

    auto info = uc::sequence::check(m_current, m_end, m_endian);
    m_seq_len = info.length;
    m_status = info.status;
    // gr::console::logl("constuct: len: {} status: {}", info.length, uc::get_status_info(m_status));
    // std::cout << m_seq_len << "  " << uc::get_status_info(m_status) << std::endl;
    // seek_forward();
  }
  explicit uiter(std::basic_string_view<char_type> sv, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : uiter(sv.data(), sv.size(), pos, fb, endian) {}

  uiter(const uiter &) noexcept = default;
  uiter(uiter &&) noexcept = default;
  ~uiter() = default;

  uiter &operator=(const uiter &) noexcept = default;
  uiter &operator=(uiter &&) noexcept = default;

  uc::codepoint& operator*() const {
    if(m_codepoint == 0){
      m_codepoint = uc::sequence::decode(m_current, m_seq_len, m_status);
    }
    return m_codepoint;
  }

  uiter &operator++() {
    seek_forward();
    return *this;
  }

  explicit operator bool() const {
    return m_current >= m_beg && m_current < m_end && m_seq_len > 0;
  }
private:
  void seek_forward() {
    m_codepoint = 0;
    m_current += (m_status == uc::sequence_status::valid) ? m_seq_len : 1;

    // search next sequence
    while (m_current < m_end && m_status != uc::sequence_status::valid) {
      auto res = uc::sequence::check(m_current, m_end, m_endian);

      m_seq_len = res.length;
      m_status = res.status;

      // if(m_status == uc::sequence_status::valid) {
      //   return;
      // }
      switch (m_failed) {
      case uc::on_failed::skip:
        m_current += (m_seq_len > 0 ? m_seq_len : 1);
        continue;
      case uc::on_failed::keep:
        return;
      case uc::on_failed::error:
        throw std::runtime_error("Invalid UTF sequence encountered");
      }
    }

    if (m_current >= m_end) {
      m_status = uc::sequence_status::truncated;
      m_seq_len = 0;
    }
  }
};



inline u8
to_utf8_original(u16v utf16,
                 gr::endian endian_ = gr::endian::native) {
  u8 result(utf16.size() * 3);
  // use gr::str::foreach_utf
  // auto func = [&result](uc::codepoint& cp, uc::sequence_status status){
  //   if(status == uc::sequence_status::valid){
  //     auto chunk = cp.chunk_u8();
  //     result.append(chunk.buf, chunk.size());
  //   }
  //   return true;
  // };
  // gr::str::batch_process_utf<char16_t>(utf16, func, endian_ );

  // auto it = uiter<char16_t>(utf16);
  // for(; it; ++it){
  //   result.append((*it).chunk_u8().view());
  // }

  for (auto it = uc::make_iterator(utf16, 0, uc::on_failed::skip, endian_); it;
       ++it) {
    result.append(it.value().chunk_u8().view());
  }
  return result;
}
} // namespace gr::str

void test_iconv(){
  using namespace gr;
  using namespace std;
  using namespace gr::literals;

  ifstream fi;
  fi.open("gbk.txt", ios::in | ios::binary);
  // std::string content((std::istreambuf_iterator<char>(fi)),
  //                       std::istreambuf_iterator<char>());

  fi.seekg(0, std::ios::end);
  std::streamsize size = fi.tellg();
  str::u8 gbk_str(size, '\0');
  fi.seekg(0, std::ios::beg);
  fi.read(gbk_str.data(), size);
  fi.close();

  std::cout << "orig bytes: " << gbk_str.bytes() << std::endl;

  cout << "\n---------------------------------- convert from gbk" << endl;
  auto utf_str = str::code_converter("utf-32le", "gb18030").transform_as<str::u16>(gbk_str);

  cout << "\n----------------------------------org" << endl;
  auto start1 = chrono::high_resolution_clock::now();
  // auto utfxx_2_u8_str = utf_str.to_u8();
  auto utf8_str1 = gr::str::to_utf8_original(utf_str.as_view());
  auto end1 = chrono::high_resolution_clock::now();
  cout << utf8_str1 << endl;

  cout << "\n----------------------------------iconv" << endl;
  auto start2 = chrono::high_resolution_clock::now();
  auto utf8_str2 = str::code_converter("utf-8", "utf-32le").transform_as<str::u8>(utf_str);
  auto end2 = chrono::high_resolution_clock::now();
  cout << utf8_str2 << endl;

  cout << "\n----------------------------------opt" << endl;
  auto start3 = chrono::high_resolution_clock::now();
  auto utf8_str3 = utf_str.to_u8();
  auto end3 = chrono::high_resolution_clock::now();
  cout << utf8_str3 << endl;

  auto duration1 = chrono::duration_cast<chrono::microseconds>(end1 - start1);
  auto duration2 = chrono::duration_cast<chrono::microseconds>(end2 - start2);
  auto duration3 = chrono::duration_cast<chrono::microseconds>(end3 - start3);

  cout << "\ngr::str::to_utf8_original: "<< duration1 << "\niconv: " << duration2  << "\ngr::str::to_utf8: " << duration3 << endl;
}

