#CXX = x86_64-w64-mingw32-g++
CXX = g++
CC = $(CXX)

# Explanation of some compiler arguments, which are paraphrased or copied straight from the GCC manual.
# - Wshadow         warns about declaring variables with the same name as another variable in the current scope.
# - Wpointer-arith  warns about using the operator sizeof with (void*), functions and using NULL as arithmetic operations for pointers.
# - Wcast-qual      warns about casts that remove type qualifier (for example, casting (const char*) to (char*).
# -fsanitize=undefine	aborts the program at runtime if it detects undefined behavior
CXXFLAGS = -g3 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -fsanitize=undefined -fsanitize-trap

all: basic_test arena_test

basic_test: basic.o basic_test.o

arena_test: basic.o arena_test.o

test: basic_test arena_test
	./basic_test
	./arena_test

clean:
	rm -f *.o *.exe basic_test arena_test
