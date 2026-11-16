CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude -pthread

SRCS = src/main.cpp
TARGET = thread_pool_test

all: $(TARGET)

$(TARGET): $(SRCS) include/thread_pool.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
