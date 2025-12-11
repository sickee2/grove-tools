#include <gr/utf_iter.hh>
#include <gr/console.hh>
#include <gr/string.hh>
#include <gr/utf_sequence.hh>
#include <gr/performance_timer.hh>
#include <random>
using namespace std;

namespace gr::str {
template <typename char_type> class iter {
  const char_type *m_beg = nullptr;
  const char_type *m_end = nullptr;
  const char_type *m_current = nullptr;
  mutable uc::codepoint m_codepoint{0};
  uint8_t m_seq_len = 0;
  uc::sequence_status m_status = uc::sequence_status::valid;
  uc::on_failed m_failed = uc::on_failed::skip;
  gr::endian m_endian = gr::endian::native;

public:
  explicit iter(const char_type *data, size_t size, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : m_beg(data), m_end(data + size), m_current(data + std::min(pos, size)),
        m_codepoint(0), /*m_seq_len(0), m_status(uc::sequence_status::valid),*/ m_failed(fb),
        m_endian(endian) {

    auto info = uc::sequence::check(m_current, m_end, m_endian);
    m_seq_len = info.length;
    m_status = info.status;
  }
  explicit iter(std::basic_string_view<char_type> sv, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : iter(sv.data(), sv.size(), pos, fb, endian) {}

  iter(const iter &) noexcept = default;
  iter(iter &&) noexcept = default;
  ~iter() = default;

  iter &operator=(const iter &) noexcept = default;
  iter &operator=(iter &&) noexcept = default;

  uc::codepoint& operator*() const {
    if(m_codepoint == 0){
      m_codepoint = uc::sequence::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return m_codepoint;
  }

  uc::codepoint *operator->() {
    if(m_codepoint == 0){
      m_codepoint = uc::sequence::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return &m_codepoint;
  }

  iter &operator++() {
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
    while (m_current < m_end) {
      auto res = uc::sequence::check(m_current, m_end, m_endian);

      m_seq_len = res.length;
      m_status = res.status;

      if (m_status == uc::sequence_status::valid || m_failed == uc::on_failed::keep) {
        return;
      }
      if(m_failed == uc::on_failed::skip){ 
        m_current += (res.length > 0 ? res.length : 1);
        if(m_current >= m_end){
          m_status = uc::sequence_status::truncated;
          m_seq_len = 0;
          return;
        }
        continue;
      }else if(m_failed == uc::on_failed::error){
          throw std::runtime_error("Invalid UTF sequence encountered");
      }
    }
    if (m_current >= m_end) {
      m_status = uc::sequence_status::truncated;
      m_seq_len = 0;
    }
  }
  void _seek_valid_backward() noexcept {
    m_codepoint = 0;
    if (m_current == m_beg) {
      m_status = uc::sequence_status::truncated;
      m_seq_len = 0;
      return;
    }

    m_current--;

    while (m_current >= m_beg) {
      auto res = uc::sequence::check(m_current, m_end, m_endian);

      m_status = res.status;
      m_seq_len = res.length;

      if (res.status == uc::sequence_status::valid || m_failed == uc::on_failed::keep) {
        return;
      }

      if (m_current == m_beg)
        break;
      m_current--;
    }

    m_status = uc::sequence_status::truncated;
    m_seq_len = 0;
    m_current = m_beg;
  }
};


// Generate test data
std::vector<char16_t> generate_test_data(size_t count) {
  std::vector<char16_t> data;
  data.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());

  // Generate various characters including ASCII, BMP characters, and surrogate pairs
  std::uniform_int_distribution<uint16_t> dist_ascii(0x0020, 0x007F);
  std::uniform_int_distribution<uint16_t> dist_bmp(0x0080, 0xD7FF);
  std::uniform_int_distribution<uint16_t> dist_high_surrogate(0xD800, 0xDBFF);
  std::uniform_int_distribution<uint16_t> dist_low_surrogate(0xDC00, 0xDFFF);

  for (size_t i = 0; i < count; ++i) {
    int type = i % 10;
    if (type < 5) {
      // 50% ASCII
      data.push_back(dist_ascii(gen));
    } else if (type < 8) {
      // 30% BMP characters
      data.push_back(dist_bmp(gen));
    } else {
      // 20% surrogate pairs (requires two characters)
      if (i + 1 < count) {
        data.push_back(dist_high_surrogate(gen));
        data.push_back(dist_low_surrogate(gen));
        ++i;
      } else {
        data.push_back(dist_ascii(gen));
      }
    }
  }

  return data;
}
} // namespace gr::str

void test_utf_iter_performance(){
  using namespace gr;
  using namespace std;
  using namespace gr::literals;

/*
//   auto u16 = str::u16v(u"中文测试\n\
// 1. **参数文档**：在函数声明中添加注释说明 `buffer_size` 的单位\n\
// 2. **边界检查**：确保 `buffer_size/sizeof(out_type)` 不会产生除零错误\n\
// 3. **默认值调整**：1024字节对于某些编码可能偏小\n\
// **下一步建议**：现在 `code_convertor` 的实现看起来正确且一致了。您希望我分析其他部分的功能，还是测试这个转换器的实际使用效果\n\
// 1. **参数重命名**：`chunk_size` → `buffer_size`，更清晰地表明这是缓冲区字节大小\n\
// 2. **统一单位**：现在所有相关计算都基于字节单位\n\
// 3. **简化逻辑**：移除了不必要的中间变量");
*/

  auto data = gr::str::generate_test_data(5000);
  auto u16 = gr::str::u16v(data.data(), data.size());
  str::u8 result1;

  {
    PerformanceTimer t1("xiter");
    auto xit = gr::str::iter<char16_t>(u16);
    for(;xit;++xit){
      result1.append((*xit).chunk_u8().view());
    }
  }

  str::u8 result2;

  {
    PerformanceTimer t2("uc::iter::value().chunk_u8()");
    auto uit = u16.ubegin();
    for(;uit;++uit){
      result2.append(uit.value().chunk_u8().view());
    }
  }
  str::u8 result_x;

  {
    PerformanceTimer t2("uc::iter->chunk_u8()");
    auto uit = u16.ubegin();
    for(;uit;++uit){
      result_x.append(uit->chunk_u8().view());
    }
  }
  str::u8 result_x2;

  {
    PerformanceTimer t2("(*uc::iter)::chunk_u8");
    auto uit = u16.ubegin();
    for(;uit;++uit){
      result_x2.append((*uit).chunk_u8().view());
    }
  }

  str::u8 result3;
  {
    PerformanceTimer t3("str::to_u8");
    result3 = u16.to_u8();
  }


  if(result2 != result3){
    console::errorln("convert error! result2 != result3");
  } else if(result1 != result2){
    console::errorln("convert error! result1 != result2");
  } else if(result1 != result3){
    console::errorln("convert error! result1 != result3");
  }else{
    console::writeln("convert completed!");
  }
}
