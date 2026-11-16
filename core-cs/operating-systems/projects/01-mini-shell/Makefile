CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude -Isrc

SRCS = src/main.cpp src/shell.cpp
TARGET = mini_shell

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
