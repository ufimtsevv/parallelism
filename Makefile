CC = gcc
CFLAGS = -O3 -march=native -fopenmp
LDFLAGS = -lm

TARGETS = task1 task2 task3

all: $(TARGETS)

task1: task1.c
	$(CC) $(CFLAGS) -o $@ $<

test20000: task1
	./task1 20000 8

test40000: task1
	./task1 40000 8

scalability1: task1
	./task1

task2: task2.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

task2.o: task2.c
	$(CC) $(CFLAGS) -Wno-unused-result -c $<

test_integration: task2
	./task2

scalability2: task2
	./task2

task3: task3.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

task3.o: task3.cpp
	$(CC) $(CFLAGS) -Wno-unused-result -c $<

test_system: task3
	./task3

scalability3: task3
	./task3

clean:
	rm -f $(TARGETS) *.o

.PHONY: all test20000 test40000 scalability1 test_integration scalability2 test_system scalability3 clean