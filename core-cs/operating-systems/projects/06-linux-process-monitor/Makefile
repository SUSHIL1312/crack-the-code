CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude

SRCS = src/main.cpp src/process_monitor.cpp
TARGET = proc_monitor

all: $(TARGET)

$(TARGET): $(SRCS) include/process_monitor.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
