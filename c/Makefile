CXX = g++
CXXFLAGS = -g

all: basic_test arena_test

basic_test: basic.o basic_test.o arena_virtual_memory.o

arena_test: basic.o arena_test.o  arena_virtual_memory.o

clean:
	rm *.o basic_test arena_test
