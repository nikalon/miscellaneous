# C
This directory contains common utilities for C/C++ projects. A suite of tests has been included and can be compiled with `build.bat` in Windows.

## TODO
- Enable support for paths longer than MAX_PATH (260) characters in Windows.
    - Add tests to verify read_entire_file() works with paths longer than 260 characters in Windows
- Maybe add write_file() ???
- Add tests for invalid situations. For example, test arena_push_nozero() aborts the program if size == 0.

## Supported languages and platforms
The following languages are supported:
- C >= 99
- C++ >= 98

The following platforms are supported:
- Linux
- Windows 10 (it might work fine in older editions but I haven't tested)
