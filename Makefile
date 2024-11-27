# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -g -std=c++17 -Wall -Wextra -Isrc
LDFLAGS :=

# OpenSSL
CXXFLAGS += $(shell pkg-config --cflags openssl)
LDFLAGS += $(shell pkg-config --libs openssl)

# Zlib
CXXFLAGS += $(shell pkg-config --cflags zlib)
LDFLAGS += $(shell pkg-config --libs zlib)

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

UNAME_S := $(shell uname -s)
CPU_ARCH := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
	LDFLAGS += -framework CoreFoundation -framework Security
	ifeq ($(CPU_ARCH),arm64)
		CXXFLAGS += -I/opt/homebrew/include
		LDFLAGS += -L/opt/homebrew/lib
	else ifeq ($(CPU_ARCH),x86_64)
		CXXFLAGS += -I/usr/local/include
		LDFLAGS += -L/usr/local/lib
	endif
else ifeq ($(UNAME_S),Linux)
	CXXFLAGS += -I/usr/include
	LDFLAGS += -L/usr/lib $(shell pkg-config --libs tss2-esys) $(shell pkg-config --libs tss2-rc)
endif

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR)