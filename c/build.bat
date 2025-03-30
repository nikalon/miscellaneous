@echo off

if not exist Build mkdir Build

rem Preserve current working directory
pushd .
cd Build

cl /Zi /Fe:"arena_test.exe" ..\basic.cpp ..\arena_test.cpp ..\arena_virtual_memory.cpp
cl /Zi /Fe:"basic_test.exe" ..\basic.cpp ..\basic_test.cpp

copy /Y "arena_test.exe" ..
copy /Y "basic_test.exe" ..

rem Restore original working directory
popd
