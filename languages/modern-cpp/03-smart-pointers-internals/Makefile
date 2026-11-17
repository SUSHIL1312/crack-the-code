CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic

TARGET = smart_pointers_internals
SRC = smart_pointers_internals.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
