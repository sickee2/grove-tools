#include <iostream>
#include <gr/string.hh>


void test_re2(){
  using namespace std;
  // 原始字符串包含数字
  gr::str::u8 text = "Hello world! Email: test@example.com, Phone: 123-456-7890, ID: 98765";

  cout << "--> spilt by re2: " << endl;
  auto array = text.split_by_re2(R"(\s+)");
  for(auto &i : array){
    cout << "[" << i <<"] ";
  }
  cout << endl;
  // 先测试提取功能，再测试替换
  cout << "Original: " << text << endl;

  // 1. 先提取数字
  bool has_digits = text.contains(R"(\d+)");
  cout << "Contains digits: " << boolalpha << has_digits << endl;

  cout << "--> extract by re2: " << endl;
  auto all_numbers = text.extract_all(R"(\d+)");
  cout << "All numbers: ";
  for (const auto& num : all_numbers) {
      cout << num << " ";
  }
  cout << endl;

  // 2. 提取电话号码
  cout << "--> extract phone number by re2: " << endl;
  auto phone_number = text.extract(R"((\d{3}-\d{3}-\d{4}))");
  cout << phone_number << endl;

  cout << "--> replace phone number by re2: " << endl;
  // 3. 然后替换电话号码
  text.replace_by_re2_inplace(R"(\d{3}-\d{3}-\d{4})", "***-***-****");
  cout << "After phone replacement: " << text << endl;

  // 4. 验证替换后是否还有数字（ID部分）
  has_digits = text.contains(R"(\d+)");
  cout << "Still contains digits after replacement: " << has_digits << endl;

  // 5. 迭代器拷贝
  gr::str::u8 k(text.begin(), text.end());
  cout <<"-->: "<< k << endl;
}
// int main(){
//   test_re2();
//   return 0;
// }
