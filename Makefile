# 编译器设置
# CXX = g++
CXX = clang++
CXXFLAGS = -O2 -march=native -Wall -Wextra -std=c++2c -MMD -MP -MF $(@:%.o=%.d) -Iinclude
# CXXFLAGS = -g -march=native -Wall -Wextra -std=c++2c -MMD -MP -MF $(@:%.o=%.d) -Iinclude

# 目标可执行文件
TARGET = grstr

# 源文件
SRC_DIR = src
CPP_SRC = $(SRC_DIR)/main.cpp \
	 $(SRC_DIR)/gr/utf_sequence.cpp \
	 $(SRC_DIR)/gr/utf_string.cpp

	 # $(SRC_DIR)/format.cpp

# 构建目录
BUILD_DIR = build_out
# OBJ = $(BUILD_DIR)/main.o
OBJECTS = $(CPP_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# 依赖文件
DEP_DIR = $(BUILD_DIR)/.deps

# 检测操作系统
UNAME_S := $(shell uname -s)

# 根据操作系统设置链接选项
ifeq ($(UNAME_S), Linux)
    LDFLAGS = $(shell pkg-config --libs re2) -liconv
else
    LDFLAGS = $(shell pkg-config --libs re2) -liconv
endif

# 默认目标
all: $(BUILD_DIR) $(TARGET)

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/gr

# 编译
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 链接
$(TARGET): $(OBJECTS)
	@echo linking...
	@$(CXX) $^ -o $@ $(LDFLAGS)


# 包含依赖文件
-include $(wildcard $(BUILD_DIR)/*.d)
-include $(wildcard $(BUILD_DIR)/**/*.d)

# 清理
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
