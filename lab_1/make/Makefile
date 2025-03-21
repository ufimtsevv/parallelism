CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17
LDFLAGS = 

ifeq ($(TYPE), float)
    CXXFLAGS += -DUSE_FLOAT
endif

TARGET = sin_sum

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean