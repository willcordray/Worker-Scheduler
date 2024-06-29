###
### Makefile for Worker Scheduler Project
### Worker Scheduler is a program that schedules "Workers" into "TimeSlots"
###
### Author: Will Cordray


CXX      = clang++ -O2
CXXFLAGS = -Iinclude -std=gnu++17 -g3 -Wall -Wextra -Wpedantic -Wshadow

# Source files
SRC = $(wildcard src/*.cpp)

# Object files
OBJ = $(SRC:src/%.cpp=build/%.o)

# Output executable
TARGET = workerScheduler


# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files in the build directory
build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

# Clean target to remove build artifacts
clean:
	rm -f $(TARGET) build/*.o
