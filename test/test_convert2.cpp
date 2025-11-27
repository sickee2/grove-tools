#include <gr/utf_iter.hh>
#include <chrono>
#include <gr/console.hh>
#include <gr/string.hh>
#include <gr/utf_sequence.hh>
#include <random>
using namespace std;

namespace gr::str {
template <typename char_type> class xiter {
  const char_type *m_beg = nullptr;
  const char_type *m_end = nullptr;
  const char_type *m_current = nullptr;
  mutable uc::codepoint m_codepoint{0};
  uint8_t m_seq_len = 0;
  uc::sequence_status m_status = uc::sequence_status::valid;
  uc::on_failed m_failed = uc::on_failed::skip;
  gr::endian m_endian = gr::endian::native;

public:
  explicit xiter(const char_type *data, size_t size, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : m_beg(data), m_end(data + size), m_current(data + std::min(pos, size)),
        m_codepoint(0), /*m_seq_len(0), m_status(uc::sequence_status::valid),*/ m_failed(fb),
        m_endian(endian) {

    auto info = uc::sequence<char_type>::check(m_current, m_end, m_endian);
    m_seq_len = info.length;
    m_status = info.status;
  }
  explicit xiter(std::basic_string_view<char_type> sv, size_t pos = 0,
                uc::on_failed fb = uc::on_failed::skip,
                gr::endian endian = gr::endian::native) noexcept
      : xiter(sv.data(), sv.size(), pos, fb, endian) {}

  xiter(const xiter &) noexcept = default;
  xiter(xiter &&) noexcept = default;
  ~xiter() = default;

  xiter &operator=(const xiter &) noexcept = default;
  xiter &operator=(xiter &&) noexcept = default;

  uc::codepoint& operator*() const {
    if(m_codepoint == 0){
      m_codepoint = uc::sequence<char_type>::decode(m_current, m_seq_len, m_status, m_endian);
    }
    return m_codepoint;
  }

  xiter &operator++() {
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
      auto res = uc::sequence<char_type>::check(m_current, m_end, m_endian);

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
};


// Generate test data
std::vector<char16_t> generate_test_data(size_t count) {
  std::vector<char16_t> data;
  data.reserve(count);

  std::random_device rd;
  std::mt19937 gen(rd());

  // Generate various characters including ASCII, BMP characters, and surrogate pairs
  std::uniform_int_distribution<char16_t> dist_ascii(0x0020, 0x007F);
  std::uniform_int_distribution<char16_t> dist_bmp(0x0080, 0xD7FF);
  std::uniform_int_distribution<char16_t> dist_high_surrogate(0xD800, 0xDBFF);
  std::uniform_int_distribution<char16_t> dist_low_surrogate(0xDC00, 0xDFFF);

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

inline u8
to_utf8_batch(u16v utf16,
                 gr::endian endian_ = gr::endian::native) {
  u8 result(utf16.size() * 3);
  auto func = [&result](uc::codepoint& cp, uc::sequence_status status){
    if(status == uc::sequence_status::valid){
      auto chunk = cp.chunk_u8();
      result.append(chunk.buf, chunk.size());
    }
    return true;
  };
  gr::str::batch_process_utf<char16_t>(utf16, func, endian_ );
  return result;
}

inline u8
to_utf8_uc_iter(u16v utf16,
                 gr::endian endian_ = gr::endian::native) {
  u8 result(utf16.size() * 3);
  for (auto it = uc::make_iterator(utf16, 0, uc::on_failed::skip, endian_); it;
       ++it) {
    result.append(it.to_u8().view());
  }
  return result;
}

inline u8 to_utf8_xiter(u16v utf16, gr::endian endian_ = gr::endian::native){
  u8 result(utf16.size() * 3);
  auto it = xiter<char16_t>(utf16, 0, uc::on_failed::skip, endian_);
  for(; it; ++it){
    result.append((*it).chunk_u8().view());
  }
  return result;
}

} // namespace gr::str

class Timer{
  std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> _start;
  std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> _end;
  gr::str::u8 _desc;
public:
  Timer(const char* desc) : _desc(desc){
    _start = std::chrono::high_resolution_clock::now();
  }
  void stop() {
    _end = std::chrono::high_resolution_clock::now();
  }
  void log() const {
  gr::console::writeln("{:>15} ==> {:.4U} us", _desc, _end - _start);
  }
};

int main() {
  using namespace gr;
  using namespace std;
  using namespace gr::literals;

//   auto u16 = str::u16v(u"中文测试\n\
// 1. **参数文档**：在函数声明中添加注释说明 `buffer_size` 的单位\n\
// 2. **边界检查**：确保 `buffer_size/sizeof(out_type)` 不会产生除零错误\n\
// 3. **默认值调整**：1024字节对于某些编码可能偏小\n\
// **下一步建议**：现在 `code_convertor` 的实现看起来正确且一致了。您希望我分析其他部分的功能，还是测试这个转换器的实际使用效果\n\
// 1. **参数重命名**：`chunk_size` → `buffer_size`，更清晰地表明这是缓冲区字节大小\n\
// 2. **统一单位**：现在所有相关计算都基于字节单位\n\
// 3. **简化逻辑**：移除了不必要的中间变量");

  auto data = gr::str::generate_test_data(50);
  auto u16 = gr::str::u16v(data.data(), data.size());
  str::u8 result1;

  Timer t1("xiter");
  auto xit = gr::str::xiter<char16_t>(u16);
  for(;xit;++xit){
    result1.append((*xit).chunk_u8().view());
  }
  t1.stop();

  // console::outln(result1);
  str::u8 result2;

  Timer t2("uc::iter");
  auto uit = u16.ubegin();
  for(;uit;++uit){
    result2.append(uit.to_u8().view());
  }
  t2.stop();

  Timer t3("str::to_u8");
  str::u8 result3 = u16.to_u8();
  t3.stop();

  t1.log();
  t2.log();
  t3.log();

  if(result2 != result3){
    console::errorln("convert error! result2 != result3");
  } else if(result1 != result2){
    console::errorln("convert error! result1 != result2");
  } else if(result1 != result3){
    console::errorln("convert error! result1 != result3");
  }else{
    console::writeln("convert completed!");
  }
  return 0;
}
