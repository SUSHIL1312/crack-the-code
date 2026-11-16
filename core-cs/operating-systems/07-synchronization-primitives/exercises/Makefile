CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic -pthread

TARGET = test_lab07
SRC = lab_sync_primitives.cpp

SOLUTION_TARGET = test_lab07_solution
SOLUTION_SRC = ../solutions/lab_sync_primitives_solution.cpp

all: test

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

$(SOLUTION_TARGET): $(SOLUTION_SRC)
	$(CXX) $(CXXFLAGS) $(SOLUTION_SRC) -o $(SOLUTION_TARGET)

solution: $(SOLUTION_TARGET)
	./$(SOLUTION_TARGET)

clean:
	rm -f $(TARGET) $(SOLUTION_TARGET)
