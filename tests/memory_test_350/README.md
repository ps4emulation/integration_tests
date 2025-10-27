# OpenOrbis CMake project

This is a template for quick start with PS4 development using OpenOrbis toolchain.

## Usage

```bash
cmake -B./build/ -S./ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=OpenOrbis-tc.cmake
cmake --build ./build/ -j8
cmake --install .
```
