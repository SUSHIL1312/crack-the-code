CXX = clang++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Wpedantic

TARGET = test_lab02
SRC = lab_cpu_scheduler.cpp

SOLUTION_TARGET = test_lab02_solution
SOLUTION_SRC = ../solutions/lab_cpu_scheduler_solution.cpp

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
