project(memory_test_5 VERSION 0.0.1)

link_libraries(SceSystemService )

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("MEMT00300" 3 00 ${SRC_FILES})