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

SUBDIRLIST(SUBDIRS SUBDIR_RESULT ${CMAKE_CURRENT_SOURCE_DIR}/code)
FOREACH(subdir ${SUBDIRS})
  message(STATUS "Added ${subdir}")
  ADD_SUBDIRECTORY(code/${subdir})
ENDFOREACH()
