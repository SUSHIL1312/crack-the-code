CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude -pthread

SRCS = src/main.cpp src/event_loop.cpp
TARGET = event_server

all: $(TARGET)

$(TARGET): $(SRCS) include/event_loop.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
