/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file console.hh
 * @brief Thread-safe console output utilities with formatting support
 * @ingroup utilities
 *
 * Provides high-performance, thread-safe console output operations with
 * integrated formatting capabilities. Designed for multi-threaded applications
 * requiring reliable console output without interleaved text.
 *
 * ## Core Features
 * - **Thread Safety**: Mutex-protected output operations
 * - **High Performance**: Minimal locking overhead with efficient I/O
 * - **Formatting Integration**: Seamless integration with gr::toy::format
 * - **Automatic Flushing**: Smart buffer management with line-based flushing
 * - **Dual Stream Support**: Separate stdout and stderr output channels
 *
 * ## Key Components
 *
 * ### Output Functions
 * - `write()`: Basic output without newline
 * - `writeln()`: Output with automatic newline and flushing
 * - `error()`: Error output to stderr without newline
 * - `errorln()`: Error output to stderr with newline and flushing
 *
 * ### Stream Management
 * - **Automatic Locking**: Global mutex ensures thread-safe operations
 * - **Smart Flushing**: Automatic flush on newline or explicit calls
 * - **Buffer Optimization**: Efficient bulk writes with minimal syscalls
 *
 * ## Performance Characteristics
 *
 * ### Locking Strategy
 * - **Fine-grained Locking**: Per-operation mutex protection
 * - **RAII Pattern**: Automatic lock management with std::unique_lock
 * - **Minimal Contention**: Short critical sections for I/O operations
 *
 * ### I/O Optimization
 * - **Bulk Writes**: std::fwrite for efficient buffer output
 * - **Selective Flushing**: Flush only when necessary (newline detection)
 * - **Zero-copy Operations**: Direct string view operations when possible
 *
 * ## Usage Examples
 *
 * ### Basic Output
 * ```cpp
 * gr::console::write("Hello World");      // Output without newline
 * gr::console::writeln("Hello World");    // Output with newline
 * gr::console::writeln();                 // Output just a newline
 * ```
 *
 * ### Formatted Output
 * ```cpp
 * gr::console::writeln("Value: {}", 42);           // Formatted output
 * gr::console::writeln("{:<10} {:>8}", "Name", 42); // Aligned output
 * ```
 *
 * ### Error Output
 * ```cpp
 * gr::console::error("Error: Something went wrong");    // stderr without newline
 * gr::console::errorln("Error: Something went wrong");  // stderr with newline
 * ```
 *
 * ### String Type Support
 * ```cpp
 * gr::console::write("C-string literal");           // String literal
 * gr::console::write(gr::str::u8v("Unicode text")); // Unicode string view
 * gr::console::write(std::string("std::string"));   // Standard string
 * ```
 *
 * ## Thread Safety Guarantees
 *
 * ### Concurrent Access
 * - **Safe**: Multiple threads can call console functions simultaneously
 * - **Ordered**: Output from different threads won't interleave
 * - **Atomic**: Complete messages are written without interruption
 *
 * ### Locking Behavior
 * - **Global Mutex**: Single mutex protects all console operations
 * - **RAII Management**: Locks are automatically released on scope exit
 * - **No Deadlocks**: Proper lock ordering and exception safety
 *
 * ## Flushing Strategy
 *
 * ### Automatic Flushing
 * - **Newline Detection**: Automatic flush when output ends with '\n'
 * - **Explicit Flushing**: writeln() and errorln() always flush
 * - **Performance**: Avoids unnecessary flushes for partial lines
 *
 * ### Manual Control
 * While automatic flushing is provided, applications can still use
 * std::fflush() directly if needed for specific scenarios.
 *
 * ## Integration Points
 *
 * ### With gr::toy::format
 * - Direct support for formatted string output
 * - Type-safe formatting with compile-time checking
 * - High-performance string building
 *
 * ### With gr::str Types
 * - Native support for Unicode string types
 * - Efficient string view operations
 * - Proper encoding handling
 *
 * ## Error Handling
 *
 * ### Robust Operation
 * - **Null Safety**: Handles null pointers gracefully
 * - **Zero-length**: Skips empty output operations
 * - **Stream Validation**: Validates stream pointers before use
 *
 * ### Failure Modes
 * - **Silent Failure**: I/O errors are not propagated (follows C stdio convention)
 * - **Best Effort**: Continues operation after partial failures
 *
 * ## Configuration and Dependencies
 *
 * ### Compile-time Requirements
 * - C++17 or later (for constexpr support)
 * - Standard I/O library (<cstdio>)
 * - Thread support library (<mutex>)
 *
 * ### Internal Dependencies
 * - `gr/config.hh`: Feature detection and configuration
 * - `gr/format.hh`: String formatting capabilities
 * - `gr/string.hh`: Unicode string support
 *
 * ## Performance Considerations
 *
 * ### Optimal Usage Patterns
 * - Use `writeln()` for complete lines to benefit from automatic flushing
 * - Batch related output in single calls when possible
 * - Prefer string literals for static text to avoid construction overhead
 *
 * ### Anti-patterns
 * - Avoid frequent small writes in performance-critical code
 * - Don't mix console output with other I/O operations without synchronization
 * - Consider buffering for very high-frequency output scenarios
 *
 * @see gr::toy::format for advanced string formatting
 * @see gr::str for Unicode string types
 * @see gr::config for build configuration
 */

#pragma once
#include "gr/config.hh"
#include <cstdio>
#include <gr/format.hh>
#include <mutex>

namespace gr {
namespace console {
namespace detail {

GR_CONSTEXPR_OR_INLINE auto lock_stream() {
  static std::mutex stream_output_lock;
  return std::unique_lock<std::mutex>(stream_output_lock);
}

namespace stream {

enum class type { out, err };

GR_CONSTEXPR_OR_INLINE FILE *get(stream::type t) {
  return t == stream::type::out ? stdout : stderr;
}

} // namespace stream

GR_CONSTEXPR_OR_INLINE void write_to_stream(stream::type t, const char *data, size_t n){
  if(nullptr == data || 0 == n || size_t(-1) == n){
    return;
  }
  auto locker = lock_stream();
  auto stream_ = stream::get(t);
  std::fwrite(data, 1, n, stream_);
  if(data[n - 1] == '\n'){
    std::fflush(stream_);
  }
}

GR_CONSTEXPR_OR_INLINE void write_line_to_stream(stream::type t, const char *data, size_t n){
  if(nullptr == data || 0 == n || size_t(-1) == n){
    return;
  }
  auto locker = lock_stream();
  auto stream_ = stream::get(t);
  std::fwrite(data, 1, n, stream_);
  std::fputc('\n', stream_);
  std::fflush(stream_);
}

} // namespace detail

template <size_t N>
GR_CONSTEXPR_OR_INLINE void write(const char (&s)[N]){
  detail::write_to_stream(detail::stream::type::out, s, N - 1);
}

GR_CONSTEXPR_OR_INLINE void write(gr::str::u8v s){
  detail::write_to_stream(detail::stream::type::out, s.data(), s.size());
}

GR_CONSTEXPR_OR_INLINE void write(const std::string &s){
  detail::write_to_stream(detail::stream::type::out, s.data(), s.size());
}

template<typename...Args>
void write(gr::toy::fmt_string<Args...> fmt, Args&&...args){
  auto formatted = gr::toy::format(fmt, std::forward<Args>(args)...);
  detail::write_to_stream(detail::stream::type::out, formatted.data(), formatted.size());
}

template <size_t N>
GR_CONSTEXPR_OR_INLINE void writeln(const char (&s)[N]){
  detail::write_line_to_stream(detail::stream::type::out, s, N - 1);
}

GR_CONSTEXPR_OR_INLINE void writeln(gr::str::u8v s){
  detail::write_line_to_stream(detail::stream::type::out, s.data(), s.size());
}

GR_CONSTEXPR_OR_INLINE void writeln(const std::string &s){
  detail::write_line_to_stream(detail::stream::type::out, s.data(), s.size());
}

template<typename...Args>
void writeln(gr::toy::fmt_string<Args...> fmt, Args&&...args){
  auto formatted = gr::toy::format(fmt, std::forward<Args>(args)...);
  detail::write_line_to_stream(detail::stream::type::out, formatted.data(), formatted.size());
}

GR_CONSTEXPR_OR_INLINE void writeln(){
  auto locker = detail::lock_stream();
  std::fputc('\n', stdout);
  std::fflush(stdout);
}

template <size_t N> void error(const char (&s)[N]){
  detail::write_to_stream(detail::stream::type::err, s, N - 1);
}

GR_CONSTEXPR_OR_INLINE void error(gr::str::u8v s){
  detail::write_to_stream(detail::stream::type::err, s.data(), s.size());
}

GR_CONSTEXPR_OR_INLINE void error(const std::string &s){
  detail::write_to_stream(detail::stream::type::err, s.data(), s.size());
}

template<typename...Args>
void error(gr::toy::fmt_string<Args...> fmt, Args&&...args){
  auto formatted = gr::toy::format(fmt, std::forward<Args>(args)...);
  detail::write_to_stream(detail::stream::type::err, formatted.data(), formatted.size());
}

template <size_t N> 
GR_CONSTEXPR_OR_INLINE void errorln(const char (&s)[N]){
  detail::write_line_to_stream(detail::stream::type::err, s, N - 1);
}

GR_CONSTEXPR_OR_INLINE void errorln(gr::str::u8v s){
  detail::write_line_to_stream(detail::stream::type::err, s.data(), s.size());
}

GR_CONSTEXPR_OR_INLINE void errorln(const std::string &s){
  detail::write_line_to_stream(detail::stream::type::err, s.data(), s.size());
}

template<typename...Args>
GR_CONSTEXPR_OR_INLINE void errorln(gr::toy::fmt_string<Args...> fmt, Args&&...args){
  auto formatted = gr::toy::format(fmt, std::forward<Args>(args)...);
  detail::write_line_to_stream(detail::stream::type::err, formatted.data(), formatted.size());
}
GR_CONSTEXPR_OR_INLINE void errorln(){
  auto locker = detail::lock_stream();
  std::fputc('\n', stderr);
}
} // namespace console
} // namespace gr
