#include <gr/format.hh>
#include <gr/logger.hh>
#include <gr/string.hh>

int main() {
  using namespace gr;
  using namespace gr::literals;

  gr::log::init_logger("sickee");
  auto logger = gr::log::get_default_logger();


  // 设置日志级别
  logger->set_level(gr::log::level::trace);
  // 添加文件输出
  logger->add_file_sink("app.log");

  // 使用日志 - 完全支持 Unicode
  GR_TRACE("这是一个跟踪消息: {}", 42);
  GR_DEBUG("调试信息: {}", "hello world");
  GR_INFO("用户 {} 登录成功", "张三");
  GR_WARN("内存使用率: {:.1f}%", 85.5);
  GR_ERROR("数据库连接失败: {}", "连接超时");
  GR_FATAL("内部错误: {}", "运行出错");

  // 使用宏
  GR_INFO("使用宏记录: 计数 = {}", 100);

  // 创建不同名称的日志记录器
  auto network_logger = gr::log::logger_manager::get_logger("network");
  network_logger->info("网络连接已建立");

  return 0;
}
