# GR Library - High-Performance C++ Utility Library (English Version)

## Project Overview

GR Library is a modern C++ utility library collection focused on providing high-performance, type-safe, and easy-to-use components. The project implements optimized versions of several standard library functionalities, with exceptional performance in string processing, formatting, and logging.

## Core Modules

### 1. High-Performance Logging System (`gr/logger.hh`)
**Enterprise-grade logging framework with multiple outputs and thread safety**

- **Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- **Output Targets**:
  - Console output (with ANSI color support)
  - File output
  - Rotating files (size-based automatic rotation)
- **Performance Features**:
  - Compile-time log level filtering (zero runtime overhead)
  - Fine-grained locking strategy
  - Smart resource management

### 2. Thread-Safe Console Output (`gr/console.hh`)
**High-performance, thread-safe console I/O operations**

- **Output Types**: Standard output (stdout) and error output (stderr)
- **Thread Safety**: Global mutex protection prevents output interleaving
- **Formatting Integration**: Seamless integration with `gr::toy::format`
- **Automatic Flushing**: Smart buffer management with line-based flushing

### 3. String Formatting (`gr/format.hh`)
**High-performance string formatting library, 25-60% faster than std::format**

- **Performance Advantages**:
  - Integer formatting: 55% faster
  - Floating-point formatting: 25% faster
  - String formatting: 36% faster
  - Overall throughput: 60% higher

### 4. Unicode String Processing (`gr/string.hh`)
**Comprehensive Unicode string processing toolkit**

- **Encoding Support**: UTF-8, UTF-16, UTF-32
- **Core Capabilities**:
  - Unicode-aware iteration and manipulation
  - Automatic BOM detection and handling
  - Encoding conversion utilities
  - Text alignment and display width calculation

### 5. Character Conversion (`gr/detail/toy_charconv.hh`)
**Educational character conversion implementation**

- **Performance Comparison**:
  - Integer to string: 38% faster
  - Float to string: 20% faster
  - String to integer: 6% faster
  - String to float: 15% slower (pending optimization)

### 6. Tree Iterator (`gr/tree_iter.hh`)
**Generic tree structure traversal utility**

- **Traversal Strategies**: Depth-first and breadth-first
- **Interface Features**:
  - STL-compatible iterator interface
  - Level tracking
  - Compile-time interface validation

## Performance Benchmarks

Based on 100,000 iteration tests:

| Operation Type | std::format | toy::format | Performance Gain |
|---------------|-------------|-------------|------------------|
| Integer Formatting | 5859μs | 2598μs | 55% |
| Floating-point Formatting | 7904μs | 5928μs | 25% |
| String Formatting | 3859μs | 2455μs | 36% |
| Overall Throughput | 7.67M ops/sec | 12.33M ops/sec | 60% |

## Module Integration Architecture

### Logging System Integration
```
Application
    ↓
gr::logger (Logging)
    ↓
gr::console (Console Output) + gr::toy::format (Formatting)
    ↓
gr::str (Unicode Strings) + Standard I/O
```

### Data Flow Example
```cpp
// Application code
GR_INFO("User {} logged in successfully, IP: {}", username, ip_address);

// Processing flow:
1. Compile-time log level checking
2. String formatting (gr::toy::format)
3. Timestamp and context addition
4. Thread-safe output to console/file
5. Automatic buffer flushing
```

## Usage Examples

### Complete Application Logging
```cpp
#include <gr/logger.hh>
#include <gr/console.hh>

int main() {
    // Initialize logging system
    gr::log::init_logger("my_app", true); // Enable color output
    
    // Add file outputs
    auto logger = gr::log::get_default_logger();
    logger->add_file_sink("app.log");
    logger->add_rotating_file_sink("rotating.log", 10*1024*1024, 5);
    
    // Log at different levels
    GR_INFO("Application started");
    GR_DEBUG("Configuration loaded: {} items", config_count);
    GR_WARN("High memory usage: {:.1f}%", memory_usage);
    GR_ERROR("Database connection failed: {}", error_message);
    
    return 0;
}
```

### High-Performance Console Output
```cpp
#include <gr/console.hh>

void process_data(const std::vector<Data>& data) {
    gr::console::writeln("Processing {} data items", data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 1000 == 0) {
            gr::console::write("\rProgress: {}/{}", i, data.size());
        }
        // Process data...
    }
    
    gr::console::writeln("\nProcessing completed");
}
```

### Unicode String Operations
```cpp
#include <gr/string.hh>

void unicode_operations() {
    gr::str::u8 text = "Hello 世界 🌍"_u8;
    
    // Unicode-aware trimming and case conversion
    auto processed = text.utrim().to_upper();
    
    // Display width calculation (considering full-width characters)
    size_t display_width = processed.udisplay_width();
    
    // Text alignment
    auto centered = processed.ucenter(20);
    gr::console::writeln("Centered: {}", centered);
}
```

## Technical Features

### Compile-time Optimization
- C++20 concepts and C++17 type traits
- Compile-time format string checking
- Static tables and constexpr expressions
- Conditional compilation to eliminate runtime overhead

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

### Unicode Support
- Full Unicode standard compliance
- Proper multi-byte sequence handling
- Accurate display width calculation
- Automatic BOM handling

## Compilation Requirements

- **C++ Standard**: C++17 or higher (C++20 recommended)
- **Dependencies**:
  - RE2 (optional, for regex support)
  - iconv (optional, for encoding conversion)
- **Platform Support**: Linux, Windows (MSYS2)

## License

**MIT License**
- Copyright (c) 2025 SICKEE2
- Permits commercial use, modification, and distribution

## Project Status

### Stable Features
- String formatting and logging system
- Unicode string processing
- Console I/O and tree iterator
- Integer character conversion

### Features Pending Optimization
- Floating-point character conversion (currently 15% slower than standard library)
- Extended log filtering capabilities
- Enhanced error recovery mechanisms

## Design Philosophy

1. **Performance First**: Pursue extreme performance while ensuring correctness
2. **Zero-overhead Abstractions**: Compile-time optimizations eliminate runtime costs
3. **Thread Safety**: Reliable operations in multi-threaded environments
4. **Pragmatism**: Solve real-world performance bottlenecks in development
5. **Incremental Optimization**: Continuous improvement and gradual refinement

This project demonstrates the significant potential of modern C++ in performance optimization, providing developers with more efficient alternatives to standard library components, particularly suitable for applications with strict performance requirements.

