#include "gr/utils.hh"
#include <cassert>
#include <iostream>
#include <string>

// 测试用的简单类
struct TestObject {
  int value;
  std::string name;

  TestObject(int v = 0, const std::string &n = "") : value(v), name(n) {}

  bool operator==(const TestObject &other) const {
    return value == other.value && name == other.name;
  }
};

void test_construction() {
  std::cout << "=== 测试构造和析构 ===\n";

  // 默认构造
  gr::utils::cptr<TestObject> ptr1;
  assert(ptr1.is_null());
  assert(!ptr1);

  // 工厂方法构造
  auto ptr2 = gr::utils::cptr<TestObject>::make(42, "test");
  assert(!ptr2.is_null());
  assert(ptr2);
  assert(ptr2->value == 42);
  assert(ptr2->name == "test");

  std::cout << "✓ 构造和析构测试通过\n";
}

void test_move_semantics() {
  std::cout << "=== 测试移动语义 ===\n";

  auto ptr1 = gr::utils::cptr<TestObject>::make(100, "move_test");

  // 移动构造
  gr::utils::cptr<TestObject> ptr2(std::move(ptr1));
  assert(ptr1.is_null());  // 源对象应为空
  assert(!ptr2.is_null()); // 目标对象应有数据
  assert(ptr2->value == 100);

  // 移动赋值
  auto ptr3 = gr::utils::cptr<TestObject>::make(200, "another");
  ptr3 = std::move(ptr2);
  assert(ptr2.is_null()); // 源对象应为空
  assert(ptr3->value == 100);

  std::cout << "✓ 移动语义测试通过\n";
}

void test_pointer_operations() {
  std::cout << "=== 测试指针操作 ===\n";

  auto ptr = gr::utils::cptr<TestObject>::make(999, "operations");

  // 解引用操作
  TestObject &ref = *ptr;
  assert(ref.value == 999);
  ref.value = 888;
  assert(ptr->value == 888);

  // 箭头操作符
  assert(ptr->name == "operations");
  ptr->name = "modified";
  assert(ptr->name == "modified");

  // get() 方法
  TestObject *raw_ptr = ptr.get();
  assert(raw_ptr->value == 888);

  std::cout << "✓ 指针操作测试通过\n";
}

void test_reset_functionality() {
  std::cout << "=== 测试重置功能 ===\n";

  auto ptr = gr::utils::cptr<TestObject>::make(1, "original");
  assert(ptr->value == 1);

  // 重置为新对象
  ptr.reset(2, "reset");
  assert(ptr->value == 2);
  assert(ptr->name == "reset");

  // 重置为默认构造对象
  ptr.reset();            // 这会调用 TestObject 的默认构造函数
  assert(!ptr.is_null()); // 指针不应为空，而是指向默认构造的对象
  assert(ptr->value == 0);
  assert(ptr->name == "");

  std::cout << "✓ 重置功能测试通过\n";
}

void test_clone_functionality() {
  std::cout << "=== 测试克隆功能 ===\n";

  auto original = gr::utils::cptr<TestObject>::make(123, "clone_test");
  auto cloned = original.clone();

  // 验证克隆对象内容相同
  assert(*original == *cloned);

  // 验证是深拷贝（修改一个不影响另一个）
  original->value = 456;
  assert(cloned->value == 123); // 克隆对象不应改变

  // 空指针克隆
  gr::utils::cptr<TestObject> empty;
  auto empty_clone = empty.clone();
  assert(empty_clone.is_null());

  std::cout << "✓ 克隆功能测试通过\n";
}

void test_swap_functionality() {
  std::cout << "=== 测试交换功能 ===\n";

  auto ptr1 = gr::utils::cptr<TestObject>::make(1, "first");
  auto ptr2 = gr::utils::cptr<TestObject>::make(2, "second");

  ptr1.swap(ptr2);

  assert(ptr1->value == 2);
  assert(ptr1->name == "second");
  assert(ptr2->value == 1);
  assert(ptr2->name == "first");

  std::cout << "✓ 交换功能测试通过\n";
}

void test_global_make_function() {
  std::cout << "=== 测试全局make函数 ===\n";

  // 使用命名空间内的make_cptr
  auto ptr1 = gr::utils::make_cptr<TestObject>(777, "utils_make");
  assert(ptr1->value == 777);

  // 使用全局make_cptr
  auto ptr2 = gr::make_cptr<TestObject>(888, "global_make");
  assert(ptr2->value == 888);

  std::cout << "✓ 全局make函数测试通过\n";
}

void test_edge_cases() {
  std::cout << "=== 测试边界情况 ===\n";

  // 测试空指针操作
  gr::utils::cptr<TestObject> empty;
  assert(empty.is_null());
  assert(!empty);

  // 测试从空指针移动
  gr::utils::cptr<TestObject> moved_from_empty(std::move(empty));
  assert(moved_from_empty.is_null());

  // 测试移动到空指针
  auto ptr = gr::utils::cptr<TestObject>::make(999, "edge_case");
  empty = std::move(ptr);
  assert(!empty.is_null());
  assert(empty->value == 999);
  assert(ptr.is_null());

  std::cout << "✓ 边界情况测试通过\n";
}

int main() {
  try {
    std::cout << "开始测试 utils::cptr 类...\n\n";

    test_construction();
    test_move_semantics();
    test_pointer_operations();
    test_reset_functionality();
    test_clone_functionality();
    test_swap_functionality();
    test_global_make_function();
    test_edge_cases();

    std::cout << "\n🎉 所有测试通过！\n";
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "❌ 测试失败: " << e.what() << std::endl;
    return 1;
  }
}
