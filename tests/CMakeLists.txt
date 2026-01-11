cmake_minimum_required(VERSION 3.24)

include(FetchContent)
include(ExternalProject)
include(ps4_package.cmake)

project(integration_tests LANGUAGES C CXX)

set(CPPUTEST_EXTENSIONS ON CACHE BOOL "")
set(CPPUTEST_STD_C_LIB_DISABLED OFF CACHE BOOL "")
set(CPPUTEST_STD_CPP_LIB_DISABLED OFF CACHE BOOL "")
set(CPPUTEST_BUILD_TESTING OFF CACHE BOOL "")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG master
)
FetchContent_MakeAvailable(CppUTest)

include_directories(BEFORE ${cpputest_SOURCE_DIR}/include)
include_directories(BEFORE ${openorbis_SOURCE_DIR}/include)
link_libraries(CppUTest::CppUTest)

# # Add all tests
MACRO(SUBDIRLIST result resultLibs curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist)
  SET(liblist)

  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child} AND NOT child STREQUAL "template")
      LIST(APPEND dirlist ${child})
      LIST(APPEND liblist ${child})
    ENDIF()
  ENDFOREACH()

  SET(${result} ${dirlist})
  SET(${resultLibs} ${liblist})
ENDMACRO()

SUBDIRLIST(SUBDIRS SUBDIR_RESULT ${CMAKE_CURRENT_SOURCE_DIR})

FOREACH(subdir ${SUBDIRS})
  message(STATUS "Added ${subdir}")
  ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()
