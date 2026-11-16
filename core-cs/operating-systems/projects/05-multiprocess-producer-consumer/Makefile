CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude -pthread

SRCS = src/main.cpp
TARGET = shm_ipc_test

all: $(TARGET)

$(TARGET): $(SRCS) include/ipc_ring_buffer.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
