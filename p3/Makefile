# Makefile for compiling source code individually in src folder

# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++11 -O2

# Source directory
SRC_DIR := src

# Build directory
BUILD_DIR := build

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cc)

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRCS))

# Executable files
EXECS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%,$(SRCS))

# Rule to build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build executable files
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CXX) $(CXXFLAGS) $< -o $@

# Default target
all: $(EXECS)

# Clean target
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean