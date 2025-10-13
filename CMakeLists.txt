cmake_minimum_required(VERSION 3.24)

set(CMAKE_SYSTEM_NAME FreeBSD CACHE STRING "" FORCE)
set(CMAKE_C_COMPILER_TARGET "x86_64-pc-freebsd12-elf" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "${CMAKE_C_COMPILER_TARGET}" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" CACHE STRING "" FORCE)

include(FetchContent)
include(ExternalProject)

project(integration_tests_super NONE)

# # Get toolchain before project
if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
message(STATUS "Downloading openOrbis")
  FetchContent_Declare(
    openorbis
    URL https://github.com/ps4emulation/OpenOrbis-PS4-Toolchain/releases/download/0.5.4-2/v0.5.4-2.tar.gz
    URL_HASH SHA256=85cc12f388d06268873401ad80b42e2a059468063156180491c60821c0ff6568 # optional, hash of zip
  )
  FetchContent_MakeAvailable(openorbis)

  set(OO_PS4_TOOLCHAIN ${openorbis_SOURCE_DIR}/PS4Toolchain)
else()
  STRING(REGEX REPLACE "\\\\" "/" OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")
endif()

# # project
ExternalProject_Add(
  integration_tests
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
  BINARY_DIR ${CMAKE_BINARY_DIR}/tests
  CMAKE_ARGS
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/OpenOrbis-tc.cmake
	-DOO_PS4_LINKER_SUFFIX=${OO_PS4_LINKER_SUFFIX}
	-DOO_PS4_NOPKG=${OO_PS4_NOPKG}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
	-DINTEST_SOURCE_ROOT=${CMAKE_SOURCE_DIR}
  -DOO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}
  BUILD_ALWAYS 1
)
