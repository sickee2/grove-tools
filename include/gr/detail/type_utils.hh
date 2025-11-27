#pragma once
#include <string>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

class type_utils {
public:
  template <typename T> static std::string get_type_name() {
    return get_readable_name(typeid(T));
  }

  template <typename T> static std::string getReadableName(const T &obj) {
    return get_readable_name(typeid(obj));
  }

private:
  static std::string get_readable_name(const std::type_info &typeInfo) {
#ifdef __GNUC__
    int status = -1;
    char *name =
        abi::__cxa_demangle(typeInfo.name(), nullptr, nullptr, &status);
    if (status == 0) {
      std::string result(name);
      free(name);
      return result;
    }
#endif
    return typeInfo.name();
  }
};
