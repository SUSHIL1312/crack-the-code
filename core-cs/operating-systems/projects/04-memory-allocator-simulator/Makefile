CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude

SRCS = src/main.cpp src/allocator.cpp
TARGET = allocator_test

all: $(TARGET)

$(TARGET): $(SRCS) include/allocator.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
