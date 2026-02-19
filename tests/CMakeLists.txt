cmake_minimum_required(VERSION 3.24)

include(FetchContent)
include(ExternalProject)

project(integration_tests LANGUAGES CXX)

set(CPPUTEST_EXTENSIONS ON CACHE BOOL "")
set(CPPUTEST_STD_C_LIB_DISABLED OFF CACHE BOOL "")
set(CPPUTEST_STD_CPP_LIB_DISABLED OFF CACHE BOOL "")
set(CPPUTEST_BUILD_TESTING OFF CACHE BOOL "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES 0)

add_compile_definitions(CPPUTEST_USE_NEW_MACROS=0) # won't build otherwise

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG master
)
FetchContent_MakeAvailable(CppUTest)

include_directories(BEFORE ${cpputest_SOURCE_DIR}/include)
link_libraries(CppUTest::CppUTest)

include(ps4_package.cmake)

# # Add all tests
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/code)

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist)

  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child} AND NOT child STREQUAL "template" AND EXISTS "${curdir}/${child}/CMakeLists.txt")
      LIST(APPEND dirlist ${child})
    endif()
  endforeach()

  set(${result} ${dirlist})
endmacro()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/code)

foreach(subdir ${SUBDIRS})
  message(STATUS "Added ${subdir}")
  add_subdirectory(code/${subdir})
endforeach()

OpenOrbisPackage_Validate()
