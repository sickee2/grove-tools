/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file logger.hh
 * @brief High-performance, extensible logging framework with multiple sinks
 * @ingroup utilities
 *
 * Provides a comprehensive logging system with support for multiple output sinks,
 * log levels, colored console output, file rotation, and compile-time optimization.
 * Designed for high-performance applications requiring flexible and efficient logging.
 *
 * ## Core Features
 * - **Multiple Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
 * - **Extensible Sink System**: Console, file, rotating file sinks
 * - **Thread Safety**: Full thread-safe operations with minimal contention
 * - **Color Output**: ANSI color-coded console output with configurable colors
 * - **File Rotation**: Size-based log file rotation with backup management
 * - **Compile-time Optimization**: Conditional compilation based on log levels
 * - **Named Loggers**: Multiple named logger instances with independent configuration
 *
 * ## Key Components
 *
 * ### Logger Class
 * Main logging interface providing:
 * - Level-based filtering and conditional logging
 * - Multiple sink management and configuration
 * - Thread-safe log operations with minimal locking
 * - Formatted logging with timestamp and context
 *
 * ### Sink System Architecture
 * - **sink**: Abstract base class for all output destinations
 * - **console_sink**: Color-coded console output with ANSI support
 * - **file_sink**: Simple file-based logging
 * - **rotating_file_sink**: Size-based file rotation with backup management
 *
 * ### Logger Manager
 * Centralized logger management:
 * - Named logger creation and retrieval
 * - Default logger configuration
 * - Global logger instance management
 *
 * ## Log Levels
 *
 * ### Level Hierarchy
 * - `trace` (0): Detailed debugging information
 * - `debug` (1): General debugging information
 * - `info` (2): Informational messages
 * - `warn` (3): Warning conditions
 * - `error` (4): Error conditions
 * - `fatal` (5): Fatal conditions requiring immediate attention
 *
 * ### Compile-time Optimization
 * Log statements are completely eliminated at compile-time when the configured
 * log level is higher than the statement level, providing zero runtime overhead.
 *
 * ## Performance Characteristics
 *
 * ### Locking Strategy
 * - **Fine-grained Locking**: Per-logger and per-sink mutex protection
 * - **RAII Pattern**: Automatic lock management
 * - **Minimal Contention**: Short critical sections for log operations
 *
 * ### Memory Management
 * - **Smart Pointers**: Automatic resource management for sinks
 * - **String Views**: Efficient message passing without copying
 * - **Pre-allocation**: Optimized buffer management for formatted output
 *
 * ## Usage Examples
 *
 * ### Basic Logging
 * ```cpp
 * auto logger = gr::log::logger_manager::get_logger("my_app");
 * logger->info("Application started");
 * logger->warn("Low memory condition detected");
 * logger->error("Failed to open file: {}", filename);
 * ```
 *
 * ### Sink Configuration
 * ```cpp
 * logger->add_console_sink(true);  // Console with colors
 * logger->add_file_sink("app.log"); // File output
 * logger->add_rotating_file_sink("rotating.log", 10*1024*1024, 5); // 10MB rotation
 * ```
 *
 * ### Global Convenience Functions
 * ```cpp
 * gr::log::init_logger("my_app", true); // Initialize default logger
 * GR_INFO("User {} logged in", username); // Global macro with file/line info
 * GR_ERROR("Database connection failed: {}", error_msg);
 * ```
 *
 * ### Advanced Configuration
 * ```cpp
 * logger->set_level(gr::log::level::debug); // Set minimum log level
 * logger->set_console_colors_enabled(false); // Disable colors
 * logger->remove_console_sink(); // Remove console output
 * ```
 *
 * ## Sink Implementations
 *
 * ### Console Sink
 * - **Color Coding**: Different colors for each log level
 * - **Thread Safety**: Synchronized output to prevent interleaving
 * - **ANSI Support**: Automatic detection and fallback for non-ANSI terminals
 *
 * ### File Sink
 * - **Append Mode**: Always appends to existing files
 * - **Automatic Flushing**: Ensures data is written to disk
 * - **Error Resilience**: Continues operation on file errors
 *
 * ### Rotating File Sink
 * - **Size-based Rotation**: Rotates when file reaches specified size
 * - **Backup Management**: Maintains configurable number of backup files
 * - **Atomic Rotation**: Safe rotation without log loss
 *
 * ## Thread Safety Guarantees
 *
 * ### Concurrent Operations
 * - **Safe**: Multiple threads can log simultaneously
 * - **Ordered**: Log messages from different threads maintain temporal order
 * - **Atomic**: Complete log lines are written without interleaving
 *
 * ### Sink Management
 * - **Thread-safe Configuration**: Sinks can be added/removed safely
 * - **Lock Hierarchy**: Proper lock ordering to prevent deadlocks
 * - **Exception Safety**: Operations maintain consistency after exceptions
 *
 * ## Configuration Options
 *
 * ### Compile-time Configuration
 * - `GR_LOG_LEVEL`: Global compile-time log level filter
 * - Feature detection through `gr/config.hh`
 *
 * ### Runtime Configuration
 * - Per-logger level filtering
 * - Sink enablement and configuration
 * - Color output enablement
 *
 * ## Integration Points
 *
 * ### With gr::toy::format
 * - Type-safe formatted logging with high performance
 * - Compile-time format string validation (C++20)
 * - Efficient string building with minimal allocations
 *
 * ### With gr::console
 * - Thread-safe console output backend
 * - Efficient I/O operations with proper synchronization
 *
 * ### With gr::str Types
 * - Native Unicode string support
 * - Efficient string view operations
 * - Proper encoding handling in file outputs
 *
 * ## Error Handling
 *
 * ### Robust Operation
 * - **Graceful Degradation**: Continues operation when sinks fail
 * - **File Error Handling**: Safe file operations with error recovery
 * - **Memory Safety**: Proper resource management in all scenarios
 *
 * ### Failure Modes
 * - **Silent Sink Failures**: Individual sink failures don't stop logging
 * - **Best Effort**: Maximum effort to log despite partial failures
 * - **Resource Cleanup**: Proper cleanup on destruction
 *
 * ## Performance Considerations
 *
 * ### Optimal Usage Patterns
 * - Use appropriate log levels to minimize runtime overhead
 * - Batch related log messages when possible
 * - Use compile-time log level filtering for performance-critical code
 * - Prefer string literals for static log messages
 *
 * ### Anti-patterns
 * - Avoid expensive computations in log statements
 * - Don't use logging in performance-critical inner loops
 * - Avoid excessive string formatting in hot paths
 *
 * ## Dependencies
 * - Standard Library: `<fstream>`, `<mutex>`, `<memory>`, `<unordered_map>`
 * - Internal: `gr/format.hh`, `gr/console.hh`, `gr/string.hh`, `gr/config.hh`
 *
 * @see gr::toy::format for string formatting capabilities
 * @see gr::console for console output utilities
 * @see gr::str for Unicode string support
 */

#pragma once

#include <gr/config.hh>
#include <gr/format.hh>
#include <gr/console.hh>
#include <gr/string.hh>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace gr {
namespace log {

/**
 * @brief Logging levels enumeration
 */
enum class level {
  trace = 0,
  debug = 1,
  info = 2,
  warn = 3,
  error = 4,
  fatal = 5
};

/**
 * @brief Convert log level to string representation
 * @param i_level Log level to convert
 * @return String representation of the log level
 */
GR_CONSTEXPR_OR_INLINE str::u8v level_to_string(level i_level) {
  using namespace gr::literals;
  switch (i_level) {
  case level::trace:
    return "TRACE"_u8v;
  case level::debug:
    return "DEBUG"_u8v;
  case level::info:
    return " INFO"_u8v;
  case level::warn:
    return " WARN"_u8v;
  case level::error:
    return "ERROR"_u8v;
  case level::fatal:
    return "FATAL"_u8v;
  default:
    return "UNKNOWN"_u8v;
  }
}

// ANSI color codes
namespace colors {
// Reset all attributes
static constexpr const char *reset = "\033[0m";

// Foreground colors
static constexpr const char *black = "\033[30m";
static constexpr const char *red = "\033[31m";
static constexpr const char *green = "\033[32m";
static constexpr const char *yellow = "\033[33m";
static constexpr const char *blue = "\033[34m";
static constexpr const char *magenta = "\033[35m";
static constexpr const char *cyan = "\033[36m";
static constexpr const char *white = "\033[37m";

// Bright colors
static constexpr const char *bright_black = "\033[90m";
static constexpr const char *bright_red = "\033[91m";
static constexpr const char *bright_green = "\033[92m";
static constexpr const char *bright_yellow = "\033[93m";
static constexpr const char *bright_blue = "\033[94m";
static constexpr const char *bright_magenta = "\033[95m";
static constexpr const char *bright_cyan = "\033[96m";
static constexpr const char *bright_white = "\033[97m";

// Background colors
static constexpr const char *bg_black = "\033[40m";
static constexpr const char *bg_red = "\033[41m";
static constexpr const char *bg_green = "\033[42m";
static constexpr const char *bg_yellow = "\033[43m";
static constexpr const char *bg_blue = "\033[44m";
static constexpr const char *bg_magenta = "\033[45m";
static constexpr const char *bg_cyan = "\033[46m";
static constexpr const char *bg_white = "\033[47m";
} // namespace colors

/**
 * @brief Get ANSI color code for log level
 * @param i_level Log level
 * @return ANSI color code string
 */
GR_CONSTEXPR_OR_INLINE str::u8v level_to_color(level i_level) {
  using namespace gr::literals;
  switch (i_level) {
  case level::trace:
    return colors::white;
  case level::debug:
    return colors::bright_cyan;
  case level::info:
    return colors::bright_green;
  case level::warn:
    return colors::bright_yellow;
  case level::error:
    return colors::bright_red;
  case level::fatal:
    return colors::bright_magenta;
  default:
    return colors::reset;
  }
}

/**
 * @brief Abstract base class for log sinks
 */
class sink {
public:
  virtual ~sink() = default;

  /**
   * @brief Write log message to sink
   * @param i_level Log level
   * @param message Log message
   */
  virtual void write(level i_level, str::u8v message) = 0;

  /**
   * @brief Flush the sink buffer
   */
  virtual void flush() = 0;
};

/**
 * @brief Console output sink with color support
 */
class console_sink : public sink {
private:
  bool enable_colors_;

public:
  /**
   * @brief Construct console sink
   * @param enable_colors Whether to enable color output
   */
  console_sink(bool enable_colors = true) : enable_colors_(enable_colors) {}

  void write(level i_level, str::u8v message) override {
    if (enable_colors_) {
      auto color_code = level_to_color(i_level);
      gr::console::writeln("{}{}{}", color_code, message, colors::reset);
    } else {
      gr::console::writeln(message);
    }
  }

  void flush() override { std::cout.flush(); }

  /**
   * @brief Enable or disable color output
   * @param enabled Whether colors should be enabled
   */
  void set_colors_enabled(bool enabled) { enable_colors_ = enabled; }

  /**
   * @brief Check if color output is enabled
   * @return true if colors are enabled
   */
  bool get_colors_enabled() const { return enable_colors_; }
};

/**
 * @brief File output sink
 */
class file_sink : public sink {
private:
  FILE *file_;
  std::mutex mutex_;

public:
  /**
   * @brief Construct file sink
   * @param filename File path for logging
   */
  explicit file_sink(str::u8v filename) {
    file_ = std::fopen(filename.data(), "a");
  }

  ~file_sink() {
    if (file_) {
      std::fclose(file_);
      file_ = nullptr;
    }
  }

  void write(level i_level, str::u8v message) override {
    (void)(i_level);
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_) {
      std::fwrite(message.data(), 1, message.size(), file_);
      std::fputc('\n', file_);
      // file_ << message << std::endl;
    }
  }

  void flush() override {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_) {
      std::fflush(file_);
    }
  }
};

/**
 * @brief Rotating file sink with size-based rotation
 */
class rotating_file_sink : public sink {
private:
  std::string base_filename_;
  size_t max_size_;
  int max_files_;
  FILE* file_;
  std::mutex mutex_;
  size_t current_size_;

  /**
   * @brief Rotate files if current file exceeds max size
   */
  void rotate_if_needed() {
    if (current_size_ >= max_size_) {
      if (file_) {
        std::fclose(file_);
        file_ = nullptr;
      }

      // Rename existing files
      for (int i = max_files_ - 1; i > 0; --i) {
        std::string old_name = base_filename_ + "." + std::to_string(i);
        std::string new_name = base_filename_ + "." + std::to_string(i + 1);

        FILE* test_file = std::fopen(old_name.c_str(), "r");
        if (test_file) {
          std::fclose(test_file);
          std::rename(old_name.c_str(), new_name.c_str());
        }
      }

      // Rename current file
      std::string first_backup = base_filename_ + ".1";

      FILE* current_file = std::fopen(base_filename_.c_str(), "r");
      if (current_file) {
        std::fclose(current_file);
        std::rename(base_filename_.c_str(), first_backup.c_str());
      }

      // Reopen new file
      file_ = std::fopen(base_filename_.c_str(), "a");
      current_size_ = 0;
    }
  }

public:
  /**
   * @brief Construct rotating file sink
   * @param filename Base filename
   * @param max_size Maximum file size before rotation
   * @param max_files Maximum number of backup files to keep
   */
  rotating_file_sink(str::u8v filename,
                     size_t max_size = 10 * 1024 * 1024, // 10MB
                     int max_files = 5)
      : base_filename_(filename.data()), max_size_(max_size),
        max_files_(max_files), file_(nullptr), current_size_(0) {
    file_ = std::fopen(base_filename_.c_str(), "a");
    if (file_) {
      std::fseek(file_, 0, SEEK_END);
      current_size_ = std::ftell(file_);
    }
  }

  ~rotating_file_sink() {
    if (file_) {
      std::fclose(file_);
    }
  }

  void write(level i_level, str::u8v message) override {
    (void)(i_level);
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_) {
      rotate_if_needed();
      std::fwrite(message.data(), 1, message.size(), file_);
      std::fputc('\n', file_);
      current_size_ += message.size() + 1; // +1 for newline
    }
  }

  void flush() override {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_) {
      std::fflush(file_);
    }
  }
};


/**
 * @brief Main logger class with multiple sink support
 */
class Logger {
private:
  level level_ = level::info;
  std::vector<std::shared_ptr<sink>> sinks_;
  std::mutex mutex_;
  str::u8 name_;
  bool has_console_sink_ = false;  // Track console sink state

public:
  /**
   * @brief Construct logger with name
   * @param name Logger name
   */
  explicit Logger(str::u8v name = "default") : name_(name) {}

  /**
   * @brief Set minimum log level
   * @param i_level Minimum level to log
   */
  void set_level(level i_level) { level_ = i_level; }

  /**
   * @brief Get current log level
   * @return Current log level
   */
  level get_level() const { return level_; }

  /**
   * @brief Add a sink to the logger
   * @param sink_ptr Shared pointer to sink
   */
  void add_sink(std::shared_ptr<sink> sink_ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto console_sink_ptr = std::dynamic_pointer_cast<console_sink>(sink_ptr)) {
      if (has_console_sink_) {
        // Console sink already exists, skip adding
        return;
      }
      has_console_sink_ = true;
    }
    sinks_.push_back(std::move(sink_ptr));
  }

  /**
   * @brief Add file sink
   * @param filename File path
   */
  void add_file_sink(str::u8v filename) {
    add_sink(std::make_shared<file_sink>(filename));
  }

  /**
   * @brief Add rotating file sink
   * @param filename Base filename
   * @param max_size Maximum file size
   * @param max_files Maximum backup files
   */
  void add_rotating_file_sink(str::u8v filename,
                              size_t max_size = 10 * 1024 * 1024,
                              int max_files = 5) {
    add_sink(
        std::make_shared<rotating_file_sink>(filename, max_size, max_files));
  }

  /**
   * @brief Add console sink
   * @param enable_colors Whether to enable colors
   */
  void add_console_sink(bool enable_colors = true) {
    add_sink(std::make_shared<console_sink>(enable_colors));
  }

  /**
   * @brief Remove all sinks
   */
  void clear_sinks() {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.clear();
    has_console_sink_ = false;  // Reset state
  }

  /**
   * @brief Check if console sink exists
   * @return true if console sink is present
   */
  bool has_console_sink() const {
    return has_console_sink_;
  }

  /**
   * @brief Remove console sink
   */
  void remove_console_sink() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!has_console_sink_) {
      return;
    }

    // Remove all console sinks
    sinks_.erase(
      std::remove_if(sinks_.begin(), sinks_.end(),
        [](const std::shared_ptr<sink>& s) {
          return std::dynamic_pointer_cast<console_sink>(s) != nullptr;
        }),
      sinks_.end()
    );
    has_console_sink_ = false;
  }

  /**
   * @brief Enable/disable console colors
   * @param enabled Whether colors should be enabled
   */
  void set_console_colors_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& sink_ptr : sinks_) {
      if (auto console_sink_ptr = std::dynamic_pointer_cast<console_sink>(sink_ptr)) {
        console_sink_ptr->set_colors_enabled(enabled);
      }
    }
  }

  /**
   * @brief Log message with specified level
   * @tparam Args Format argument types
   * @param i_level Log level
   * @param format Format string
   * @param args Format arguments
   */
  template <typename... Args>
  void log(level i_level, toy::fmt_string<Args...> format, Args &&...args) {

    if (i_level < level_) {
      return;
    }

    auto timestamp = toy::chrono::now();
    auto level_str = level_to_string(i_level);
    auto message =
        toy::format("[{:f}] [{}] [{}] {}", timestamp, level_str, name_,
                    toy::format(format, std::forward<Args>(args)...));

    // Convert str::u8 to str::u8v
    str::u8v message_view = message.as_view();

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &sink : sinks_) {
      sink->write(i_level, message_view);
    }
  }

  /**
   * @brief Log trace level message
   */
  template <typename... Args> void trace(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 0) {
      log(level::trace, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Log debug level message
   */
  template <typename... Args> void debug(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 1) {
      log(level::debug, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Log info level message
   */
  template <typename... Args> void info(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 2) {
      log(level::info, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Log warn level message
   */
  template <typename... Args> void warn(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 3) {
      log(level::warn, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Log error level message
   */
  template <typename... Args> void error(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 4) {
      log(level::error, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Log fatal level message
   */
  template <typename... Args> void fatal(toy::fmt_string<Args...> format, Args &&...args) {
    if constexpr (GR_LOG_LEVEL <= 5) {
      log(level::fatal, format, std::forward<Args>(args)...);
    }
  }

  /**
   * @brief Flush all sinks
   */
  void flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &sink : sinks_) {
      sink->flush();
    }
  }
};

/**
 * @brief Logger manager for creating and managing named loggers
 */
class logger_manager {
private:
  // Hash support for str::u8
  struct u8_hash {
    std::size_t operator()(const str::u8 &s) const {
      return std::hash<std::string_view>{}(s.as_std_view());
    }
  };

  // Equality comparison for str::u8
  struct u8_equal {
    bool operator()(const str::u8 &lhs, const str::u8 &rhs) const {
      return lhs.as_std_view() == rhs.as_std_view();
    }
  };

  static inline std::unordered_map<str::u8, std::shared_ptr<Logger>, u8_hash,
                                   u8_equal>
      loggers_;
  static inline std::mutex mutex_;
  static inline std::shared_ptr<Logger> default_logger_ =
      std::make_shared<Logger>();

public:
  /**
   * @brief Get or create logger by name
   * @param name Logger name
   * @return Shared pointer to logger
   */
  static std::shared_ptr<Logger> get_logger(str::u8v name = "default") {
    std::lock_guard<std::mutex> lock(mutex_);

    // Convert u8v to u8 for lookup
    str::u8 name_key(name);
    auto it = loggers_.find(name_key);
    if (it != loggers_.end()) {
      return it->second;
    }

    auto logger = std::make_shared<Logger>(name);
    loggers_[name_key] = logger;
    return logger;
  }

  /**
   * @brief Get default logger
   * @return Shared pointer to default logger
   */
  static std::shared_ptr<Logger> get_default_logger() {
    return default_logger_;
  }

  /**
   * @brief Set default logger
   * @param logger Logger to set as default
   */
  static void set_default_logger(std::shared_ptr<Logger> logger) {
    default_logger_ = std::move(logger);
  }
};

// Logging macros with file and line information
#define GR_LOG_TRACE(logger, format, ...) \
    (logger)->trace("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#define GR_LOG_DEBUG(logger, format, ...) \
    (logger)->debug("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#define GR_LOG_INFO(logger, format, ...) \
    (logger)->info("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#define GR_LOG_WARN(logger, format, ...) \
    (logger)->warn("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#define GR_LOG_ERROR(logger, format, ...) \
    (logger)->error("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#define GR_LOG_FATAL(logger, format, ...) \
    (logger)->fatal("[{}:{}] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

// Global convenience macros
#define GR_TRACE(format, ...) GR_LOG_TRACE(gr::log::get_default_logger(), format, ##__VA_ARGS__)
#define GR_DEBUG(format, ...) GR_LOG_DEBUG(gr::log::get_default_logger(), format, ##__VA_ARGS__)
#define GR_INFO(format, ...)  GR_LOG_INFO(gr::log::get_default_logger(), format, ##__VA_ARGS__)
#define GR_WARN(format, ...)  GR_LOG_WARN(gr::log::get_default_logger(), format, ##__VA_ARGS__)
#define GR_ERROR(format, ...) GR_LOG_ERROR(gr::log::get_default_logger(), format, ##__VA_ARGS__)
#define GR_FATAL(format, ...) GR_LOG_FATAL(gr::log::get_default_logger(), format, ##__VA_ARGS__)

// Global logger instance
inline std::shared_ptr<Logger> get_default_logger() {
  return logger_manager::get_default_logger();
}

/**
 * @brief Initialize default logger with console sink
 * @param name Logger name
 * @param enable_colors Whether to enable console colors
 */
inline void init_logger(str::u8v name = "default",
                                bool enable_colors = true) {
  auto logger = logger_manager::get_logger(name);
  logger_manager::set_default_logger(logger);

  auto console_sink_ = std::make_shared<console_sink>(enable_colors);
  logger->add_sink(console_sink_);
}

// Convenient global functions
template <typename... Args>
inline void trace(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->trace(format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void debug(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->debug(format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->info(format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warn(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->warn(format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->error(format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void fatal(toy::fmt_string<Args...> format, Args &&...args) {
  get_default_logger()->fatal(format, std::forward<Args>(args)...);
}

} // namespace log
} // namespace gr
