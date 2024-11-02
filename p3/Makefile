# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++11 -O2

# Source and build directories
SRC_DIR := src
BUILD_DIR := build

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cc)

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRCS))

# Executable files
EXECS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%,$(SRCS))

# Default target to build all executables
all: $(EXECS)

# Rule to compile object files from source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to build executables from object files
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CXX) $(CXXFLAGS) $< -o $@

# Clean target to remove build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
