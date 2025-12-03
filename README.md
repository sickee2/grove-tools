# GR Format Library - High-Performance C++ Formatting and Logging System

## Project Overview

GR Format Library is a modern C++ formatting and logging library focused on extreme performance. The core component `gr::toy::format` significantly outperforms the standard library in benchmark tests, while providing a complete logging system and thread-safe console output.

## Performance Advantages

Based on 100,000 iteration performance tests:

| Operation Type | std::format | fmt::format | toy::format | Performance Improvement |
|---------------|-------------|-------------|-------------|-------------------------|
| Integer Formatting | 5997μs | 2621μs | 2575μs | **133% faster than std::format, 1.8% faster than fmt::format** |
| Floating-point Formatting | 7947μs | 5358μs | 4068μs | **95% faster than std::format, 32% faster than fmt::format** |
| String Formatting | 4039μs | 3234μs | 2063μs | **96% faster than std::format, 57% faster than fmt::format** |
| Pointer Formatting | 1252μs | 1171μs | 561μs | **123% faster than std::format, 109% faster than fmt::format** |
| Overall Throughput | 7.03M ops/sec | 12.98M ops/sec | 13.74M ops/sec | **96% higher than std::format, 6% higher than fmt::format** |
| Memory Allocation | 1574μs | - | 879μs | **79% faster than std::format** |

## Core Components

### 1. High-Performance Formatting Library (`gr/format.hh`)
**String formatting engine that surpasses std::format**

```cpp
#include <gr/format.hh>

// Basic formatting
auto str1 = "Hello {}!"_fmt("World");
auto str2 = gr::toy::format("Value: {:.2f}", 3.14159);

// Advanced formatting
auto str3 = format("{:<10} {:>8}", "Name", 42);    // Alignment
auto str4 = format("{:#x}", 255);                  // Hexadecimal
auto str5 = format("{:.3}", 1.234567);             // Precision control

// Time formatting
auto now = std::chrono::system_clock::now();
auto time_str = format("{:f}", now);               // Full timestamp
```

**Features:**
- Type-safe Python-style formatting syntax
- Compile-time format string checking (C++20)
- Support for custom type formatting
- Built-in chrono time/duration formatting

### 2. Thread-Safe Console Output (`gr/console.hh`)
**High-performance console I/O based on gr::toy::format**

```cpp
#include <gr/console.hh>

// Basic output
gr::console::write("Processing...");
gr::console::writeln("Completed: {} items", count);

// Formatted output
gr::console::writeln("User: {}, Score: {:.1f}", username, score);

// Error output
gr::console::error("Failed to open: {}", filename);
gr::console::errorln("Critical error occurred");

// Progress display
for (size_t i = 0; i < total; ++i) {
    gr::console::write("\rProgress: {}/{}", i, total);
}
```

**Features:**
- Thread-safe output protected by global mutex
- Automatic newline detection and buffer flushing
- Zero-copy string view operations
- Seamless integration with gr::toy::format

### 3. Logging System (`gr/logger.hh`)
**Multi-sink logging framework based on gr::toy::format**

```cpp
#include <gr/logger.hh>

// Initialize logging system
gr::log::init_logger("my_app", true);  // Enable color output

// Add output targets
auto logger = gr::log::get_default_logger();
logger->add_file_sink("app.log");
logger->add_rotating_file_sink("rotating.log", 10*1024*1024, 5);

// Level-based logging
GR_TRACE("Detailed debug: {}", debug_info);
GR_DEBUG("Configuration: {} items", config_count);
GR_INFO("User {} logged in", username);
GR_WARN("High memory: {:.1f}%", memory_usage);
GR_ERROR("DB connection failed: {}", error_msg);
GR_FATAL("System shutdown required");
```

**Log Levels:**
- `TRACE` (0): Detailed debugging information
- `DEBUG` (1): General debugging information  
- `INFO` (2): Informational messages
- `WARN` (3): Warning conditions
- `ERROR` (4): Error conditions
- `FATAL` (5): Fatal conditions requiring immediate attention

**Sink Types:**
- **Console Sink**: ANSI color-coded output
- **File Sink**: Simple file logging
- **Rotating File Sink**: Size-based automatic file rotation

### 4. Unicode String Utilities (`gr/string.hh`)
**Enhanced Unicode string processing**

```cpp
#include <gr/string.hh>

gr::str::u8 text = "Hello 世界 🌍"_u8;

// Unicode-aware operations
auto processed = text.utrim().to_upper();
size_t display_width = processed.udisplay_width();

// Encoding conversion
auto utf16_text = text.to_u16();
auto utf32_text = text.to_u32();

// Text alignment
auto centered = processed.ucenter(20);
```

### 5. Unicode Iterator (`gr/utf_iter.hh`)
**Multi-encoding Unicode code point iteration**

```cpp
#include <gr/utf_iter.hh>

auto iter = gr::uc::make_iterator(u8"Hello 世界");
while (iter) {
    auto cp = *iter;
    if (cp.is_alphabetic()) {
        // Process alphabetic characters
    }
    ++iter;
}
```

## Technical Features

### Compile-time Optimization
- C++20 concepts and C++17 type traits
- Compile-time format string validation
- Static tables and constexpr expressions
- Log level-based conditional compilation

### Memory Efficiency
- Stack buffers to avoid dynamic allocation
- Smart pre-allocation strategies
- Zero-copy view operations
- RAII resource management

### Thread Safety Design
- Fine-grained locking strategy
- Lock-free read operations (compile-time filtering)
- Atomic operation guarantees
- Deadlock prevention

## Quick Start

### Basic Usage

```cpp
#include <gr/logger.hh>
#include <gr/console.hh>

int main() {
    // Initialize logging system
    gr::log::init_logger("demo_app");
    
    GR_INFO("Application starting");
    
    // High-performance formatting
    auto result = gr::toy::format("Processing {} items in {:.2f}s", 
                                 item_count, duration);
    
    gr::console::writeln("Result: {}", result);
    
    GR_INFO("Application completed");
    return 0;
}
```

### Compilation Requirements

- **C++ Standard**: C++17 or higher (C++20 recommended)
- **Dependencies**:
  - RE2 (optional, for regex support)
  - iconv (optional, for encoding conversion)
- **Platform Support**: Linux, Windows (MSYS2)

## Architecture Design

### Core Data Flow

```
Application Code
    ↓
gr::logger (Logging) → gr::toy::format (String Building)
    ↓  
gr::console (Console) + File Sinks (Persistence)
    ↓
Operating System I/O
```

### Performance Critical Path Optimization

1. **Compile-time Filtering**: Log levels checked at compile time
2. **Zero-allocation Formatting**: Uses stack buffers and pre-allocation
3. **Batched I/O**: Minimizes system call frequency
4. **Lock Optimization**: Fine-grained locks and short critical sections

## License

**MIT License**
- Copyright (c) 2025 SICKEE2
- Permits commercial use, modification, and distribution

## Project Status

### Stable Features
- High-performance string formatting (gr::toy::format)
- Thread-safe console output (gr::console)  
- Enterprise-grade logging system (gr::logger)
- Unicode string processing (gr::string)

### Features Pending Optimization
- Floating-point character conversion
- Extended log filtering capabilities
- Enhanced error recovery mechanisms

## Design Philosophy

1. **Performance First**: Pursue extreme performance while ensuring correctness
2. **Zero-overhead Abstractions**: Compile-time optimizations eliminate runtime costs
3. **Thread Safety**: Reliable operations in multi-threaded environments
4. **Pragmatism**: Solve real-world performance bottlenecks in development
5. **Incremental Optimization**: Continuous improvement and gradual refinement

This project demonstrates the significant potential of modern C++ in performance optimization, providing developers with more efficient alternatives to standard library components, particularly suitable for applications with strict performance requirements.
