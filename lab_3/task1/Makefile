CXX := g++
CXXFLAGS := -std=c++17 -O3 -march=native -Wall -Wextra -pthread
SRC := task1.cpp
TARGET := task1

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(TARGET) *.o
