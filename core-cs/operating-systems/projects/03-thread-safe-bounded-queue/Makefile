CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude -pthread

SRCS = src/main.cpp
TARGET = bounded_queue_test

all: $(TARGET)

$(TARGET): $(SRCS) include/bounded_queue.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
