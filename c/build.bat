@echo off
rem To compile as C code:
rem cl /TC /W4 /O2 /Zi arena_test.cpp arena_virtual_memory.cpp
rem cl /TC /W4 /O2 /Zi basic_test.cpp

rem To compile as C++ code:
cl /W4 /O2 /Zi arena_test.cpp arena_virtual_memory.cpp
cl /W4 /O2 /Zi basic_test.cpp
