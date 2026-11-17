CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic -pthread

TARGET = concurrent_lru_cache
SRC = concurrent_lru_cache.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
