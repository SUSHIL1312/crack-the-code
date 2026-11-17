CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic

TARGET = custom_shared_ptr
SRC = custom_shared_ptr.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
