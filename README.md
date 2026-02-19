# integration_tests
OpenOrbis Homebrews running tests

## Running
All dependencies are downloaded on demand
* `OO_PS4_TOOLCHAIN`: If environment variable is not set, download latest into build folder
* CppUTest: Always download latest into build

Toolchain, compiler (clang) is automatically selected. just run:

```
cmake -B./build/ -S./
cmake --build ./build/
```

After build, tests are in `./build/install` and are ready to use (no cmake install needed)

## Adding new test
Create a copy of `template` directory in `./tests/code` and rename it. Add tests into test's `code/test.cpp`.
If sys libs are needed, then add them using `link_libraries()` call inside the test's CMake file.
It is recommended to read comments in `.tests/code/ps4_packatge.cmake` and `./OpenOrbis-tc.cmake` to get the
better understanding of how targets are working and learn how to set them up.

> [!NOTE]
> No need to add tests folder into CMakeLists. All folder under tests are automatically build.
