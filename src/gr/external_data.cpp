#include <gr/logger.hh>

namespace gr::log{
std::unordered_map<std::string, std::shared_ptr<Logger>> logger_manager::loggers_;
std::mutex logger_manager::mutex_;
std::shared_ptr<Logger> logger_manager::default_logger_ = std::make_shared<Logger>();
}

namespace gr::toy::detail{

const uint64_t POW10_TABLE[19] = {1,
                          10,
                          100,
                          1000,
                          10000,
                          100000,
                          1000000,
                          10000000,
                          100000000,
                          1000000000,
                          10000000000,
                          100000000000,
                          1000000000000,
                          10000000000000,
                          100000000000000,
                          1000000000000000,
                          10000000000000000,
                          100000000000000000,
                          1000000000000000000};

}
//
// gr::toy::detail::POW10_TABLE = gr::toy::detail::POW10_TABLE_data;
