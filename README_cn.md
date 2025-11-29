# GR Format Library - 高性能 C++ 格式化与日志系统

## 项目概述

GR Format Library 是一个专注于极致性能的现代 C++ 格式化与日志库。核心组件 `gr::toy::format` 在性能基准测试中显著超越标准库，同时提供了完整的日志系统和线程安全的控制台输出。

## 性能优势

基于 100,000 次迭代的性能测试：

| 操作类型 | std::format | toy::format | 性能提升 |
|---------|-------------|-------------|----------|
| 整数格式化 | 5859μs | 2598μs | **55% 更快** |
| 浮点数格式化 | 7904μs | 5928μs | **25% 更快** |
| 字符串格式化 | 3859μs | 2455μs | **36% 更快** |
| 总体吞吐量 | 7.67M ops/sec | 12.33M ops/sec | **60% 更高** |

## 核心组件

### 1. 高性能格式化库 (`gr/format.hh`)
**超越 std::format 的字符串格式化引擎**

```cpp
#include <gr/format.hh>

// 基本格式化
auto str1 = "Hello {}!"_fmt("World");
auto str2 = gr::toy::format("Value: {:.2f}", 3.14159);

// 高级格式化
auto str3 = format("{:<10} {:>8}", "Name", 42);    // 对齐
auto str4 = format("{:#x}", 255);                  // 十六进制
auto str5 = format("{:.3}", 1.234567);             // 精度控制

// 时间格式化
auto now = std::chrono::system_clock::now();
auto time_str = format("{:f}", now);               // 完整时间戳
```

**特性：**
- 类型安全的 Python 风格格式化语法
- 编译时格式字符串检查 (C++20)
- 支持自定义类型格式化
- 内置 chrono 时间/时长格式化

### 2. 线程安全控制台输出 (`gr/console.hh`)
**基于 gr::toy::format 的高性能控制台 I/O**

```cpp
#include <gr/console.hh>

// 基础输出
gr::console::write("Processing...");
gr::console::writeln("Completed: {} items", count);

// 格式化输出
gr::console::writeln("User: {}, Score: {:.1f}", username, score);

// 错误输出
gr::console::error("Failed to open: {}", filename);
gr::console::errorln("Critical error occurred");

// 进度显示
for (size_t i = 0; i < total; ++i) {
    gr::console::write("\rProgress: {}/{}", i, total);
}
```

**特性：**
- 全局互斥锁保护的线程安全输出
- 自动换行检测和缓冲区刷新
- 零拷贝字符串视图操作
- 与 gr::toy::format 无缝集成

### 3. 日志系统 (`gr/logger.hh`)
**基于 gr::toy::format 的多接收器日志框架**

```cpp
#include <gr/logger.hh>

// 初始化日志系统
gr::log::init_logger("my_app", true);  // 启用颜色输出

// 添加输出目标
auto logger = gr::log::get_default_logger();
logger->add_file_sink("app.log");
logger->add_rotating_file_sink("rotating.log", 10*1024*1024, 5);

// 分级日志记录
GR_TRACE("Detailed debug: {}", debug_info);
GR_DEBUG("Configuration: {} items", config_count);
GR_INFO("User {} logged in", username);
GR_WARN("High memory: {:.1f}%", memory_usage);
GR_ERROR("DB connection failed: {}", error_msg);
GR_FATAL("System shutdown required");
```

**日志级别：**
- `TRACE` (0): 详细调试信息
- `DEBUG` (1): 常规调试信息  
- `INFO` (2): 信息性消息
- `WARN` (3): 警告条件
- `ERROR` (4): 错误条件
- `FATAL` (5): 需要立即关注的致命条件

**接收器类型：**
- **控制台接收器**: ANSI 颜色编码输出
- **文件接收器**: 简单的文件日志记录
- **轮转文件接收器**: 基于大小的自动文件轮转

### 4. Unicode 字符串工具 (`gr/string.hh`)
**增强的 Unicode 字符串处理**

```cpp
#include <gr/string.hh>

gr::str::u8 text = "Hello 世界 🌍"_u8;

// Unicode 感知操作
auto processed = text.utrim().to_upper();
size_t display_width = processed.udisplay_width();

// 编码转换
auto utf16_text = text.to_u16();
auto utf32_text = text.to_u32();

// 文本对齐
auto centered = processed.ucenter(20);
```

### 5. Unicode 迭代器 (`gr/utf_iter.hh`)
**多编码 Unicode 代码点迭代**

```cpp
#include <gr/utf_iter.hh>

auto iter = gr::uc::make_iterator(u8"Hello 世界");
while (iter) {
    auto cp = *iter;
    if (cp.is_alphabetic()) {
        // 处理字母字符
    }
    ++iter;
}
```

## 技术特性

### 编译时优化
- C++20 概念和 C++17 类型特征
- 编译时格式字符串验证
- 静态表格和 constexpr 表达式
- 基于日志级别的条件编译

### 内存效率
- 栈缓冲区避免动态分配
- 智能预分配策略
- 零拷贝视图操作
- RAII 资源管理

### 线程安全设计
- 细粒度锁定策略
- 无锁读取操作（编译时过滤）
- 原子操作保证
- 死锁预防

## 快速开始

### 基本使用

```cpp
#include <gr/logger.hh>
#include <gr/console.hh>

int main() {
    // 初始化日志系统
    gr::log::init_logger("demo_app");
    
    GR_INFO("Application starting");
    
    // 高性能格式化
    auto result = gr::toy::format("Processing {} items in {:.2f}s", 
                                 item_count, duration);
    
    gr::console::writeln("Result: {}", result);
    
    GR_INFO("Application completed");
    return 0;
}
```

### 编译要求

- **C++ 标准**: C++17 或更高 (推荐 C++20)
- **依赖项**:
  - RE2 (可选，正则表达式支持)
  - iconv (可选，编码转换)
- **平台支持**: Linux, Windows (MSYS2)

## 架构设计

### 核心数据流

```
应用程序代码
    ↓
gr::logger (日志记录) → gr::toy::format (字符串构建)
    ↓  
gr::console (控制台) + 文件接收器 (持久化)
    ↓
操作系统 I/O
```

### 性能关键路径优化

1. **编译时过滤**: 日志级别在编译期检查
2. **零分配格式化**: 使用栈缓冲区和预分配
3. **批量 I/O**: 最小化系统调用次数
4. **锁优化**: 细粒度锁和短临界区

## 许可证

**MIT 许可证**
- 版权所有 (c) 2025 SICKEE2
- 允许商业使用、修改和分发

## 项目状态

### 稳定功能
- 高性能字符串格式化 (gr::toy::format)
- 线程安全控制台输出 (gr::console)  
- 企业级日志系统 (gr::logger)
- Unicode 字符串处理 (gr::string)

### 待优化功能
- 浮点数字符转换
- 扩展日志过滤能力
- 增强错误恢复机制

## 设计理念

1. **性能优先**: 在确保正确性的前提下追求极致性能
2. **零开销抽象**: 编译期优化消除运行时成本
3. **线程安全**: 多线程环境下的可靠操作
4. **实用性**: 解决实际开发中的性能瓶颈
5. **渐进优化**: 持续改进和逐步完善

该项目展示了现代 C++ 在性能优化方面的巨大潜力，为开发者提供了比标准库组件更高效的替代方案，特别适合对性能有严格要求的应用程序。
