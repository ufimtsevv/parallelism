CXX := g++
CXXFLAGS := -std=c++20 -pthread -Wall -Wextra -O3
TEST_CXXFLAGS := -std=c++17 -Wall -Wextra -O3

SRC := task2.cpp
TEST_SRC := test_results.cpp

TARGET := task_server
TEST_TARGET := test_results

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(TEST_CXXFLAGS) $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.txt

.PHONY: all clean test