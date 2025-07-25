#CXX = x86_64-w64-mingw32-g++
CXX = g++
CXXFLAGS = -g -Wall -Wextra
CC = $(CXX)

all: basic_test arena_test

basic_test: basic.o basic_test.o arena_virtual_memory.o

arena_test: basic.o arena_test.o  arena_virtual_memory.o

test: basic_test arena_test
	./basic_test
	./arena_test

clean:
	rm -f *.o *.exe basic_test arena_test
