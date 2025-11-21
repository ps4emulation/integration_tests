project(${PKG_TITLE_ID}_obj LANGUAGES CXX C)

add_library(${PKG_TITLE_ID}_obj OBJECT
  main.cpp
  fs_test_tools.cpp
  fs_test.cpp
  log.cpp
)
