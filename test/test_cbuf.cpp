#include "gr/utils.hh"
#include <cassert>
#include <cstring>
#include <iostream>

// 测试用的POD类型
struct TestPOD {
  int id;
  double value;
  char name[16];
};

void test_construction() {
  std::cout << "=== 测试构造和析构 ===\n";

  // 默认构造
  gr::utils::cbuf<int> buf1;
  assert(!buf1); // 空缓冲区应为false
  assert(buf1.size() == 0);
  assert(buf1.begin() == nullptr);
  assert(buf1.end() == nullptr);

  // 工厂方法构造
  auto buf2 = gr::utils::cbuf<int>::create(10);
  assert(buf2); // 非空缓冲区应为true
  assert(buf2.size() == 10);
  assert(buf2.begin() != nullptr);
  assert(buf2.end() == buf2.begin() + 10);

  std::cout << "✓ 构造和析构测试通过\n";
}

void test_move_semantics() {
  std::cout << "=== 测试移动语义 ===\n";

  auto buf1 = gr::utils::cbuf<int>::create(5);
  // 初始化数据
  for (size_t i = 0; i < buf1.size(); ++i) {
    buf1[i] = static_cast<int>(i * 10);
  }

  // 移动构造
  gr::utils::cbuf<int> buf2(std::move(buf1));
  assert(!buf1); // 源对象应为空
  assert(buf2);  // 目标对象应有数据
  assert(buf2.size() == 5);
  assert(buf2[0] == 0);
  assert(buf2[4] == 40);

  // 移动赋值
  auto buf3 = gr::utils::cbuf<int>::create(3);
  buf3 = std::move(buf2);
  assert(!buf2); // 源对象应为空
  assert(buf3.size() == 5);
  assert(buf3[2] == 20);

  std::cout << "✓ 移动语义测试通过\n";
}

void test_element_access() {
  std::cout << "=== 测试元素访问 ===\n";

  auto buf = gr::utils::cbuf<double>::create(8);

  // 通过operator[]访问和修改
  for (size_t i = 0; i < buf.size(); ++i) {
    buf[i] = i * 1.5;
  }

  // 验证数据
  for (size_t i = 0; i < buf.size(); ++i) {
    assert(buf[i] == i * 1.5);
  }

  // 测试const访问
  const auto &const_buf = buf;
  assert(const_buf[3] == 4.5);

  std::cout << "✓ 元素访问测试通过\n";
}

void test_iterators() {
  std::cout << "=== 测试迭代器 ===\n";

  auto buf = gr::utils::cbuf<int>::create(6);

  // 初始化数据
  for (size_t i = 0; i < buf.size(); ++i) {
    buf[i] = static_cast<int>(i + 1);
  }

  // 使用迭代器遍历
  int sum = 0;
  for (auto it = buf.begin(); it != buf.end(); ++it) {
    sum += *it;
  }
  assert(sum == 21); // 1+2+3+4+5+6 = 21

  // 使用范围for循环（需要begin/end支持）
  sum = 0;
  for (int val : buf) {
    sum += val;
  }
  assert(sum == 21);

  std::cout << "✓ 迭代器测试通过\n";
}

void test_reallocation() {
  std::cout << "=== 测试重新分配 ===\n";

  auto buf = gr::utils::cbuf<int>::create(5);

  // 初始化数据
  for (size_t i = 0; i < buf.size(); ++i) {
    buf[i] = static_cast<int>(i * 100);
  }

  // 保存原始指针用于验证
  int *old_ptr = buf.begin();

  // 重新分配到更大容量
  buf.realloc(15);
  assert(buf.size() == 15);

  // 验证原有数据保持
  for (size_t i = 0; i < 5; ++i) {
    assert(buf[i] == static_cast<int>(i * 100));
  }

  // 验证新分配的区域可写
  for (size_t i = 5; i < buf.size(); ++i) {
    buf[i] = static_cast<int>(i * 200);
  }

  // 尝试重新分配到更小容量（应该无变化）
  buf.realloc(10);
  assert(buf.size() == 15); // 应该保持原大小

  std::cout << "✓ 重新分配测试通过\n";
}

void test_clone_functionality() {
  std::cout << "=== 测试克隆功能 ===\n";

  auto original = gr::utils::cbuf<TestPOD>::create(4);

  // 初始化数据
  for (size_t i = 0; i < original.size(); ++i) {
    original[i].id = static_cast<int>(i);
    original[i].value = i * 2.5;
    std::snprintf(original[i].name, sizeof(original[i].name), "item_%zu", i);
  }

  auto cloned = original.clone();
  assert(cloned.size() == original.size());

  // 验证数据相同
  for (size_t i = 0; i < original.size(); ++i) {
    assert(cloned[i].id == original[i].id);
    assert(cloned[i].value == original[i].value);
    assert(std::strcmp(cloned[i].name, original[i].name) == 0);
  }

  // 验证是深拷贝
  original[0].id = 999;
  assert(cloned[0].id == 0); // 克隆对象不应改变

  // 空缓冲区克隆
  gr::utils::cbuf<int> empty;
  auto empty_clone = empty.clone();
  assert(!empty_clone);
  assert(empty_clone.size() == 0);

  std::cout << "✓ 克隆功能测试通过\n";
}

void test_swap_functionality() {
  std::cout << "=== 测试交换功能 ===\n";

  auto buf1 = gr::utils::cbuf<int>::create(3);
  auto buf2 = gr::utils::cbuf<int>::create(5);

  // 初始化数据
  for (size_t i = 0; i < buf1.size(); ++i) {
    buf1[i] = static_cast<int>(i + 10);
  }
  for (size_t i = 0; i < buf2.size(); ++i) {
    buf2[i] = static_cast<int>(i + 20);
  }

  buf1.swap(buf2);

  // 验证交换后
  assert(buf1.size() == 5);
  assert(buf2.size() == 3);
  assert(buf1[0] == 20);
  assert(buf2[0] == 10);

  std::cout << "✓ 交换功能测试通过\n";
}

void test_fillzero_and_bytes() {
  std::cout << "=== 测试填充零和字节计算 ===\n";

  auto buf = gr::utils::cbuf<int>::create(8);

  // 填充非零数据
  for (size_t i = 0; i < buf.size(); ++i) {
    buf[i] = static_cast<int>(i + 100);
  }

  // 验证字节计算
  assert(buf.bytes() == 8 * sizeof(int));

  // 填充零
  buf.fillzero();

  // 验证所有元素为零
  for (size_t i = 0; i < buf.size(); ++i) {
    assert(buf[i] == 0);
  }

  std::cout << "✓ 填充零和字节计算测试通过\n";
}

void test_detach_and_release() {
  std::cout << "=== 测试分离和释放 ===\n";

  auto buf = gr::utils::cbuf<int>::create(6);

  // 初始化数据
  for (size_t i = 0; i < buf.size(); ++i) {
    buf[i] = static_cast<int>(i * 50);
  }

  // 分离缓冲区
  auto [ptr, size] = buf.detach();
  assert(!buf); // 分离后应为空
  assert(buf.size() == 0);
  assert(ptr != nullptr);
  assert(size == 6);

  // 验证分离的数据
  assert(ptr[0] == 0);
  assert(ptr[5] == 250);

  // 手动释放分离的内存
  std::free(ptr);

  // 测试释放功能
  auto buf2 = gr::utils::cbuf<double>::create(4);
  buf2.release();
  assert(!buf2);
  assert(buf2.size() == 0);

  std::cout << "✓ 分离和释放测试通过\n";
}

void test_edge_cases() {
  std::cout << "=== 测试边界情况 ===\n";

  // 测试零大小缓冲区
  auto zero_buf = gr::utils::cbuf<int>::create(0);
  assert(!zero_buf);
  assert(zero_buf.size() == 0);

  // 测试自交换
  auto buf = gr::utils::cbuf<int>::create(3);
  int *original_ptr = buf.begin();
  buf.swap(buf);                       // 自交换
  assert(buf.begin() == original_ptr); // 指针不应改变

  // 测试移动到自身（通过移动赋值）
  buf = std::move(buf);
  assert(buf); // 自移动后不应为空

  // 测试空缓冲区的各种操作
  gr::utils::cbuf<int> empty;
  assert(empty.size() == 0);
  assert(empty.bytes() == 0);
  empty.release();  // 空缓冲区释放应该没问题
  empty.fillzero(); // 空缓冲区填充零应该没问题

  std::cout << "✓ 边界情况测试通过\n";
}

int main() {
  try {
    std::cout << "开始测试 utils::cbuf 类...\n\n";

    test_construction();
    test_move_semantics();
    test_element_access();
    test_iterators();
    test_reallocation();
    test_clone_functionality();
    test_swap_functionality();
    test_fillzero_and_bytes();
    test_detach_and_release();
    test_edge_cases();

    std::cout << "\n🎉 所有测试通过！\n";
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "❌ 测试失败: " << e.what() << std::endl;
    return 1;
  }
}
