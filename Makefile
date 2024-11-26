# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -g -std=c++11 -Wall -Wextra -Isrc

# Source directory
SRC_DIR := src

# Build directory
BUILD_DIR := build

# Object files directory
OBJ_DIR := $(BUILD_DIR)/obj

# Find all C++ files recursively
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Object files (place in build/obj directory)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Target executable
TARGET := $(BUILD_DIR)/kagi

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule to compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR)