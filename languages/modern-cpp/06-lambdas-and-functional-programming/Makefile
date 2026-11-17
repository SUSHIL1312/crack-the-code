CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic

TARGET = lambdas_and_functional
SRC = lambdas_and_functional.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
