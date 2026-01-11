project(memory_test_1 VERSION 0.0.1)

link_libraries(SceSystemService )

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("MEMT00100" 1 00 ${SRC_FILES})
