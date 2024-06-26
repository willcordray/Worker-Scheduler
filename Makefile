###
### Makefile for Worker Scheduler Project
### Worker Scheduler is a program that schedules "Workers" into "TimeSlots"
###
### Author: Will Cordray


CXX      = clang++
CXXFLAGS = -Iinclude -std=gnu++17 -g3 -Wall -Wextra -Wpedantic -Wshadow

# TARGET = workerScheduler

# worker_scheduler: main.o Scheduler.o WorkerNode.o TimeSlotNode.o WorkerInputData.o PrintSchedule.o
# 	$(CXX) $(CXXFLAGS) $^ -o (TARGET)


# # This rule builds main.o
# build/main.o: main.cpp Scheduler.h WorkerInputData.h
# 	$(CXX) $(CXXFLAGS) -c $<


# # This rule build Scheduler.o
# build/Scheduler.o: Scheduler.cpp Scheduler.h WorkerNode.h TimeSlotNode.h ScheduleData.h WorkerInputData.h PrintSchedule.o
# 	$(CXX) $(CXXFLAGS) -c $<

# # This rule build WorkerNode.o
# build/WorkerNode.o: WorkerNode.cpp WorkerNode.h TimeSlotNode.h ScheduleData.h
# 	$(CXX) $(CXXFLAGS) -c $<

# # This rule build TimeSlotNode.o
# build/TimeSlotNode.o: TimeSlotNode.cpp TimeSlotNode.h WorkerNode.h ScheduleData.h
# 	$(CXX) $(CXXFLAGS) -c $<

# build/WorkerInputData.o: WorkerInputData.cpp WorkerInputData.h WorkerNode.h TimeSlotNode.h ScheduleData.h
# 	$(CXX) $(CXXFLAGS) -c $<


# clean:
# 	rm build/*.o
# 	rm (TARGET)


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

# Clean target to remove build artifacts
clean:
	rm -f $(TARGET) build/*.o
