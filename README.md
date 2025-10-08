# integration_tests
OpenOrbis Homebrews running tests

## Running
All dependencies are downloaded on demand
* OO_PS4_TOOLCHAIN: If environment variable is not set, download latest into build folder
* CppUTest: Always download latest into build

Toolchain, compiler (clang) is automatically selected. just run:

```
cmake -B./build/ -S./
cmake --build ./build/
```

After build, tests are in build/install and are ready to use (no cmake install needed)

## Adding new test
copy template into tests/ and rename it. Add tests into code/test.cpp
If sys libs are needed, then add them to set(PRJ_ADD_LIBS SceVideoOut SceAudioOut ScePad SceUserService)

> **_NOTE:_** No need to add tests folder into CMakeLists. All folder under tests are automatically build.

