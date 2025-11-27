# GR Library - 高性能 C++ 工具库

## 项目概述

GR Library 是一个现代化的 C++ 工具库集合，专注于提供高性能、类型安全且易于使用的组件。该项目实现了多个标准库功能的优化版本，特别在字符串处理、格式化和日志记录方面表现出色。

## 核心模块

### 1. 高性能日志系统 (`gr/logger.hh`)
**企业级日志框架，支持多输出目标和线程安全**

- **日志级别**：TRACE、DEBUG、INFO、WARN、ERROR、FATAL
- **输出目标**：
  - 控制台输出（支持 ANSI 颜色）
  - 文件输出
  - 轮转文件（基于大小自动轮转）
- **性能特性**：
  - 编译时日志级别过滤（零运行时开销）
  - 细粒度锁策略
  - 智能资源管理

### 2. 线程安全控制台输出 (`gr/console.hh`)
**高性能、线程安全的控制台 I/O 操作**

- **输出类型**：标准输出(stdout)和错误输出(stderr)
- **线程安全**：全局互斥锁保护，防止输出交错
- **格式化集成**：与 `gr::toy::format` 无缝集成
- **自动刷新**：智能缓冲管理和行级刷新

### 3. 字符串格式化 (`gr/format.hh`)
**高性能字符串格式化库，比 std::format 快 25-60%**

- **性能优势**：
  - 整数格式化：快 55%
  - 浮点数格式化：快 25%
  - 字符串格式化：快 36%
  - 总体吞吐量：高 60%

### 4. Unicode 字符串处理 (`gr/string.hh`)
**完整的 Unicode 字符串处理工具集**

- **编码支持**：UTF-8、UTF-16、UTF-32
- **核心功能**：
  - Unicode 感知的迭代和操作
  - BOM 自动检测和处理
  - 编码转换工具
  - 文本对齐和显示宽度计算

### 5. 字符转换 (`gr/detail/toy_charconv.hh`)
**学习用途的字符转换实现**

- **性能对比**：
  - 整数转字符串：快 38%
  - 浮点数转字符串：快 20%
  - 字符串转整数：快 6%
  - 字符串转浮点数：慢 15%（待优化）

### 6. 树迭代器 (`gr/tree_iter.hh`)
**通用的树结构遍历工具**

- **遍历策略**：深度优先和广度优先
- **接口特性**：
  - STL 兼容的迭代器接口
  - 层级跟踪
  - 编译时接口验证

## 性能基准

基于 100,000 次迭代的测试结果：

| 操作类型 | std::format | toy::format | 性能提升 |
|---------|-------------|-------------|----------|
| 整数格式化 | 5859μs | 2598μs | 55% |
| 浮点数格式化 | 7904μs | 5928μs | 25% |
| 字符串格式化 | 3859μs | 2455μs | 36% |
| 总体吞吐量 | 7.67M ops/sec | 12.33M ops/sec | 60% |

## 模块集成架构

### 日志系统集成
```
应用程序
    ↓
gr::logger (日志记录)
    ↓
gr::console (控制台输出) + gr::toy::format (格式化)
    ↓
gr::str (Unicode 字符串) + 标准 I/O
```

### 数据流示例
```cpp
// 应用程序代码
GR_INFO("用户 {} 登录成功，IP: {}", username, ip_address);

// 处理流程
1. 编译时日志级别检查
2. 字符串格式化 (gr::toy::format)
3. 时间戳和上下文添加
4. 线程安全输出到控制台/文件
5. 自动刷新缓冲区
```

## 使用示例

### 完整的应用程序日志
```cpp
#include <gr/logger.hh>
#include <gr/console.hh>

int main() {
    // 初始化日志系统
    gr::log::init_logger("my_app", true); // 启用颜色输出
    
    // 添加文件输出
    auto logger = gr::log::get_default_logger();
    logger->add_file_sink("app.log");
    logger->add_rotating_file_sink("rotating.log", 10*1024*1024, 5);
    
    // 记录不同级别的日志
    GR_INFO("应用程序启动");
    GR_DEBUG("配置加载完成: {} 项", config_count);
    GR_WARN("内存使用率较高: {:.1f}%", memory_usage);
    GR_ERROR("数据库连接失败: {}", error_message);
    
    return 0;
}
```

### 高性能控制台输出
```cpp
#include <gr/console.hh>

void process_data(const std::vector<Data>& data) {
    gr::console::writeln("开始处理 {} 条数据", data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 1000 == 0) {
            gr::console::write("\r进度: {}/{}", i, data.size());
        }
        // 处理数据...
    }
    
    gr::console::writeln("\n处理完成");
}
```

### Unicode 字符串操作
```cpp
#include <gr/string.hh>

void unicode_operations() {
    gr::str::u8 text = "Hello 世界 🌍"_u8;
    
    // Unicode 感知的修剪和大小写转换
    auto processed = text.utrim().to_upper();
    
    // 显示宽度计算（考虑全角字符）
    size_t display_width = processed.udisplay_width();
    
    // 文本对齐
    auto centered = processed.ucenter(20);
    gr::console::writeln("居中: {}", centered);
}
```

## 技术特色

### 编译时优化
- C++20 概念和 C++17 类型特征
- 编译时格式字符串检查
- 静态表格和常量表达式
- 条件编译消除运行时开销

### 内存效率
- 栈缓冲区避免动态分配
- 智能预分配策略
- 零拷贝视图操作
- RAII 资源管理

### 线程安全设计
- 细粒度锁策略
- 无锁读操作（编译时过滤）
- 原子性操作保证
- 死锁预防

### Unicode 支持
- 完整的 Unicode 标准兼容
- 多字节序列正确处理
- 显示宽度精确计算
- BOM 自动处理

## 编译要求

- **C++ 标准**：C++17 或更高（推荐 C++20）
- **依赖项**：
  - RE2（可选，用于正则表达式）
  - iconv（可选，用于编码转换）
- **平台支持**：Linux、Windows（MSYS2）

## 许可证

**MIT License**
- 版权所有 (c) 2025 SICKEE2
- 允许商业使用、修改和分发

## 项目状态

### 稳定功能
- 字符串格式化和日志系统
- Unicode 字符串处理
- 控制台 I/O 和树迭代器
- 整数字符转换

### 待优化功能
- 浮点数字符转换（当前比标准库慢 15%）
- 扩展日志过滤功能
- 增强的错误恢复机制

## 设计哲学

1. **性能优先**：在保证正确性的前提下追求极致性能
2. **零开销抽象**：编译时优化消除运行时成本
3. **线程安全**：多线程环境下的可靠操作
4. **实用主义**：解决实际开发中的性能瓶颈
5. **渐进优化**：持续改进，逐步完善

这个项目展示了现代 C++ 在性能优化方面的巨大潜力，为开发者提供了比标准库更高效的替代方案，特别适合对性能有严格要求的应用场景。
