module;
#include <gr/logger.hh>
export module gr.logger;

export namespace gr {
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
} // namespace gr
