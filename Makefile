# Compiler settings
# CXX = g++
CXX = clang++

INCLUDE = include

CXXFLAGS = -O2 -march=native -Wall -Wextra -std=c++2c -MMD -MP -MF $(@:%.o=%.d) -I$(INCLUDE)
# CXXFLAGS = -g -march=native -Wall -Wextra -std=c++2c -MMD -MP -MF $(@:%.o=%.d) -Iinclude

# Build directory
BUILD_DIR = build

# Target executable
TARGET = $(BUILD_DIR)/grstr

# Source files
SRC_DIR = src
TEST_DIR = test

CPP_SRC = $(SRC_DIR)/gr/utf_sequence.cpp \
$(SRC_DIR)/gr/utf_string.cpp \
$(TEST_DIR)/test.cpp \
$(TEST_DIR)/test_bom.cpp \
$(TEST_DIR)/test_cbuf.cpp \
$(TEST_DIR)/test_char_conv.cpp \
$(TEST_DIR)/test_convert.cpp \
$(TEST_DIR)/test_convert2.cpp \
$(TEST_DIR)/test_cptr.cpp \
$(TEST_DIR)/test_endian.cpp \
$(TEST_DIR)/test_format.cpp \
$(TEST_DIR)/test_format_performaence.cpp \
$(TEST_DIR)/test_from_chars.cpp \
$(TEST_DIR)/test_iconv.cpp \
$(TEST_DIR)/test_logger.cpp \
$(TEST_DIR)/test_print.cpp \
$(TEST_DIR)/test_printable.cpp \
$(TEST_DIR)/test_RE2.cpp \
$(TEST_DIR)/test_stoi.cpp \
$(TEST_DIR)/test_trim.cpp \
$(TEST_DIR)/test_upper_lower.cpp \
$(TEST_DIR)/test_utf_string.cpp \
$(TEST_DIR)/test_utfiter.cpp


# OBJ = $(BUILD_DIR)/main.o
OBJECTS = $(CPP_SRC:%.cpp=$(BUILD_DIR)/%.o)

# Dependency files
DEP_DIR = $(BUILD_DIR)/.deps

# Detect operating system
UNAME_S := $(shell uname -s)
MSYSTEM := $(shell echo $$MSYSTEM 2>/dev/null)

# Set linker flags based on operating system
ifeq ($(UNAME_S), Linux)
    LDFLAGS = $(shell pkg-config --libs re2) -liconv
else
    LDFLAGS = $(shell pkg-config --libs re2) -liconv
endif

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/gr
	@mkdir -p $(BUILD_DIR)/test

# Compile
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link
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
