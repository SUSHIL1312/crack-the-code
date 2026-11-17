CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic -pthread

TARGET = concurrency_and_atomics
SRC = concurrency_and_atomics.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
